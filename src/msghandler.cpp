#include "msghandler.h"
#include <QFileInfo>

MsgHandler::MsgHandler(WebSocketClient *client, QObject *parent)
    : QObject(parent), m_client(client)
{
    connect(m_client, &WebSocketClient::serverResponseReceived,
            this, &MsgHandler::handleIncomingServerData);
}

void MsgHandler::handleIncomingServerData(const pict_data::ServerEnvelope &data){
    qDebug() << "MsgHandler::handleIncomingServerData";
    if(data.contentField() == pict_data::ServerEnvelope::ContentFields::Buckets){
        const auto &response = data.buckets();
        QStringList sl;
        for (const auto &info : response.bucketInf()) {
            sl.append(info.bucketName());
            sl.append(info.url());
        }
        emit bucketsReceived(sl);
    }
    else if(data.contentField() == pict_data::ServerEnvelope::ContentFields::ListResponse) {
        const auto &response = data.listResponse();
        QList<QStringList> sl;
        for (const auto &info : response.folders()) {
            sl.append({info.folderName(), info.url(), "folder", ""});
        }
        for (const auto &info : response.files()) {
            sl.append({info.fileName(), info.url(), "file", info.mongoId()});
        }
        emit pathsReceived(sl, response.folderName());
    }
    else if(data.contentField() == pict_data::ServerEnvelope::ContentFields::ServerResp) {
        qDebug() << "pict_data::ServerEnvelope::ContentFields::ServerResp";
        const auto &response = data.serverResp();
        if(response.status() == "success") emit resultSuccess(response.content());
        else emit resultError(response.content());
    }
    else if(data.contentField() == pict_data:: ServerEnvelope::ContentFields::FilesIdsResponse){
        qDebug() << "pict_data:: ServerEnvelope::ContentFields::FilesIdsResponse info.url()";
        const auto &response = data.filesIdsResponse();
        QVector<QUrl> urls;

        for(const auto &info : response.files()) {
            urls.append(info.url());
        }
        emit writeUrlsToLocal(urls);
    }

    else if(data.contentField() == pict_data::ServerEnvelope::ContentFields::PathInfResponse){
        qDebug() << "pict_data::ServerEnvelope::ContentFields::PathInfResponse";
        const auto &response = data.pathInfResponse();
        QString res = response.result();
        QString netP = response.netPath();

        if(res == "file") {
            emit pathInfoResp('f', netP, extCleanNetworkFilePath(netP));
        }
        else if (res == "folder") {
            emit pathInfoResp('d', netP, extCleanNetworkFilePath(netP));
        }
        else if (res == "not exist") {
            emit pathInfoResp('n',  netP, extCleanNetworkFilePath(netP));
        }

    }
    else if(data.contentField() == pict_data::ServerEnvelope::ContentFields::FilePathResponse){
        qDebug() << "pict_data::ServerEnvelope::ContentFields::FilePathResponse";
        const auto &response = data.filePathResponse();
        QString fileName = response.fileName();
        QString url = response.url();
        QString mId = response.mongoId();
        emit filePathResp(fileName, url, mId);
    }
}

