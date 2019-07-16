#include "doctesttool.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "quazip.h"
#include "quazipfile.h"
#include "quazipnewinfo.h"
#include <algorithm>

namespace
{
const char * const kBaseFolder = "base";
const char * const kDocsFolder = "docs";
const char * const kDefaultTagsFile = "config.json";
const char * const kInfoDocFile = "info.json";
const char * const kComment = "comment";
const char * const kFilename = "filename";
const char * const kTags = "tags";
const QString kUploadFilters = "Documents (*.pdf *.tiff);;Images (*.jpg *.jpeg *.png);;Data Only (*.txt *json *html);;XML (*.xml);;All files (*.*)";
const char * const kDelimiter = ", ";
}

struct DocInfo
{
    QString filePath;
    QString fileName;
    QList<QString> tags;
    QString comment;
};

QString getConfigFilePath()
{
    return QDir(kBaseFolder).filePath(kDefaultTagsFile);
}

QString getDocsFilePath()
{
    return QDir(kBaseFolder).filePath(kDocsFolder);
}

DocTestTool::DocTestTool(QWidget * parent)
    : QMainWindow(parent)
    , m_viewMode(ViewMode::Main)
    , m_docsCount(0)
{
    ui.setupUi(this);

    viewMainScreen(true);
    viewUploadScreen(false);
    viewEditScreen(false);
    viewSearchScreen(false);

    QObject::connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QObject::connect(ui.uploadBtn, SIGNAL(clicked()), this, SLOT(OnUploadButtonClicked()));
    QObject::connect(ui.editBtn, SIGNAL(clicked()), this, SLOT(OnEditButtonClicked()));
    QObject::connect(ui.searchBtn, SIGNAL(clicked()), this, SLOT(OnSearchButtonClicked()));

    QObject::connect(ui.editCancelBtn, SIGNAL(clicked()), this, SLOT(OnEditCancelButtonClicked()));
    QObject::connect(ui.editSaveBtn, SIGNAL(clicked()), this, SLOT(OnEditSaveButtonClicked()));
    
    QObject::connect(ui.uploadCancelBtn, SIGNAL(clicked()), this, SLOT(OnUploadCancelButtonClicked()));
    QObject::connect(ui.uploadOkBtn, SIGNAL(clicked()), this, SLOT(OnUploadOkButtonClicked()));

    QObject::connect(ui.uploadTagBtn, SIGNAL(clicked()), this, SLOT(OnUploadTagButtonClicked()));
    QObject::connect(ui.uploadDeleteBtn, SIGNAL(clicked()), this, SLOT(OnUploadDeleteButtonClicked()));
    QObject::connect(ui.uploadCommentBtn, SIGNAL(clicked()), this, SLOT(OnUploadCommentButtonClicked()));
    QObject::connect(ui.uploadAddBtn, SIGNAL(clicked()), this, SLOT(OnUploadAddButtonClicked()));

    QObject::connect(ui.findTagBtn, SIGNAL(clicked()), this, SLOT(OnFindTagButtonClicked()));
    QObject::connect(ui.findCommentBtn, SIGNAL(clicked()), this, SLOT(onFindCommentButtonClicked()));
    QObject::connect(ui.searchDownloadBtn, SIGNAL(clicked()), this, SLOT(onDownloadButtonClicked()));
    QObject::connect(ui.clearTagBtn, SIGNAL(clicked()), this, SLOT(onClearTagButtonClicked()));
    QObject::connect(ui.clearCommentBtn, SIGNAL(clicked()), this, SLOT(onClearCommentButtonClicked()));

    QObject::connect(ui.tagsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnTagsListDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(OnListWidgetClicked(QListWidgetItem *)));
    QObject::connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnListWidgetDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.searchCancelBtn, SIGNAL(clicked()), this, SLOT(onSearchBackButtonClicked()));

    ui.listWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    // create base folder
    if (!QDir(kBaseFolder).exists())
    {
        QDir().mkdir(kBaseFolder);
    }

    if (!QDir(getDocsFilePath()).exists())
    {
        QDir().mkdir(getDocsFilePath());
    }

    // create file for default tags
    QFile tagsFile(getConfigFilePath());
    if (!tagsFile.exists())
    {
        const QString val = "{\"tags\":[], \"templates\":{}}";
        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

        if (tagsFile.open(QIODevice::WriteOnly))
        {
            tagsFile.write(json);
            tagsFile.close();
        }
    }

    loadConfig();
    loadFilesData();

    tuneView();
}

