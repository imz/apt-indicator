
#include <QCoreApplication>
#include <QTimer>

#include "../config.h"
#include "checker.h"

Checker::Checker(QObject *parent):
    QObject(parent)
{
    dist_upgrade = new DistUpgrade(this, "", true, true);
    connect(dist_upgrade, SIGNAL(endDistUpgrade()), this, SLOT(onEndDistUpgrade()));

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
		// FIXME
		qDebug("EVENT_END_UPGRADE");
		switch( dist_upgrade->status() )
		{
		    case DistUpgrade::Working: stat_str = "Working"; break;
		    case DistUpgrade::Normal: stat_str = "Normal"; break;
		    case DistUpgrade::Danger: stat_str = "Danger"; break;
		    case DistUpgrade::Problem: stat_str = "Problem"; break;
		    case DistUpgrade::TryAgain: stat_str = "TryAgain"; break;
		    default: stat_str = "__undefined__"; break;
		}
		qDebug("Status: %s\nResult:\n%s", qPrintable(stat_str), qPrintable(dist_upgrade->result()));
}
