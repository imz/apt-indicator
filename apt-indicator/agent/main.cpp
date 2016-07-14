/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <getopt.h>

//#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <QSettings>

#include "qtsingleapplication.h"

#include "agent.h"
#include "../config.h"

extern const char *__progname;

void usage(FILE *out)
{
    fprintf(out, "Usage: %s [-hva] [--help] [--version] [--autostart]\n", __progname);
    fprintf(out, "This is a simple applet both for Gnome and KDE which\n");
    fprintf(out, " made notifications for users that newer packages are available\n");
    fprintf(out, "  -h, --help             display help screen\n");
    fprintf(out, "  -v, --version          display version information\n");
    fprintf(out, "  -a, --autostart        for running by Window Manager's autostart\n\n");
    fprintf(out, "Report bugs to http://bugzilla.altlinux.ru\n");
}


int main( int argc, char **argv )
{
    int ret = EXIT_SUCCESS;
    bool autostarted = false;

	/* process args */
	while (true)
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
			usage(stdout);
			return EXIT_SUCCESS;
			break;
		case 'v':
			qDebug("%s %s\n", __progname, VERSION);
			qDebug("Written by Stanislav Ievlev and Sergey V Turchin\n");
			qDebug("Copyright (C) 2003-2004 ALT Linux Team\n");
			return (EXIT_SUCCESS);
		case 'a':
			autostarted = true;
			break;
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}


    Q_INIT_RESOURCE(pixmaps);
    QtSingleApplication app( argc, argv );

    if( app.isRunning() )
    {
	app.sendMessage(MSG_WAKEUP, 0);
	app.quit();
	ret = 0;
    } else
    {
	bool startup_agent = true;

	QCoreApplication::setApplicationName(PROGRAM_NAME);
	if(QString(ORGANISATION_DOMAIN).isEmpty())
	    QCoreApplication::setOrganizationName(ORGANISATION_NAME);
	else
	    QCoreApplication::setOrganizationDomain(ORGANISATION_DOMAIN);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::homePath()+"/.config");

	if( autostarted )
	{ // check need start
	    QSettings cfg(&app);
	    startup_agent = cfg.value("autostart", DEF_AUTOSTART).toBool() != false;
	}

	if( startup_agent )
	{
	    QTranslator translator(&app);
	    QTranslator qt_translator(&app);
	    QString lang = QLocale::system().name();
#ifndef NDEBUG
	    translator.load( "apt_indicator_agent_" + lang, "./translations" );
#else
	    translator.load( "apt_indicator_agent_" + lang, DATADIR"/translations" );
#endif
	    qt_translator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	    app.installTranslator( &qt_translator );
	    app.installTranslator( &translator );

	    app.setWindowIcon(QIcon::fromTheme("package-available",QIcon(":/pixmaps/package-available.png")));
	    app.setQuitOnLastWindowClosed(false);

	    Agent agent(&app, PROGRAM_NAME, QString(getenv("HOME")));
	    QObject::connect(&app, &QtSingleApplication::messageReceived,
		     &agent, &Agent::onMessageReceived);

	    ret = app.exec();
	}
    }

    return ret;
}
