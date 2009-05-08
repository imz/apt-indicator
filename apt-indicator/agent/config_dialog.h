#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include "ui_config_dialog.h"

class ConfigDialog: public QDialog
{
Q_OBJECT
public:
    ConfigDialog(QWidget*);

public slots:
    void fileSelect();

public:
    Ui::ConfigDialogUI ui;
};

#endif
