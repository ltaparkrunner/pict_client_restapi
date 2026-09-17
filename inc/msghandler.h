#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QtProtobuf/QProtobufSerializer>
//#include <QUrl>
#include "websocketclient.h"
#include "auxilary.h"


class MsgHandler : public QObject {
    Q_OBJECT
public:
    explicit MsgHandler(WebSocketClient *client, QObject *parent = nullptr);

    Q_INVOKABLE int getBucketsListRequest() const;
    int addFileRequest(const QString &folder, const QString &path, const QString &mId);
    int getFilesFoldersListfromBucketRequest(const QString &path, const QString &name);
    int getFilesFoldersListfromBucketRequest2(const QString &path, const bool isDir);
    //  int getFileRequest(const QString &path, const QString &id, const QString &arrival);
    int getFilesRequest(const QStringList &path, const QStringList &id, const QString &arrival);
    int getNetStore(const QString &netPath);
    int getFileNetStore(const QString &netPath);
    // int getFilesOnlyListfromBucketRequest(const QString &path);

    // int deleteMinioBucketsRequest(const QStringList &buckets);
    int deleteFileFromServerRequest(const QStringList &fileData);
    int rewriteFileRequest(const QString &folderPath, const QString &filePath, const QString &id);

signals:
    //  void startWebSocket(QString token);
    void bucketsReceived(const QStringList &buckets);
    void pathsReceived(const QList<QStringList> &paths, const QString &folderName);
    void resultSuccess(const QString &msg);
    void resultError(const QString &msg);
    void writeUrlsToLocal(const QVector<QUrl> &paths);
    void pathInfoResp(const int, const QString &netPath, const QString &cleanNetPath);
    void filePathResp(const QString &fileName, const QString &url, const QString &mId);
public slots:

private slots:
    void handleIncomingServerData(const pict_data::ServerEnvelope &data);


private:
    WebSocketClient *m_client;
    QString token;
};
#endif // MSGHANDLER_H
