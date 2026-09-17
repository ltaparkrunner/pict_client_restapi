#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QtProtobuf/QProtobufSerializer>
#include <QSettings>
//  #include <websocketclient.h>


class AuthHandler : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
public:
    enum class AuthCond {
        LoginSucc,
        LoginErr,
        LoginTimeoutErr,
        LoginConnErr,
        LogoutSucc,
        LogoutErr,
        LogoutTimeoutErr,
        LogoutConnErr
    };
    explicit AuthHandler(QObject *parent = nullptr);
    Q_INVOKABLE void sendLogin(QString user, QString pass);
    Q_INVOKABLE void sendRegister(QString user, QString pass);
    Q_INVOKABLE void sendAuth(QString user, QString pass, QString path);

    Q_INVOKABLE void logout();

    QString authToken() const { return m_authToken; }
    QString username() const { return m_username; }
    void setUsername(const QString &newUsername);

signals:
    void startWebSocket(/*QString token*/);
    void authErr(AuthCond authc, QString errmsg);
    void authSucc(AuthCond authc, QString succmsg);

    void usernameChanged();
    void logoffSuccess();

private:
    QSettings settings;
    QString m_authToken;
    QString m_username;
    bool m_loggedIn;
};

#endif // AUTHHANDLER_H
