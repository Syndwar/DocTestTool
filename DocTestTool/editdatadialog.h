#ifndef EDITDATADIALOG_H
#define EDITDATADIALOG_H

#include <QtWidgets/QDialog>

#include "ui_editdatadialog.h"

class EditDataDialog : public QDialog
{
    Q_OBJECT
public:
    EditDataDialog(QWidget * parent = Q_NULLPTR);

    ~EditDataDialog();
signals:
    void okButtonClicked();
private:
    Ui::EditDataDialog ui;
};

#endif // EDITDATADIALOG_H
