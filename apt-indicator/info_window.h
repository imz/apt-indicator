#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <ui_info_window.h>

class InfoWindow: public QDialog
{
public:
    InfoWindow(QWidget *);
    ~InfoWindow();

    Ui::InfoWindowUI ui;
};

#endif
