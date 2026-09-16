#include "restclient.h"
#include <QFileInfo>
#include <QDebug>

RestClient::RestClient(const QString &baseUrl, QObject *parent)
    : QObject(parent), m_baseUrl(baseUrl)
{
    m_manager = new QNetworkAccessManager(this);
}

QNetworkRequest RestClient::createRequest(const QString &endpoint)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Если токен уже есть, добавляем его в заголовок Authorization
    if (!m_token.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    }
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

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onLoginReply(reply); });
}

void RestClient::onLoginReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        m_token = doc.object().value("token").toString();
        emit loginSuccess(m_token);
    } else {
        emit errorOccurred("Логин не удался: " + reply->errorString());
    }
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
