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

	void setTrayVisibility(bool);

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
	void setTrayVisible();

private:
	void updateTrayIcon();
	void updateInfoWindow();
	void setupContextMenu();

	QMenu *menu_;
	QString homedir_; /**< user's home directory */
	Configuration *cfg_;
	UpgradeStatus status_; /**< semaphore: Are we have any files for update */

	QPointer<InfoWindow> m_info_window; /**< information window */
	QRect m_infowindow_geometry;
	QSystemTrayIcon *m_tray_icon; /**< icon on the tray */
	QTimer timer_; /**< update timer */
	QString result_; /**< result in information window */
	QDateTime m_last_report_time; /**< time of last report */

	QProcess *checker_proc;
	QProcess *upgrader_proc;
	QString upgrader_cmd;
};

#endif
