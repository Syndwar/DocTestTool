#include "doctesttool.h"

#include <QFileDialog>
#include <QJsonDocument>

namespace
{
const char * const kBaseFolder = "base";
const char * const kDefaultTagsFile = "config.json";
}

DocTestTool::DocTestTool(QWidget * parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    viewMainScreen(true);
    viewUploadScreen(false);
    viewEditScreen(false);
    viewSearchScreen(false);

    QObject::connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QObject::connect(ui.uploadBtn, SIGNAL(clicked()), this, SLOT(OnUploadButtonClicked()));
    QObject::connect(ui.editBtn, SIGNAL(clicked()), this, SLOT(OnEditButtonClicked()));
    QObject::connect(ui.searchBtn, SIGNAL(clicked()), this, SLOT(OnSearchButtonClicked()));

    QObject::connect(ui.editCancelBtn, SIGNAL(clicked()), this, SLOT(OnEditCancelButtonClicked()));
    QObject::connect(ui.editSaveBtn, SIGNAL(clicked()), this, SLOT(OnEditSaveButtonClicked()));
    
    QObject::connect(ui.uploadCancelBtn, SIGNAL(clicked()), this, SLOT(OnUploadCancelButtonClicked()));
    QObject::connect(ui.uploadOkBtn, SIGNAL(clicked()), this, SLOT(OnUploadOkButtonClicked()));

    QObject::connect(ui.uploadTagBtn, SIGNAL(clicked()), this, SLOT(OnUploadTagButtonClicked()));
    QObject::connect(ui.uploadDeleteBtn, SIGNAL(clicked()), this, SLOT(OnUploadDeleteButtonClicked()));
    QObject::connect(ui.uploadCommentBtn, SIGNAL(clicked()), this, SLOT(OnUploadCommentButtonClicked()));
    QObject::connect(ui.uploadAddBtn, SIGNAL(clicked()), this, SLOT(OnUploadAddButtonClicked()));

    ui.listWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    // create base folder
    if (!QDir(kBaseFolder).exists())
    {
        QDir().mkdir(kBaseFolder);
    }

    // create file for default tags
    const QString filepath = QDir(kBaseFolder).filePath(kDefaultTagsFile);
    QFile tagsFile(filepath);
    if (!tagsFile.exists())
    {
        const QString val = "{\"tags\":[]}";
        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

        if (tagsFile.open(QIODevice::WriteOnly))
        {
            tagsFile.write(json);
            tagsFile.close();
        }
    }

}

DocTestTool::~DocTestTool()
{
}

void DocTestTool::OnEditButtonClicked()
{
    viewMainScreen(false);
    viewEditScreen(true);
}

void DocTestTool::OnUploadButtonClicked()
{
    QStringList filters;
    filters << "Pdf files (*.pdf)"
        << "XML files (*.xml)"
        << "Image files (*.png *.xpm *.jpg)"
        << "Text files (*.txt)"
        << "Any files (*)";

    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    QStringList fileNames = dialog.getOpenFileNames();
    
    for (QString & fileName : fileNames)
    {
        QFile file(fileName);
        QFileInfo info(file);

        QListWidgetItem * newItem = new QListWidgetItem;
        newItem->setText(info.fileName());
        ui.listWidget->addItem(newItem);
    }
    viewMainScreen(false);
    viewUploadScreen(true);
}

void DocTestTool::OnEditSaveButtonClicked()
{
    const QString val = ui.editTextWnd->toPlainText();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    if (!jsonDoc.isNull())
    {
        QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);
        const QString filepath = QDir(kBaseFolder).filePath(kDefaultTagsFile);
        QFile tagsFile(filepath);
        if (tagsFile.exists())
        {
            QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);

            if (tagsFile.open(QIODevice::WriteOnly))
            {
                tagsFile.write(json);
                tagsFile.close();
            }
        }
        ui.statusBar->clearMessage();
        ui.editTextWnd->clear();
        viewMainScreen(true);
        viewEditScreen(false);
    }
    else
    {
        ui.statusBar->setStyleSheet("color: red");
        ui.statusBar->showMessage("Json is invalid!", 2000);
    }
}

void DocTestTool::OnEditCancelButtonClicked()
{
    ui.statusBar->clearMessage();
    ui.editTextWnd->clear();
    viewMainScreen(true);
    viewEditScreen(false);
}

void DocTestTool::OnUploadOkButtonClicked()
{
    for (int i = 0, iEnd = ui.listWidget->count(); i < iEnd; ++i)
    {
        QListWidgetItem * widget = ui.listWidget->item(i);
        QFile file(widget->text());
        QFileInfo fileInfo(file);
        const QString filepath = QDir(kBaseFolder).filePath(fileInfo.fileName());
        file.copy(filepath);
    }
    ui.listWidget->clear();
    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::OnUploadCommentButtonClicked()
{
}

void DocTestTool::OnUploadAddButtonClicked()
{
    // TODO check for duplicates
    QStringList filters;
    filters << "Pdf files (*.pdf)"
        << "XML files (*.xml)"
        << "Image files (*.png *.xpm *.jpg)"
        << "Text files (*.txt)"
        << "Any files (*)";

    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    QStringList fileNames = dialog.getOpenFileNames();
    if (!fileNames.isEmpty())
    {
        ui.listWidget->addItems(fileNames);
    }
}

void DocTestTool::OnUploadDeleteButtonClicked()
{
    QList<QListWidgetItem*> selectedWidgets = ui.listWidget->selectedItems();
    for (QListWidgetItem * item : selectedWidgets)
    {
        ui.listWidget->removeItemWidget(item);
        delete item;
    }
}

void DocTestTool::OnUploadTagButtonClicked()
{
}

void DocTestTool::OnUploadCancelButtonClicked()
{
    ui.listWidget->clear();
    viewUploadScreen(false);
    viewMainScreen(true);
}

void DocTestTool::viewMainScreen(const bool value)
{
    ui.uploadBtn->setVisible(value);
    ui.editBtn->setVisible(value);
    ui.searchBtn->setVisible(value);
}

void DocTestTool::viewSearchScreen(const bool value)
{

}

void DocTestTool::viewUploadScreen(const bool value)
{
    ui.listWidget->setVisible(value);
    ui.uploadOkBtn->setVisible(value);
    ui.uploadCancelBtn->setVisible(value);
    ui.uploadTagBtn->setVisible(value);
    ui.uploadCommentBtn->setVisible(value);
    ui.uploadDeleteBtn->setVisible(value);
    ui.uploadAddBtn->setVisible(value);
}

void DocTestTool::viewEditScreen(const bool value)
{
    const QString filepath = QDir(kBaseFolder).filePath(kDefaultTagsFile);
    QFile tagsFile(filepath);
    if (tagsFile.exists())
    {
        if (tagsFile.open(QIODevice::ReadOnly))
        {
            QByteArray fileData = tagsFile.readAll();
            ui.editTextWnd->setText(fileData);
            tagsFile.close();
        }
    }

    ui.editTextWnd->setVisible(value);
    ui.editCancelBtn->setVisible(value);
    ui.editSaveBtn->setVisible(value);
}

void DocTestTool::OnSearchButtonClicked()
{
}
