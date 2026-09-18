#ifndef LISTSTRINGMODEL_H
#define LISTSTRINGMODEL_H

// ImageModel.h
#include <QAbstractListModel>
#include <QStringList>
#include <QUrl>
#include <QDir>
#include <QGuiApplication>
#include <QClipboard>

//#include "websocketclient.h"
#include "restclient.h"
// #include "msghandler.h"
#include "auxilary.h"

struct ImageItem {
    QString name;
    QString path;
    QString cleanPath;
    bool isNetwork;               // Флаг для отличия локального от облачного
    bool isDir;
    QString mongoId;
};

class ImageModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum ImageRoles {
        ImageNameRole = Qt::UserRole + 16, ImagePathRole, ImageCleanPathRole, ImageIsNetworkRole, ImageIsDirRole, ImageMongoIdRole
    };

//    explicit ImageModel(WebSocketClient *wsc, MsgHandler *msgh, QObject *parent = nullptr) :
    explicit ImageModel(RestClient *rc, QObject *parent = nullptr) :
        QAbstractListModel(parent)
        , restClient(rc)
    {
        // connect(msghandler, &MsgHandler::filePathResp, this, &ImageModel:: minioImgToQML);
        // connect(wsclient, &WebSocketClient::filesReceived, this, &ImageModel::minioPathsToQML);
    }

    // 1. Return number of items
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_imageItems.count();
    }

    // 2. Provide data for a specific row and "role"
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_imageItems.count())
            return QVariant();

        const ImageItem &item = m_imageItems.at(index.row());
        switch (role) {
        case ImageNameRole:
            return item.name;
        case ImagePathRole:
            return item.path;
        case ImageCleanPathRole:
            return item.cleanPath;
        case ImageIsNetworkRole:
            return item.isNetwork;
        case ImageIsDirRole:
            return item.isDir;
        case ImageMongoIdRole:
            return item.mongoId;
        default:
            return QVariant();
        }
    }

    // 3. Map integer roles to string names used in QML
protected:
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[ImageNameRole] = "name";
        roles[ImagePathRole] = "path";
        roles[ImageCleanPathRole] = "cleanPath";
        roles[ImageIsNetworkRole] = "isNetwork";
        roles[ImageIsDirRole] = "isDir";
        roles[ImageMongoIdRole] = "mongoId";
        return roles;
    }

