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
const QStringList kUploadFilters = { "Pdf files (*.pdf)", "XML files (*.xml)" ,"Image files (*.png *.xpm *.jpg)", "Text files (*.txt)", "Any files (*)" };
}

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
        const QString val = "{}";
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
            QJsonValue tagsValue = obj["tags"];
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
    QDir docsDir(getDocsFilePath());
    QFileInfoList infoList = docsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    m_docsCount = infoList.size();
    for (int i = 0; i < m_docsCount; ++i)
    {
        QFileInfo & info = infoList[i];
        QString path = info.absoluteFilePath();
        // TODO create files data in the application
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
    QFileDialog dialog(this);
    dialog.setNameFilters(kUploadFilters);
    QStringList fileNames = dialog.getOpenFileNames();

    for (QString & fileName : fileNames)
    {
        QFile file(fileName);
        QFileInfo info(file);

        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName());
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
    for (int i = 0, iEnd = ui.listWidget->count(); i < iEnd; ++i)
    {
        QListWidgetItem * widget = ui.listWidget->item(i);
        QFile file(widget->text());
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

void DocTestTool::OnUploadCommentButtonClicked()
{
}

void DocTestTool::OnUploadAddButtonClicked()
{
    // TODO check for duplicates
    QFileDialog dialog(this);
    dialog.setNameFilters(kUploadFilters);
    QStringList fileNames = dialog.getOpenFileNames();
    if (!fileNames.isEmpty())
    {
        ui.listWidget->addItems(fileNames);
    }
}

void DocTestTool::OnUploadDeleteButtonClicked()
{
    QList<QListWidgetItem*> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui.listWidget->removeItemWidget(item);
        delete item;
    }
}

void DocTestTool::OnUploadTagButtonClicked()
{
}

void DocTestTool::OnUploadCancelButtonClicked()
{
    ui.listWidget->clear();
    viewUploadScreen(false);
    viewMainScreen(true);
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
    if (ViewMode::Search == m_viewMode)
    {
        const QString key = item->text();
        if (!key.isEmpty())
        {
            ui.searchTextEdit->setText(m_templates[key].join(" "));
        }
    }
}
