#include "unifiedstoragemodel.h"
#include "filedownloader.h"
#include "auxilary.h"

UnifiedStorageModel::UnifiedStorageModel(WebSocketClient *wsc, MsgHandler *svrHndlr, QObject *parent)
    : QAbstractListModel{parent}
    , wsclient (wsc)
    , msghandler (svrHndlr)
//    , workPath("https://minio:9000")
    , workPath (netPrefixes[0])
{
    connect(msghandler, &MsgHandler::pathsReceived, this, &UnifiedStorageModel::minioPathsToQML);
    connect(wsclient, &WebSocketClient::errReceived, this, [=](){

        beginResetModel();
        m_items.clear();
        endResetModel();
    });
    connect(msghandler, &MsgHandler::writeUrlsToLocal, this, &UnifiedStorageModel::writeUrlsToLocal);
    connect(msghandler, &MsgHandler::pathInfoResp, this, &UnifiedStorageModel::openNetStoreDialog);
    connect(msghandler, &MsgHandler::bucketsReceived, this, &UnifiedStorageModel::minioBucketsToQML);
}

void UnifiedStorageModel::enterLocal(const QString &path) {

    beginResetModel();
    m_items.clear();
    QDir dir{path};
    if(!dir.exists()){
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        dir = QDir{defaultPath};
        m_parentItem = {dir.dirName(), defaultPath, defaultPath, true, false, false, false, ""};
    }
    else m_parentItem = {dir.dirName(), dir.absolutePath(), dir.absolutePath(), true, false, false, false, ""};
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp" << "*.webp";

    QFileInfoList files = dir.entryInfoList(filters, QDir::AllEntries | QDir::NoDot, QDir::DirsFirst);

    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDot, QDir::DirsFirst);
    QFileInfoList fullList = dirs + files;

    for (const QFileInfo &info : std::as_const(fullList)) {
        m_items.append({info.fileName(), info.absoluteFilePath(), info.absoluteFilePath(), info.isDir(), false, false, false});
    }
    endResetModel();
}

void UnifiedStorageModel::enterNetStore(QString path) {
        msghandler->getBucketsListRequest();
}

void UnifiedStorageModel::enterMinioBucket(const QString &path) {

    msghandler->getFilesFoldersListfromBucketRequest(m_parentItem.path, "" /*, usmodel*/);
}

void UnifiedStorageModel::minioBucketsToQML(const QStringList &buckets) {

    beginResetModel();
    m_items.clear();

    for (int i = 0; i < buckets.size(); i += 2) {
        m_items.append({buckets[i], buckets[i+1], buckets[i+1], true, true, true, false});
    }
    endResetModel();
}

void UnifiedStorageModel::minioPathsToQML(const QList<QStringList> &paths, const QString &netFolderPath) {

    beginResetModel();
    m_items.clear();    
    QUrl url(netFolderPath);

    QFileInfo info(url.toString());
    info.fileName();
    QString npath = extCleanNetworkFilePath(netFolderPath);

    bool bucket = false;
    if(netFolderPath == netPrefixes[0] || netFolderPath == netPrefixes[1] || netFolderPath == netPrefixes[2] ||
        netFolderPath == netPrefixes[0]+"/" || netFolderPath == netPrefixes[1]+"/" || netFolderPath == netPrefixes[2]+"/")
        bucket = true;
    m_parentItem = {info.fileName(), netFolderPath, extCleanNetworkFilePath(netFolderPath), true, true, bucket, true};

    int symbs = npath.count('/');

    if(symbs <=4) {
        m_items.append({"..", netPrefixes[0]+"/" , netPrefixes[0]+"/", true, true, true, false});
    }
    else {
        int prevSlashIdx = npath.lastIndexOf('/', -2);
        if (prevSlashIdx != -1) {
            m_items.append({"..", npath.left(prevSlashIdx + 1), npath.left(prevSlashIdx + 1), true, true, false, false});
        }
        else {
            m_items.append({"..", netPrefixes[0]+"/",  netPrefixes[0]+"/", true, true, true, false});
        }
    }
    for (const QStringList& image : paths) {
        if(image[2] != "folder") m_items.append({image[0], image[1],  extCleanNetworkFilePath(image[1]), false, true, false, false, image[3]});
        else m_items.append({image[0], image[1],  image[1], true, true, false, false, image[3]});
    }
    endResetModel();
}

// 1. Return number of items
int UnifiedStorageModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

