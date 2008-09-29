#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H


#include <QDialog>

#include <ui_help_browser.h>

class HelpBrowser: public QDialog
{
Q_OBJECT
public:
    HelpBrowser(QWidget *);

    Ui::HelpBrowserUI ui;

public slots:
    void execLink(const QUrl&);
};

#endif
