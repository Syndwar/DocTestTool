#include "screen.h"

#include "savedata.h"

//=============================================================================
// class Screen
//=============================================================================
Screen::Screen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : parent_(parent)
    , ui_(ui)
    , save_(save)
{
    clearWidgets(ClearMode::ClearAll);

    ui_->loginBtn->setVisible(false);
    ui_->loginTextEdit->setVisible(false);
    ui_->uploadBtn->setVisible(false);
    ui_->editBtn->setVisible(false);
    ui_->searchBtn->setVisible(false);
    ui_->progressBar->setVisible(false);
    ui_->findBtn->setVisible(false);
    ui_->inputTextEdit->setVisible(false);
    ui_->inputTextEdit2->setVisible(false);
    ui_->docsListWidget->setVisible(false);
    ui_->saveBtn->setVisible(false);
    ui_->tagsListWidget->setVisible(false);
    ui_->clearBtn->setVisible(false);
    ui_->fullMatchBox->setVisible(false);
    ui_->deleteBtn->setVisible(false);
    ui_->backBtn->setVisible(false);
    ui_->searchComboBox->setVisible(false);
    ui_->tagsBrowser->setVisible(false);
    ui_->commentBrowser->setVisible(false);
    ui_->tagsLbl->setVisible(false);
    ui_->commentLbl->setVisible(false);
    ui_->okBtn->setVisible(false);
    ui_->setTextBtn->setVisible(false);
    ui_->addBtn->setVisible(false);
    ui_->editorComboBox->setVisible(false);
    ui_->editorComboBox->setCurrentIndex(0);
}

Screen::~Screen()
{
}

void Screen::clearWidgets(ClearMode mode)
{
    if (mode & ClearStatusBar)
    {
        ui_->statusBar->clearMessage();
    }
    if (mode & ClearInputText)
    {
        ui_->inputTextEdit->clear();
        ui_->inputTextEdit2->clear();
    }
    if (mode & ClearTagsList)
    {
        ui_->tagsListWidget->clear();
    }
    if (mode & ClearDocsList)
    {
        ui_->docsListWidget->clear();
    }
    if (mode & ClearTagsBrowser)
    {
        ui_->tagsBrowser->clear();
    }
    if (mode & ClearCommentBrowser)
    {
        ui_->commentBrowser->clear();
    }
}

void Screen::processUserEvent(UserEvent event)
{
}