// 2. Provide data for a specific row and "role"
QVariant UnifiedStorageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())//m_imagePaths.count())
        return QVariant();
    const StorageItem &item = m_items.at(index.row());
    switch (role) {
    case NameRole:
        return item.name;
    case PathRole:
        return item.path;
    case CleanPathRole:
        return item.cleanPath;
    case IsDirRole:
        return item.isDirectory;
    case IsMinioRole:
        return item.isMinio;
    case IsMinioBucketRole:
        return item.isMinioBucket;
    case IsVirtualDirRole:
        return item.isVirtualDir;
    case MongoIdRole:
        return item.mongoId;
    default:
        return QVariant();
    }
}

// 3. Map integer roles to string names used in QML
QHash<int, QByteArray> UnifiedStorageModel::roleNames() const{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[CleanPathRole] = "cleanPath";
    roles[IsDirRole] = "isDir";
    roles[IsMinioRole] = "isMinio";
    roles[IsMinioBucketRole] = "isMinioBucket";
    roles[IsVirtualDirRole] = "isVirtualDir";
    roles[MongoIdRole] = "mongoId";
    return roles;
}

void UnifiedStorageModel::loadRoot() {

    beginResetModel();
    m_items.clear();

    // Добавляем две виртуальные "папки"
    m_items.append({"Локальные файлы", "/",  "/", true, false, false, false});
    m_items.append({"Облако MinIO", "minio_root", "minio_root", true, true, true, false});

    endResetModel();
}

Q_INVOKABLE QVariantMap UnifiedStorageModel::get(int row) const {
    // Проверка границ, чтобы избежать падения
    if (row < 0 || row >= m_items.count()) {
        return QVariantMap();
    }

    const StorageItem &item = m_items.at(row);
    QVariantMap res;

//    Вручную наполняем карту данными
    res["name"] = item.name;
    res["path"] = item.path;
    res["cleanPath"] = item.cleanPath;
    res["isDir"] = item.isDirectory;
    res["isMinio"] = item.isMinio;
    res["isMinioBucket"] = item.isMinioBucket;
    res["isVirtualDir"] = item.isVirtualDir;
    res["mongoId"] = item.mongoId;
    return res;
}

Q_INVOKABLE int UnifiedStorageModel::addVirtual(const QString &virtFolderName, const QString &currPath){

    beginResetModel();
    if(m_parentItem.isMinioBucket) m_items.append({virtFolderName, currPath+virtFolderName + "/", currPath+virtFolderName + "/",
                        true, true, false, true, ""});
    else m_items.append({virtFolderName, currPath+virtFolderName + "/",  currPath+virtFolderName + "/",
                        true, true, false, true, ""});

    endResetModel();
    return 0;
}

Q_INVOKABLE int UnifiedStorageModel::openFolderImages(int indx){  // show folder files Images in Main window

    if(indx < m_items.size()) m_parentItem = m_items[indx];
    else return -1;
    qDebug() << " m_parentItem: " << m_parentItem.path << "  isMinioBucket: " << m_parentItem.isMinioBucket;
    if(!m_parentItem.isMinio && m_parentItem.isDirectory){ // Local Directory
        beginResetModel();
        m_items.clear();
        QDir dir(m_parentItem.path);

        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp" << "*.webp";

        QFileInfoList files = dir.entryInfoList(filters, QDir::AllEntries | QDir::NoDot, QDir::DirsFirst);

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDot, QDir::DirsFirst);
        QFileInfoList fullList = dirs + files;

        for (const QFileInfo &info : std::as_const(fullList)) {
            m_items.append({info.fileName(), info.absoluteFilePath(), info.absoluteFilePath(), info.isDir(), false, false, false});
        }
        endResetModel();
        return 0;
    }
    else if(m_parentItem.isMinio && m_parentItem.isMinioBucket) { // Minio Bucket
        qDebug() << " m_parentItem: " << m_parentItem.path;
        msghandler->getFilesFoldersListfromBucketRequest(m_parentItem.path, "" /*, usmodel*/);
        return 0;
    }
    else if(m_parentItem.isMinio && !m_parentItem.isMinioBucket && m_parentItem.isDirectory && !m_parentItem.isVirtualDir) {
        qDebug() << " m_parentItem: " << m_items[indx].path << "UnifiedStorageModel::openFolderImages";
        msghandler->getFilesFoldersListfromBucketRequest(m_parentItem.path, m_parentItem.name  /*, usmodel*/);
        return 0;               // Minio simple folder
    }
    else if(m_parentItem.isMinio && !m_parentItem.isMinioBucket && m_parentItem.isDirectory && m_parentItem.isVirtualDir) {
        qDebug() << "Virtual Minio Folder path: " << m_parentItem.path << "  name: " << m_parentItem.name;
        beginResetModel();
        m_items.clear();
        endResetModel();
        return 0;   // Minio simple folder
    }
    return 0;
}

