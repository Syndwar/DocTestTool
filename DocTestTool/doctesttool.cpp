#include "doctesttool.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace
{
const char * const kBaseFolder = "base";
const char * const kDocsFolder = "docs";
const char * const kDefaultTagsFile = "config.json";
const char * const kInfoDocFile = "info.json";
const char * const kComment = "comment";
const char * const kFilename = "filename";
const char * const kTags = "tags";
const QString kUploadFilters = "Documents (*.pdf *.xml)";
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

    QObject::connect(ui.searchFindBtn, SIGNAL(clicked()), this, SLOT(onSearchFindButtonClicked()));

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
        const QString val = "{\"tags\":[], \"comment\":\"\"}";
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
    ui.listWidget->clear();

    for (QString & tag : m_defaultTags)
    {
        ui.listWidget->addItem(tag);
    }

    auto it = m_templates.constBegin();
    while (it != m_templates.constEnd())
    {
        ui.listWidget->addItem(it.key());
        ++it;
    }

    ui.searchTextEdit->clear();
    ui.searchFindBtn->setVisible(value);
    ui.searchTextEdit->setVisible(value);
    ui.listWidget->setVisible(value);
    ui.searchCancelBtn->setVisible(value);
}

void DocTestTool::viewUploadScreen(const bool value)
{
    if (value) m_viewMode = ViewMode::Upload;

    ui.listWidget->setVisible(value);
    ui.uploadOkBtn->setVisible(value);
    ui.uploadCancelBtn->setVisible(value);
    ui.uploadTagBtn->setVisible(value);
    ui.uploadCommentBtn->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.uploadAddBtn->setVisible(value);
    ui.uploadTagsTextEdit->setVisible(value);
    ui.uploadCommentsTextEdit->setVisible(value);
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
    for (DocInfo & info : m_loadedDocsData)
    {
        QFile file(info.filePath);
        QFileInfo fileInfo(file);

        QString folderPath(getDocsFilePath());
        folderPath.append("/").append(QString::number(++m_docsCount));
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
    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui.listWidget->removeItemWidget(item);
        delete item;
    }
}

void DocTestTool::OnUploadTagButtonClicked()
{
    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor("blue");
    }

    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            info.tags = ui.uploadTagsTextEdit->text().split(" ");
        }
    }
}

void DocTestTool::OnUploadCommentButtonClicked()
{
    QList<QListWidgetItem *> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor("green");
    }

    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            info.comment = ui.uploadCommentsTextEdit->text();
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
    if (ViewMode::Upload != m_viewMode) return;

    QModelIndexList indexes = ui.listWidget->selectionModel()->selectedIndexes();
    if (1 == indexes.size())
    {
        QModelIndex index = indexes[0];
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            const DocInfo & info = m_loadedDocsData[i];
            ui.uploadCommentsTextEdit->setText(info.comment);
            ui.uploadTagsTextEdit->setText(info.tags.join(" "));
        }
    }
}

//========================================
// Search screen
//========================================
void DocTestTool::onSearchFindButtonClicked()
{
    const QString findText = ui.searchTextEdit->text();
    if (!findText.isEmpty())
    {
        ui.searchTextEdit->clear();
    }
}

void DocTestTool::onSearchBackButtonClicked()
{
    viewSearchScreen(false);
    viewMainScreen(true);
}

void DocTestTool::OnListWidgetDoubleClicked(QListWidgetItem * item)
{
    if (ViewMode::Search != m_viewMode) return;

    const QString key = item->text();
    if (!key.isEmpty())
    {
        auto it = m_templates.find(key);
        if (it != m_templates.constEnd())
        {
            ui.searchTextEdit->setText(it.value().join(" "));
        }
        else
        {
            ui.searchTextEdit->setText(key);
        }
    }
}
