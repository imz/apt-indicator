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
		upgrade_thread_(0),
		upgrader_proc(0)
{
	setObjectName(name);
	status_ = DistUpgrade::Normal;
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
	//close all active threads: let's synaptic continue it's work, but close dist-upgrade thread
	if (upgrade_thread_ && upgrade_thread_->isRunning())
	{
	    upgrade_thread_->terminate();
	    upgrade_thread_->wait();
	}
	delete upgrade_thread_;
}

void Agent::startProgram()
{
	if (autostart_ && cfg_->skipAutostart())
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
	.arg(QDate::currentDate().toString(Qt::LocalDate))
	.arg(QTime::currentTime().toString(Qt::LocalDate));
    info_window_->setWindowTitle(title);

    QString info_window_text;
    switch (status_)
    {
	case DistUpgrade::Normal:
	    info_window_text = tr("Nothing to update...");
	    break;
	case DistUpgrade::Working:
	    info_window_text = result_.isEmpty() ? tr("Checking in progress...") : result_;
	    break;
	default:
	    info_window_text = result_.isEmpty() ? tr("No status info available") : result_;
	    break;
    }
    info_window_->ui.infoText->setText(info_window_text);

    if ( status_ != DistUpgrade::Danger && status_ != DistUpgrade::Problem )
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
		QStringList arguments(cfg_->pathToUpgrader().split(" ", QString::SkipEmptyParts));
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
	timer_.setInterval(cfg_->checkInterval()*1000); //then update change interval
}

void Agent::doCheck()
{
	if ( timer_.isActive() )
		timer_.stop(); //stop timer during this stage

	//check if tread exist
	if (upgrade_thread_)
	{
		if( upgrade_thread_->isFinished() )
		{ //thread finish work
			delete upgrade_thread_;
			upgrade_thread_ = 0;
		}
	}

	if (!upgrade_thread_)
	{
		upgrade_thread_ = new DistUpgrade(this, homedir_, 
						  cfg_->showBroken(),cfg_->ignoreErrors());
		connect(upgrade_thread_, SIGNAL(endDistUpgrade()), this, SLOT(onEndDistUpgrade()));
		status_ = DistUpgrade::Working; //change status
		setTrayIcon();
		upgrade_thread_->start(); //and run thread
	}

	timer_.start( cfg_->checkInterval()*1000 );
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
	const QString message = tr("Should <b>%1</b> start automatically \nwhen you login?").arg(PROGRAM_NAME);
	const int res = QMessageBox::information(0,tr("Exit program"),message,
				QMessageBox::Yes|QMessageBox::Default,
				QMessageBox::No,
				Qt::NoButton
				);
	cfg_->setSkipAutostart(QMessageBox::No == res);
	QCoreApplication::quit();
}

void Agent::setTrayIcon()
{
	QString iconname;
	QString	tip;
	switch (status_)
	{
	case DistUpgrade::Danger:
		iconname = ":/pixmaps/danger.png";
		tip = tr("There are updates for your system available...");
		break;
	case DistUpgrade::Normal:
		iconname = ":/pixmaps/normal.png";
		tip = tr("Nothing to update");
		break;
	case DistUpgrade::Problem:
		iconname = ":/pixmaps/problem.png";
		tip = tr("Problems with Dist-Upgrade");
		break;
	case DistUpgrade::Working:
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

void Agent::changeTrayIcon()
{
	result_ = upgrade_thread_->result(); //fetch processing result

	//check if we need do uprade system
	const DistUpgrade::Status new_status = upgrade_thread_->status();
	if (status_ != new_status) //change icon if we need it
	{
		status_ = new_status;
		setTrayIcon();
	}
}

void Agent::onEndDistUpgrade()
{
    changeTrayIcon();
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
    if( exitState == QProcess::NormalExit && exitCode != 0 )
	QMessageBox::warning(0, tr("Run upgrade process"), tr("child was exited with code %1").arg(exitCode));
    doCheck();
}

void Agent::onActivateSysTray(QSystemTrayIcon::ActivationReason reason)
{
    if( reason == QSystemTrayIcon::Trigger )
	doInfo();
}