Q_INVOKABLE int UnifiedStorageModel::enterFolder(int indx){ // Open folder in File/Folder Dialog
    qDebug() << "int UnifiedStorageModel::enterFolder(StorageItem item): " << m_items[indx].path << "  name: " << m_items[indx].name <<
        "m_items[indx].isMinio" << m_items[indx].isMinio << "m_items[indx].isDir" << m_items[indx].isDirectory;
    StorageItem prevprevItem = m_parentItem;
    if(indx < m_items.size()) m_parentItem = m_items[indx];
    else return -1;

    if(!m_parentItem.isMinio && m_parentItem.isDirectory){ // Local Directory
        beginResetModel();
        m_items.clear();
        QDir dir(m_parentItem.path);

        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp" << "*.webp";

        QFileInfoList files = dir.entryInfoList(filters, QDir::AllEntries | QDir::NoDot, QDir::DirsFirst);

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDot, QDir::DirsFirst);
        QFileInfoList fullList = dirs + files;

        for (const QFileInfo &info : std::as_const(fullList)) {
            m_items.append({info.fileName(), info.absoluteFilePath(),  info.absoluteFilePath(), info.isDir(), false, false, false});
        }
        endResetModel();
        return 0;
    }
    else if(m_parentItem.isMinio && m_parentItem.isMinioBucket) { // Minio Bucket
        if(m_parentItem.path == netPrefixes[0]+"/" || m_parentItem.path == netPrefixes[1]+"/" || m_parentItem.path == netPrefixes[2]+"/") {
            msghandler->getBucketsListRequest();
        }
        else {
            msghandler->getFilesFoldersListfromBucketRequest(m_parentItem.path, "" /*, usmodel*/);
        }
        return 0;
    }
    else if(m_parentItem.isMinio && !m_parentItem.isMinioBucket && m_parentItem.isDirectory && !m_parentItem.isVirtualDir) {
        msghandler->getFilesFoldersListfromBucketRequest2(m_parentItem.path, true  /*, us:model*/);
        return 0;               // Minio simple folder
    }
    else if(m_parentItem.isMinio && !m_parentItem.isMinioBucket && m_parentItem.isDirectory && m_parentItem.isVirtualDir) {
        beginResetModel();
        m_items.clear();
        m_items.append({"..", prevprevItem.path,  prevprevItem.path, prevprevItem.isDirectory, prevprevItem.isMinio,
                        prevprevItem.isMinioBucket, prevprevItem.isVirtualDir});
        endResetModel();
        return 0;   // Minio simple folder
    }
    return 0;
}

Q_INVOKABLE int UnifiedStorageModel::deleteIndices(const QList<int> &indxs){
    qDebug() << "int UnifiedStorageModel::deleteIndices";
    for(int indx : indxs){
        if(m_items[indx].isMinio && !m_items[indx].isDirectory){
            QStringList sl;
            sl.append(m_items[indx].name);

            QUrl netUrl(m_items[indx].path);
            QString path = netUrl.path(); // Вернет "/photos/holiday/sun.jpg"

            if (path.startsWith('/')) {
                path.remove(0, 1);
            }

            QStringList parts = path.split('/');
            QString bucket = parts.takeFirst();

            sl.append(bucket);
            sl.append(m_items[indx].mongoId);
            sl.append("Ivon");

            connect(msghandler, &MsgHandler::resultSuccess, this, &UnifiedStorageModel::successToQML);
            msghandler->deleteFileFromServerRequest(sl);
        }
        if(!m_items[indx].isMinio && !m_items[indx].isDirectory){
            QFileInfo fileInfo(m_items[indx].path);
            QString dirPath = fileInfo.absolutePath();

            QDir dir{dirPath};
            bool success = dir.remove(m_items[indx].name);
            //  if(!success) qDebug() << "int UnifiedStorageModel::deleteIndices something went wrong";
        }
    }
    return 0;
}

