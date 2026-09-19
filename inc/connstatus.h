#ifndef CONNSTATUS_H
#define CONNSTATUS_H

#include <QObject>
#include <QtQml/qqml.h>
#include <QtQml/qqmlregistration.h>

class ConnStatus : public QObject {
    Q_OBJECT
    QML_ELEMENT // Регистрируем именно ConnStatus в QML

    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus WRITE setConnectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(AuthStatus authStatus READ authStatus WRITE setAuthStatus NOTIFY authStatusChanged)

public:
    enum ConnectionStatus { Unchecked, Connected, Disconnected };
    Q_ENUM(ConnectionStatus)

    enum AuthStatus { LoggedOut = 10, Authenticating, LoggedIn, AuthFailed };
    Q_ENUM(AuthStatus)

    explicit ConnStatus(QObject *parent = nullptr) : QObject(parent) {}

    ConnectionStatus connectionStatus() const { return m_connectionStatus; }
    AuthStatus authStatus() const { return m_authStatus; }

public slots: // Делаем сеттеры публичными слотами, чтобы RestClient мог их менять
    void setConnectionStatus(ConnectionStatus status) {
        if (m_connectionStatus != status) { m_connectionStatus = status; emit connectionStatusChanged(status); }
    }
    void setAuthStatus(AuthStatus status) {
        if (m_authStatus != status) { m_authStatus = status;
            switch(m_authStatus){
                case LoggedOut: qDebug() << "LoggedOut"; break;
                case Authenticating: qDebug() << "Authenticating"; break;
                case LoggedIn: qDebug() << "LoggedIn"; break;
                case AuthFailed: qDebug() << "AuthFailed"; break;
            }
            emit authStatusChanged(status); }
    }

signals:
    void connectionStatusChanged(ConnectionStatus status);
    void authStatusChanged(AuthStatus status);

private:
    ConnectionStatus m_connectionStatus = Unchecked;
    AuthStatus m_authStatus = LoggedOut;
};

#endif // CONNSTATUS_H
