#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <QStringList>
#include <QMap>
#include <QFile>

struct DocInfo;

struct SaveData
{
    QString workingFolder; // path to the database folder
    QStringList defaultTags; // list of default tags
    QMap<QString, QStringList> templates; // templates with tag lists
    QList<DocInfo> folderDocsData; // files that are stored in the app folder
    //
    bool prepareFolders();
    //
    void loadConfig();
    //
    void loadFilesData();
    //
    bool exportTagsToFile(QFile & file);
    //
    QString getConfigFilePath();
    //
    QString getDocsFilePath();
};

#endif // SAVE_DATA_H
