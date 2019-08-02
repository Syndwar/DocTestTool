#include "savedata.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QDir>

#include "constants.h"
#include "screen.h"
#include "docinfo.h"

bool SaveData::prepareFolders()
{
    // create base folder
    if (!QDir(workingFolder).exists())
    {
        if (!QDir(Constants::kBaseFolder).exists())
        {
            QDir().mkdir(Constants::kBaseFolder);
        }
    }

    if (!QDir(getDocsFilePath()).exists())
    {
        QDir().mkdir(getDocsFilePath());
    }

    // create file for default tags
    QFile tagsFile(getConfigFilePath());
    if (!tagsFile.exists())
    {
        if (!exportTagsToFile(tagsFile))
        {
            return false;
        }
    }
    return true;
}

void SaveData::loadConfig()
{
    defaultTags.clear();
    QFile tagsFile(SaveData::getConfigFilePath());
    if (tagsFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = tagsFile.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        QJsonObject obj = jsonDoc.object();
        if (!obj.isEmpty())
        {
            // load default tags
            QJsonValue tagsValue = obj[Constants::kTags];
            if (tagsValue.isArray())
            {
                QJsonArray array = tagsValue.toArray();
                for (int i = 0, iEnd = array.size(); i < iEnd; ++i)
                {
                    defaultTags.push_back(array[i].toString());
                }
            }
            // load templates
            QJsonValue templatesValue = obj[Constants::kTemplates];
            if (templatesValue.isObject())
            {
                QJsonObject templObj = templatesValue.toObject();
                QVariantMap templMap = templObj.toVariantMap();
                auto it = templMap.constBegin();
                while (it != templMap.constEnd())
                {
                    templates[it.key()] = it.value().toStringList();
                    ++it;
                }

            }
        }
        tagsFile.close();
    }
    defaultTags.sort();
}

QString SaveData::getConfigFilePath()
{
    if (QDir(workingFolder).exists())
    {
        return QDir(workingFolder).filePath(Constants::kDefaultTagsFile);
    }
    return QDir(Constants::kBaseFolder).filePath(Constants::kDefaultTagsFile);
}

QString SaveData::getDocsFilePath()
{
    if (QDir(workingFolder).exists())
    {
        return QDir(workingFolder).filePath(Constants::kDocsFolder);
    }
    return QDir(Constants::kBaseFolder).filePath(Constants::kDocsFolder);
}

bool SaveData::exportTagsToFile(QFile & file)
{
    QString defaultTagsStr;
    if (!defaultTags.isEmpty())
    {
        defaultTagsStr.append("\"").append(defaultTags.join("\", \"")).append("\"");
    }
    QString templatesStr;
    auto it = templates.constBegin();
    while (it != templates.constEnd())
    {
        if (!templatesStr.isEmpty())
        {
            templatesStr.append(", ");
        }
        templatesStr.append("\"").append(it.key()).append("\":[");
        if (!it.value().isEmpty())
        {
            templatesStr.append("\"").append(it.value().join("\", \"")).append("\"");
        }
        templatesStr.append("]");
        ++it;
    }

    const QString valTmpl = "{\"tags\":[%1], \"templates\":{%2}}";
    const QString val = valTmpl.arg(defaultTagsStr).arg(templatesStr);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);
    if (!jsonDoc.isNull())
    {
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(json);
            file.close();
        }
    }
    else
    {
        return false;
    }
    return true;
}

void SaveData::loadFilesData()
{
    folderDocsData.clear();

    QDir docsDir(SaveData::getDocsFilePath());
    QFileInfoList infoList = docsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0, iEnd = infoList.size(); i < iEnd; ++i)
    {
        QFileInfo & info = infoList[i];
        QString path = info.absoluteFilePath();
        QFile infoFile(QDir(path).absoluteFilePath(Constants::kInfoDocFile));
        if (infoFile.open(QIODevice::ReadOnly))
        {
            QByteArray fileData = infoFile.readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
            QJsonObject obj = jsonDoc.object();
            if (!obj.isEmpty())
            {
                DocInfo docInfo;
                // load default tags
                QJsonValue tagsValue = obj[Constants::kTags];
                if (tagsValue.isArray())
                {
                    QJsonArray array = tagsValue.toArray();
                    for (int i = 0, iEnd = array.size(); i < iEnd; ++i)
                    {
                        docInfo.tags.push_back(array[i].toString());
                    }
                }
                // load templates
                QJsonValue commentValue = obj[Constants::kComment];
                if (commentValue.isString())
                {
                    docInfo.comment = commentValue.toString();
                }
                QJsonValue fileValue = obj[Constants::kFilename];
                if (fileValue.isString())
                {
                    docInfo.fileName = fileValue.toString();
                }

                docInfo.filePath = QDir(path).absoluteFilePath(docInfo.fileName);

                folderDocsData.append(docInfo);
            }
            infoFile.close();
        }
    }
}