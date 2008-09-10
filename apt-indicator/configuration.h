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
	struct update_period
	{
		int time_; /**< how many times*/
		int period_; /**< what period */
	};
public:
	Configuration(QObject *);
	~Configuration();

	void showDialog();

	int checkInterval() const;
	QString pathToUpgrader() const;
	bool showBroken() const;
	bool ignoreErrors() const;
	void setSkipAutostart(bool value = true);
	bool skipAutostart() const;
private:
	update_period toPeriod(int interval) const; /**< convert interval in sec's to period */
	int toInterval(const update_period& per) const; /**< convert period to interval in sec's */

	QSettings *cfg_; /**< QT Settings object */
	QMap<int,int> iperiods;
};

#endif
