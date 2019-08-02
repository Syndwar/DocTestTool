#ifndef LOGIN_SCREEN_INFO_H
#define LOGIN_SCREEN_INFO_H

#include "screen.h"

class LoginScreen : public Screen
{
private:
    void login();
public:
    //
    LoginScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    ~LoginScreen();
    //
    virtual void processUserEvent(Screen::UserEvent event) override;
};
#endif // LOGIN_SCREEN_INFO_H