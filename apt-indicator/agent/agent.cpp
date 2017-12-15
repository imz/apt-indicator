/* `apt-indicator' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <sys/wait.h>

#include <QMessageBox>
#include <QTextCodec>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QDir>
#include <QMenu>
#include <QLocale>
#include <QWindow>

#include "agent.h"
#include "../config.h"

#include "help_browser.h"
#include "info_window.h"

extern const char *__progname;

Agent::Agent( QObject *parent, const char *name , const QString &homedir):
		QObject(parent),
		cfg_(),
		m_timer(),
		checker_proc(0)
{
	m_menu = 0;
	upgrader_proc = 0;

	setObjectName(name);
	m_last_report_time = QDateTime::currentDateTime();
	m_status = Nothing;
	cfg_ = new Configuration(this);
	if( !QSystemTrayIcon::isSystemTrayAvailable() )
	{
	    qWarning("%s: No system tray available.", __progname);
	}
	m_tray_icon = new QSystemTrayIcon(this);
	connect(m_tray_icon, &QSystemTrayIcon::activated, this, &Agent::onActivateSysTray);
	connect(m_tray_icon, &QSystemTrayIcon::messageClicked, this, &Agent::onClickTrayMessage);

	setupContextMenu();
	updateTrayIcon();

	connect( &m_timer, &QTimer::timeout, this, &Agent::doCheck );
	m_timer.start( CHECK_INTERVAL_FIRST*1000 );
}

Agent::~Agent()
{
    // terminate dist-upgrade process
    if (checker_proc && checker_proc->pid() > 0)
    {
        checker_proc->terminate();
    }
}

void Agent::setupContextMenu()
{
	if( m_menu )
	    delete m_menu;
	m_menu = new QMenu();
	m_menu->addAction( tr("&Upgrade automatically..."), this, &Agent::doRunAuto);
	m_menu->addAction( tr("&Run upgrade program..."), this, &Agent::doRunPlain);
	m_menu->addAction(QIcon::fromTheme("package-upgrade",QIcon(":/pixmaps/package-upgrade.png")), tr("Chec&k for updates"), this, &Agent::doCheck);
	m_menu->addAction( tr("D&etailed info..."), this, &Agent::doInfo);
	m_menu->addAction( tr("H&ide"), this, &Agent::setTrayHidden);
	m_menu->addSeparator();
	m_menu->addAction( tr("&Settings..."), this, &Agent::doConfigure);
	m_menu->addAction( tr("&Repository settings..."), this, &Agent::doConfigureRepos);
	m_menu->addSeparator();
	m_menu->addAction( tr("&Help"), this, &Agent::helpBrowser);
	m_menu->addAction( tr("&About"), this, &Agent::aboutProgram);
	m_menu->addSeparator();
	m_menu->addAction( tr("&Quit"), this, &Agent::exitProgram);
	m_tray_icon->setContextMenu(m_menu);
}

void Agent::doInfo()
{
    if( m_info_window )
    {
	m_infowindow_geometry = m_info_window->geometry();
	m_info_window->deleteLater();
	return;
    }
    else
    {
	m_info_window = new InfoWindow(0);
	if( !m_infowindow_geometry.isNull() )
	    m_info_window->setGeometry(m_infowindow_geometry);
	connect(m_info_window.data(), &InfoWindow::upgradeAuto, this, &Agent::doRunAuto);
	connect(m_info_window.data(), &InfoWindow::upgradeNoauto, this, &Agent::doRunPlain);
	updateInfoWindow();
	m_info_window->show();
	QWindow *inf_win = m_info_window->window()->windowHandle();
	if( inf_win ) {
	    inf_win->raise();
	    inf_win->requestActivate();
	}
    }
}

void Agent::updateInfoWindow()
{
    if( !m_info_window ) return;

    QString title = tr("Report at %1 %2")
	.arg(m_last_report_time.date().toString(Qt::LocalDate))
	.arg(m_last_report_time.time().toString(Qt::LocalDate));
    m_info_window->setWindowTitle(title);

    QString info_window_text;
    switch (m_status)
    {
	case Normal:
	    info_window_text = tr("Nothing to update...");
	    break;
	case Working:
	    info_window_text = m_result.isEmpty() ? tr("Checking in progress...") : m_result;
	    break;
	default:
	    info_window_text = m_result.isEmpty() ? tr("No status info available") : m_result;
	    break;
    }
    m_info_window->setText(info_window_text);

    if ( m_status != Danger && m_status != Problem )
	m_info_window->setButtonsVisible(false);
    else
	m_info_window->setButtonsVisible(true);
}

void Agent::doRunAuto()
{
    doRun(true);
}

void Agent::doRunPlain()
{
    doRun(false);
}

void Agent::doRun(bool automatic)
{
	//stop all active works until we run synaptic program
	if ( m_timer.isActive() )
		m_timer.stop();
	
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
		QString program("xdg-su");
		QStringList arguments;
		QString cmd_upgrader_line(cfg_->commandUprader(Configuration::CmdUpgrader));
		QString cmd_upgrader_program;
		if( !cmd_upgrader_line.isEmpty() )
		{
		    cmd_upgrader_program = cmd_upgrader_line.split(" ", QString::SkipEmptyParts).first();
		}
		if( automatic )
		{
		    arguments << cmd_upgrader_line;
		    upgrader_cmd = "xdg-su -c \"" + cmd_upgrader_line + "\"";
		}
		else
		{
		    arguments << cmd_upgrader_program;
		    upgrader_cmd = "xdg-su -c \"" + cmd_upgrader_program + "\"";
		}

		if( !arguments.isEmpty() )
		{
		    arguments.prepend("-c");
		    upgrader_proc = new QProcess(this);
		    connect(upgrader_proc, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished, this, &Agent::onEndRun);
		    connect(upgrader_proc, (void (QProcess::*)(QProcess::ProcessError))&QProcess::error, this, &Agent::onEndRunError);
		    upgrader_proc->start(program, arguments, QIODevice::ReadOnly);
		}
		else
		{
		    QMessageBox::critical(0,tr("Run upgrade process"),  tr("No upgrade program found"), QMessageBox::Ok, Qt::NoButton);
		    //doConfigure();
		}
	}
}

void Agent::doConfigure()
{
	cfg_->showDialog(); //run dialog
	m_timer.setInterval(cfg_->getInt(Configuration::CheckInterval)*1000); //then update change interval
}

void Agent::doConfigureRepos()
{
	QString program("xdg-su");
	QStringList arguments;
	QString command_line = cfg_->commandUprader(Configuration::CmdRepos);
	if( !command_line.isEmpty() )
	{
	    arguments << "-c" << command_line;
	    if( !QProcess::startDetached(program, arguments) )
		QMessageBox::critical(0,tr("Repositories configuration"),  tr("Failed to start \"%1\"").arg(command_line), QMessageBox::Ok, Qt::NoButton);
	}
}

void Agent::doCheck()
{
	if ( m_timer.isActive() )
		m_timer.stop(); //stop timer during this stage

	//check if tread exist
	if (checker_proc)
	{
		if(checker_proc->pid() == 0)
		{ // checker finish work
			QProcess *dead_checker_proc = checker_proc;
			checker_proc = 0;
			delete dead_checker_proc;
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
		connect(checker_proc, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished, this, &Agent::onCheckerEnd);
		connect(checker_proc, (void (QProcess::*)(QProcess::ProcessError))&QProcess::error, this, &Agent::onCheckerEndError);
		connect(checker_proc, &QProcess::readyReadStandardOutput, this, &Agent::onCheckerOutput);
		m_status = Working; //change status
		updateTrayIcon();
		checker_proc->start(program, arguments, QIODevice::ReadOnly); // and run checker
	}

	m_timer.start( cfg_->getInt(Configuration::CheckInterval)*1000 );
}

void Agent::helpBrowser()
{
	HelpBrowser help(0);

	help.exec();
}

void Agent::aboutProgram()
{
	QMessageBox::about(0, QString(PROGRAM_NAME) + QString(" - ") + QString(VERSION),
			    tr("This program make notification of updates.\n"
			       "Copyright (C) 2003-2004 ALT Linux Team\n\n"
			       "Written by Stanislav Ievlev and Sergey V. Turchin"));
}

void Agent::exitProgram()
{
	const QString message = tr("Should <strong>%1</strong> start automatically \nwhen you login?").arg(PROGRAM_NAME);
	const int res = QMessageBox::information(0,tr("Exit program"),message,
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
				QMessageBox::Yes
				);
	if( QMessageBox::Yes == res || QMessageBox::No == res )
	{
	    if( cfg_->setParam(Configuration::Autostart, QMessageBox::Yes == res) )
		cfg_->save();
	}
	if( QMessageBox::Cancel != res )
	    QCoreApplication::quit();
}

void Agent::updateTrayIcon()
{
	QString iconname;
	QString iconfile;
	QString	tip;
	switch (m_status)
	{
	case Nothing:
		iconname = "package-available";
		iconfile = ":/pixmaps/package-available.png";
		tip = tr("Waiting...");
		break;
	case Danger:
		iconname = "package-installed-outdated";
		iconfile = ":/pixmaps/package-installed-outdated.png";
		tip = tr("There are updates for your system available...");
		break;
	case Normal:
		iconname = "package-installed-updated";
		iconfile = ":/pixmaps/package-installed-updated.png";
		tip = tr("Nothing to update");
		break;
	case Problem:
		iconname = "package-broken";
		iconfile = ":/pixmaps/package-broken.png";
		tip = tr("Problems with Dist-Upgrade");
		break;
	case Working:
		iconname = "package-upgrade";
		iconfile = ":/pixmaps/package-upgrade.png";
		tip = tr("Working...");
		break;
	default:
		iconname = "package-available";
		iconfile = ":/pixmaps/package-available.png";
		tip = tr("Unknown status");
	};


	m_tray_icon->setIcon(QIcon::fromTheme(iconname, QIcon(iconfile)));
	m_tray_icon->setToolTip(tip);

	if( m_status != Nothing )
	    setTrayVisible();
}

void Agent::onCheckerOutput()
{
    if( !checker_proc ) return;

	UpgradeStatus new_status = m_status;
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

	if (m_status != new_status) //change icon if we need it
	{
	    m_last_report_time = QDateTime::currentDateTime();
	    m_status = new_status;
	    switch(m_status)
	    {
		case Normal:
		{
#ifndef NDEBUG
		    QTimer::singleShot(7000, this, &Agent::onSleepHide);
#else
		    if( cfg_->getInt(Configuration::CheckInterval) > 300 )
			QTimer::singleShot(300000, this, &Agent::onSleepHide);
#endif
		    break;
		}
		case Danger:
		{
		    if( cfg_->getBool(Configuration::PopupTray) )
			m_tray_icon->showMessage(PROGRAM_NAME, tr("There are updates for your system available..."), QSystemTrayIcon::Warning, 30000);
		    break;
		}
		default:
		    break;
	    }
	}
	m_result = new_result.join("\n");
	//qDebug("result<%d>: %s", new_result.size(), qPrintable(m_result));
}

void Agent::onCheckerEnd(int exitCode, QProcess::ExitStatus exitState)
{
    if( exitState == QProcess::NormalExit && exitCode != 0 )
    {
	m_status = Problem;
	qWarning(PROGRAM_NAME ": checker was exited with code %d", exitCode);
    }
    else if( exitState == QProcess::CrashExit )
    {
	m_status = Problem;
	qWarning(PROGRAM_NAME ": checker crashed");
	m_result = tr("Update checking program crashed");
    }
    updateTrayIcon();
    updateInfoWindow();
}

void Agent::onCheckerEndError(QProcess::ProcessError error)
{
    switch(error)
    {
	case QProcess::FailedToStart:
	    qWarning(PROGRAM_NAME ": failed to start checking program");
	case QProcess::Crashed:
	    qWarning(PROGRAM_NAME ": checking program crashed");
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
	if( upgrader_cmd.startsWith("xdg-su ") && exitCode == 3 ) {
	    QMessageBox::warning(0, tr("Run upgrade process"), tr("Assume a user's identity utility not found. Try to install <strong>%1</strong>.").arg("gksu"));
	} else {
	    QMessageBox::warning(0, tr("Run upgrade process"), tr("<strong>%1</strong> was exited with code %2").arg(upgrader_cmd).arg(exitCode));
	}
    } else {
	if( m_info_window )
	{
	    delete m_info_window;
	    m_info_window = 0;
	}
    }
    doCheck();
}

void Agent::onActivateSysTray(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
	case QSystemTrayIcon::Trigger:
	    doInfo(); break;
	default:
	    break;
    }
}

void Agent::onClickTrayMessage()
{
    switch(m_status)
    {
	case Danger:
	{
	    doInfo(); break;
	}
	default:
	    break;
    }
}

void Agent::onSleepHide()
{
    if( m_status == Normal && cfg_->getBool(Configuration::HideWhenSleep) )
    {
	if( (m_info_window && m_info_window->isVisible()) || (m_menu && m_menu->isVisible()) ) {
	    QTimer::singleShot(60000, this, &Agent::onSleepHide);
	} else {
	    m_status = Nothing;
	    setTrayHidden();
	    updateTrayIcon();
	}
    }
}

void Agent::setTrayVisible()
{
    setTrayVisibility(true);
}

void Agent::setTrayHidden()
{
    setTrayVisibility(false);
}

void Agent::setTrayVisibility(bool vis)
{
    m_tray_icon->setVisible(vis);
    if(vis)
	setupContextMenu(); // workaround against QDbusMenu problem
}

void Agent::onMessageReceived(const QString &msg)
{
    if( msg == MSG_WAKEUP )
	setTrayVisible();
}
