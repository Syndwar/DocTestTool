#ifndef UPLOAD_SCREEN_INFO_H
#define UPLOAD_SCREEN_INFO_H

#include "screen.h"

struct DocInfo;
struct SaveData;

class UploadScreen : public Screen
{
private:
    QList<DocInfo> loadedDocsData_;  // files that are loaded into application and are processed
private:
    //
    void setTags();
    //
    void setName();
    //
    void setComment();
    //
    void finishUpload();
    //
    void openSelectedDoc();
    //
    void addToDocs();
    //
    void deleteFromDocs();
    //
    void loadDocs(QStringList & fileNames);
public:
    //
    UploadScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    virtual ~UploadScreen();
    //
    virtual bool init() override;
    //
    virtual bool isUpload() const { return true; }
    //
    virtual void processUserEvent(Screen::UserEvent event) override;
};
#endif // UPLOAD_SCREEN_INFO_H