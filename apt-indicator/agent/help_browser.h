#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H


#include <QDialog>

#include <ui_help_browser.h>

class QTextDocument;

class HelpBrowser: public QDialog
{
Q_OBJECT
public:
    HelpBrowser(QWidget *);

    Ui::HelpBrowserUI ui;

private Q_SLOTS:
    void execLink(const QUrl&);
    void loadIndex();
private:
    QString help_dir;
};

#endif
