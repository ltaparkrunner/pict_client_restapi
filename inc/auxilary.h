#ifndef AUXILARY_H
#define AUXILARY_H
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

QString cleanNetworkFilePath(QString networkFilePath);
QString extCleanNetworkFilePath(QString networkFilePath);
QString cleanLocalFilePath(QString localFilePath);
QString cleanFilePath(QString filePath, bool isNetwork);

bool copyFileWithUniqueName(const QString &sourceFilePath, const QString &targetDirPath);
#endif // AUXILARY_H
