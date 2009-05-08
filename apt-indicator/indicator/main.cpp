/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QTextStream>
#include <QDir>

#include "agent.h"
#include "../config.h"

extern const char *__progname;

int main( int argc, char **argv )
{

	Q_INIT_RESOURCE(pixmaps);
	QApplication app( argc, argv );
	app.setQuitOnLastWindowClosed(false);

	QTranslator translator(&app);
	QTranslator qt_translator(&app);
	QString lang = QLocale::system().name();
#ifndef NDEBUG
	translator.load( "apt_indicator_" + lang, "./translations" );
#else
	translator.load( "apt_indicator_" + lang, DATADIR"/translations" );
#endif
	qt_translator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator( &qt_translator );
	app.installTranslator( &translator );

	QCoreApplication::setApplicationName(PROGRAM_NAME);
	if(QString(ORGANISATION_DOMAIN).isEmpty())
	    QCoreApplication::setOrganizationName(ORGANISATION_NAME);
	else
	    QCoreApplication::setOrganizationDomain(ORGANISATION_DOMAIN);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::homePath()+"/.config");
	Agent agent(0, PROGRAM_NAME, QString(getenv("HOME")), autostart );

	return app.exec();
}