void DocTestTool::tuneView()
{
    //ui.searchCancelBtn->setStyleSheet(QString("background-color: red"));
    //ui.uploadCancelBtn->setStyleSheet(QString("background-color: red"));
    //ui.editCancelBtn->setStyleSheet(QString("background-color: red"));
    //ui.searchDownloadBtn->setStyleSheet(QString("background-color: green; color:white"));
    //ui.editSaveBtn->setStyleSheet(QString("background-color: green; color:white"));
    //ui.uploadOkBtn->setStyleSheet(QString("background-color: green; color:white"));
    //ui.findTagBtn->setStyleSheet(QString("background-color: blue; color:white"));
    //ui.findCommentBtn->setStyleSheet(QString("background-color: blue; color:white"));
}

void DocTestTool::loadConfig()
{
    m_defaultTags.clear();
    QFile tagsFile(getConfigFilePath());
    if (tagsFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = tagsFile.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        QJsonObject obj = jsonDoc.object();
        if (!obj.isEmpty())
        {
            // load default tags
            QJsonValue tagsValue = obj[kTags];
            if (tagsValue.isArray())
            {
                QJsonArray array = tagsValue.toArray();
                for (int i = 0, iEnd = array.size(); i < iEnd; ++i)
                {
                    m_defaultTags.push_back(array[i].toString());
                }
            }
            // load templates
            QJsonValue templatesValue = obj["templates"];
            if (templatesValue.isObject())
            {
                QJsonObject templObj = templatesValue.toObject();
                QVariantMap templMap = templObj.toVariantMap();
                auto it = templMap.constBegin();
                while (it != templMap.constEnd())
                {
                    m_templates[it.key()] = it.value().toStringList();
                    ++it;
                }

            }
        }
        tagsFile.close();
    }
}

void DocTestTool::loadFilesData()
{
    m_folderDocsData.clear();

    QDir docsDir(getDocsFilePath());
    QFileInfoList infoList = docsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    m_docsCount = infoList.size();
    for (int i = 0; i < m_docsCount; ++i)
    {
        QFileInfo & info = infoList[i];
        QString path = info.absoluteFilePath();
        QFile infoFile(QDir(path).absoluteFilePath(kInfoDocFile));
        if (infoFile.open(QIODevice::ReadOnly))
        {
            QByteArray fileData = infoFile.readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
            QJsonObject obj = jsonDoc.object();
            if (!obj.isEmpty())
            {
                DocInfo docInfo;
                // load default tags
                QJsonValue tagsValue = obj[kTags];
                if (tagsValue.isArray())
                {
                    QJsonArray array = tagsValue.toArray();
                    for (int i = 0, iEnd = array.size(); i < iEnd; ++i)
                    {
                        docInfo.tags.push_back(array[i].toString());
                    }
                }
                // load templates
                QJsonValue commentValue = obj[kComment];
                if (commentValue.isString())
                {
                    docInfo.comment = commentValue.toString();
                }
                QJsonValue fileValue = obj[kFilename];
                if (fileValue.isString())
                {
                    docInfo.fileName = fileValue.toString();
                }

                docInfo.filePath = QDir(path).absoluteFilePath(docInfo.fileName);

                m_folderDocsData.append(docInfo);
            }
            infoFile.close();
        }
    }



}

void DocTestTool::loadDocsRepo(QStringList & fileNames)
{
    for (QString & fileName : fileNames)
    {
        QFile file(fileName);
        QFileInfo info(file);
        
        DocInfo docInfo;
        docInfo.filePath = fileName;
        docInfo.fileName = info.fileName();
        m_loadedDocsData.append(docInfo);
    }
}

DocTestTool::~DocTestTool()
{
}

//========================================
// Screen selection
//========================================
void DocTestTool::viewMainScreen(const bool value)
{
    if (value) m_viewMode = ViewMode::Main;
    ui.uploadBtn->setVisible(value);
    ui.editBtn->setVisible(value);
    ui.searchBtn->setVisible(value);
}

