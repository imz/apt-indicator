
#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QStringList>

#include "../config.h"
#include "checker.h"

Checker::Checker(QObject *parent):
    QObject(parent)
{
    bool show_broken = false;
    bool ignore_errors = false;
    QStringList sargs = QCoreApplication::arguments();
    sargs.removeFirst();
    foreach(QString sarg, sargs)
    {
	if( sarg == "--show-broken" )
	    show_broken = true;
	else if( sarg == "--ignore-errors" )
	    ignore_errors = true;
    }
    dist_upgrade = new DistUpgrade(this, "", show_broken, ignore_errors);
    QTimer::singleShot(0, this, SLOT(startProgram()));
}

Checker::~Checker()
{
}

void Checker::startProgram()
{
    dist_upgrade->start();

    QString stat_str;
    switch( dist_upgrade->status() )
    {
        case DistUpgrade::Working: stat_str = "WORKING"; break;
        case DistUpgrade::Normal: stat_str = "NORMAL"; break;
        case DistUpgrade::Danger: stat_str = "DANGER"; break;
        case DistUpgrade::Problem: stat_str = "PROBLEM"; break;
        default: stat_str = "__undefined__"; break;
    }
    QTextStream out(stdout, QIODevice::WriteOnly);
    out << "CHECKER_STATUS\n" << stat_str << "\n";
    out << "CHECKER_RESULT\n" << dist_upgrade->result() << "\n";

    QCoreApplication::quit();
}
