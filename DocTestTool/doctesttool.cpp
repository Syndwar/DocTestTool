#include "doctesttool.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
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
const char * const kTemplates = "templates";
const QString kUploadFilters = "Documents (*.pdf *.tiff);;Images (*.jpg *.jpeg *.png);;Data Only (*.txt *json *html);;XML (*.xml);;All files (*.*)";
const char * const kDelimiter = ", ";
const QString kTagsCombo = "Tags";
const QString kCommentsCombo = "Comments";
const QString kName = "Name";
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

    QObject::connect(ui.uploadBtn, SIGNAL(clicked()), this, SLOT(onUploadButtonClicked()));
    QObject::connect(ui.editBtn, SIGNAL(clicked()), this, SLOT(onEditButtonClicked()));
    QObject::connect(ui.searchBtn, SIGNAL(clicked()), this, SLOT(onSearchButtonClicked()));

    QObject::connect(ui.backBtn, SIGNAL(clicked()), this, SLOT(onBackButtonClicked()));
    
    QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));

    QObject::connect(ui.setTextBtn, SIGNAL(clicked()), this, SLOT(onSetTextButtonClicked()));
    QObject::connect(ui.uploadDeleteBtn, SIGNAL(clicked()), this, SLOT(onUploadDeleteButtonClicked()));
    QObject::connect(ui.uploadAddBtn, SIGNAL(clicked()), this, SLOT(onUploadAddButtonClicked()));

    QObject::connect(ui.findBtn, SIGNAL(clicked()), this, SLOT(onFindButtonClicked()));
    QObject::connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));
    QObject::connect(ui.clearBtn, SIGNAL(clicked()), this, SLOT(onClearTagButtonClicked()));

    QObject::connect(ui.tagsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onTagsListDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.docsListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(onListWidgetClicked(QListWidgetItem *)));
    QObject::connect(ui.docsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem *)));

    ui.docsListWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    prepareFolders();
    loadConfig();
    loadFilesData();
}

bool DocTestTool::isUpload() const
{
    return ViewMode::Upload == m_viewMode;
}

bool DocTestTool::isMain() const
{
    return ViewMode::Main == m_viewMode;
}

bool DocTestTool::isEdit() const
{
    return ViewMode::Edit == m_viewMode;
}

bool DocTestTool::isSearch() const
{
    return ViewMode::Search == m_viewMode;
}

void DocTestTool::setMode(const ViewMode mode)
{
    m_viewMode = mode;
}

void DocTestTool::prepareFolders()
{
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
        exportTagsToFile(tagsFile);
    }
}

bool DocTestTool::exportTagsToFile(QFile & file)
{
    QString defaultTagsStr;
    if (!m_defaultTags.isEmpty())
    {
        defaultTagsStr.append("\"").append(m_defaultTags.join("\", \"")).append("\"");
    }
    QString templatesStr;
    auto it = m_templates.constBegin();
    while (it != m_templates.constEnd())
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
        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Json is invalid!", 2000);
        return false;
    }
    return true;
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
            QJsonValue templatesValue = obj[kTemplates];
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
        bool duplicate = false;
        for (const DocInfo & info : m_loadedDocsData)
        {
            if (info.filePath == fileName)
            {
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate)
        {
            QFile file(fileName);
            QFileInfo info(file);

            DocInfo docInfo;
            docInfo.filePath = fileName;
            docInfo.fileName = info.fileName();
            m_loadedDocsData.append(docInfo);
        }
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
    if (value)
    {
        setMode(ViewMode::Main);
        clearWidgets(ClearMode::ClearAll);
    }

    ui.uploadBtn->setVisible(value);
    ui.editBtn->setVisible(value);
    ui.searchBtn->setVisible(value);
    ui.progressBar->setVisible(false);
}

void DocTestTool::viewSearchScreen(const bool value)
{
    if (value)
    {
        setMode(ViewMode::Search);
        clearWidgets(ClearMode::ClearAll);
        
        m_foundDocsData.clear();
        addTagsToListWidget();
        addTemplatesToListWidget();
    }

    ui.findBtn->setVisible(value);
    ui.inputTextEdit->setVisible(value);
    ui.docsListWidget->setVisible(value);
    ui.saveBtn->setVisible(value);
    ui.tagsListWidget->setVisible(value);
    ui.clearBtn->setVisible(value);
    ui.fullMatchBox->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.backBtn->setVisible(value);
    ui.progressBar->setVisible(false);
    ui.searchComboBox->setVisible(value);
    ui.tagsBrowser->setVisible(value);
    ui.commentBrowser->setVisible(value);
    ui.label->setVisible(value);
    ui.label_2->setVisible(value);
}