void DocTestTool::viewSearchScreen(const bool value)
{
    if (value) m_viewMode = ViewMode::Search;

    m_foundDocsData.clear();

    updateTagsListWidget();
    ui.listWidget->clear();
    ui.uploadTagsTextEdit->clear();
    ui.uploadCommentsTextEdit->clear();
    ui.findTagBtn->setVisible(value);
    ui.findCommentBtn->setVisible(value);
    ui.uploadTagsTextEdit->setVisible(value);
    ui.uploadCommentsTextEdit->setVisible(value);
    ui.listWidget->setVisible(value);
    ui.searchCancelBtn->setVisible(value);
    ui.searchDownloadBtn->setVisible(value);
    ui.tagsListWidget->setVisible(value);
    ui.clearTagBtn->setVisible(value);
    ui.clearCommentBtn->setVisible(value);
    ui.fullMatchBox->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.singleFolderCheckBox->setVisible(value);
}

void DocTestTool::viewUploadScreen(const bool value)
{
    if (value) m_viewMode = ViewMode::Upload;

    updateTagsListWidget();

    ui.listWidget->setVisible(value);
    ui.uploadOkBtn->setVisible(value);
    ui.uploadCancelBtn->setVisible(value);
    ui.uploadTagBtn->setVisible(value);
    ui.uploadCommentBtn->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.uploadAddBtn->setVisible(value);
    ui.uploadTagsTextEdit->setVisible(value);
    ui.uploadCommentsTextEdit->setVisible(value);
    ui.tagsListWidget->setVisible(value);
    ui.clearTagBtn->setVisible(value);
    ui.clearCommentBtn->setVisible(value);
}

void DocTestTool::updateTagsListWidget()
{
    ui.tagsListWidget->clear();

    for (QString & tag : m_defaultTags)
    {
        ui.tagsListWidget->addItem(tag);
    }

    auto it = m_templates.constBegin();
    while (it != m_templates.constEnd())
    {
        QListWidgetItem * item = new QListWidgetItem(it.key());
        item->setTextColor("blue");
        ui.tagsListWidget->addItem(item);
        ++it;
    }
}

void DocTestTool::viewEditScreen(const bool value)
{
    if (value) m_viewMode = ViewMode::Edit;
    QFile tagsFile(getConfigFilePath());
    if (tagsFile.exists())
    {
        if (tagsFile.open(QIODevice::ReadOnly))
        {
            QByteArray fileData = tagsFile.readAll();
            ui.editTextWnd->setText(fileData);
            tagsFile.close();
        }
    }

    ui.editTextWnd->setVisible(value);
    ui.editCancelBtn->setVisible(value);
    ui.editSaveBtn->setVisible(value);
}

//========================================
// Main screen
//========================================
void DocTestTool::OnSearchButtonClicked()
{
    viewMainScreen(false);
    viewSearchScreen(true);
}

void DocTestTool::OnUploadButtonClicked()
{
    ui.listWidget->clear();
    m_loadedDocsData.clear();

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select one or more files to open", QString(), kUploadFilters);

    loadDocsRepo(fileNames);

    for (DocInfo & info : m_loadedDocsData)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui.listWidget->addItem(newItem);
    }

    if (!fileNames.empty())
    {
        viewMainScreen(false);
        viewUploadScreen(true);
    }
}

void DocTestTool::OnEditButtonClicked()
{
    viewMainScreen(false);
    viewEditScreen(true);
}
//========================================
// Edit screen
//========================================
void DocTestTool::OnEditSaveButtonClicked()
{
    const QString val = ui.editTextWnd->toPlainText();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    if (!jsonDoc.isNull())
    {
        QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);
        QFile tagsFile(getConfigFilePath());
        if (tagsFile.exists())
        {
            QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

            if (tagsFile.open(QIODevice::WriteOnly))
            {
                tagsFile.write(json);
                tagsFile.close();
            }
        }
        loadConfig();
        ui.statusBar->clearMessage();
        ui.editTextWnd->clear();
        viewMainScreen(true);
        viewEditScreen(false);
    }
    else
    {
        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Json is invalid!", 2000);
    }
}

