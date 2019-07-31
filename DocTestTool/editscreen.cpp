#include "editscreen.h"
#include "editortemplateitem.h"
#include "constants.h"
#include "savedata.h"

#include <QFile>

//=============================================================================
// class EditScreen
//=============================================================================
EditScreen::EditScreen(QWidget * parent, Ui::DocTestToolClass * ui, SaveData * save)
    : Screen(parent, ui, save)
{
    for (QString & tag : save_->default_tags)
    {
        ui->tagsListWidget->addItem(tag);
    }

    QListWidget * obj = ui_->docsListWidget;
    auto it = save_->templates.constBegin();
    while (it != save_->templates.constEnd())
    {
        EditorTemplateItem * item = new EditorTemplateItem(it.key());
        item->setTextColor("blue");
        item->setTags(it.value());
        obj->addItem(item);
        ++it;
    }

    ui->editorComboBox->setVisible(true);
    ui->deleteBtn->setVisible(true);
    ui->clearBtn->setVisible(true);
    ui->setTextBtn->setVisible(true);
    ui->inputTextEdit->setVisible(true);
    ui->tagsListWidget->setVisible(true);
    ui->docsListWidget->setVisible(true);
    ui->okBtn->setVisible(true);
    ui->backBtn->setVisible(true);
}

EditScreen::~EditScreen()
{
}

void EditScreen::processUserEvent(Screen::UserEvent event)
{
    switch (event)
    {
        case Screen::UserEvent::SetTextBtnClicked:
        {
            const QString text = ui_->editorComboBox->currentText();
            if (text == Constants::kTagsCombo)
            {
                addTags();
            }
            else if (text == Constants::kTemplatesCombo)
            {
                addTemplates();
            }
        }
        break;
        case Screen::UserEvent::OkBtnClicked:
        {
            finishEdit();
        }
        break;
        case Screen::UserEvent::EditComboBoxChanged:
        {
            ui_->inputTextEdit->clear();
            ui_->inputTextEdit2->clear();
            const QString text = ui_->editorComboBox->currentText();
            ui_->inputTextEdit2->setVisible(text == Constants::kTemplatesCombo);
        }
        break;
        case Screen::UserEvent::TagsListClicked:
        {
            ui_->docsListWidget->setCurrentItem(Q_NULLPTR);
        }
        break;
        case Screen::UserEvent::ClearBtnClicked:
        {
            clearWidgets(ClearMode::ClearInputText);
        }
        break;
        case Screen::UserEvent::DocsListClicked:
        {
            ui_->tagsListWidget->setCurrentItem(Q_NULLPTR);

            QModelIndexList indexes = ui_->docsListWidget->selectionModel()->selectedIndexes();
            if (indexes.size() == 1)
            {
                const int i = indexes[0].row();
                if (ui_->editorComboBox->currentText() == Constants::kTemplatesCombo)
                {
                    EditorTemplateItem * item = dynamic_cast<EditorTemplateItem *>(ui_->docsListWidget->item(i));
                    ui_->inputTextEdit->setText(item->text());
                    ui_->inputTextEdit2->setText(item->tags().join(Constants::kDelimiter));
                }
            }
        }
        break;
        case Screen::UserEvent::TagsListDoubleClicked:
        {
            const bool isTagsSelected = ui_->editorComboBox->currentText() == Constants::kTagsCombo;
            QList<QListWidgetItem *> selectedItems = ui_->tagsListWidget->selectedItems();
            const QString key = selectedItems.first()->text();
            if (!key.isEmpty())
            {
                auto it = save_->templates.find(key);
                if (it != save_->templates.constEnd())
                {
                    QString curText = it.value().join(Constants::kDelimiter);
                    if (isTagsSelected)
                    {
                        ui_->inputTextEdit->setText(curText);
                    }
                    else
                    {
                        ui_->inputTextEdit2->setText(curText);
                    }
                }
                else
                {
                    QString curText = isTagsSelected ? ui_->inputTextEdit->text() : ui_->inputTextEdit2->toPlainText();
                    curText = curText.isEmpty() ? key : (curText.append(Constants::kDelimiter).append(key));
                    if (isTagsSelected)
                    {
                        ui_->inputTextEdit->setText(curText);
                    }
                    else
                    {
                        ui_->inputTextEdit2->setText(curText);
                    }
                }
            }
        }
        break;
        case Screen::UserEvent::DeleteBtnClicked:
        {
            deleteTags();
        }
        break;
    }
}

