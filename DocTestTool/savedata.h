#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <QStringList>
#include <QMap>
#include <QFile>

struct DocInfo;

struct SaveData
{
    QStringList default_tags; // list of default tags
    QMap<QString, QStringList> templates; // templates with tag lists
    int docs_count = 0; // amount of files found in the folder
    QList<DocInfo> folder_docs_data; // files that are stored in the app folder
    //
    void loadConfig();
    //
    void loadFilesData();
    //
    bool exportTagsToFile(QFile & file);
    //
    static QString getConfigFilePath();
    //
    static QString getDocsFilePath();
};

#endif // SAVE_DATA_H
