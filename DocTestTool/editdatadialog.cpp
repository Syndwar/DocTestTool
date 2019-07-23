#include "editdatadialog.h"

EditDataDialog::EditDataDialog(QWidget * parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SIGNAL(okButtonClicked()));
}

EditDataDialog::~EditDataDialog()
{
}