void DocTestTool::OnEditCancelButtonClicked()
{
    ui.statusBar->clearMessage();
    ui.editTextWnd->clear();
    viewMainScreen(true);
    viewEditScreen(false);
}
//========================================
// Upload screen
//========================================
void DocTestTool::OnUploadOkButtonClicked()
{
    // check if all documents has tags
    int missingTagIndex = -1;
    for (int i = 0, iEnd = m_loadedDocsData.size(); i < iEnd; ++i)
    {
        DocInfo & info = m_loadedDocsData[i];
        if (info.tags.empty())
        {
            missingTagIndex = i;
            break;
        }
    }
    // if tag is missing than set colour to red and scroll to this item
    if (missingTagIndex >= 0)
    {
        QListWidgetItem * item = ui.listWidget->item(missingTagIndex);
        item->setTextColor("red");
        ui.listWidget->scrollToItem(item);

        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Tag is missing!", 2000);
        return;
    }

    for (DocInfo & info : m_loadedDocsData)
    {
        QFile file(info.filePath);
        QFileInfo fileInfo(file);

        QString folderPath(getDocsFilePath());
        ++m_docsCount;
        folderPath.append("/").append(QString::number(m_docsCount));
        if (!QDir(folderPath).exists())
        {
            QDir().mkdir(folderPath);
            const QString filepath = QDir(folderPath).filePath(fileInfo.fileName());
            file.copy(filepath);

            const QString infoPath = QDir(folderPath).filePath(kInfoDocFile);
            QFile infoFile(infoPath);
            const QString val = "{}";
            QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
            QJsonObject obj = jsonDoc.object();
            obj[kComment] = info.comment;
            obj[kFilename] = info.fileName;
            obj[kTags] = QJsonArray::fromStringList(info.tags);
            jsonDoc.setObject(obj);
            
            QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

            if (infoFile.open(QIODevice::WriteOnly))
            {
                infoFile.write(json);
                infoFile.close();
            }
        }
    }
    loadFilesData();
    ui.listWidget->clear();
    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::OnUploadAddButtonClicked()
{
    // TODO check for duplicates
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select one or more files to open", QString(), kUploadFilters);

    loadDocsRepo(fileNames);

    ui.listWidget->clear();
    for (DocInfo & info : m_loadedDocsData)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui.listWidget->addItem(newItem);
    }
}

void DocTestTool::OnUploadDeleteButtonClicked()
{
    // get rows
    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    QList<int> indexList;
    for (QModelIndex & index : indexes)
    {
        indexList.append(index.row());
    }
    // sort in reverse order
    std::sort(indexList.begin(), indexList.end());
    std::reverse(indexList.begin(), indexList.end());

    for (int i : indexList)
    {
        if (m_viewMode == ViewMode::Upload)
        {
            m_loadedDocsData.removeAt(i);
        }
        else if (m_viewMode == ViewMode::Search)
        {
            m_foundDocsData.removeAt(i);
        }
    }

    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui.listWidget->removeItemWidget(item);
        delete item;
    }
}

void DocTestTool::OnUploadTagButtonClicked()
{
    const QString text = ui.uploadTagsTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "blue");
    }

    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            if (text.isEmpty())
            {
                info.tags.clear();
            }
            else
            {
                info.tags = text.split(kDelimiter);
            }
        }
    }
}

void DocTestTool::OnUploadCommentButtonClicked()
{
    const QString text = ui.uploadCommentsTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "green");
    }

    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            info.comment = text;
        }
    }
}

