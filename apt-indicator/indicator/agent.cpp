/* `apt-indicator' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <QMessageBox>
#include <QTextCodec>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QDir>
#include <QMenu>
#include <QLocale>

#include "agent.h"
#include "config.h"

#include "help_browser.h"
#include "info_window.h"

Agent::Agent( QObject *parent, const char *name , const QString &homedir, bool autostart ):
		QObject(parent),
		homedir_(homedir),
		cfg_(),
		timer_(),
		autostart_(autostart),
		checker_proc(0),
		upgrader_proc(0)
{
	setObjectName(name);
	last_report_time_ = QDateTime::currentDateTime();
	status_ = Normal;
	info_window_ = 0;
	cfg_ = new Configuration(this);
	tray_icon_ = new QSystemTrayIcon(this);
	setTrayIcon();
	tray_icon_->show();
	connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onActivateSysTray(QSystemTrayIcon::ActivationReason)));

	menu_ = new QMenu();
	menu_->addAction( tr("&Upgrade..."), this, SLOT(doRun()));
	menu_->addAction( tr("Chec&k for updates"), this, SLOT(doCheck()));
	menu_->addAction( tr("D&etailed info..."), this, SLOT(doInfo()));
	menu_->addSeparator();
	menu_->addAction( tr("&Settings..."), this, SLOT(doConfigure()));
	menu_->addSeparator();
	menu_->addAction( tr("&Help"), this, SLOT(helpBrowser()));
	menu_->addAction( tr("&About"), this, SLOT(aboutProgram()));
	menu_->addSeparator();
	menu_->addAction( tr("&Quit"), this, SLOT(exitProgram()));
	tray_icon_->setContextMenu(menu_);

	QTimer::singleShot(0, this, SLOT(startProgram()));
}

Agent::~Agent()
{
    // terminate dist-upgrade process
    if (checker_proc && checker_proc->pid() > 0)
    {
        checker_proc->terminate();
    }
}

void Agent::startProgram()
{
	if (autostart_ && !cfg_->getBool(Configuration::Autostart))
	{
		QCoreApplication::quit();
		return;
	}

	//connect timer with update method
	connect( &timer_, SIGNAL( timeout() ), this, SLOT( doCheck() ) );
	timer_.start( CHECK_INTERVAL_FIRST*1000 );
}


void Agent::doInfo()
{
    if( info_window_ )
    {
	delete info_window_;
	info_window_ = 0;
	return;
    }

    info_window_ = new InfoWindow(0);
    connect(info_window_->ui.upgradeButton, SIGNAL(pressed()), this, SLOT(doRun()));

    QString title = tr("Report at %1 %2")
	.arg(last_report_time_.date().toString(Qt::LocalDate))
	.arg(last_report_time_.time().toString(Qt::LocalDate));
    info_window_->setWindowTitle(title);

    QString info_window_text;
    switch (status_)
    {
	case Normal:
	    info_window_text = tr("Nothing to update...");
	    break;
	case Working:
	    info_window_text = result_.isEmpty() ? tr("Checking in progress...") : result_;
	    break;
	default:
	    info_window_text = result_.isEmpty() ? tr("No status info available") : result_;
	    break;
    }
    info_window_->ui.infoText->setText(info_window_text);

    if ( status_ != Danger && status_ != Problem )
	info_window_->ui.upgradeButton->hide();
    else
	info_window_->ui.upgradeButton->show();

    info_window_->show();
}

void Agent::doRun()
{
	//stop all active works until we run synaptic program
	if ( timer_.isActive() )
		timer_.stop();
	
	if (upgrader_proc)
	{
		if(upgrader_proc->pid() > 0)
		{
			QMessageBox::information(0,tr("Run upgrade process"), tr("Program already running"));
			return;
		}
		else
		{
			delete upgrader_proc;
			upgrader_proc = 0;
		}
	}

	if (!upgrader_proc)
	{
		QStringList arguments(cfg_->getString(Configuration::PathUpgrader).split(" ", QString::SkipEmptyParts));
		QString program(arguments.takeAt(0));
		if( !program.isEmpty() )
		{
		    upgrader_proc = new QProcess(this);
		    connect(upgrader_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onEndRun(int, QProcess::ExitStatus)));
		    connect(upgrader_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onEndRunError(QProcess::ProcessError)));
		    upgrader_proc->start(program, arguments, QIODevice::ReadOnly);
		}
		else
		{
		    QMessageBox::critical(0,tr("Run upgrade process"),  tr("No upgrade program configured"), QMessageBox::Ok, Qt::NoButton);
		    //doConfigure();
		}
	}
}

void Agent::doConfigure()
{
	cfg_->showDialog(); //run dialog
	timer_.setInterval(cfg_->getInt(Configuration::CheckInterval)*1000); //then update change interval
}

void Agent::doCheck()
{
	if ( timer_.isActive() )
		timer_.stop(); //stop timer during this stage

	//check if tread exist
	if (checker_proc)
	{
		if(checker_proc->pid() == 0)
		{ // checker finish work
			delete checker_proc;
			checker_proc = 0;
		}
	}

	if (!checker_proc)
	{
		QString program(QApplication::instance()->applicationDirPath() + "/apt-indicator-checker");
		QStringList arguments;
		if( cfg_->getBool(Configuration::ShowBroken) )
		    arguments << "--show-broken";
		if( cfg_->getBool(Configuration::IgnoreAptErrors) )
		    arguments << "--ignore-errors";
		checker_proc = new QProcess(this);
		connect(checker_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCheckerEnd(int, QProcess::ExitStatus)));
		connect(checker_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onCheckerEndError(QProcess::ProcessError)));
		connect(checker_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(onCheckerOutput()));
		status_ = Working; //change status
		setTrayIcon();
		checker_proc->start(program, arguments, QIODevice::ReadOnly); // and run checker
	}

	timer_.start( cfg_->getInt(Configuration::CheckInterval)*1000 );
}

void Agent::helpBrowser()
{
	HelpBrowser help(0);

//	help.ui.helpText->mimeSourceFactory()->setExtensionType("html", "text/html;charset=UTF-8");
//	help.ui.helpText->mimeSourceFactory()->setFilePath( "." );

	const QString locale = QLocale::system().name();
	const int country_delim = locale.indexOf('_');
	QString country = (country_delim>0)?locale.left(country_delim):locale;

#ifdef NDEBUG
	const QString help_path = DATADIR"/doc/";
#else
	const QString help_path = QDir::currentPath() + "/doc/html/";
#endif
	if (!QDir(help_path+country).exists("index.html")) country = "en";//default help is English
	
	help.ui.helpText->setSource(help_path+country+"/index.html");
	help.exec();
}

void Agent::aboutProgram()
{
	QMessageBox::about(0,objectName() + QString(" - ") + QString(VERSION),
			    tr("This program make notification of updates.\n"
			       "Copyright (C) 2003-2004 ALT Linux Team\n\n"
			       "Written by Stanislav Ievlev and Sergey V. Turchin"));
}

void Agent::exitProgram()
{
#if 0
	const QString message = tr("Should <b>%1</b> start automatically \nwhen you login?").arg(PROGRAM_NAME);
	const int res = QMessageBox::information(0,tr("Exit program"),message,
				QMessageBox::Yes|QMessageBox::Default,
				QMessageBox::No,
				Qt::NoButton
				);
	cfg_->setParam(Configuration::Autostart, QMessageBox::Yes == res);
#endif
	QCoreApplication::quit();
}

void Agent::setTrayIcon()
{
	QString iconname;
	QString	tip;
	switch (status_)
	{
	case Danger:
		iconname = ":/pixmaps/danger.png";
		tip = tr("There are updates for your system available...");
		break;
	case Normal:
		iconname = ":/pixmaps/normal.png";
		tip = tr("Nothing to update");
		break;
	case Problem:
		iconname = ":/pixmaps/problem.png";
		tip = tr("Problems with Dist-Upgrade");
		break;
	case Working:
		iconname = ":/pixmaps/working.png";
		tip = tr("Working...");
		break;
	default:
		iconname = ":/pixmaps/working.png";
		tip = tr("Unknown status");
	};

	QPixmap pix;
	if (!pix.load(iconname))
	{
		QMessageBox::warning(0,tr("setting Icon"),
			    	       QString(tr("Could not load the icon file %1")).arg(iconname),
			    	       QMessageBox::Ok,Qt::NoButton);
		QCoreApplication::quit();
	}

	tray_icon_->setIcon(pix);
	tray_icon_->setToolTip(tip);
}

void Agent::onCheckerOutput()
{
    if( !checker_proc ) return;

	UpgradeStatus new_status = status_;
	QStringList new_result;
	int read_state = 0;
	while( !checker_proc->atEnd() && checker_proc->canReadLine() )
	{
	    QByteArray line = checker_proc->readLine();
	    line.truncate(line.size()-1);
	    if(line == "CHECKER_STATUS") {
		read_state = 1; continue;
	    } else if(line == "CHECKER_RESULT") {
		read_state = 2; continue;
	    }
	    switch( read_state )
	    {
		case 1:
		{
		    if( line == "WORKING" )
			new_status = Working;
		    else if( line == "NORMAL" )
			new_status = Normal;
		    else if( line == "DANGER" )
			new_status = Danger;
		    else
			new_status = Problem;
		    //qDebug("status: %s", line.data());
		    read_state = 0;
		    break;
		}
		case 2:
		{
		    new_result << QString::fromLocal8Bit(line);
		    break;
		}
		default:
		    break;
	    }
	}

	if (status_ != new_status) //change icon if we need it
	{
	    status_ = new_status;
	    last_report_time_ = QDateTime::currentDateTime();
	    setTrayIcon();
	}
	result_ = new_result.join("\n");
	//qDebug("result<%d>: %s", new_result.size(), qPrintable(result_));
}

void Agent::onCheckerEnd(int exitCode, QProcess::ExitStatus exitState)
{
    if( exitState == QProcess::NormalExit && exitCode != 0 )
	qWarning(PROGRAM_NAME ": checker was exited with code %d", exitCode);
    else if( exitState == QProcess::CrashExit )
	qWarning(PROGRAM_NAME ": checker crashed");
}

void Agent::onCheckerEndError(QProcess::ProcessError error)
{
    switch(error)
    {
	case QProcess::FailedToStart:
	    qWarning(PROGRAM_NAME ": failed to start checking program");
	case QProcess::Crashed:
	    qWarning(PROGRAM_NAME ": checker crashed");
	default:
	    qWarning(PROGRAM_NAME ": execution of checking program failed");
    }
}

void Agent::onEndRunError(QProcess::ProcessError error)
{
    QString msg;
    switch(error)
    {
	case QProcess::FailedToStart:
	    msg = tr("Failed to start upgrade program");
	case QProcess::Crashed:
	    msg = tr("Program crashed");
	default:
	    msg = tr("Execution of program failed");
    }
    QMessageBox::critical(0, tr("Run upgrade process"), msg);
}

void Agent::onEndRun(int exitCode, QProcess::ExitStatus exitState)
{
    if( exitState == QProcess::NormalExit && exitCode != 0 ) {
	QMessageBox::warning(0, tr("Run upgrade process"), tr("child was exited with code %1").arg(exitCode));
    } else {
	if( info_window_ )
	{
	    delete info_window_;
	    info_window_ = 0;
	    return;
	}
    }
    doCheck();
}

void Agent::onActivateSysTray(QSystemTrayIcon::ActivationReason reason)
{
    if( reason == QSystemTrayIcon::Trigger )
	doInfo();
}
