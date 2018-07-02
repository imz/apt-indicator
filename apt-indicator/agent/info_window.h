#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <ui_info_window.h>

class InfoWindow: public QWidget, private Ui::InfoWindow
{
Q_OBJECT
public:
    InfoWindow(QWidget *);
    ~InfoWindow();

    void setText(const QString&);
    void setButtonsVisible(bool);
    void raiseIt();

Q_SIGNALS:
    void upgradeAuto();
};

#endif
