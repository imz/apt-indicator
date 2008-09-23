/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QDebug>
#include <QTextStream>

#include "../indicator/config.h"
#include "checker.h"

int main( int argc, char **argv )
{

	QCoreApplication app( argc, argv );
	Checker checker;

	return app.exec();
}
