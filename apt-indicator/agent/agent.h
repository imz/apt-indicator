/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#ifndef ALT_UPDATE_AGENT_H
#define ALT_UPDATE_AGENT_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QDateTime>
#include <QProcess>
#include <QDateTime>
#include <QPointer>


#include "configuration.h"
#include "info_window.h"

class Agent : public QObject
{
	Q_OBJECT
public:
	enum UpgradeStatus {Nothing, Working, Normal, Danger, Problem, TryAgain};

	explicit
	Agent( QObject *parent = 0, const char *name = 0, const QString &homedir = "");
	~Agent();

	void setTrayVisible(bool);

public Q_SLOTS:
	void onMessageReceived(const QString&);

private Q_SLOTS:
	void helpBrowser();
	void aboutProgram();
	void exitProgram();
	void doCheck();
	void doInfo();
	void doRun(bool automatic=true);
	void doRunAuto();
	void doRunPlain();
	void doConfigure();
	void doConfigureRepos();
	void onActivateSysTray(QSystemTrayIcon::ActivationReason);
	void onClickTrayMessage();
	void onEndRun(int, QProcess::ExitStatus);
	void onEndRunError(QProcess::ProcessError);
	void onCheckerEnd(int, QProcess::ExitStatus);
	void onCheckerEndError(QProcess::ProcessError);
	void onCheckerOutput();
	void onSleepHide();
	void setTrayHidden();

private:
	void setTrayIcon();
	void updateInfoWindow();

	QMenu *menu_;
	QString homedir_; /**< user's home directory */
	Configuration *cfg_;
	UpgradeStatus status_; /**< semaphore: Are we have any files for update */

	QPointer<InfoWindow> info_window_; /**< information window */
	QRect info_window_geometry_;
	QSystemTrayIcon *tray_icon_; /**< icon on the tray */
	QTimer timer_; /**< update timer */
	QString result_; /**< result in information window */
	QDateTime last_report_time_; /**< time of last report */
	bool	autostart_; /**< are we in autostart mode */

	QProcess *checker_proc;
	QProcess *upgrader_proc;
	QString upgrader_cmd;
};

#endif
