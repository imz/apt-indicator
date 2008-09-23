
#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>

#include "../indicator/config.h"
#include "checker.h"

Checker::Checker(QObject *parent):
    QObject(parent)
{
    dist_upgrade = new DistUpgrade(this, "", true, true);
    connect(dist_upgrade, SIGNAL(endDistUpgrade()), this, SLOT(onEndDistUpgrade()));
    QTimer::singleShot(0, this, SLOT(startProgram()));
}

Checker::~Checker()
{
}

void Checker::startProgram()
{
    dist_upgrade->start();
}

void Checker::onEndDistUpgrade()
{
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
    out << "CHECKER_STATUS:" << stat_str << "\n";
    out << "CHECKER_RESULT:" << dist_upgrade->result() << "\n";

    QCoreApplication::quit();
}