QStringList UnifiedStorageModel::getBacketNameFromPath(const QString &path){
    qDebug() << "getBacketNameFromPath(const QString &path)" << path;
    return {};
}

QString UnifiedStorageModel::resolveImageIndex(int indx) {
    if(indx < m_items.size()){
        StorageItem item = m_items[indx];
        emit udsmToIm(item.name, item.path, item.isMinio, item.isDirectory, item.mongoId);
        return item.path;
    }
    return "";
}

Q_INVOKABLE QVariantMap UnifiedStorageModel::getData(int indx){
    QVariantMap map;
    StorageItem item = m_items[indx];

    map["name"] = item.name;
    map["path"] = item.path;
    map["cleanPath"] = item.cleanPath;
    map["isDir"] = item.isDirectory;
    map["isMinio"] = item.isMinio;
    map["isMinioBucket"] = item.isMinioBucket;
    map["isVirtualDir"] = item.isVirtualDir;
    map["mongoId"] = item.mongoId;
    return map;
}

// write files to Net or Local
Q_INVOKABLE int UnifiedStorageModel::writeImagesToFolder(const QVariantList &lf, QString path){
    qDebug() << "int UnifiedStorageModel::writeImagesToFolder: " << m_parentItem.path <<
        "  isMinio: " << m_parentItem.isMinio << "  isDir: " << m_parentItem.isDirectory;
    if(!m_parentItem.isMinio && m_parentItem.isDirectory){  //  to local directory
        qDebug() << "!m_parentItem.isMinio && m_parentItem.isDirectory";
        QStringList paths;
        QStringList ids;
        // QStringList paths2;
        for(const QVariant &v : lf){
            QVariantMap item = v.toMap();
            //  qDebug() << item["path"].toString() << "  " << item["mongoId"].toString();
            if(item["isNetwork"].toBool() && !item["isDir"].toBool()){  // from Network to local
                paths.append(item["path"].toString());
                ids.append(item["mongoId"].toString());
            }
            else if(!item["isNetwork"].toBool() && !item["isDir"].toBool()){  // from local to local
                copyFileWithUniqueName(item["path"].toString(), m_parentItem.cleanPath);
            }
        }
        msghandler->getFilesRequest(paths, ids, m_parentItem.path);
        return 0;
    }
    else if(m_parentItem.isMinio && m_parentItem.isMinioBucket) {   //  to Net bucket
        qDebug() << "m_parentItem.isMinio && m_parentItem.isMinioBucket";
        QStringList paths;
        for(const QVariant &v : lf){
            QVariantMap item = v.toMap();
            if(!item["isNetwork"].toBool() && !item["isDir"].toBool()) {// from local to network
                QString localPath = QUrl(item["path"].toString()).toLocalFile();
                msghandler->addFileRequest(m_parentItem.path, localPath, item["mongoId"].toString());
            }
            else if(item["isNetwork"].toBool() && !item["isDir"].toBool()) {// from network to network
                msghandler->rewriteFileRequest(m_parentItem.path, item["path"].toString(), item["mongoId"].toString());
            }
        }
        return 0;
    }
    else if(m_parentItem.isMinio && !m_parentItem.isMinioBucket && m_parentItem.isDirectory) {  //  to Net Directory
        qDebug() << "int UnifiedStorageModel::writeImagesMinioDirectory";
        QStringList paths;
        for(const QVariant &v : lf){
            QVariantMap item = v.toMap();
//            if(!item["isNetwork"].toBool() && !item["isDir"].toBool())
            qDebug() << item["path"].toString() << "  " << item["mongoId"].toString() << " target folder: " << m_parentItem.path
                     << "  isNetwork: " << item["isNetwork"].toBool() << "  isDir: " << item["isDir"].toBool();
            if(!item["isNetwork"].toBool() && !item["isDir"].toBool()) { // from local to network
                QString localPath = QUrl(item["path"].toString()).toLocalFile();
                msghandler->addFileRequest(m_parentItem.path, localPath, item["mongoId"].toString());
            }
            else if(item["isNetwork"].toBool() && !item["isDir"].toBool()) {// from network to network
                msghandler->rewriteFileRequest(m_parentItem.path, item["path"].toString(), item["mongoId"].toString());
                //  paths.append(item["path"].toString());
            }
        }
        return 0;
    }
    return 0;
}

Q_INVOKABLE void UnifiedStorageModel::successToQML(const QString &msg){
    qDebug() << "Executed successfully";
}

