#ifndef MAIN_SCREEN_INFO_H
#define MAIN_SCREEN_INFO_H

#include "screen.h"

struct DocInfo;
struct SaveData;

class MainScreen : public Screen
{
public:
    //
    MainScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    ~MainScreen();
    //
    virtual bool isMain() const override { return true; }
    //
    virtual void processUserEvent(Screen::UserEvent event) override;
};

#endif // MAIN_SCREEN_INFO_H