void DocTestTool::viewUploadScreen(const bool value)
{
    if (value)
    {
        setMode(ViewMode::Upload);
        clearWidgets(ClearMode::ClearAll);
        
        addTagsToListWidget();
        addTemplatesToListWidget();
    }

    ui.docsListWidget->setVisible(value);
    ui.okBtn->setVisible(value);
    ui.setTextBtn->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.uploadAddBtn->setVisible(value);
    ui.inputTextEdit->setVisible(value);
    ui.tagsListWidget->setVisible(value);
    ui.clearBtn->setVisible(value);
    ui.backBtn->setVisible(value);
    ui.progressBar->setVisible(false);
    ui.searchComboBox->setVisible(value);
    ui.tagsBrowser->setVisible(value);
    ui.commentBrowser->setVisible(value);
    ui.label->setVisible(value);
    ui.label_2->setVisible(value);
}

void DocTestTool::viewEditScreen(const bool value)
{
    if (value)
    {
        setMode(ViewMode::Edit);
        clearWidgets(ClearMode::ClearAll);
     
        addTagsToListWidget();
    }

    ui.clearBtn->setVisible(value);
    ui.setTextBtn->setVisible(value);
    ui.inputTextEdit->setVisible(value);
    ui.tagsListWidget->setVisible(value);
    ui.docsListWidget->setVisible(value);
    ui.okBtn->setVisible(value);
    ui.backBtn->setVisible(value);
}

void DocTestTool::clearWidgets(ClearMode mode)
{
    if (mode & ClearStatusBar)
    {
        ui.statusBar->clearMessage();
    }
    if (mode & ClearInputText)
    {
        ui.inputTextEdit->clear();
    }
    if (mode & ClearTagsList)
    {
        ui.tagsListWidget->clear();
    }
    if (mode & ClearDocsList)
    {
        ui.docsListWidget->clear();
    }
    if (mode & ClearTagsBrowser)
    {
        ui.tagsBrowser->clear();
    }
    if (mode & ClearCommentBrowser)
    {
        ui.commentBrowser->clear();
    }
}

void DocTestTool::onBackButtonClicked()
{
    viewEditScreen(false);
    viewSearchScreen(false);
    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::addTagsToListWidget()
{
    for (QString & tag : m_defaultTags)
    {
        ui.tagsListWidget->addItem(tag);
    }
}

void DocTestTool::addTemplatesToListWidget()
{
    auto it = m_templates.constBegin();
    while (it != m_templates.constEnd())
    {
        QListWidgetItem * item = new QListWidgetItem(it.key());
        item->setTextColor("blue");
        ui.tagsListWidget->addItem(item);
        ++it;
    }
}

//========================================
// Main screen
//========================================
void DocTestTool::onSearchButtonClicked()
{
    viewMainScreen(false);
    viewSearchScreen(true);
}

void DocTestTool::onUploadButtonClicked()
{
    m_loadedDocsData.clear();

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select one or more files to open", QString(), kUploadFilters);

    loadDocsRepo(fileNames);

    for (DocInfo & info : m_loadedDocsData)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui.docsListWidget->addItem(newItem);
    }

    if (!fileNames.empty())
    {
        viewMainScreen(false);
        viewUploadScreen(true);
    }
}

void DocTestTool::onEditButtonClicked()
{
    viewMainScreen(false);
    viewEditScreen(true);
}

void DocTestTool::onOkButtonClicked()
{
   if (isEdit())
   {
       finishEdit();
   }
   else if (isUpload())
   {
       finishUpload();
   }
}

void DocTestTool::finishUpload()
{
    if (m_loadedDocsData.isEmpty())
    {
        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Add documents!", 2000);
        return;
    }

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
        QListWidgetItem * item = ui.docsListWidget->item(missingTagIndex);
        item->setTextColor("red");
        ui.docsListWidget->scrollToItem(item);

        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Tag is missing!", 2000);
        return;
    }

    ui.progressBar->setVisible(true);
    ui.progressBar->setValue(0);
    ui.progressBar->setMaximum(m_loadedDocsData.size());
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
            const QString filepath = QDir(folderPath).filePath(info.fileName);
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
        ui.progressBar->setValue(ui.progressBar->value() + 1);
    }
    loadFilesData();
    ui.progressBar->setValue(ui.progressBar->maximum());
    ui.progressBar->setVisible(true);

    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::finishEdit()
{
    QFile tagsFile(getConfigFilePath());
    if (tagsFile.exists())
    {
        if (exportTagsToFile(tagsFile))
        {
            loadConfig();
            viewMainScreen(true);
            viewEditScreen(false);
        }
    }
}

//========================================
// Upload screen
//========================================
void DocTestTool::onUploadAddButtonClicked()
{
    // TODO check for duplicates
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select one or more files to open", QString(), kUploadFilters);

    loadDocsRepo(fileNames);

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : m_loadedDocsData)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui.docsListWidget->addItem(newItem);
    }
}

void DocTestTool::onUploadDeleteButtonClicked()
{
    // get rows
    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
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
        if (isUpload())
        {
            m_loadedDocsData.removeAt(i);
        }
        else if (isSearch())
        {
            m_foundDocsData.removeAt(i);
        }
    }

    QList<QListWidgetItem *> selectedWidgets = ui.docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui.docsListWidget->removeItemWidget(item);
        delete item;
    }
}