Q_INVOKABLE void UnifiedStorageModel::errorToQML(const QString &msg){
    qCritical() << "Error when executing command";
}

#include <QString>
#include <QDir>
#include <QFileInfo>

bool checkAndRenameFile(const QString &folderPath, const QString &sourceFileName, const QString &targetFileName) {
    QDir dir(folderPath);

    // Проверяем существование папки и исходного файла
    if (!dir.exists() || !dir.exists(sourceFileName)) {
        return false;
    }

    QString finalName = targetFileName;

    // Если целевой файл уже существует, подбираем имя вида name(number).ext
    if (dir.exists(targetFileName)) {
        QFileInfo fileInfo(targetFileName);
        QString baseName = fileInfo.completeBaseName();
        QString extension = fileInfo.suffix();

        // Добавляем точку перед расширением, если оно есть
        if (!extension.isEmpty()) {
            extension = "." + extension;
        }

        int counter = 1;
        // Цикл работает, пока файл с новым именем существует
        while (dir.exists(baseName + "(" + QString::number(counter) + ")" + extension)) {
            counter++;
        }

        finalName = baseName + "(" + QString::number(counter) + ")" + extension;
    }

    // Переименовываем исходный файл в целевое (или уникальное) имя
    return dir.rename(sourceFileName, finalName);
}

int UnifiedStorageModel::writeUrlsToLocal(const QVector<QUrl> &paths) {
    qDebug() << "UnifiedStorageModel::writeUrlsToLocal" << paths[0];
    if(m_parentItem.isDirectory && !m_parentItem.isMinio){
        QFileInfo checkPath{m_parentItem.path};
        if(checkPath.exists() && checkPath.isDir() && checkPath.isWritable()){
            for(const QUrl &urlpath : paths ){
                FileDownloader *downloader = new FileDownloader(); // Создаем экземпляр
                // Подписываемся на результат
                QObject::connect(downloader, &FileDownloader::downloadFinished, [downloader](const QString &path) {
                    downloader->deleteLater(); // Безопасно удаляем объект из памяти
                });

                QObject::connect(downloader, &FileDownloader::downloadError, [downloader](const QString &err) {
                    downloader->deleteLater();
                });
                // Запуск скачивания
                downloader->downloadFile(QUrl(urlpath), m_parentItem.path);
            }
        }
    }
    return 0;
}

Q_INVOKABLE bool UnifiedStorageModel::getNetPath(const QString &path, int type){
    qDebug() << "UnifiedStorageModel::getNetPath netPrefixes[0]: " << netPrefixes[0] << " netPrefixes[1]: " << netPrefixes[1] << "  path: " << path;
    workPath = path;
    if (path.startsWith(netPrefixes[2], Qt::CaseInsensitive)) {
        workPath.replace(0, 4, netPrefixes[0]);
    }
    if(path.startsWith(netPrefixes[0]) || path.startsWith(netPrefixes[1]) || path.startsWith(netPrefixes[2]) ) {
        if(type != 'f') msghandler -> getNetStore(workPath);
        else   msghandler -> getFileNetStore(workPath);
        return true;
    }  // if exist and file, if exist and dir
    else return false;
}

Q_INVOKABLE void UnifiedStorageModel::setParent(const QString &fullPath, const QString &type){
    QFileInfo fileInfo(fullPath);
    qDebug() << "UnifiedStorageModel::setParent fileInfo.fileName():" << fileInfo.fileName();
    if(type == "mb") m_parentItem = {fileInfo.fileName(), fullPath, fullPath, true, true, true, false};
    if(type == "md") m_parentItem = {fileInfo.fileName(), fullPath, fullPath, true, true, false, false};
    if(type == "mf") m_parentItem = {fileInfo.fileName(), fullPath, fullPath, false, true, false, false};
}

Q_INVOKABLE QVariantMap UnifiedStorageModel::getParent(){
    QVariantMap res;

    //    Вручную наполняем карту данными
    res["name"] = m_parentItem.name;
    res["path"] = m_parentItem.path;
    res["cleanPath"] = m_parentItem.cleanPath;
    res["isDir"] = m_parentItem.isDirectory;
    res["isMinio"] = m_parentItem.isMinio;
    res["isMinioBucket"] = m_parentItem.isMinioBucket;
    res["isVirtualDir"] = m_parentItem.isVirtualDir;
    res["mongoId"] = m_parentItem.mongoId;
    return res;
}
