#include "filedownloader.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

FileDownloader::FileDownloader(QObject *parent) : QObject(parent) {}

void FileDownloader::downloadFile(const QUrl &url, const QString &destinationFolder) {
    if (!url.isValid()) {
        emit downloadError("Invalid URL passed.");
        return;
    }

    // Извлекаем имя файла из URL (отсекаем параметры авторизации Minio)
    QString fileName = QFileInfo(url.path()).fileName();
    if (fileName.isEmpty()) {
        fileName = "downloaded_file.dat"; // Имя по умолчанию, если в URL только путь
    }

    // Гарантируем, что целевая директория существует
    QDir dir(destinationFolder);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString fullLocalPath = dir.filePath(fileName);
    m_localFile.setFileName(fullLocalPath);

    // Открываем файл для записи. Если файл существовал, он будет перезаписан.
    if (!m_localFile.open(QIODevice::WriteOnly)) {
        emit downloadError(QString("Failed to open local file for writing: %1").arg(m_localFile.errorString()));
        return;
    }

    // Создаем сеть-запрос. Minio Signed URLs обычно работают через стандартный GET
    QNetworkRequest request(url);

    // Запускаем асинправку запроса
    m_currentReply = m_networkManager.get(request);

    // Связываем сигналы ответа с нашими слотами
    connect(m_currentReply, &QNetworkReply::readyRead, this, &FileDownloader::onReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &FileDownloader::onFinished);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &FileDownloader::downloadProgress);
    connect(m_currentReply, &QNetworkReply::errorOccurred, this, &FileDownloader::onErrorOccurred);
}

void FileDownloader::onReadyRead() {
    // Пишем куски данных в файл по мере их поступления из сети (экономит ОЗУ)
    if (m_currentReply && m_localFile.isOpen()) {
        m_localFile.write(m_currentReply->readAll());
    }
}

void FileDownloader::onFinished() {
    if (!m_currentReply) return;

    m_localFile.close();

    // Проверяем HTTP статус ответа (Minio возвращает 200 OK при успехе)
    int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_currentReply->error() == QNetworkReply::NoError && (statusCode == 200 || statusCode == 206)) {
        emit downloadFinished(m_localFile.fileName());
    } else if (m_currentReply->error() == QNetworkReply::NoError) {
        // Ошибка на уровне протокола HTTP (например, 403 Forbidden из-за протухшего токена)
        m_localFile.remove(); // Удаляем пустой/битый файл
        emit downloadError(QString("HTTP Error: %1").arg(statusCode));
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void FileDownloader::onErrorOccurred(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    if (m_currentReply) {
        m_localFile.close();
        m_localFile.remove(); // Удаляем недокачанный файл в случае сетевого сбоя
        emit downloadError(m_currentReply->errorString());
    }
}
