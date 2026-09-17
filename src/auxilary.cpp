#include "auxilary.h"
#include <QRegularExpression>

QString cleanNetworkFilePath(QString networkFilePath){
    QUrl url(networkFilePath);
    url.setQuery(QUrlQuery());
    return url.toString();
}

QString extCleanNetworkFilePath(QString networkFilePath){
    QUrl url(networkFilePath);
    url.setQuery(QUrlQuery());

    QStringList parts = url.toString().split('/' /*, Qt::SkipEmptyParts*/);

    // Проверяем, что в пути есть как минимум 4 части
    if (parts.size() >= 6) {
        const QString& fourthPart = parts.at(5); // Индекс 3 — это 4-я часть

        // Регулярное выражение: только hex-символы, длина от 24 и более (> 23)
        static QRegularExpression hexRegex("^[0-9a-fA-F]{24,}$");

        if (hexRegex.match(fourthPart).hasMatch()) {
            // Удаляем 4-ю часть (индекс 3) и 3-ю часть (индекс 2)
            // Удаляем дважды индекс 2, так как после первого удаления элементы сдвигаются
            parts.removeAt(4);
            parts.removeAt(4);
        }
    }

    // Собираем оставшиеся части обратно
    // Добавляем начальный слэш, если исходный путь был абсолютным (начинался с '/')
    QString newPath = parts.join("/");
    if (networkFilePath.startsWith('/')) {
        newPath = "/" + newPath;
    }

    return newPath;
}

QString cleanLocalFilePath(QString localFilePath){
    if (localFilePath.startsWith("file:///")) {
        return QUrl(localFilePath).toLocalFile();
    }
    return localFilePath;
}

QString cleanFilePath(QString filePath, bool isNetwork){
    if(isNetwork){
        QUrl url(filePath);
        url.setQuery(QUrlQuery());
        return url.toString();
    }
    else return QUrl(filePath).toLocalFile();
}

bool copyFileWithUniqueName(const QString &sourceFilePath, const QString &targetDirPath) {
    QFileInfo sourceInfo(sourceFilePath);
    if (!sourceInfo.exists()) {
        return false;
    }

    // Extract base name and extension separately
    QString baseName = sourceInfo.baseName();    // e.g., "data" from "data.tar.gz"
    QString completeSuffix = sourceInfo.completeSuffix(); // e.g., "tar.gz"
    if (!completeSuffix.isEmpty()) {
        completeSuffix = "." + completeSuffix;
    }

    QDir targetDir(targetDirPath);
    QString destinationPath = targetDir.filePath(sourceInfo.fileName());

    int counter = 1;
    // Loop until a unique file name is found
    while (QFile::exists(destinationPath)) {
        QString newFileName = QString("%1(%2)%3").arg(baseName).arg(counter).arg(completeSuffix);
        destinationPath = targetDir.filePath(newFileName);
        counter++;
    }

    // Copy the file to the unique destination path
    if (QFile::copy(sourceFilePath, destinationPath)) {
        return true;
    } else {
        return false;
    }
}
