#ifndef DOCTESTTOOL_H
#define DOCTESTTOOL_H

#include <QtWidgets/QMainWindow>
#include "ui_doctesttool.h"

class DocTestTool : public QMainWindow
{
    Q_OBJECT
private:
    enum class ViewMode
    {
        Main = 0,
        Upload,
        Edit,
        Search
    };

    ViewMode m_viewMode;

    QStringList m_defaultTags;
    QMap<QString, QStringList> m_templates;
    int m_docsCount;

public:
    DocTestTool(QWidget *parent = 0);
    
    ~DocTestTool();

    void viewMainScreen(const bool value);
    void viewUploadScreen(const bool value);
    void viewSearchScreen(const bool value);
    void viewEditScreen(const bool value);
    void loadConfig();
    void loadFilesData();
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
    void onSearchFindButtonClicked();
    void onSearchBackButtonClicked();
    void OnListWidgetDoubleClicked(QListWidgetItem * item);

private:
    Ui::DocTestToolClass ui;
};

#endif // DOCTESTTOOL_H
