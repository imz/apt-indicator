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

#include "configuration.h"

class InfoWindow;//forward declaration

class Agent : public QObject
{
	Q_OBJECT
public:
	enum UpgradeStatus {Working, Normal, Danger, Problem, TryAgain};

	explicit
	Agent( QObject *parent = 0, const char *name = 0, const QString &homedir = "", bool autostart = false);
	~Agent();

private slots:
	void startProgram();
	void helpBrowser();
	void aboutProgram();
	void exitProgram();
	void doCheck();
	void doInfo();
	void doRun();
	void doConfigure();
	void onActivateSysTray(QSystemTrayIcon::ActivationReason);
	void onEndRun(int, QProcess::ExitStatus);
	void onEndRunError(QProcess::ProcessError);
	void onCheckerEnd(int, QProcess::ExitStatus);
	void onCheckerEndError(QProcess::ProcessError);
	void onCheckerOutput();
#if 0
	void changeTrayIcon();
	void onEndDistUpgrade();
#endif

private:
	void setTrayIcon();

	QMenu *menu_;
	QString homedir_; /**< user's home directory */
	Configuration *cfg_;
	UpgradeStatus status_; /**< semaphore: Are we have any files for update */

	InfoWindow *info_window_; /**< information window */
	QSystemTrayIcon *tray_icon_; /**< icon on the tray */
	QTimer timer_; /**< update timer */
	QString result_; /**< result in information window */
	bool	autostart_; /**< are we in autostart mode */

	QProcess *checker_proc;
	QProcess *upgrader_proc;
};

#endif
