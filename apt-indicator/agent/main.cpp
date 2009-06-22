/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <csignal>

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


static void agentSignalHandler(int sig)
{
    Agent::unixSignalHandler(SIGUSR1);
}


int main( int argc, char **argv )
{
    bool is_running = false;
    int ret = 0;
    QString tmpdir(getenv("TMPDIR"));
    if( tmpdir.isEmpty() )
	tmpdir = "/tmp";
    QString pidfile_path = QString("%1/apt-indicator-agent-%2.pid").arg(tmpdir).arg(getuid());
    { // check agent is running
	QFile pidfile(pidfile_path);
	if( pidfile.open(QIODevice::ReadOnly) )
	{
	    char pid_buf[1024];
	    qint64 line_len = pidfile.readLine(pid_buf, sizeof(pid_buf));
	    if( line_len > 0 )
	    {
		int pid = QString::fromLatin1(pid_buf).trimmed().toInt();
		if(pid > 0)
		{
		    QString cmdline_path = QString("/proc/%1/cmdline").arg(pid);
		    QFile cmdline_file(cmdline_path);
		    if(cmdline_file.open(QIODevice::ReadOnly))
		    {
	    		char cmd_buf[1024];
	    		qint64 line_len = cmdline_file.readLine(cmd_buf, sizeof(cmd_buf));
	    		if( line_len > 0 )
	    		{
	    		    if( QString::fromLatin1(cmd_buf).trimmed() == PROGRAM_NAME_BIN"-agent" )
	    		    {
				qWarning("apt-indicator-agent is already running at pid %d", pid);
	    			kill(pid, SIGUSR1);
	    			is_running = true;
	    		    }
	    		}
	    		cmdline_file.close();
	    	    }
		}
	    }
	    pidfile.close();
	}
    }

    if( !is_running )
    { // create pid file
	QFile pidfile(pidfile_path);
	if( pidfile.open(QIODevice::WriteOnly|QIODevice::Truncate) )
	{
	    QString write_line = QString("%1").arg(getpid());
	    pidfile.write(write_line.toLocal8Bit());
	    pidfile.close();
	}
	else
	{
	    qFatal("Unable to create pidfile %s", qPrintable(pidfile_path));
	}

	Q_INIT_RESOURCE(pixmaps);
	QApplication app( argc, argv );
	app.setQuitOnLastWindowClosed(false);
#if 0
	app.watchUnixSignal(SIGUSR1, true);
#else
        struct sigaction sa;
        sigemptyset(&(sa.sa_mask));
        sa.sa_flags = 0;
        sa.sa_handler = agentSignalHandler;
        sigaction(SIGUSR1, &sa, 0);
#endif

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
	Agent agent(0, PROGRAM_NAME, QString(getenv("HOME")));
#if 0
	QObject::connect(QCoreApplication::instance(), SIGNAL(unixSignal(int)), &agent, SLOT(onUnixSignal(int)));
#else
	QObject::connect(&agent, SIGNAL(unixSignal(int)), &agent, SLOT(onUnixSignal(int)));
#endif

	ret = app.exec();
    }

    return ret;
}
