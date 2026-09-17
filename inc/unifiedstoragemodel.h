#ifndef UNifIEDSTORAGEMODEL_H
#define UNifIEDSTORAGEMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include "websocketclient.h"
#include "msghandler.h"
#include "globals.h"

// enum PathType {
//     Unknown,
//     LocalFile,
//     LocalFolder,
//     MinioFile,
//     MinioBucket
// };
// For now:         localFile    LocalDir     MinioBucket       MinioFile      MinioDirInServer   MinioDirVirtual
// isDirectory        false      true          X/true            false           true              true
// isMinio            false      false         true              true            true              true

// isMinioBucket       X          X            true              false           false             false
// isVirtualDir        X          X            X                 X               false             true
// VirtualMinioDir: only create new virtual folder, or paste file there.

struct StorageItem {
    QString name;
    QString path;
    QString cleanPath;
    bool isDirectory;
    bool isMinio;               // Флаг для отличия локального от облачного
    bool isMinioBucket;
    bool isVirtualDir;
    QString mongoId;
};

class UnifiedStorageModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { NameRole = Qt::UserRole + 2, PathRole, CleanPathRole, IsDirRole, IsMinioRole,  IsMinioBucketRole, IsVirtualDirRole, MongoIdRole};

    explicit UnifiedStorageModel(WebSocketClient *wsc, MsgHandler *svrHndlr, QObject *parent = nullptr);
    // Основные методы модели
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariantMap getData(int indx);

    // Логика навигации
    Q_INVOKABLE void loadRoot();          // Показать выбор: "Local" и "MinIO"
    Q_INVOKABLE void enterLocal(const QString &path); // Зайти в локальную папку
    Q_INVOKABLE void enterNetStore(QString path);        // Зайти в список бакетов MinIO
//    Q_INVOKABLE void enterNetStore2(const QString &path);        // Зайти в список бакетов MinIO
    Q_INVOKABLE void enterMinioBucket(const QString &path);
    Q_INVOKABLE int openFolderImages(int indx);
    Q_INVOKABLE int enterFolder(int indx);
    Q_INVOKABLE bool getNetPath(const QString &path, int type);
    Q_INVOKABLE int deleteIndices(const QList<int> &indxs);
    //  Q_INVOKABLE int writeToFolder(const QStringList &ls);
    Q_INVOKABLE void minioPathsToQML(const QList<QStringList> &paths, const QString &folderName);
    Q_INVOKABLE void minioBucketsToQML(const QStringList &paths);
    Q_INVOKABLE QVariantMap get(int row) const;

    Q_INVOKABLE int addVirtual(const QString &virtFolderName, const QString &currPath);
    QStringList getBacketNameFromPath(const QString &path);
    Q_INVOKABLE QString resolveImageIndex(int indx);
    Q_INVOKABLE int writeImagesToFolder(const QVariantList &lf, QString path);
    Q_INVOKABLE void successToQML(const QString &msg);
    Q_INVOKABLE void errorToQML(const QString &msg);
    int writeUrlsToLocal(const QVector<QUrl> &paths);

    Q_INVOKABLE void setParent(const QString &path, const QString &type);
    Q_INVOKABLE QVariantMap getParent();
//    Q_INVOKABLE  getParent();

signals:
    void udsmToIm(QString, QString, bool, bool, QString);
    void openNetStoreDialog(const int sel, const QString &netPath, const QString &cleanPath);
private:
    QVector<StorageItem> m_items;
    StorageItem m_parentItem;
    WebSocketClient *wsclient;
    MsgHandler *msghandler;
    const QStringList allowedExtensions = {"jpg", "jpeg", "png", "bmp", "gif"};
    QString prefix = "http://minio:9000/";
    const QStringList netPrefixes = Config::netPrefixes();
    QString workPath;
};

#endif // UNifIEDSTORAGEMODEL_H
