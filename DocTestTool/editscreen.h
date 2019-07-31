#ifndef EDIT_SCREEN_INFO_H
#define EDIT_SCREEN_INFO_H

#include "screen.h"
#include "savedata.h"

class EditScreen : public Screen
{
private:
    //
    void addTags();
    //
    void addTemplates();
    //
    void deleteTags();
    //
    void finishEdit();
public:
    //
    EditScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    ~EditScreen();
    //
    virtual bool isEdit() const override { return true; }
    //
    virtual void processUserEvent(Screen::UserEvent event) override;
};
#endif // EDIT_SCREEN_INFO_H