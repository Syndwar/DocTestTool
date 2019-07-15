#ifndef DOCTESTTOOL_H
#define DOCTESTTOOL_H

#include <QtWidgets/QMainWindow>
#include "ui_doctesttool.h"

class DocTestTool : public QMainWindow
{
    Q_OBJECT

public:
    DocTestTool(QWidget *parent = 0);
    
    ~DocTestTool();

    void viewMainScreen(const bool value);
    void viewUploadScreen(const bool value);
    void viewSearchScreen(const bool value);
    void viewEditScreen(const bool value);
public slots:
    void OnEditButtonClicked();
    void OnUploadButtonClicked();
    void OnSearchButtonClicked();
    void OnEditSaveButtonClicked();
    void OnEditCancelButtonClicked();
    void OnUploadOkButtonClicked();
    void OnUploadCancelButtonClicked();
    void OnUploadTagButtonClicked();
    void OnUploadDeleteButtonClicked();
    void OnUploadCommentButtonClicked();
    void OnUploadAddButtonClicked();

private:
    Ui::DocTestToolClass ui;
};

#endif // DOCTESTTOOL_H