Q_INVOKABLE int MsgHandler::getBucketsListRequest() const{
    pict_data::ClientEnvelope cenv;
    pict_data::BucketsRequest message;

    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setReqUserBuckets(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz =*/ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::addFileRequest(const QString &netFolderPath, const QString &filepath, const QString &id) {
    qDebug() << "MsgHandler::addFileRequest" << filepath << " " << id << "  " << netFolderPath;
    pict_data::ClientEnvelope cenv;
    pict_data::AddFileRequest message;

    QFile *file = new QFile(filepath);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        return -1; // Ошибка открытия файла
    }

    QByteArray fileData = file->readAll();

    QFileInfo fileInfo(filepath);
    QString fileName = fileInfo.fileName();
    QString completeSuffix = fileInfo.completeSuffix();

    /*--------------------*/
    QUrl netUrl(netFolderPath);
    QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"
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

    message.setFileName(fileName);

    message.setFolder(folder);
    message.setInfo(completeSuffix);
    message.setData(fileData);

    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setAddFile(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz =*/ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::getFilesFoldersListfromBucketRequest(const QString &netPath, const QString &fname){
    qDebug() << "MsgHandler::getFilesFoldersListfromBucketRequest(const QString &minioPath)" << netPath << " fname: " << fname;
    QUrl netUrl(netPath);
    QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"
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
    //  qDebug() << "bucket: " << bucket << " folder " << folder << ", fname: " << fname;

    pict_data::ClientEnvelope cenv;
    pict_data::FilesFoldersListRequest message;
    message.setFolderName(fname);

    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setListRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz =*/ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::getFilesFoldersListfromBucketRequest2(const QString &netPath, const bool isDir){
    qDebug() << "int getFilesFoldersListfromBucketRequest2(const QString &path, const bool isDir)" << netPath;
    QUrl netUrl(netPath);
    QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();

    QString folder = "";
    //  int l = parts.length();

    if(parts.length()>1) {
        folder = parts.join('/');
    }
    //  qDebug() << "bucket: " << bucket << " folder " << folder;
    pict_data::ClientEnvelope cenv;
    pict_data::FilesFoldersListRequest message;
    message.setFolderName(folder);

    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setListRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz =*/ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::deleteFileFromServerRequest(const QStringList &fileData){
    if(fileData.size() >= 4) {
        pict_data::ClientEnvelope cenv;
        pict_data::DeleteFileRequest message;
        message.setFileName(fileData.at(0));
        message.setMongoId(fileData.at(2));

        cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
        cenv.setDeleteFile(message);
        QProtobufSerializer serializer;
        QByteArray data = cenv.serialize(&serializer);
        /*qint64 sz = */ m_client->sendBinaryMessage(data);
        return 0;
    }
    return -1;
}

int MsgHandler::getFilesRequest(const QStringList &paths, const QStringList &ids, const QString &arrival){
    pict_data::ClientEnvelope cenv;
    pict_data::FilesIds message;
    message.setMongoIds(ids);
    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setFilesIdsRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz = */ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::getNetStore(const QString &netPath){
    QUrl netUrl(netPath);
    QString path = netUrl.path();
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    qsizetype bucketIdx = path.indexOf(bucket);
    QString fPath = "";
    if (bucketIdx != -1) {
        qsizetype startPos = bucketIdx + bucket.length();
        qsizetype endPos = path.lastIndexOf('/');

        if (endPos > startPos) {
            if (path.at(startPos) == '/') {
                startPos++;
            }
            qsizetype length = endPos - startPos;
            fPath = path.sliced(startPos, length);
        }
    }
    pict_data::ClientEnvelope cenv;
    pict_data::PathInfoRequest message;

    message.setNetPath(fPath);
    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setPathInfRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);

    /*qint64 sz = */ m_client->sendBinaryMessage(data);
    return 0;
}

int MsgHandler::getFileNetStore(const QString &netPath){
    qDebug() << "MsgHandler::getNetStore netPath: " << netPath;
    QUrl netUrl(netPath);
    QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    QStringList parts = path.split('/');
    QString bucket = parts.takeFirst();
    qsizetype bucketIdx = path.indexOf(bucket);
    QString fPath = "";
    if (bucketIdx != -1) {
        qsizetype startPos = bucketIdx + bucket.length();

        qsizetype endPos = path.length();
        if (endPos > startPos) {
            if (path.at(startPos) == '/') {
                startPos++;
            }
            qsizetype length = endPos - startPos;
            fPath = path.sliced(startPos, length);
        }
    }
    pict_data::ClientEnvelope cenv;
    pict_data::FilePathRequest message;

    message.setNetPath(fPath);
    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setFilePathRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);

    /*qint64 sz = */ m_client->sendBinaryMessage(data);
    return 0;
}
int MsgHandler::rewriteFileRequest(const QString &netFolderPath, const QString &filepath, const QString &id) {

    qDebug() << "MsgHandler::rewriteFileRequest" << filepath << " " << id << "  " << netFolderPath;
    pict_data::ClientEnvelope cenv;
    pict_data::RewriteFileRequest message;
    /*--------------------*/
    QUrl netUrl(netFolderPath);
    QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"
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
    //  qDebug() << "MsgHandler::rewriteFileRequest bucket: " << bucket << " folder " << folder;

    QUrl netUrl2(filepath);
    QFileInfo netFileInfo(netUrl2.toString());
    QString fname = netFileInfo.fileName();

    message.setFileName(fname);

    message.setFolder(folder);
    message.setMongoId(id);

    cenv.setType(pict_data::ClientEnvelope::Type::CLIENT_MESSAGE);
    cenv.setRewriteFileRequest(message);
    QProtobufSerializer serializer;
    QByteArray data = cenv.serialize(&serializer);
    /*qint64 sz =*/ m_client->sendBinaryMessage(data);
    return 0;
}
