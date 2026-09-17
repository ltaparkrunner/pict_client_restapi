#include "websocketclient.h"
#include "pict_data/message.qpb.h"
#include <QFileInfo>
#include <QtProtobuf/QProtobufSerializer>
//  #include <QSettings>

void WebSocketClient::onBinaryMessageReceived(const QByteArray &data) {

    pict_data::BaseMessage base;
    QProtobufSerializer serializer;
    if (!base.deserialize(&serializer, data)) return;
    if (base.contentField() == pict_data::BaseMessage::ContentFields::ListResponse) {
        const auto &response = base.listResponse();
        QList<QStringList> sl;
        for (const auto &info : response.files()) {
            sl.append({info.fileName(), info.url(), "file", info.mongoId()});
        }
        for (const auto &info : response.folders()) {
            sl.append({info.folderName(), info.url(), "folder", ""});
        }
            emit pathsReceived(sl);
    }
    if (base.contentField() == pict_data::BaseMessage::ContentFields::FilesListResponse) {
        const auto &response = base.filesListResponse();
        QList<QStringList> sl;
        for (const auto &info : response.files()) {
            sl.append({info.fileName(), info.url(), "file", info.mongoId()});
        }
        emit filesReceived(sl);
    }
    if (base.contentField() == pict_data::BaseMessage::ContentFields::Buckets) {
        const auto &response = base.buckets();
        QStringList sl;
        for (const auto &info : response.bucketInf()) {
            sl.append(info.bucketName());
            sl.append(info.url());
        }
        emit bucketsReceived(sl);
    }
    if (base.contentField() == pict_data::BaseMessage::ContentFields::ServerResp) {
        const auto &response = base.serverResp();
    }
}

WebSocketClient::WebSocketClient(AuthHandler *authHandler, const QUrl &url, QObject *parent) : QObject(parent)
    , m_authHandler(authHandler)
    , m_webSocket (new QWebSocket())
    , m_url(url)
    , m_path("")
    //, m_authState(AuthState::Idle)
    , m_authConnectState(AuthConnectState::Idle)
    //, m_tokenExpiredDetected(false)
    , m_reconnectAttempts(0)
    , m_pongReceived(true)
{
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::connectToServer);
    connect(&m_pingTimer, &QTimer::timeout, this, [&]() {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState && m_pongReceived) {
            m_webSocket->ping();
        }
        else {
            if(m_authConnectState == AuthConnectState::Authorized) setConnectionState(AuthConnectState::AuthorizedNoPingRespond);
            else if(m_authConnectState == AuthConnectState::LoggedOut) setConnectionState(AuthConnectState::LoggedOutNoPingRespond);
        }
    });
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    m_webSocket->setSslConfiguration(sslConf);

    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &WebSocketClient::onErrorOccurred);

    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketClient::onBinaryMessageReceived2);


    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

//    authenticationRequired(QAuthenticator *authenticator)
    connect(m_webSocket, &QWebSocket::authenticationRequired, this, &WebSocketClient::onAuthRequired);
    connect(m_webSocket, &QWebSocket::pong, this, [this](quint64 elapsedTime, const QByteArray &payload){
        Q_UNUSED(elapsedTime);
        Q_UNUSED(payload);
        m_pongReceived = true;
    });
    connectToServer();
}

Q_INVOKABLE void WebSocketClient::connectToServer(/*const QString &url*/) {
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) return;
//    if (!m_authHandler || !m_authHandler->loggedIn()) return;
    setConnectionState(AuthConnectState::Connecting);
    // m_tokenExpiredDetected = false;
    QNetworkRequest request(m_url);
    request.setRawHeader("Authorization", ("Bearer " + m_authHandler->authToken()).toUtf8());
    m_webSocket->open(request);
}
QString WebSocketClient::lastReceivedPath() const { return m_path; }

Q_INVOKABLE int WebSocketClient::deleteFileFromBucketRequest(const QString &filePath){
    QUrl minioUrl(filePath);
    QString path = minioUrl.path(); // Вернет "/photos/holiday/sun.jpg"

    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    QString key = parts.join('/');

    pict_data::BaseMessage base;
    pict_data::DeleteFileRequest message;

    message.setFileName(key);
    message.setMongoId("1111111");

    base.setDeleteFile(message);
    QProtobufSerializer serializer;
    QByteArray data = base.serialize(&serializer);
    /*qint64 sz =*/ m_webSocket->sendBinaryMessage(data);
    return m_webSocket->sendBinaryMessage(data);
}

