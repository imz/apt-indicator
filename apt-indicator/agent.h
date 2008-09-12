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
#include <QEvent>

#include "dist_upgrade.h"
#include "run_program.h"
#include "configuration.h"

class InfoWindow;//forward declaration

class Agent : public QObject
{
	Q_OBJECT
public:
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
	void changeTrayIcon();
	void doConfigure();
	void onActivateSysTray(QSystemTrayIcon::ActivationReason);
	void onEndDistUpgrade();
	void onEndRun();

private:
	void setTrayIcon();

	QMenu *menu_;
	QString homedir_; /**< user's home directory */
	Configuration *cfg_;
	DistUpgrade::Status status_; /**< semaphore: Are we have any files for update */

	InfoWindow *info_window_; /**< information window */
	QSystemTrayIcon *tray_icon_; /**< icon on the tray */
	QTimer timer_; /**< update timer */
	QString result_; /**< result in information window */
	bool	autostart_; /**< are we in autostart mode */

	DistUpgrade *upgrade_thread_;
	RunProgram *run_thread_;
};

#endif
