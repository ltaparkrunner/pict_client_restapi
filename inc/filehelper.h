#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>
// #include "websocketclient.h"
#include "restclient.h"
// #include "auxilary.h"
#include "globals.h"
#include <QtQml/qqml.h>

class FileHelper : public QObject {
    Q_OBJECT
//    QML_ELEMENT
public:
    enum PathType {
        Unknown,
        LocalFile,
        LocalFolder,
        MinioFile,
        MinioBucket,
        MinioFolder
    };
    Q_ENUM(PathType) // Регистрируем enum в системе метаданных Qt

    // explicit FileHelper(/*WebSocketClient *wsc, */QObject *parent = nullptr);
    explicit FileHelper(RestClient *rc, QObject *parent = nullptr);
    // Q_INVOKABLE bool exists(const QString &path);
    // Q_INVOKABLE bool fileExists(const QString &path);
    // Q_INVOKABLE int checkPathType(const QString &path);
    Q_INVOKABLE QVariantMap extCheckPathType(const QString &path);
    // Q_INVOKABLE bool writeToFile(const QString &fileUrl, const QString &content);
    // Q_INVOKABLE bool saveFilesToFolder(const QString &folderUrl, const QStringList &fileUrls);

    // Q_INVOKABLE int processWritePathsLocal(const QStringList &ls, const QString &path);
    // Q_INVOKABLE int processWritePathsMinio(const QStringList &ls, const QString &path);
    // Q_INVOKABLE int processDeleteFolderLocal(const QString &path);
    // Q_INVOKABLE int processDeleteFolderMinio(const QString &path);
    // Q_INVOKABLE int processDeleteFileLocal(const QString &path);
    // Q_INVOKABLE int processDeleteFileMinio(const QString &path);

    // Q_INVOKABLE int deleteMinioBuckets(const QStringList &paths);

private:
    // WebSocketClient *wsclient;
    RestClient *restClient;
    const QStringList netPrefixes = Config::netPrefixes();
};

#endif // FILEHELPER_H
