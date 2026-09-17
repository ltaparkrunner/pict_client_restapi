#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

// foldermodel.h
#pragma once
#include <QStandardItemModel>
#include <QModelIndex>

class FolderModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit FolderModel(QObject *parent = nullptr) : QStandardItemModel(parent) {
        // Указываем имя колонки (нужно для некоторых типов View)
        setItemRoleNames({{Qt::DisplayRole, "display"}});
    }

    // Метод для добавления папки в дерево
    Q_INVOKABLE void addFolder(const QModelIndex &parentIndex, const QString &name) {
        QStandardItem *parentItem = parentIndex.isValid() ? itemFromIndex(parentIndex) : invisibleRootItem();
        QStandardItem *newFolder = new QStandardItem(name);
        // Можно добавить иконку: newFolder->setData(QUrl("qrc:/icons/folder.png"), Qt::DecorationRole);
        parentItem->appendRow(newFolder);
    }

    // Получение полного пути (например: "Documents/Work/Invoices")
    Q_INVOKABLE QString getFullFolderPath(const QModelIndex &index) {
        QStringList path;
        QModelIndex current = index;
        while (current.isValid()) {
            path.prepend(data(current, Qt::DisplayRole).toString());
            current = current.parent();
        }
        return path.join("/");
    }
};

#endif // FOLDERMODEL_H
