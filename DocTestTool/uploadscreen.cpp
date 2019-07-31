#include "uploadscreen.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "constants.h"
#include "docinfo.h"
#include "savedata.h"

//=============================================================================
// class UploadScreen
//=============================================================================
UploadScreen::UploadScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : Screen(parent, ui, save)
{
    for (QString & tag : save_->default_tags)
    {
        ui->tagsListWidget->addItem(tag);
    }

    auto it = save_->templates.constBegin();
    while (it != save_->templates.constEnd())
    {
        QListWidgetItem * item = new QListWidgetItem(it.key());
        item->setTextColor("blue");
        ui_->tagsListWidget->addItem(item);
        ++it;
    }

    ui->docsListWidget->setVisible(true);
    ui->okBtn->setVisible(true);
    ui->setTextBtn->setVisible(true);
    ui->deleteBtn->setVisible(true);
    ui->addBtn->setVisible(true);
    ui->inputTextEdit->setVisible(true);
    ui->tagsListWidget->setVisible(true);
    ui->clearBtn->setVisible(true);
    ui->backBtn->setVisible(true);
    ui->searchComboBox->setVisible(true);
    ui->tagsBrowser->setVisible(true);
    ui->commentBrowser->setVisible(true);
    ui->label->setVisible(true);
    ui->label_2->setVisible(true);
}

UploadScreen::~UploadScreen()
{
}

bool UploadScreen::init()
{
    loaded_docs_data_.clear();

    QStringList fileNames = QFileDialog::getOpenFileNames(parent_, "Select one or more files to open", QString(), Constants::kUploadFilters);

    loadDocs(fileNames);

    for (DocInfo & info : loaded_docs_data_)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui_->docsListWidget->addItem(newItem);
    }

    if (!fileNames.empty())
    {
        return true;
    }
    return false;
}

void UploadScreen::loadDocs(QStringList & fileNames)
{
    for (QString & fileName : fileNames)
    {
        bool duplicate = false;
        for (const DocInfo & info : loaded_docs_data_)
        {
            if (info.filePath == fileName)
            {
                duplicate = true;
                break;
            }
        }

        if (!duplicate)
        {
            QFile file(fileName);
            QFileInfo info(file);

            DocInfo docInfo;
            docInfo.filePath = fileName;
            docInfo.fileName = info.fileName();
            loaded_docs_data_.append(docInfo);
        }
    }
}

void UploadScreen::processUserEvent(Screen::UserEvent event)
{
    switch (event)
    {
        case Screen::UserEvent::SetTextBtnClicked:
        {
            const QString text = ui_->searchComboBox->currentText();
            if (text == Constants::kTagsCombo)
            {
                setTags();
            }
            else if (text == Constants::kCommentsCombo)
            {
                setComment();
            }
            else if (text == Constants::kName)
            {
                setName();
            }
        }
        break;
        case Screen::UserEvent::OkBtnClicked:
        {
            finishUpload();
        }
        break;
        case Screen::UserEvent::DocsListDoubleClicked:
        {
            openSelectedDoc();
        }
        break;
        case Screen::UserEvent::AddBtnClicked:
        {
            addToDocs();
        }
        break;
        case Screen::UserEvent::ClearBtnClicked:
        {
            clearWidgets(ClearMode::ClearInputText);
        }
        break;
        case Screen::UserEvent::TagsListDoubleClicked:
        {
            QList<QListWidgetItem *> selectedItems = ui_->tagsListWidget->selectedItems();
            const QString key = selectedItems.first()->text();
            if (!key.isEmpty())
            {
                auto it = save_->templates.find(key);
                if (it != save_->templates.constEnd())
                {
                    QString curText = it.value().join(Constants::kDelimiter);
                    ui_->inputTextEdit->setText(curText);
                }
                else
                {
                    QString curText = ui_->inputTextEdit->text();
                    curText = curText.isEmpty() ? key : (curText.append(Constants::kDelimiter).append(key));
                    ui_->inputTextEdit->setText(curText);
                }
            }
        }
        break;
        case Screen::UserEvent::DocsListClicked:
        {
            QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
            if (indexes.size() == 1)
            {
                const int i = indexes[0].row();
                if (i < loaded_docs_data_.size())
                {
                    const DocInfo & info = loaded_docs_data_[i];
                    ui_->commentBrowser->setText(info.comment);
                    ui_->tagsBrowser->setText(info.tags.join(Constants::kDelimiter));
                }
            }
        }
        break;
        case Screen::UserEvent::DeleteBtnClicked:
        {
            deleteFromDocs();
        }
        break;
    }
}