public:
    // Helper to add data and notify the view
    Q_INVOKABLE void addImagePath(const QString &path) {
        QUrl url = QUrl::fromLocalFile(path);
        QString pathForQml = url.toString();
        QFileInfo fileInfo(pathForQml);
        QString fileName = fileInfo.fileName();

        beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());
        m_imageItems.append({fileName, pathForQml, cleanLocalFilePath(path), false, false, ""});
        endInsertRows(); // This triggers the QML view update
    }

    Q_INVOKABLE QString resolvePath(const QString &path) {
        QUrl url = QUrl::fromLocalFile(path);
        return url.toString();
    }

    Q_INVOKABLE QString addImagesFromFolder(const QString &folderPath) {
        QDir dir(folderPath);

        // 1. Устанавливаем фильтры: только файлы, игнорируем "." и ".."
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        // 2. Устанавливаем маски для изображений
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif";
        dir.setNameFilters(filters);

        // 3. Получаем список имен файлов (без вложенных папок)
        QStringList fileList = dir.entryList();

        // 4. Добавляем каждый файл в модель
        for (const QString &fileName : std::as_const(fileList)) {
            // Формируем полный путь
            QString fullPath = dir.absoluteFilePath(fileName);

            // Вызываем вашу функцию (убедитесь, что она внутри вызывает beginInsertRows)
            addImagePath(fullPath);
        }
        if(!fileList.isEmpty()) {
            QUrl url = QUrl::fromLocalFile(dir.absoluteFilePath(fileList.first()));
            return url.toString();
        }
        return {};
    }

    Q_INVOKABLE QString addMinioImagePath(const QString &path) {
        QUrl url = QUrl::fromUserInput(path);
        QString pathForQml = url.toString();
        beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

        m_imageItems.append({"img1", pathForQml, extCleanNetworkFilePath(path), true, false, ""});  // TODO:
        endInsertRows(); // This triggers the QML view update
        return pathForQml;
    }

    // Q_INVOKABLE QStringList addImagesFromMinioBucket(const QString &path) {
    //     QString fileName = QUrl(path).fileName();
    //     wsclient->getFilesOnlyListfromBucketRequest(fileName/*, imodel*/);
    //     return {};
    // }

    Q_INVOKABLE QString  minioPathsToQML(const QList<QStringList> &files) {
        for (const QStringList &image : std::as_const(files)) {

            QUrl url = QUrl::fromUserInput(image.at(1));
            QString pathForQml = url.toString();
            beginInsertRows(QModelIndex(), m_imageItems.count(),m_imageItems.count());

            m_imageItems.append({image.at(0), pathForQml, extCleanNetworkFilePath(pathForQml), true, false, image.at(3)});
            endInsertRows(); // This triggers the QML view update
        }
        if(!files.isEmpty()) {
            QUrl url = QUrl::fromUserInput(files.first()[1]);
            minioImageToQML(url.toString());
            return url.toString();
        }
        return {};
    }

    Q_INVOKABLE void minioImgToQML(const QString &fileName, const QString &fileNetPath, const QString &mId){
        QUrl url = QUrl::fromUserInput(fileNetPath);
        QString pathForQml = url.toString();
        beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

        m_imageItems.append({fileName, pathForQml, extCleanNetworkFilePath(pathForQml), true, false, mId});
        endInsertRows();

        minioImageToQML(pathForQml);
    }

    Q_INVOKABLE QVariantMap get(int row) const {
        // Проверка границ, чтобы избежать падения
        if (row < 0 || row >= m_imageItems.count()) {
            return QVariantMap();
        }

//        const QString &item = m_imageItems.at(row);
        const ImageItem &item = m_imageItems.at(row);
        QVariantMap res;

        res["name"] = item.name;
        res["path"] = item.path;
        res["cleanPath"] = item.cleanPath;
        res["isNetwork"] = item.isNetwork;
        res["isDir"] = item.isDir;
        res["mongoId"] = item.mongoId;
        return res;
    }

    Q_INVOKABLE bool removeItem(int index) {
        if (index < 0 || index >= m_imageItems.count()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), index, index);
        m_imageItems.removeAt(index);
        endRemoveRows();
        return true;
    }

    void getImageFromUdsm(QString name, QString path, bool isNet, bool isDir, QString mongoID) {

        if(isNet && !isDir){
            QUrl url = QUrl::fromUserInput(path);
            QString pathForQml = url.toString();
            beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

            m_imageItems.append({name, pathForQml, extCleanNetworkFilePath(pathForQml), isNet, isDir, mongoID});

            endInsertRows(); // This triggers the QML view update
            return;
        }
        if(!isNet && !isDir){
            QUrl url = QUrl::fromUserInput(path);
            QString pathForQml = url.toString();
            beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

            m_imageItems.append({name, pathForQml, cleanLocalFilePath(pathForQml), isNet, isDir, mongoID});

            endInsertRows(); // This triggers the QML view update
            return;
        }

    }
    Q_INVOKABLE int insertImage(const QVariantMap &map) {
        qDebug() << "Q_INVOKABLE int insertImage(const QVariantMap &map)";
        bool isDir = map["isDir"].toBool();
//        bool isNetwork = map["isNetwork"].toBool();
        bool isNetwork = map["isMinio"].toBool();
        if(isNetwork && !isDir){
            QUrl url = QUrl::fromUserInput(map["path"].toString());
            QString pathForQml = url.toString();
            beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

            m_imageItems.append({map["name"].toString(), pathForQml, extCleanNetworkFilePath(pathForQml), isNetwork, isDir, map["mongoId"].toString()});

            endInsertRows(); // This triggers the QML view update
            return 0;
        }
        if(!isNetwork && !isDir){
            QUrl url = QUrl::fromUserInput(map["path"].toString());
            QString pathForQml = url.toString();
            beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count());

            m_imageItems.append({map["name"].toString(), pathForQml, extCleanNetworkFilePath(pathForQml), isNetwork, isDir, map["mongoId"].toString()});
            endInsertRows(); // This triggers the QML view update
            return 0;
        }
        return 0;
    }

    Q_INVOKABLE int insertImages(const QVariantList &lf) {
        QVector<ImageItem> ImItm;
        for(const QVariant &v : lf) {
            QVariantMap map = v.toMap();
            bool isDir = map["isDir"].toBool();
            bool isNetwork = map["isNetwork"].toBool();
            if(isNetwork && !isDir){
                QUrl url = QUrl::fromUserInput(map["path"].toString());
                QString pathForQml = url.toString();
                ImItm.append({map["name"].toString(), pathForQml, extCleanNetworkFilePath(pathForQml), isNetwork, isDir, map["mongoId"].toString()});
            }
            if(!isNetwork && !isDir){
                QUrl url = QUrl::fromUserInput(map["path"].toString());
                QString pathForQml = url.toString();

                ImItm.append({map["name"].toString(), pathForQml, extCleanNetworkFilePath(pathForQml), isNetwork, isDir, map["mongoId"].toString()});
            }
        }
        beginInsertRows(QModelIndex(), m_imageItems.count(), m_imageItems.count() + ImItm.size() - 1);
        m_imageItems.append(ImItm);
        endInsertRows();
        return 0;
    }

    Q_INVOKABLE void copyToClipboard(const QString &text) {
        QGuiApplication::clipboard()->setText(text);
    }
signals:
    void minioImageToQML(const QString &path);

private:
//    QStringList m_imagePaths;
    QVector<ImageItem> m_imageItems;
//    WebSocketClient *wsclient;
//    MsgHandler *msghandler;
    RestClient *restClient;
};

#endif // LISTSTRINGMODEL_H
