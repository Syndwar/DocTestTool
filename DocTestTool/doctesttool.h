#ifndef DOCTESTTOOL_H
#define DOCTESTTOOL_H

#include <QtWidgets/QMainWindow>
#include "ui_doctesttool.h"

struct DocInfo;

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

    QStringList m_defaultTags; // list of default tags
    QMap<QString, QStringList> m_templates; // templates with tag lists
    int m_docsCount; // amount of files found in the folder
    QList<DocInfo> m_loadedDocsData;  // files that are loaded into application and are processed
    QList<DocInfo> m_folderDocsData; // files that are stored in the app folder
    QList<DocInfo> m_foundDocsData; // files that are found using search filter

public:
    DocTestTool(QWidget *parent = 0);
    
    ~DocTestTool();

    void viewMainScreen(const bool value);
    void viewUploadScreen(const bool value);
    void viewSearchScreen(const bool value);
    void viewEditScreen(const bool value);
    void prepareFolders();
    void loadConfig();
    void loadFilesData();
    void loadDocsRepo(QStringList & fileNames);
    void doGreedySearch();
    void doStrictSearch();
    void updateTagsListWidget();
    void findComments();
    void findName();
    void findTags();
    void setTags();
    void setName();
    void setComment();
public slots:
    void onEditButtonClicked();
    void onUploadButtonClicked();
    void onSearchButtonClicked();
    void onEditSaveButtonClicked();
    void onBackButtonClicked();
    void onUploadOkButtonClicked();
    void onSetTextButtonClicked();
    void onUploadDeleteButtonClicked();
    void onUploadAddButtonClicked();
    void onSaveButtonClicked();
    void onClearTagButtonClicked();
    void onFindButtonClicked();
    void onTagsListDoubleClicked(QListWidgetItem * item);
    void onListWidgetClicked(QListWidgetItem * item);
    void onListWidgetDoubleClicked(QListWidgetItem * item);

private:
    Ui::DocTestToolClass ui;
};

#endif // DOCTESTTOOL_H
