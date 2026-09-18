#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QtQml/qqmlregistration.h> // Recommended for Qt 6 QML registration macros
#include "connstatus.h"
#include <QTimer>

class RestClient : public QObject
{
    Q_OBJECT
    QML_ELEMENT // Если RestClient тоже регистрируется в QML

    // Expose baseUrl and token as QML properties so QML can read/bind them if needed
    // Q_PROPERTY(ConnStatus* status READ status CONSTANT)
    Q_PROPERTY(ConnStatus::ConnectionStatus connStatus READ connStatus CONSTANT)
    Q_PROPERTY(ConnStatus::AuthStatus authStatus READ authStatus CONSTANT)
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)

public:
    // Added a default constructor path for QML instantiation
    explicit RestClient(QObject *parent = nullptr);
    explicit RestClient(const QString &baseUrl, QObject *parent = nullptr);

    // Getters and setters required for Q_PROPERTY
    QString baseUrl() const { return m_baseUrl; }
    void setBaseUrl(const QString &url);

    QString token() const { return m_token; }

    // ConnStatus* status() const { return m_status; }
    ConnStatus::ConnectionStatus connStatus() const {return m_status.connectionStatus();}
    ConnStatus::AuthStatus authStatus() const {return m_status.authStatus();}

    // Mark methods as Q_INVOKABLE so they can be called directly from QML Javascript elements
    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE void registerUser(const QString &username, const QString &password);

    Q_INVOKABLE void uploadFile(const QString &filePath, const QString &targetFolder, const QString &info);
    Q_INVOKABLE void fetchFolderList(const QString &folderName);

    Q_INVOKABLE void checkConnection();
    Q_INVOKABLE void startAutoPing(int intervalSeconds = 30); // Запуск таймера (по умолчанию 30 сек)
    Q_INVOKABLE void stopAutoPing();                          // Остановка таймера


signals:
    void loginSuccess(const QString &token);

    void uploadSuccess();
    // Tip: QJsonObject converts automatically to a JavaScript Object/Dictionary in QML
    void folderListReceived(const QJsonObject &listData);
    void errorOccurred(const QString &errorMsg);

    // Property change notifier signals
    void baseUrlChanged();
    void tokenChanged();
    void connectionStatusChanged(bool connected);

    void errReceived();
private slots:
    void onLoginReply(QNetworkReply *reply);
    void onUploadReply(QNetworkReply *reply);
    void onFolderListReply(QNetworkReply *reply);
    void onRegisterReply(QNetworkReply *reply);
    void onPingReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    ConnStatus m_status; // Объект статуса
    QString m_baseUrl;
    QString m_token;

    QTimer *m_pingTimer = nullptr;
    QNetworkRequest createRequest(const QString &endpoint);
    void setupSslConfiguration();
};

#endif // RESTCLIENT_H
