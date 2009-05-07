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
#include <QLocale>
#include <QTranslator>

#include "../config.h"
#include "checker.h"

int main( int argc, char **argv )
{

	QCoreApplication app( argc, argv );
	Checker checker;

	QTranslator translator(&app);
	QString lang = QLocale::system().name();
#ifndef NDEBUG
	translator.load( "apt_indicator_checker_" + lang, "./translations" );
#else
	translator.load( "apt_indicator_checker_" + lang, DATADIR"/translations" );
#endif
	app.installTranslator( &translator );

	return app.exec();
}
