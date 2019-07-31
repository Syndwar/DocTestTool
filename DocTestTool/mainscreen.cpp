#include "mainscreen.h"

//=============================================================================
// class MainScreen
//=============================================================================
MainScreen::MainScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : Screen(parent, ui, save)
{
    ui->uploadBtn->setVisible(true);
    ui->editBtn->setVisible(true);
    ui->searchBtn->setVisible(true);
}

MainScreen::~MainScreen()
{
}

void MainScreen::processUserEvent(Screen::UserEvent event)
{
}