void UploadScreen::addToDocs()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(parent_, "Select one or more files to open", QString(), Constants::kUploadFilters);

    loadDocs(fileNames);

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : loaded_docs_data_)
    {
        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName);
        ui_->docsListWidget->addItem(newItem);
    }
}

void UploadScreen::deleteFromDocs()
{
    // get rows
    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    QList<int> indexList;
    for (QModelIndex & index : indexes)
    {
        indexList.append(index.row());
    }

    // sort in reverse order
    std::sort(indexList.begin(), indexList.end());
    std::reverse(indexList.begin(), indexList.end());

    QList<QListWidgetItem *> selectedWidgets = ui_->docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui_->docsListWidget->removeItemWidget(item);
        delete item;
    }

    for (int i : indexList)
    {
        loaded_docs_data_.removeAt(i);
    }
}

void UploadScreen::openSelectedDoc()
{
    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < loaded_docs_data_.size())
        {
            DocInfo & info = loaded_docs_data_[i];
            QDesktopServices::openUrl(QUrl(QString("file:///").append(info.filePath)));
            break;
        }
    }
}

void UploadScreen::setTags()
{
    const QString text = ui_->inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui_->docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "blue");
    }

    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < loaded_docs_data_.size())
        {
            DocInfo & info = loaded_docs_data_[i];
            if (text.isEmpty())
            {
                info.tags.clear();
            }
            else
            {
                info.tags = text.simplified().split(Constants::kDelimiter);
            }
        }
    }
}

void UploadScreen::setComment()
{
    const QString text = ui_->inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui_->docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setTextColor(text.isEmpty() ? "black" : "green");
    }

    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < loaded_docs_data_.size())
        {
            DocInfo & info = loaded_docs_data_[i];
            info.comment = text;
        }
    }
}

void UploadScreen::setName()
{
    const QString text = ui_->inputTextEdit->text();
    QList<QListWidgetItem *> selectedWidgets = ui_->docsListWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        item->setText(text);
    }

    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < loaded_docs_data_.size())
        {
            DocInfo & info = loaded_docs_data_[i];
            info.fileName = text;
        }
    }
}

void UploadScreen::finishUpload()
{
    if (loaded_docs_data_.isEmpty())
    {
        ui_->statusBar->setStyleSheet("color: red");
        ui_->statusBar->showMessage("Add documents!", 2000);
        return;
    }

    // check if all documents has tags
    int missingTagIndex = -1;
    for (int i = 0, iEnd = loaded_docs_data_.size(); i < iEnd; ++i)
    {
        DocInfo & info = loaded_docs_data_[i];
        if (info.tags.empty())
        {
            missingTagIndex = i;
            break;
        }
    }
    // if tag is missing than set color to red and scroll to this item
    if (missingTagIndex >= 0)
    {
        QListWidgetItem * item = ui_->docsListWidget->item(missingTagIndex);
        item->setTextColor("red");
        ui_->docsListWidget->scrollToItem(item);

        ui_->statusBar->setStyleSheet("color: red");
        ui_->statusBar->showMessage("Tag is missing!", 2000);
        return;
    }

    ui_->progressBar->setVisible(true);
    ui_->progressBar->setValue(0);
    ui_->progressBar->setMaximum(loaded_docs_data_.size());
    for (DocInfo & info : loaded_docs_data_)
    {
        QFile file(info.filePath);
        QFileInfo fileInfo(file);

        QString folderPath(SaveData::getDocsFilePath());
        ++save_->docs_count;
        folderPath.append("/").append(QString::number(save_->docs_count));
        if (!QDir(folderPath).exists())
        {
            QDir().mkdir(folderPath);
            const QString filepath = QDir(folderPath).filePath(info.fileName);
            file.copy(filepath);

            const QString infoPath = QDir(folderPath).filePath(Constants::kInfoDocFile);
            QFile infoFile(infoPath);
            const QString val = "{}";
            QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
            QJsonObject obj = jsonDoc.object();
            obj[Constants::kComment] = info.comment;
            obj[Constants::kFilename] = info.fileName;
            obj[Constants::kTags] = QJsonArray::fromStringList(info.tags);
            jsonDoc.setObject(obj);

            QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

            if (infoFile.open(QIODevice::WriteOnly))
            {
                infoFile.write(json);
                infoFile.close();
            }
        }
        ui_->progressBar->setValue(ui_->progressBar->value() + 1);
    }
    save_->loadFilesData();
    ui_->progressBar->setValue(ui_->progressBar->maximum());
    ui_->progressBar->setVisible(true);

    //switchToScreen(ScreenId::Main);
}