void EditScreen::addTags()
{
    const QString text = ui_->inputTextEdit->text();
    const QString templatesText = ui_->inputTextEdit2->toPlainText();
    if (text.isEmpty())
    {
        ui_->statusBar->setStyleSheet("color: red");
        ui_->statusBar->showMessage("Tags are empty!", 2000);
        return;
    }

    QStringList tags = text.simplified().split(Constants::kDelimiter);

    for (int i = 0, iEnd = ui_->tagsListWidget->count(); i < iEnd; ++i)
    {
        QListWidgetItem * item = ui_->tagsListWidget->item(i);
        QString itemText = item->text();
        if (!tags.contains(itemText))
        {
            tags.append(itemText);
        }
    }
    tags.sort();
    ui_->tagsListWidget->clear();
    ui_->tagsListWidget->addItems(tags);
}

void EditScreen::addTemplates()
{
    const QString tagsText = ui_->inputTextEdit->text();
    const QString templatesText = ui_->inputTextEdit2->toPlainText();
    if (tagsText.isEmpty() || templatesText.isEmpty())
    {
        ui_->statusBar->setStyleSheet("color: red");
        ui_->statusBar->showMessage("Templates are empty!", 2000);
        return;
    }

    QStringList tags = tagsText.simplified().split(Constants::kDelimiter);
    tags.sort();
    QStringList templateTags = templatesText.simplified().split(Constants::kDelimiter);
    templateTags.sort();

    for (QString & tag : tags)
    {
        for (int i = 0, iEnd = ui_->docsListWidget->count(); i < iEnd; ++i)
        {
            QListWidgetItem * item = ui_->docsListWidget->item(i);
            if (tag == item->text())
            {
                ui_->docsListWidget->removeItemWidget(item);
                delete item;
                break;
            }
        }
    }

    for (QString & tag : tags)
    {
        EditorTemplateItem * item = new EditorTemplateItem(tag);
        item->setTags(templateTags);
        item->setTextColor("blue");
        ui_->docsListWidget->addItem(item);
    }
    ui_->docsListWidget->sortItems();
}

void EditScreen::deleteTags()
{
    QList<QListWidgetItem *> tags = ui_->tagsListWidget->selectedItems();
    for (QListWidgetItem * tag : tags)
    {
        ui_->tagsListWidget->removeItemWidget(tag);
        delete tag;
    }

    QList<QListWidgetItem *> tmpls = ui_->docsListWidget->selectedItems();
    for (QListWidgetItem * tmpl : tmpls)
    {
        ui_->docsListWidget->removeItemWidget(tmpl);
        delete tmpl;
    }
}

void EditScreen::finishEdit()
{
    save_->default_tags.clear();
    for (int i = 0, iEnd = ui_->tagsListWidget->count(); i < iEnd; ++i)
    {
        QListWidgetItem * item = ui_->tagsListWidget->item(i);
        save_->default_tags.append(item->text());
    }

    save_->templates.clear();
    for (int i = 0, iEnd = ui_->docsListWidget->count(); i < iEnd; ++i)
    {
        EditorTemplateItem * item = dynamic_cast<EditorTemplateItem *>(ui_->docsListWidget->item(i));
        save_->templates[item->text()] = item->tags();
    }

    QFile tagsFile(SaveData::getConfigFilePath());
    if (tagsFile.exists())
    {
        if (save_->exportTagsToFile(tagsFile))
        {
            save_->loadConfig();
            ui_->backBtn->click();
        }
        else
        {
            ui_->statusBar->setStyleSheet("color: red");
            ui_->statusBar->showMessage("Json is invalid!", 2000);
        }
    }
}