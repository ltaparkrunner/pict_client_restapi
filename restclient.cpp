#include "restclient.h"
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QFileInfo>
#include <QDebug>

RestClient::RestClient(QObject *parent)
    : QObject(parent)
    , m_settings("Alex@Co", "Alex@Co")
    , m_authToken(m_settings.value("Auth/accessToken", "").toString())  // the second argument is a default value
    , m_username(m_settings.value("Auth/username", "").toString())
    , m_manager(new QNetworkAccessManager(this))
    , m_status(this)
//    ,m_status(this))
{
    m_status.setConnectionStatus(ConnStatus::Connecting);
    setupSslConfiguration();
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &RestClient::checkConnection);
}

RestClient::RestClient(const QString &baseUrl, QObject *parent)
    : QObject(parent)
    , m_settings("Alex@Co", "Alex@Co")
    , m_manager(new QNetworkAccessManager(this))
    , m_baseUrl(baseUrl)
{
    m_status.setConnectionStatus(ConnStatus::Connecting);
    setupSslConfiguration();
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &RestClient::checkConnection);
}

// Вспомогательный метод (можно объявить в private секции хедера restclient.h)
void RestClient::setupSslConfiguration() {
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

    // Загружаем сертификат (например, из ресурсов Qt)
    QList<QSslCertificate> certs = QSslCertificate::fromPath("../assets/cert.pem");

    if (!certs.isEmpty()) {
        sslConfig.addCaCertificates(certs);
        // Важно: если используете этот вариант, в методе createRequest(const QString &endpoint)
        // перед m_manager->get/post нужно будет устанавливать эту конфигурацию в QNetworkRequest:
        // request.setSslConfiguration(sslConfig);
        QSslConfiguration::setDefaultConfiguration(sslConfig);
    } else {
        qWarning() << "Предупреждение: Сертификат не найден по пути :/certs/server.crt. Проверьте файл ресурсов .qrc";
    }
}

void RestClient::setBaseUrl(const QString &url) {
    if (m_baseUrl != url) {
        m_baseUrl = url;
        emit baseUrlChanged();
    }
}

QNetworkRequest RestClient::createRequest(const QString &endpoint)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Если токен уже есть, добавляем его в заголовок Authorization
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
    }

    // --- НАЧАЛО ИЗМЕНЕНИЙ ДЛЯ SSL ---
    // Получаем глобальную конфигурацию (в которую ваш метод setupSslConfiguration добавил сертификат)
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

    // Принудительно связываем эту конфигурацию с текущим запросом
    request.setSslConfiguration(sslConfig);
    // --- КОНЕЦ ИЗМЕНЕНИЙ ДЛЯ SSL ---

    return request;
}

// --- 1. АВТОРИЗАЦИЯ ---
void RestClient::login(const QString &username, const QString &password)
{
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QNetworkRequest request = createRequest("/auth/login");
    QNetworkReply *reply = m_manager->post(request, QJsonDocument(json).toJson());
    m_status.setAuthStatus(ConnStatus::Authenticating);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onAuthReply(reply); });
}

// void RestClient::onLoginReply(QNetworkReply *reply)
// {
//     reply->deleteLater();
//     if (reply->error() == QNetworkReply::NoError) {
//         QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//         m_authToken = doc.object().value("token").toString();
//         m_username = doc.object().value("username").toString();

//         m_settings.beginGroup("Auth");
//         m_settings.setValue("accessToken", m_authToken);
//         m_settings.setValue("username", m_username);
//         m_settings.endGroup();
//         emit loginSuccess(m_authToken);
//         m_status.setConnectionStatus(ConnStatus::Connected);
//         m_status.setAuthStatus(ConnStatus::LoggedIn);
//     } else {
//         emit errorOccurred("Логин не удался: " + reply->errorString());
//     }
// }

void RestClient::onAuthReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        m_authToken = doc.object().value("token").toString();
        m_username = doc.object().value("username").toString();
        m_settings.beginGroup("Auth");
        m_settings.setValue("accessToken", m_authToken);
        m_settings.setValue("username", m_username);
        m_settings.endGroup();

        emit authSucc(ConnStatus::LoggedIn, "LoginSucc");
    } else if(reply->error() == QNetworkReply::TimeoutError){

        emit authErr(ConnStatus::AuthTimeoutErr, "LoginTimeoutErr");
    } else if(reply->error() == QNetworkReply::ConnectionRefusedError){

        emit authErr(ConnStatus::AuthRefused, "LoginConnErr");
    }
    else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object().contains("error")) {
            emit authErr(ConnStatus::AuthFailed, doc.object()["error"].toString());
        }
        else emit authErr(ConnStatus::AuthFailed, reply->errorString());
    }
    reply->deleteLater();

}

