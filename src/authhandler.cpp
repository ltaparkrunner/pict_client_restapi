#include "authhandler.h"
//  #include "pict_data/message.qpb.h"
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTimer>

AuthHandler::AuthHandler(/*WebSocketClient *client,*/ QObject *parent)
    : QObject(parent)   //, m_client(client)
    , settings("Alex@Co", "Alex@Co")
    , m_authToken(settings.value("Auth/accessToken", "").toString())  // the second argument is a default value
    , m_username(settings.value("Auth/username", "").toString())
    , m_loggedIn(m_authToken != "")
{
    // Listen to all incoming raw traffic from the websocket client
    // connect(m_client, &WebSocketClient::authResponseReceived,
    //         this, &AuthHandler::handleIncomingAuthData);
}

//  "http://localhost:8081/login"
Q_INVOKABLE void AuthHandler::sendLogin(QString user, QString pass) {
    sendAuth(user, pass, "http://localhost:8081/auth/login");
}

//  "http://localhost:8081/register"
Q_INVOKABLE void AuthHandler::sendRegister(QString user, QString pass) {
    sendAuth(user, pass, "http://localhost:8081/auth/register");
//    sendAuth(user, pass, "http://localhost:8081/register");
}

Q_INVOKABLE void AuthHandler::sendAuth(QString user, QString pass, QString path) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QUrl url(path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = user;
    json["password"] = pass;

    request.setTransferTimeout(5000);
    QNetworkReply* reply = manager->post(request, QJsonDocument(json).toJson());// = manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            m_authToken = doc.object().value("token").toString();
            m_username = user;
            settings.beginGroup("Auth");
            settings.setValue("accessToken", m_authToken);
            settings.setValue("username", m_username);
            settings.endGroup();

            emit authSucc(AuthCond::LoginSucc, "LoginSucc");
        } else if(reply->error() == QNetworkReply::TimeoutError){

            emit authErr(AuthCond::LoginTimeoutErr, "LoginTimeoutErr");
        } else if(reply->error() == QNetworkReply::ConnectionRefusedError){

            emit authErr(AuthCond::LoginConnErr, "LoginConnErr");
        }
        else {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.object().contains("error")) {
                emit authErr(AuthCond::LoginErr, doc.object()["error"].toString());
            }
            else emit authErr(AuthCond::LoginErr, reply->errorString());
        }
        reply->deleteLater();
    });
}

Q_INVOKABLE void AuthHandler::logout(){

    QSettings settings("Alex@Co", "Alex@Co");

    if (settings.contains("Auth/accessToken")) {
        settings.remove("Auth/accessToken");
        settings.sync(); // Принудительно сохраняем изменения на диск

    }
    if (settings.contains("Auth/username")) {
        settings.remove("Auth/username");
        settings.sync(); // Принудительно сохраняем изменения на диск
    }

    this->m_authToken = "";
    this->m_username = "";

    emit this ->logoffSuccess();
}
// TODO: reset the token, if attempt to enter on wrong login/password.
// TODO: logoff button
void AuthHandler::setUsername(const QString &newUsername) {
    if (m_username == newUsername)
        return; // Если имя не изменилось, ничего не делаем

    m_username = newUsername;
    emit usernameChanged(); // КРИТИЧЕСКИ ВАЖНО: уведомляем QML об изменении
}

