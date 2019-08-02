#include "searchscreen.h"

#include <QFileDialog>
#include <QDesktopServices>
#include "quazip.h"
#include "quazipfile.h"
#include "quazipnewinfo.h"

#include "constants.h"
#include "docinfo.h"
#include "savedata.h"

//=============================================================================
// class SearchScreen
//=============================================================================
SearchScreen::SearchScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : Screen(parent, ui, save)
{
    foundDocsData_.clear();

    for (QString & tag : save_->defaultTags)
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

    ui->findBtn->setVisible(true);
    ui->inputTextEdit->setVisible(true);
    ui->docsListWidget->setVisible(true);
    ui->saveBtn->setVisible(true);
    ui->tagsListWidget->setVisible(true);
    ui->clearBtn->setVisible(true);
    ui->fullMatchBox->setVisible(true);
    ui->deleteBtn->setVisible(true);
    ui->backBtn->setVisible(true);
    ui->searchComboBox->setVisible(true);
    ui->tagsBrowser->setVisible(true);
    ui->commentBrowser->setVisible(true);
    ui->label->setVisible(true);
    ui->label_2->setVisible(true);
}

SearchScreen::~SearchScreen()
{
}

void SearchScreen::processUserEvent(Screen::UserEvent event)
{
    switch (event)
    {
        case Screen::UserEvent::FindBtnClicked:
        {
            const QString currentText = ui_->searchComboBox->currentText();
            if (currentText == Constants::kTagsCombo)
            {
                findTags();
            }
            else if (currentText == Constants::kCommentsCombo)
            {
                findComments();
            }
            else if (currentText == Constants::kName)
            {
                findName();
            }
        }
        break;
        case Screen::UserEvent::SaveBtnClicked:
        {
            save();
        }
        break;
        case Screen::UserEvent::ClearBtnClicked:
        {
            clearWidgets(ClearMode::ClearInputText);
        }
        break;
        case Screen::UserEvent::DocsListClicked:
        {
            QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
            if (indexes.size() == 1)
            {
                const int i = indexes[0].row();
                if (i < foundDocsData_.size())
                {
                    const DocInfo & info = foundDocsData_[i];
                    ui_->commentBrowser->setText(info.comment);
                    ui_->tagsBrowser->setText(info.tags.join(Constants::kDelimiter));
                }
            }
        }
        break;
        case Screen::UserEvent::DocsListDoubleClicked:
        {
            openSelectedDoc();
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
        case Screen::UserEvent::DeleteBtnClicked:
        {
            if (ui_->actionDelete_From_Disk->isChecked())
            {
                deleteFromDisk();
                deleteFromDocs();
                save_->loadFilesData();
            }
            else
            {
               deleteFromDocs();
            }
        }
        break;
    }
}

void SearchScreen::openSelectedDoc()
{
    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < foundDocsData_.size())
        {
            DocInfo & info = foundDocsData_[i];
            QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath));
            break;
        }
    }
}

void SearchScreen::deleteFromDisk()
{
    // get rows
    QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();

    for (QModelIndex & index : indexes)
    {
        const int i = index.row();
        if (i < foundDocsData_.size())
        {
            DocInfo & docInfo = foundDocsData_[i];
            QFile file(docInfo.filePath);
            QFileInfo fileInfo(file);
            QString path = fileInfo.path();
            QDir fileDir(path);
            fileDir.removeRecursively();
        }
    }
}

void SearchScreen::deleteFromDocs()
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
        foundDocsData_.removeAt(i);
    }
}

void SearchScreen::save()
{
    if (foundDocsData_.size() == 0)
    {
        ui_->statusBar->setStyleSheet("color: red");
        ui_->statusBar->showMessage("No files to save!", 2000);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(parent_, "Save File", "documents.zip", "Zip *.zip");
    if (!fileName.isEmpty())
    {
        ui_->progressBar->setVisible(true);
        ui_->progressBar->setValue(0);

        QString delimiter = ui_->actionSingleFolder->isChecked() ? "-" : "/";
        char c;
        if (!fileName.endsWith(".zip"))
        {
            fileName.append(".zip");
        }
        QuaZip zip(fileName);
        if (zip.open(QuaZip::mdCreate))
        {
            int i = 0;
            ui_->progressBar->setMaximum(foundDocsData_.size());
            for (DocInfo & info : foundDocsData_)
            {
                QFile inFile(info.filePath);
                if (inFile.open(QIODevice::ReadOnly))
                {
                    const QString zipPath = QString::number(++i).append(delimiter).append(info.fileName);
                    QuaZipNewInfo zipInfo(zipPath, info.filePath);
                    QuaZipFile zipFile(&zip);
                    if (zipFile.open(QIODevice::WriteOnly, zipInfo))
                    {
                        while (inFile.getChar(&c) && zipFile.putChar(c));
                        zipFile.close();
                    }
                    inFile.close();
                }
                ui_->progressBar->setValue(ui_->progressBar->value() + 1);
            }
            zip.close();
        }
        ui_->progressBar->setValue(ui_->progressBar->maximum());
        ui_->progressBar->setVisible(false);
    }
}

void SearchScreen::findTags()
{
    foundDocsData_.clear();

    if (ui_->fullMatchBox->isChecked())
    {
        doStrictSearch();
    }
    else
    {
        doGreedySearch();
    }

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : foundDocsData_)
    {
        ui_->docsListWidget->addItem(info.fileName);
    }
}

void SearchScreen::doGreedySearch()
{
    const QString findText = ui_->inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(Constants::kDelimiter);

        for (DocInfo & info : save_->folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.tags.contains(tag))
                {
                    foundDocsData_.append(info);
                    break;
                }
            }
        }
    }
}

void SearchScreen::doStrictSearch()
{
    const QString findText = ui_->inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(Constants::kDelimiter);

        for (DocInfo & info : save_->folderDocsData)
        {
            bool isValid = true;
            for (const QString & tag : searchTags)
            {
                if (!info.tags.contains(tag))
                {
                    isValid = false;
                    break;
                }
            }
            if (isValid)
            {
                foundDocsData_.append(info);
            }
        }
    }
}

void SearchScreen::findComments()
{
    foundDocsData_.clear();

    const QString findText = ui_->inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(Constants::kDelimiter);

        for (DocInfo & info : save_->folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.comment.contains(tag))
                {
                    foundDocsData_.append(info);
                    break;
                }
            }
        }
    }

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : foundDocsData_)
    {
        ui_->docsListWidget->addItem(info.fileName);
    }
}

void SearchScreen::findName()
{
    foundDocsData_.clear();

    const QString findText = ui_->inputTextEdit->text();
    if (!findText.isEmpty())
    {
        const QStringList searchTags = findText.simplified().split(Constants::kDelimiter);

        for (DocInfo & info : save_->folderDocsData)
        {
            for (const QString & tag : searchTags)
            {
                if (info.fileName.contains(tag))
                {
                    foundDocsData_.append(info);
                    break;
                }
            }
        }
    }

    clearWidgets(ClearMode::ClearDocsList);
    for (DocInfo & info : foundDocsData_)
    {
        ui_->docsListWidget->addItem(info.fileName);
    }
}