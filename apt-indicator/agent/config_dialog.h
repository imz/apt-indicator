#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include "ui_config_dialog.h"

class Configuration;

class ConfigDialog: public QDialog, private Ui::ConfigDialog
{
Q_OBJECT
public:
    ConfigDialog(Configuration*);
    ~ConfigDialog();

private slots:
    void onClick(QAbstractButton*);

private:
    Configuration *cfg_;

    void setUiValues();
};

#endif
