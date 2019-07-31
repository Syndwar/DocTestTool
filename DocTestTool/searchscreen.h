#ifndef SEARCH_SCREEN_INFO_H
#define SEARCH_SCREEN_INFO_H

#include "screen.h"

struct SaveData;
struct DocInfo;

class SearchScreen : public Screen
{
private:
    QList<DocInfo> found_docs_data_; // files that are found using search filter
private:
    //
    void findComments();
    //
    void findTags();
    //
    void doGreedySearch();
    //
    void doStrictSearch();
    //
    void findName();
    //
    void save();
    //
    void deleteFromDocs();
public:
    //
    SearchScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save);
    //
    ~SearchScreen();
    //
    virtual bool isSearch() const override { return true; }
    //
    virtual void processUserEvent(Screen::UserEvent event) override;
};

#endif // SEARCH_SCREEN_INFO_H