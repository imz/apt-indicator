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
		status_(DistUpgrade::Working),
		timer_(),
		autostart_(autostart),
		upgrade_thread_(0),
		run_thread_(0)
{
	setObjectName(name);
	info_window_ = 0;
	cfg_ = new Configuration(this);
	tray_icon_ = new QSystemTrayIcon(this);
	tray_icon_->show();
	connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onActivateSysTray(QSystemTrayIcon::ActivationReason)));

	QPixmap pix;
	if (pix.load(":/pixmaps/normal.png"))
	{
	    tray_icon_->setIcon(pix);
	}

	menu_ = new QMenu();
	menu_->addAction( QObject::tr("&Upgrade..."), this, SLOT(doRun()));
	menu_->addAction( QObject::tr("Chec&k for updates"), this, SLOT(doCheck()));
	menu_->addAction( QObject::tr("D&etailed info..."), this, SLOT(doInfo()));
	menu_->addSeparator();
	menu_->addAction( QObject::tr("&Settings..."), this, SLOT(doConfigure()));
	menu_->addSeparator();
	menu_->addAction( QObject::tr("&Help"), this, SLOT(helpBrowser()));
	menu_->addAction( QObject::tr("&About"), this, SLOT(aboutProgram()));
	menu_->addSeparator();
	menu_->addAction( QObject::tr("&Quit"), this, SLOT(exitProgram()));
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

	setTrayIcon();

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

    QString title = QObject::tr("Report at %1 %2")
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
	
	if (run_thread_)
	{
		if (run_thread_->isRunning())
		{
			QMessageBox::warning(0,QObject::tr("Run upgrade process"),
					       QObject::tr("Program already running"),
					       QMessageBox::Ok, Qt::NoButton);
			return;
		}
		else
		{
			delete run_thread_;
			run_thread_ = 0;
		}
	}

	if (!run_thread_)
	{
		run_thread_ = new RunProgram(this, cfg_->pathToUpgrader());
		connect(run_thread_, SIGNAL(endRun()), this, SLOT(onEndRun()));
		run_thread_->start();
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
			    QObject::tr("This program make notification of updates.\n"
			       "Copyright (C) 2003-2004 ALT Linux Team\n\n"
			       "Written by Stanislav Ievlev and Sergey V. Turchin"));
}

void Agent::exitProgram()
{
	const QString message = QObject::tr("Should <b>%1</b> start automatically \nwhen you login?").arg(PROGRAM_NAME);
	const int res = QMessageBox::information(0,QObject::tr("Exit program"),message,
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
		tip = QObject::tr("There are updates for your system available...");
		break;
	case DistUpgrade::Normal:
		iconname = ":/pixmaps/normal.png";
		tip = QObject::tr("Nothing to update");
		break;
	case DistUpgrade::Problem:
		iconname = ":/pixmaps/problem.png";
		tip = QObject::tr("Problems with Dist-Upgrade");
		break;
	case DistUpgrade::Working:
		iconname = ":/pixmaps/working.png";
		tip = QObject::tr("Working...");
		break;
	default:
		iconname = ":/pixmaps/working.png";
		tip = QObject::tr("Unknown status");
	};

	QPixmap pix;
	if (!pix.load(iconname))
	{
		QMessageBox::warning(0,QObject::tr("setting Icon"),
			    	       QString(QObject::tr("Could not load the icon file %1")).arg(iconname),
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

void Agent::onEndRun()
{
    if (run_thread_->status() != RunProgram::Good)
    {
	QMessageBox::critical(0,QObject::tr("Event processing"),
	    run_thread_->result(),
	    QMessageBox::Ok,Qt::NoButton);
    }
    doCheck();
}

void Agent::onActivateSysTray(QSystemTrayIcon::ActivationReason reason)
{
    if( reason == QSystemTrayIcon::Trigger )
	doInfo();
}
