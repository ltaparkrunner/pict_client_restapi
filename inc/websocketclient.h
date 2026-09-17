#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QTimer>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QFile>
#include <QAuthenticator>
#include "pict_data/message.qpb.h"
#include "authhandler.h"
#include <QtQml>

class WebSocketClient : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("WebSocketClient can only be instantiated from C++")
//    Q_ENUM(AuthConnectState)
public:
    Q_PROPERTY(QString lastReceivedPath READ lastReceivedPath NOTIFY pathReceived)
    // Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    enum class AuthConnectState {
        Idle,            // Начальное состояние, бездействует
        Connecting,      // Установка TCP/WSS соединения с сервером
        Connected,       // Сетевое соединение установлено, но авторизация еще не началась
        Authenticating,  // Процесс проверки токена на сервере
        Authorized,      // Успешно подключен и авторизован
        NotAuthorized,   // Ошибка авторизации (токен протух или испорчен)
        NoConnection,     // Ошибка сети (сервер недоступен, лимит попыток исчерпан)

        LoggingOut,      // В процессе выхода (отправка запроса logout / закрытие сессии)
        LoggedOut,        // Успешно вышел из системы (токен стерт, ожидаем ручного ввода)

        UserDisconnecting,
        ExternalDisconnecting,
        AuthorizedNoPingRespond,    //
        LoggedOutNoPingRespond
    };
    Q_ENUM(AuthConnectState) // Позволяет использовать enum внутри QML

    Q_PROPERTY(AuthConnectState authConnectionState READ authConnectionState NOTIFY authConnectionStateChanged)

public:
    explicit WebSocketClient(AuthHandler *authHandler, const QUrl &url, QObject *parent = nullptr);
    Q_INVOKABLE void connectToServer(/*const QString &url*/);
    QString lastReceivedPath() const;
    /*Q_INVOKABLE*/ void getFilesFoldersListfromBucketRequest(const QString &path);
    int getFilesOnlyListfromBucketRequest(const QString &path);
    Q_INVOKABLE int deleteFileFromBucketRequest(const QString &filePath);
    Q_INVOKABLE int addFileRequest(const QString &filePath, const QString &path);
    void deleteMinioBucketsRequest(const QStringList &buckets);
    int deleteFileFromServerRequest(const QStringList &fileData);

    void sendBinaryMessage(const QByteArray &data);

    AuthConnectState authConnectionState() const { return m_authConnectState; }

    Q_INVOKABLE void loginRequested(const QString &login, const QString &passw);
    Q_INVOKABLE void registerRequested(const QString &login, const QString &passw);
    Q_INVOKABLE void logout();

signals:
    void pathReceived(const QString &path);
    void pathsReceived(const QList<QStringList> &paths);
    void pathsReceived2(const QList<QStringList> &paths);
    void bucketsReceived(const QStringList &buckets);
    void filesReceived(const QList<QStringList> &paths);

    void errReceived();

    void serverResponseReceived(const pict_data::ServerEnvelope &response);
    void authResponseReceived(const pict_data::AuthResponse &response);
    void authResponseReceived2(const QByteArray &response);
    void authConnectionStateChanged();

public slots:
    void disconnectFromServer();
private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
//    void onError(QAbstractSocket::SocketError error);

    void onBinaryMessageReceived2(const QByteArray &rawBytes);
    void onAuthRequired(QAuthenticator *authenticator);
public:
    QWebSocket *m_webSocket;
private:
    AuthHandler *m_authHandler;
    QUrl m_url;
    QTimer m_reconnectTimer;
    QTimer m_pingTimer;
    QString m_path;
    AuthConnectState m_authConnectState;    // = AuthConnectState::Idle;
    int m_reconnectAttempts;
    const int m_maxReconnectAttempts = 3;

    bool m_pongReceived;
    void setConnectionState(AuthConnectState newState);
    void onErrorOccurred(QAbstractSocket::SocketError error);

    const int RECONNECT_INTERVAL = 5000;
    const int PING_INTERVAL = 30000;
};
#endif // WEBSOCKETCLIENT_H