// --- 2. ЗАГРУЗКА ФАЙЛА (Замена Protobuf бинарников на Base64) ---
void RestClient::uploadFile(const QString &filePath, const QString &targetFolder, const QString &info)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Не удалось открыть файл для чтения");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // Переводим бинарник в Base64 строку для передачи внутри JSON
    QString base64Data = QString::fromUtf8(fileData.toBase64());

    QFileInfo fileInfo(filePath);

    QJsonObject json;
    json["fileName"] = fileInfo.fileName();
    json["folder"] = targetFolder;
    json["info"] = info;
    json["data"] = base64Data; // На сервере это превратится обратно в Буфер

    QNetworkRequest request = createRequest("/api/files/add");
    QNetworkReply *reply = m_manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onUploadReply(reply); });
}

void RestClient::onUploadReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        emit uploadSuccess();
    } else {
        emit errorOccurred("Ошибка загрузки: " + reply->errorString());
    }
}

// --- 3. ПОЛУЧЕНИЕ СПИСКА ФАЙЛОВ И ПАПОК ---
void RestClient::fetchFolderList(const QString &folderName)
{
    QJsonObject json;
    json["folderName"] = folderName;

    QNetworkRequest request = createRequest("/api/files/list");
    QNetworkReply *reply = m_manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onFolderListReply(reply); });
}

void RestClient::onFolderListReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        emit folderListReceived(doc.object());
    } else {
        emit errorOccurred("Ошибка получения списка: " + reply->errorString());
    }
}

// --- 2. РЕГИСТРАЦИЯ ---
void RestClient::registerUser(const QString &username, const QString &password)
{
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    // Отправляем запрос на эндпоинт регистрации
    QNetworkRequest request = createRequest("/auth/register");
    QNetworkReply *reply = m_manager->post(request, QJsonDocument(json).toJson());

    // Лямбда-функция связывает завершение запроса с обработчиком ответа
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onAuthReply(reply); });
}

// void RestClient::onRegisterReply(QNetworkReply *reply)
// {
//     // Обязательно освобождаем память после завершения обработки
//     reply->deleteLater();

//     if (reply->error() == QNetworkReply::NoError) {
//         // Читаем успешный ответ от сервера
//         QByteArray responseData = reply->readAll();
//         QJsonDocument doc = QJsonDocument::fromJson(responseData);

//         qDebug() << "Регистрация успешна:" << responseData;

//         // Здесь можно вызывать сигнал успеха, чтобы QML переключил экран на логин
//         // emit registerSuccess();
//     }
//     else {
//         // Если сервер вернул ошибку (например, 400 Bad Request или 409 Conflict)
//         QByteArray responseData = reply->readAll();
//         QJsonDocument doc = QJsonDocument::fromJson(responseData);

//         // Пытаемся достать текст ошибки, отправленный вашим Node.js сервером
//         QString serverError = doc.object().value("error").toString();

//         if (!serverError.isEmpty()) {
//             emit errorOccurred(serverError);
//         } else {
//             emit errorOccurred(reply->errorString());
//         }
//     }
// }

// Метод отправляет легкий запрос для проверки связи
void RestClient::checkConnection() {
    // Обычно используется эндпоинт вроде /api/ping или /api/health.
    // Если такого нет, можно слать HEAD-запрос на baseUrl.
    QNetworkRequest request = createRequest("/ping");

    // Используем HEAD вместо GET, чтобы не качать тело ответа, только заголовки
    QNetworkReply *reply = m_manager->head(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->onPingReply(reply);
    });
}

// Слот для обработки ответа проверки связи
void RestClient::onPingReply(QNetworkReply *reply) {
    reply->deleteLater(); // Обязательно освобождаем память

    // Проверяем на наличие сетевых ошибок (таймаут, нет сети, DNS ошибка)
    if (reply->error() != QNetworkReply::NoError) {
        emit connectionStatusChanged(false);
        emit errorOccurred("Сервер недоступен: " + reply->errorString());
        return;
    }

    // Проверяем HTTP-статус (200 OK считается успешным)
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200) {
        qWarning("Server is available");
        emit connectionStatusChanged(true);
    } else {
        emit connectionStatusChanged(false);
    }
}

// Метод для запуска периодического пинга
void RestClient::startAutoPing(int intervalSeconds) {
    if (!m_pingTimer->isActive()) {
        // Переводим секунды в миллисекунды и запускаем
        m_pingTimer->start(intervalSeconds * 1000);

        // Рекомендуется сделать первый вызов сразу,
        // чтобы не ждать окончания первого интервала таймера
        checkConnection();
    }
}

// Метод для остановки пинга (например, при переходе приложения в спящий режим)
void RestClient::stopAutoPing() {
    if (m_pingTimer->isActive()) {
        m_pingTimer->stop();
    }
}
