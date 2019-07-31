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

#include "constants.h"
#include "docinfo.h"

#include "mainscreen.h"
#include "editscreen.h"
#include "uploadscreen.h"
#include "searchscreen.h"

//=============================================================================
// class DocTestTool
//=============================================================================
DocTestTool::DocTestTool(QWidget * parent)
    : QMainWindow(parent)
    , screen_(Q_NULLPTR)
{
    ui.setupUi(this);

    QObject::connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QObject::connect(ui.uploadBtn, SIGNAL(clicked()), this, SLOT(onUploadButtonClicked()));
    QObject::connect(ui.editBtn, SIGNAL(clicked()), this, SLOT(onEditButtonClicked()));
    QObject::connect(ui.searchBtn, SIGNAL(clicked()), this, SLOT(onSearchButtonClicked()));

    QObject::connect(ui.backBtn, SIGNAL(clicked()), this, SLOT(onBackButtonClicked()));
    
    QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));

    QObject::connect(ui.setTextBtn, SIGNAL(clicked()), this, SLOT(onSetTextButtonClicked()));
    QObject::connect(ui.deleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteButtonClicked()));
    QObject::connect(ui.addBtn, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));

    QObject::connect(ui.findBtn, SIGNAL(clicked()), this, SLOT(onFindButtonClicked()));
    QObject::connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));
    QObject::connect(ui.clearBtn, SIGNAL(clicked()), this, SLOT(onClearTagButtonClicked()));

    QObject::connect(ui.tagsListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(onTagsListClicked(QListWidgetItem *)));
    QObject::connect(ui.tagsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onTagsListDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.docsListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(onListWidgetClicked(QListWidgetItem *)));
    QObject::connect(ui.docsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onListWidgetDoubleClicked(QListWidgetItem *)));

    QObject::connect(ui.editorComboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(onEditorComboBoxChanged(const QString &)));

    ui.docsListWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);


    prepareFolders();
    save_data_.loadConfig();
    save_data_.loadFilesData();

    switchToScreen(ScreenId::Main);
}

DocTestTool::~DocTestTool()
{
    if (screen_)
    {
        delete screen_;
    }
}

void DocTestTool::onSearchButtonClicked()
{
    switchToScreen(ScreenId::Search);
}

void DocTestTool::onEditButtonClicked()
{
    switchToScreen(ScreenId::Edit);
}

void DocTestTool::onDeleteButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::DeleteBtnClicked);
    }
}

void DocTestTool::onFindButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::FindBtnClicked);
    }
}

void DocTestTool::onBackButtonClicked()
{
    switchToScreen(ScreenId::Main);
}

void DocTestTool::onOkButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::OkBtnClicked);
    }
}

void DocTestTool::onTagsListClicked(QListWidgetItem * item)
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::TagsListClicked);
    }
}

void DocTestTool::onEditorComboBoxChanged(const QString & text)
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::EditComboBoxChanged);
    }
}

void DocTestTool::onSetTextButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::SetTextBtnClicked);
    }
}

void DocTestTool::onListWidgetDoubleClicked(QListWidgetItem * item)
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::DocsListDoubleClicked);
    }
}

void DocTestTool::onSaveButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::SaveBtnClicked);
    }
}

void DocTestTool::onUploadButtonClicked()
{
    switchToScreen(ScreenId::Upload);
    if (!screen_ || !screen_->init())
    {
        switchToScreen(ScreenId::Main);
    }
}

void DocTestTool::onAddButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::AddBtnClicked);
    }
}

void DocTestTool::onClearTagButtonClicked()
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::ClearBtnClicked);
    }
}

void DocTestTool::onListWidgetClicked(QListWidgetItem * item)
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::DocsListClicked);
    }
}

void DocTestTool::onTagsListDoubleClicked(QListWidgetItem * item)
{
    if (screen_)
    {
        screen_->processUserEvent(Screen::UserEvent::TagsListDoubleClicked);
    }
}

void DocTestTool::switchToScreen(ScreenId id)
{
    if (screen_)
    {
        delete screen_;
    }
    switch (id)
    {
        case ScreenId::Main:
        {
            screen_ = new MainScreen(this, &ui, &save_data_);
        }
        break;
        case ScreenId::Search:
        {
            screen_ = new SearchScreen(this, &ui, &save_data_);
        }
        break;
        case ScreenId::Upload:
        {
            screen_ = new UploadScreen(this, &ui, &save_data_);
        }
        break;
        case ScreenId::Edit:
        {
            screen_ = new EditScreen(this, &ui, &save_data_);
        }
        break;
    }
}

void DocTestTool::prepareFolders()
{
    // create base folder
    if (!QDir(Constants::kBaseFolder).exists())
    {
        QDir().mkdir(Constants::kBaseFolder);
    }

    if (!QDir(SaveData::getDocsFilePath()).exists())
    {
        QDir().mkdir(SaveData::getDocsFilePath());
    }

    // create file for default tags
    QFile tagsFile(SaveData::getConfigFilePath());
    if (!tagsFile.exists() && screen_)
    {
        if (!save_data_.exportTagsToFile(tagsFile))
        {
            ui.statusBar->setStyleSheet("color: red");
            ui.statusBar->showMessage("Json is invalid!", 2000);
        }
    }
}