/*Q_INVOKABLE*/ void WebSocketClient::getFilesFoldersListfromBucketRequest(const QString &minioPath){

    QUrl minioUrl(minioPath);
    QString path = minioUrl.path(); // Вернет "/photos/holiday/sun.jpg"
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    qsizetype bucketIdx = path.indexOf(bucket);
    QString folder = "";
    if (bucketIdx != -1) {
        qsizetype startPos = bucketIdx + bucket.length();
        qsizetype endPos = path.lastIndexOf('/');
        if (endPos > startPos) {
            if (path.at(startPos) == '/') {
                startPos++;
            }
            qsizetype length = endPos - startPos;
            folder = path.sliced(startPos, length);
        }
    }
    //  qDebug() << "bucket: " << bucket << " folder " << folder;

    pict_data::BaseMessage base;
    pict_data::FilesFoldersListRequest message;
    message.setFolderName(folder);

    base.setListRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = base.serialize(&serializer);
    /*qint64 sz =*/ m_webSocket->sendBinaryMessage(data);
}

void WebSocketClient::onConnected() {
    qDebug() << "WSS Network layer connected.";
    m_pongReceived = true;
    m_reconnectAttempts = 0;
    m_reconnectTimer.stop();
    setConnectionState(AuthConnectState::Authorized);
    // m_authHandler->onWssConnected();
    m_pingTimer.start(PING_INTERVAL);
}

void WebSocketClient::onDisconnected() {
    m_pingTimer.stop();

    if (m_authConnectState == AuthConnectState::NotAuthorized) {
        //  qWarning() << "Stopping: Token is spoiled.";
        //  Show login screen, clear saved token
        setConnectionState(AuthConnectState::NotAuthorized);
        return;
    }
    if (m_authConnectState == AuthConnectState::LoggingOut) {
        //  qWarning() << "Stopping: Token is spoiled.";
        //  Show login screen, clear saved token
        setConnectionState(AuthConnectState::LoggedOut);
        return;
    }
    else if(m_authConnectState != AuthConnectState::UserDisconnecting && m_reconnectAttempts < m_maxReconnectAttempts) {
        //  qDebug() << "WebSocketClient::onDisconnected > m_reconnectAttempts: " << m_reconnectAttempts << " " << m_webSocket->error();
        setConnectionState(AuthConnectState::Connecting);
        m_reconnectAttempts++;
        m_reconnectTimer.start(RECONNECT_INTERVAL);
    }
    else if((m_authConnectState != AuthConnectState::UserDisconnecting && m_reconnectAttempts >= m_maxReconnectAttempts)
               || (m_authConnectState == AuthConnectState::UserDisconnecting)){
//        qDebug() << "WebSocketClient::onDisconnected <= m_reconnectAttempts: " << m_reconnectAttempts << " " << m_webSocket->error();
        setConnectionState(AuthConnectState::NoConnection);
    }
}

void WebSocketClient::onTextMessageReceived(const QString &message) {
    qDebug() << "Text from server:" << message;
}

void WebSocketClient::onErrorOccurred(QAbstractSocket::SocketError error) {
    //  qDebug() << "Error:" << m_webSocket->errorString() << " " << m_webSocket->error() << "  m_authConnectState: " << m_authConnectState;
    if (m_authConnectState == AuthConnectState::Connecting &&
        error == QAbstractSocket::ConnectionRefusedError ||
        error == QAbstractSocket::UnknownSocketError) {
        QString systemError = m_webSocket->errorString();
        if (systemError.contains("Authenticate")) {
            qDebug() << "Token expired! Initiate authentication refresh.";
            setConnectionState(AuthConnectState::NotAuthorized);
        }
    }
    else if(m_authConnectState != AuthConnectState::UserDisconnecting
                   && m_authConnectState != AuthConnectState::NoConnection)
        setConnectionState(AuthConnectState::ExternalDisconnecting);
}

Q_INVOKABLE int WebSocketClient::addFileRequest(const QString &filePath, const QString &minioPath){
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file:///")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    QFileInfo fileInfo(cleanPath);

    QString fileName = fileInfo.fileName();

    QUrl minioUrl(minioPath);
    QString path = minioUrl.path(); // Вернет "/photos/holiday/sun.jpg"
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    pict_data::BaseMessage base;
    pict_data::AddFileRequest message;

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    int sz = parts.size();
    QString folder = "";
    if(sz > 2) {
        folder = parts.sliced(1, sz - 2).join("/");
    }

    QFile *file = new QFile(cleanPath);
    if (!file->open(QIODevice::ReadOnly)) {
        //  qDebug() << "Could not open file 3:" << cleanPath;
        delete file;
        return -1; // Ошибка открытия файла
    }
    //  else qDebug() << "Open file: " << cleanPath;
    QByteArray fileData = file->readAll();
    message.setFileName(fileName);

    message.setFolder(folder);
    message.setInfo("jpg");
    message.setData(fileData);

    base.setAddFile(message);
    QProtobufSerializer serializer;
    QByteArray data = base.serialize(&serializer);
    /*qint64 sz =*/ //  m_webSocket->sendBinaryMessage(data);
    return m_webSocket->sendBinaryMessage(data);
}

void WebSocketClient::deleteMinioBucketsRequest(const QStringList &buckets){
    pict_data::BaseMessage base;
    pict_data::DeleteBucketRequest message;

    for(const QString &bucket : buckets) {
        base.setDeleteBucket(message);
        QProtobufSerializer serializer;
        QByteArray data = base.serialize(&serializer);
        /*qint64 sz = */ m_webSocket->sendBinaryMessage(data);
    }
}

