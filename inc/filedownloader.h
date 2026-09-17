#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class FileDownloader : public QObject {
    Q_OBJECT
public:
    explicit FileDownloader(QObject *parent = nullptr);

    // Основной метод для запуска скачивания
    void downloadFile(const QUrl &url, const QString &destinationFolder);

signals:
    void downloadFinished(const QString &localFilePath);
    void downloadError(const QString &errorStr);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
    void onReadyRead();
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError code);

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
    QFile m_localFile;
};

#endif // FILEDOWNLOADER_H