void DocTestTool::onSetTextButtonClicked()
{
    if (isUpload())
    {
        const QString currentText = ui.searchComboBox->currentText();
        if (currentText == kTagsCombo)
        {
            setTags();
        }
        else if (currentText == kCommentsCombo)
        {
            setComment();
        }
        else if (currentText == kName)
        {
            setName();
        }
    }
    else if (isEdit())
    {
        addTags();
    }
}

void DocTestTool::setTags()
{
    const QString text = ui.inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui.docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "blue");
    }

    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
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
                info.tags = text.simplified().split(kDelimiter);
            }
        }
    }
}

void DocTestTool::setComment()
{
    const QString text = ui.inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui.docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "green");
    }

    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
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

void DocTestTool::setName()
{
    const QString text = ui.inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui.docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setText(text);
    }

    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            info.fileName = text;
        }
    }
}

void DocTestTool::addTags()
{
    const QString text = ui.inputTextEdit->text();
    QStringList tags = text.simplified().split(kDelimiter);
    for (QString & tag : tags)
    {
        if (!m_defaultTags.contains(tag))
        {
            m_defaultTags.append(tag);
        }
    }
    clearWidgets(ClearMode::ClearTagsList);
    addTagsToListWidget();
}

void DocTestTool::onListWidgetClicked(QListWidgetItem * item)
{
    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
    if (indexes.size() > 1) return;

    const int i = indexes[0].row();

    if (isUpload())
    {
        if (i < m_loadedDocsData.size())
        {
            const DocInfo & info = m_loadedDocsData[i];
            ui.commentBrowser->setText(info.comment);
            ui.tagsBrowser->setText(info.tags.join(kDelimiter));
        }
    }
    else if (isSearch())
    {
        if (i < m_foundDocsData.size())
        {
            const DocInfo & info = m_foundDocsData[i];
            ui.commentBrowser->setText(info.comment);
            ui.tagsBrowser->setText(info.tags.join(kDelimiter));
        }
    }
}

//========================================
// Search screen
//========================================
void DocTestTool::onClearTagButtonClicked()
{
    clearWidgets(ClearMode::ClearInputText);
}

void DocTestTool::onSaveButtonClicked()
{
    if (m_foundDocsData.size() == 0)
    {
        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("No files to save!", 2000);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), tr("documents.zip"), tr("Zip *.zip"));
    if (!fileName.isEmpty())
    {
        ui.progressBar->setVisible(true);
        ui.progressBar->setValue(0);

        QString delimiter = ui.actionSingleFolder->isChecked() ? "-" : "/";
        char c;
        if (!fileName.endsWith(".zip"))
        {
            fileName.append(".zip");
        }
        QuaZip zip(fileName);
        if (zip.open(QuaZip::mdCreate))
        {
            int i = 0;
            ui.progressBar->setMaximum(m_foundDocsData.size());
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
                ui.progressBar->setValue(ui.progressBar->value() + 1);
            }
            zip.close();
        }
        ui.progressBar->setValue(ui.progressBar->maximum());
        ui.progressBar->setVisible(false);
    }
}

void DocTestTool::onTagsListDoubleClicked(QListWidgetItem * item)
{
    if (!isSearch() && !isUpload()) return;
    
    QLineEdit * editText = ui.inputTextEdit;

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

void DocTestTool::onFindButtonClicked()
{
    const QString currentText = ui.searchComboBox->currentText();
    if (currentText == kTagsCombo)
    {
        findTags();
    }
    else if (currentText == kCommentsCombo)
    {
        findComments();
    }
    else if (currentText == kName)
    {
        findName();
    }
}

void DocTestTool::findTags()
{
    m_foundDocsData.clear();

    if (ui.fullMatchBox->isChecked())
    {
        doStrictSearch();
    }
    else
    {
        doGreedySearch();
    }

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : m_foundDocsData)
    {
        ui.docsListWidget->addItem(info.fileName);
    }
}

void DocTestTool::doGreedySearch()
{
    const QString findText = ui.inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(kDelimiter);

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
    const QString findText = ui.inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(kDelimiter);

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

void DocTestTool::findComments()
{
    m_foundDocsData.clear();

    const QString findText = ui.inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(kDelimiter);

        for (DocInfo & info : m_folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.comment.contains(tag))
                {
                    m_foundDocsData.append(info);
                    break;
                }
            }
        }
    }

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : m_foundDocsData)
    {
        ui.docsListWidget->addItem(info.fileName);
    }
}

void DocTestTool::findName()
{
    m_foundDocsData.clear();

    const QString findText = ui.inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(kDelimiter);

        for (DocInfo & info : m_folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.fileName.contains(tag))
                {
                    m_foundDocsData.append(info);
                    break;
                }
            }
        }
    }
    
    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : m_foundDocsData)
    {
        ui.docsListWidget->addItem(info.fileName);
    }
}

void DocTestTool::onListWidgetDoubleClicked(QListWidgetItem * item)
{
    QModelIndexList indexes = ui.docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < m_loadedDocsData.size())
        {
            DocInfo & info = m_loadedDocsData[i];
            QDesktopServices::openUrl(QUrl(QString("file:///").append(info.filePath)));
            break;
        }
    }
}
