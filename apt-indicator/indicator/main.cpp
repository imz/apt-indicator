/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <getopt.h>

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QTextStream>

#include "agent.h"
#include "config.h"

extern const char *__progname;

void usage(int retcode)
{
    QTextStream out(stdout);
    out << "Usage: " <<__progname<<" [-hva] [--help] [--version] [--autostart]";
    out << "This is a simple applet both for Gnome and KDE which";
    out <<  " made notifications for users that newer packages are available";
    out << "  -h, --help             display help screen";
    out << "  -v, --version          display version information";
    out << "  -a, --autostart	       for running by Window Manager's autostart";
    out << "";
    out << "Report bugs to http://bugzilla.altlinux.ru";
    exit(retcode);
}


int main( int argc, char **argv )
{

	bool autostart = false; //run under autostart

	//process args
	while (1)
	{
		static struct option long_options[] =
			{
				{"help", no_argument, 0, 'h'},
				{"version", no_argument, 0, 'v'},
				{"autostart", no_argument, 0, 'a'},
				{0, 0, 0, 0}
			};
		int c = getopt_long(argc, argv, "hva", long_options, NULL);
		if (c == -1)
			break;
		switch (c)
		{
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'v':
			qDebug() << __progname << " version " << VERSION;
			qDebug() << "Written by Stanislav Ievlev and Sergey V Turchin";
			qDebug() << "";
			qDebug() << "Copyright (C) 2003-2004 ALT Linux Team";
			return (EXIT_SUCCESS);
		case 'a':
			autostart = true;
			break;
		default:
			usage(EXIT_FAILURE);
		}
	}

	Q_INIT_RESOURCE(pixmaps);
	QApplication app( argc, argv );
	app.setQuitOnLastWindowClosed(false);
	Agent agent(0, PROGRAM_NAME, QString(getenv("HOME")), autostart );

	QTranslator translator(&agent);
	QTranslator qt_translator(&agent);
	QString lang = QLocale::system().name();

#ifndef NDEBUG
	translator.load( "apt_indicator_" + lang, "./translations" );
#else
	translator.load( "apt_indicator_" + lang, DATADIR"/translations" );
#endif
	qt_translator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator( &qt_translator );
	app.installTranslator( &translator );

	return app.exec();
}