void DocTestTool::OnUploadCancelButtonClicked()
{
    ui.listWidget->clear();
    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::OnListWidgetClicked(QListWidgetItem * item)
{
    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    if (indexes.size() > 1) return;

    const int i = indexes[0].row();

    if (ViewMode::Upload == m_viewMode)
    {
        if (i < m_loadedDocsData.size())
        {
            const DocInfo & info = m_loadedDocsData[i];
            ui.uploadCommentsTextEdit->setText(info.comment);
            ui.uploadTagsTextEdit->setText(info.tags.join(kDelimiter));
        }
    }
    else if (ViewMode::Search == m_viewMode)
    {
        if (i < m_foundDocsData.size())
        {
            const DocInfo & info = m_foundDocsData[i];
            ui.uploadCommentsTextEdit->setText(info.comment);
            ui.uploadTagsTextEdit->setText(info.tags.join(kDelimiter));
        }
    }
}

//========================================
// Search screen
//========================================
void DocTestTool::onClearCommentButtonClicked()
{
    ui.uploadCommentsTextEdit->clear();
}

void DocTestTool::onClearTagButtonClicked()
{
    ui.uploadTagsTextEdit->clear();
}

void DocTestTool::onDownloadButtonClicked()
{
    if (m_foundDocsData.size() == 0) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), tr("documents.zip"), tr("Zip *.zip"));
    if (!fileName.isEmpty())
    {
        QString delimiter = ui.singleFolderCheckBox->isChecked() ? "-" : "/";
        char c;
        if (!fileName.endsWith(".zip"))
        {
            fileName.append(".zip");
        }
        QuaZip zip(fileName);
        if (zip.open(QuaZip::mdCreate))
        {
            int i = 0;
            for (DocInfo & info : m_foundDocsData)
            {
                QFile inFile(info.filePath);
                if (inFile.open(QIODevice::ReadOnly))
                {
                    const QString zipPath = QString::number(++i).append(delimiter).append(info.fileName);
                    QuaZipNewInfo zipInfo(zipPath, info.filePath);
                    QuaZipFile zipFile(&zip);
                    if (zipFile.open(QIODevice::WriteOnly, zipInfo))
                    {
                        while (inFile.getChar(&c) && zipFile.putChar(c));
                        zipFile.close();
                    }
                    inFile.close();
                }
            }
            zip.close();
        }
    }
}

void DocTestTool::OnTagsListDoubleClicked(QListWidgetItem * item)
{
    if (ViewMode::Search != m_viewMode && ViewMode::Upload != m_viewMode) return;
    
    QLineEdit * editText = ui.uploadTagsTextEdit;

    const QString key = item->text();
    if (!key.isEmpty())
    {
        auto it = m_templates.find(key);
        if (it != m_templates.constEnd())
        {
            editText->setText(it.value().join(kDelimiter));
        }
        else
        {
            QString curText = editText->text();
            if (curText.isEmpty())
            {
                editText->setText(key);
            }
            else
            {
                editText->setText(curText.append(kDelimiter).append(key));
            }
        }
    }
}

void DocTestTool::onFindCommentButtonClicked()
{
}

void DocTestTool::OnFindTagButtonClicked()
{
    ui.listWidget->clear();
    m_foundDocsData.clear();

    if (ui.fullMatchBox->isChecked())
    {
        doStrictSearch();
    }
    else
    {
        doGreedySearch();
    }

    for (DocInfo & info : m_foundDocsData)
    {
        ui.listWidget->addItem(info.fileName);
    }
}

void DocTestTool::doGreedySearch()
{
    const QString findText = ui.uploadTagsTextEdit->text();
    if (!findText.isEmpty())
    {
        const QString editText = ui.uploadTagsTextEdit->text();
        const QStringList searchTags = editText.split(kDelimiter);

        for (DocInfo & info : m_folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.tags.contains(tag))
                {
                    m_foundDocsData.append(info);
                    break;
                }
            }
        }
    }
}

void DocTestTool::doStrictSearch()
{
    const QString findText = ui.uploadTagsTextEdit->text();
    if (!findText.isEmpty())
    {
        const QString editText = ui.uploadTagsTextEdit->text();
        const QStringList searchTags = editText.split(kDelimiter);

        for (DocInfo & info : m_folderDocsData)
        {
            bool isValid = true;
            for (const QString & tag : searchTags)
            {
                if (!info.tags.contains(tag))
                {
                    isValid = false;
                    break;
                }
            }
            if (isValid)
            {
                m_foundDocsData.append(info);
            }
        }
    }
}

void DocTestTool::onSearchBackButtonClicked()
{
    viewSearchScreen(false);
    viewMainScreen(true);
}

void DocTestTool::OnListWidgetDoubleClicked(QListWidgetItem * item)
{
}