int WebSocketClient::deleteFileFromServerRequest(const QStringList &fileData){
    if(fileData.size() >= 4) {
        pict_data::BaseMessage base;
        pict_data::DeleteFileRequest message;

        message.setFileName(fileData.at(0));
        message.setMongoId(fileData.at(2));
        base.setDeleteFile(message);
        QProtobufSerializer serializer;
        QByteArray data = base.serialize(&serializer);
        /*qint64 sz = */ m_webSocket->sendBinaryMessage(data);
    }
    return 0;
}

int WebSocketClient::getFilesOnlyListfromBucketRequest(const QString &minioPath) {
    // qDebug() << "getFilesOnlyListfromBucketRequest(const QString &minioPath)" << minioPath;
    QUrl minioUrl(minioPath);
    QString path = minioUrl.path(); // Вернет "/photos/holiday/sun.jpg"
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    int sz = parts.size();
    QString folder = "";
    if(sz > 2) {
        folder = parts.sliced(1, sz - 2).join("/");
    }
    //  qDebug() << "bucket: " << bucket << " folder " << folder;

    pict_data::BaseMessage base;
    pict_data::FilesOnlyListRequest message;
    message.setFolderName(folder);

    base.setFilesListRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = base.serialize(&serializer);
    /*qint64 sz =*/ m_webSocket->sendBinaryMessage(data);
    return 0;
}

void WebSocketClient::onBinaryMessageReceived2(const QByteArray &rawBytes){
    // 1. Parse the wrapper message
    pict_data::ServerEnvelope envelope;
    QProtobufSerializer serializer;
    if (!envelope.deserialize(&serializer, rawBytes)) {
        onBinaryMessageReceived(rawBytes);
        return;
    }
    switch (envelope.type()) {
    case pict_data::ServerEnvelope::Type::AUTH_RESPONSE:{
        if (envelope.hasAuthResponse()) {
            emit authResponseReceived(envelope.authResponse());
        }
        break;
    }
    case pict_data::ServerEnvelope::Type::SERVER_MESSAGE: {
        {
            emit serverResponseReceived(envelope);
        }
        break;
    }
    default:
        onBinaryMessageReceived(rawBytes);
        break;
    }
}

void WebSocketClient::sendBinaryMessage(const QByteArray &data) {

    if (m_webSocket && m_webSocket->isValid()) {
        m_webSocket->sendBinaryMessage(data);
    }
    else {
        emit errReceived();
    }
}

void WebSocketClient::onAuthRequired(QAuthenticator *authenticator){
    qDebug() << "onAuthRequired(QAuthenticator): " << authenticator->realm() << authenticator->user();
}

void WebSocketClient::disconnectFromServer() {
    m_webSocket->close();
}

void WebSocketClient::setConnectionState(AuthConnectState newState) {
    if (m_authConnectState == newState) return;
    m_authConnectState = newState;
    emit authConnectionStateChanged();
}

Q_INVOKABLE void WebSocketClient::loginRequested(const QString &login, const QString &passw){
    setConnectionState(AuthConnectState::Connecting);
    connect(m_authHandler, &AuthHandler::authSucc, this, &WebSocketClient::connectToServer);
    connect(m_authHandler, &AuthHandler::authErr, this,
            [=](AuthHandler::AuthCond authCond, QString errmsg){
                if(authCond == AuthHandler::AuthCond::LoginTimeoutErr ||
                    authCond == AuthHandler::AuthCond::LoginConnErr ){

                    setConnectionState(AuthConnectState::NoConnection);
                }
                else setConnectionState(AuthConnectState::NotAuthorized);// errmsg transmit to
            });
    m_authHandler->sendLogin(login, passw);
}

Q_INVOKABLE void WebSocketClient::registerRequested(const QString &login, const QString &passw){
    setConnectionState(AuthConnectState::Connecting);
    connect(m_authHandler, &AuthHandler::authSucc, this, &WebSocketClient::connectToServer);
    connect(m_authHandler, &AuthHandler::authErr, this,
            [=](AuthHandler::AuthCond authCond, QString errmsg){
        if(authCond == AuthHandler::AuthCond::LoginTimeoutErr ||
           authCond == AuthHandler::AuthCond::LoginConnErr ){

            setConnectionState(AuthConnectState::NoConnection);
        }
        else setConnectionState(AuthConnectState::NotAuthorized);// errmsg transmit to
    });
    m_authHandler->sendRegister(login, passw);
}

Q_INVOKABLE void WebSocketClient::logout(){
    setConnectionState(AuthConnectState::LoggingOut);
    connect(m_authHandler, &AuthHandler::logoffSuccess, this, [=](){
        setConnectionState(AuthConnectState::LoggingOut);
        m_webSocket -> close();
    });
    m_authHandler->logout();
}
