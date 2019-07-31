#ifndef DOCTESTTOOL_H
#define DOCTESTTOOL_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>
#include "ui_doctesttool.h"
#include "savedata.h"

class Screen;
struct DocInfo;

class DocTestTool : public QMainWindow
{
    Q_OBJECT
private:

    enum class ScreenId
    {
        Main = 0,
        Search,
        Upload,
        Edit,
    };

    Screen * screen_;
    SaveData save_data_;

public:
    DocTestTool(QWidget * parent = Q_NULLPTR);
    
    ~DocTestTool();

    void switchToScreen(ScreenId id);
    void prepareFolders();
public slots:
    void onEditButtonClicked();
    void onUploadButtonClicked();
    void onSearchButtonClicked();
    void onBackButtonClicked();
    void onOkButtonClicked();
    void onSetTextButtonClicked();
    void onDeleteButtonClicked();
    void onAddButtonClicked();
    void onSaveButtonClicked();
    void onClearTagButtonClicked();
    void onFindButtonClicked();
    void onTagsListDoubleClicked(QListWidgetItem * item);
    void onTagsListClicked(QListWidgetItem * item);
    void onListWidgetClicked(QListWidgetItem * item);
    void onListWidgetDoubleClicked(QListWidgetItem * item);
    void onEditorComboBoxChanged(const QString & text);

private:
    Ui::DocTestToolClass ui;
};

#endif // DOCTESTTOOL_H
