#include "loginscreen.h"

#include <QDir>

#include "savedata.h"

//=============================================================================
// class LoginScreen
//=============================================================================
LoginScreen::LoginScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : Screen(parent, ui, save)
{
    ui_->loginBtn->setVisible(true);
    ui_->loginTextEdit->setVisible(true);
}

LoginScreen::~LoginScreen()
{
}

void LoginScreen::processUserEvent(Screen::UserEvent event)
{
    switch (event)
    {
        case Screen::UserEvent::LoginButtonClicked:
        {
            login();
        }
    }
}

void LoginScreen::login()
{
    const QString folderPath = ui_->loginTextEdit->text();
    bool isFolderCreated = QDir().exists(folderPath);
    if (!isFolderCreated)
    {
        if (QDir().mkdir(folderPath))
        {
            isFolderCreated = true;
        }
        else
        {
            ui_->statusBar->setStyleSheet("color: red");
            ui_->statusBar->showMessage("Folder path is invalid", 2000);
        }
    }
    if (isFolderCreated)
    {
        save_->workingFolder = QDir(folderPath).absolutePath();

        if (!save_->prepareFolders())
        {
            ui_->statusBar->setStyleSheet("color: red");
            ui_->statusBar->showMessage("Json is invalid!", 2000);
        }

        save_->loadConfig();
        save_->loadFilesData();

        ui_->backBtn->click();
    }
}