#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

class RestClient : public QObject
{
    Q_OBJECT
public:
    explicit RestClient(const QString &baseUrl, QObject *parent = nullptr);

    // Метод для авторизации (получения токена)
    void login(const QString &username, const QString &password);

    // Метод для загрузки файла (конвертирует байты в Base64)
    void uploadFile(const QString &filePath, const QString &targetFolder, const QString &info);

    // Метод для получения списка файлов в папке
    void fetchFolderList(const QString &folderName);

signals:
    void loginSuccess(const QString &token);
    void uploadSuccess();
    void folderListReceived(const QJsonObject &listData);
    void errorOccurred(const QString &errorMsg);

private slots:
    void onLoginReply(QNetworkReply *reply);
    void onUploadReply(QNetworkReply *reply);
    void onFolderListReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QString m_token; // Сюда сохраняем полученный JWT

    // Вспомогательный метод для настройки заголовков (включая Bearer токен)
    QNetworkRequest createRequest(const QString &endpoint);
};

#endif // RESTCLIENT_H
