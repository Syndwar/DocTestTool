#ifndef DOCTESTTOOL_H
#define DOCTESTTOOL_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>
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
    void addTagsToListWidget();
    void addTemplatesTo(QListWidget * obj);
    void findComments();
    void findName();
    void findTags();
    void setTags();
    void setName();
    void addTags();
    void setComment();
    void finishEdit();
    void finishUpload();
    bool isUpload() const;
    bool isMain() const;
    bool isEdit() const;
    bool isSearch() const;
    void setMode(const ViewMode mode);
    void clearWidgets(ClearMode mode);
    bool exportTagsToFile(QFile & file);
    void doDeleteTags();
    void doDeleteDocsFrom(QList<DocInfo> & list);
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
    void onListWidgetClicked(QListWidgetItem * item);
    void onListWidgetDoubleClicked(QListWidgetItem * item);

private:
    Ui::DocTestToolClass ui;
};

#endif // DOCTESTTOOL_H
