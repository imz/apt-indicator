
#include <QFileDialog>

#include <config_dialog.h>

ConfigDialog::ConfigDialog(QWidget *parent):
    QDialog(parent)
{
    ui.setupUi(this);
}

void ConfigDialog::fileSelect()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName( this, tr("Choose a program"), "/usr/bin", "Programs (*)");
    if ( ! fileName.isEmpty() )
	ui.pathLineEdit->setText(fileName);
}
