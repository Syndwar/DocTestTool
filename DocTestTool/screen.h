#ifndef SCREEN_INFO_H
#define SCREEN_INFO_H

#include <QFile>

#include "ui_doctesttool.h"

struct SaveData;

class Screen
{
public:
    enum class UserEvent
    {
        UploadBtnClicked = 0,
        SearchBtnClicked,
        EditBtnClicked,
        BackBtnClicked,
        OkBtnClicked,
        SetTextBtnClicked,
        DeleteBtnClicked,
        AddBtnClicked,
        FindBtnClicked,
        SaveBtnClicked,
        ClearBtnClicked,
        TagsListClicked,
        TagsListDoubleClicked,
        DocsListClicked,
        DocsListDoubleClicked,
        EditComboBoxChanged,
    };
protected:
    enum ClearMode
    {
        ClearStatusBar = 0x00000001,
        ClearCommentBrowser = 0x0000002,
        ClearTagsBrowser = 0x00000004,
        ClearDocsList = 0x00000008,
        ClearTagsList = 0x00000010,
        ClearInputText = 0x0000020,
        ClearAll = 0xFF,
    };
protected:
    Ui::DocTestToolClass * ui_;
    QWidget * parent_;
    SaveData * save_;
protected:
    //
    void clearWidgets(ClearMode mode);
public:
    //
    Screen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    ~Screen();
    //
    virtual bool isMain() const { return false; }
    //
    virtual bool isSearch() const { return false; }
    //
    virtual bool isUpload() const { return false; }
    //
    virtual bool isEdit() const { return false; }
    //
    virtual void processUserEvent(Screen::UserEvent event);
    //
    virtual bool init() { return false; };
};

#endif // SCREEN_INFO_H