/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#ifndef ALT_UPDATE_CONFIG_H
#define ALT_UPDATE_CONFIG_H

#include <QObject>

#include <QSettings>

class Configuration: public QObject
{
Q_OBJECT
public:
	enum Param { CheckInterval, UpgraderProfile, ShowBroken, IgnoreAptErrors, Autostart, PopupTray };
	enum Cmd { CmdUpgrader = 0, CmdRepos = 1 };

	Configuration(QObject *);
	~Configuration();

	void load();
	void save();

	void showDialog();
	QString commandUprader(Cmd);

	QString getString(Param);
	bool getBool(Param);
	int getInt(Param);
	bool setParam(Param, const QString&);
	bool setParam(Param, bool);
	bool setParam(Param, int);

private:
	struct update_period
	{
		int time_; /**< how many times*/
		int period_; /**< what period */
	};

	update_period toPeriod(int interval) const; /**< convert interval in sec's to period */
	int toInterval(const update_period& per) const; /**< convert period to interval in sec's */

	QMap<int,int> iperiods;

	QString upgrader_profile_;
	int check_interval_;
	bool show_broken_;
	bool ignore_apt_errors_;
	bool autostart_;
	bool popup_tray_;
};

#endif
