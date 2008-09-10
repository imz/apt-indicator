#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H


#include <QDialog>

#include <ui_help_browser.h>

class HelpBrowser: public QDialog
{
public:
    HelpBrowser(QWidget *);

    void init();
    void execLink(const QString&);

    Ui::HelpBrowserUI ui;
};

#endif
