#include "savedata.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QDir>

#include "constants.h"
#include "screen.h"
#include "docinfo.h"

void SaveData::loadConfig()
{
    default_tags.clear();
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
                    default_tags.push_back(array[i].toString());
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
    default_tags.sort();
}

QString SaveData::getConfigFilePath()
{
    return QDir(Constants::kBaseFolder).filePath(Constants::kDefaultTagsFile);
}

QString SaveData::getDocsFilePath()
{
    return QDir(Constants::kBaseFolder).filePath(Constants::kDocsFolder);
}

bool SaveData::exportTagsToFile(QFile & file)
{
    QString defaultTagsStr;
    if (!default_tags.isEmpty())
    {
        defaultTagsStr.append("\"").append(default_tags.join("\", \"")).append("\"");
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
    folder_docs_data.clear();

    QDir docsDir(SaveData::getDocsFilePath());
    QFileInfoList infoList = docsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    docs_count = infoList.size();
    for (int i = 0; i < docs_count; ++i)
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

                folder_docs_data.append(docInfo);
            }
            infoFile.close();
        }
    }
}