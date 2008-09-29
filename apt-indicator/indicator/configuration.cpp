/* `apt-indicator' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include "configuration.h"
#include "config_dialog.h"
#include "config.h"

Configuration::Configuration(QObject *parent):
    QObject(parent)
{
    iperiods[0] = 1;
    iperiods[1] = 60;
    iperiods[2] = 3600;
    iperiods[3] = 86400;
    iperiods[4] = 604800;
    iperiods[5] = 2592000;

    load();
}

Configuration::~Configuration()
{
}

void Configuration::load()
{
    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "ALT_Linux", PROGRAM_PKG_NAME, this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    path_upgrader_ = cfg.value("path_upgrader", DEF_UPGRADER_PATH).toString();
    check_interval_ = cfg.value("check_interval", DEF_CHECK_INTERVAL).toInt();
    show_broken_ = cfg.value("show_broken", false).toBool();
    ignore_apt_errors_ = cfg.value("ignore_apt_errors", false).toBool();
    autostart_ = cfg.value("autostart", true).toBool();
}

void Configuration::save()
{
    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "ALT_Linux", PROGRAM_PKG_NAME, this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    cfg.setValue("path_upgrader", path_upgrader_);
    cfg.setValue("check_interval", check_interval_);
    cfg.setValue("show_broken", show_broken_);
    cfg.setValue("ignore_apt_errors", ignore_apt_errors_);
    cfg.setValue("autostart", autostart_);
}

bool Configuration::setParam(Param param, const QString &value)
{
    bool changed = getString(param) != value;
    if( param == PathUpgrader )
	path_upgrader_ = value;
    else
	changed = false;
    return changed;
}

bool Configuration::setParam(Param param, bool value)
{
    bool changed = getBool(param) != value;
    switch(param)
    {
	case ShowBroken: { show_broken_ = value; break; }
	case IgnoreAptErrors: { ignore_apt_errors_ = value; break; }
	case Autostart: { autostart_ = value; break; }
	default: { changed = false; break; }
    }
    return changed;
}

bool Configuration::setParam(Param param, int value)
{
    bool changed = getInt(param) != value;
    if( param == CheckInterval )
	check_interval_ = value;
    else
	changed = false;
    return changed;
}

QString Configuration::getString(Param param)
{
    QString ret;
    if( param == PathUpgrader )
	ret = path_upgrader_;
    return ret;
}

int Configuration::getInt(Param param)
{
    int ret = 0;
    if( param == CheckInterval )
	ret = check_interval_;
    return ret;
}

bool Configuration::getBool(Param param)
{
    bool ret = false;
    switch(param)
    {
	case ShowBroken: { ret = show_broken_; break; }
	case IgnoreAptErrors: { ret = ignore_apt_errors_ ; break; }
	case Autostart: { ret = autostart_ ; break; }
	default: break;
    }
    return ret;
}

Configuration::update_period Configuration::toPeriod(int interval) const
{
	update_period per;
	per.time_ = CHECK_INTERVAL_FIRST;
	for (int i = iperiods.size() - 1; i >= 0; i--)
		if ( iperiods[i] < interval)
		{
			per.time_ = interval / iperiods[i];
			per.period_ = i;
			break;
		};
	return per;
}

int Configuration::toInterval(const update_period& per) const
{
	int interval = per.time_ * iperiods[per.period_];
	if ( interval < CHECK_INTERVAL_FIRST)
		interval = CHECK_INTERVAL_FIRST;
	if ( interval > INT_MAX / 1000)
		interval = INT_MAX / 1000;
	return interval;
}

void Configuration::showDialog()
{
	ConfigDialog cfgDlg(0);
	update_period per = toPeriod(getInt(CheckInterval));

	cfgDlg.ui.timesSpinBox->setValue(per.time_);
	cfgDlg.ui.periodComboBox->setCurrentIndex(per.period_);
	cfgDlg.ui.pathLineEdit->setText(getString(PathUpgrader));
	cfgDlg.ui.showBrokenCheck->setChecked(getBool(ShowBroken));
	cfgDlg.ui.ignoreErrors->setChecked(getBool(IgnoreAptErrors));
	cfgDlg.ui.autostartCheck->setChecked(getBool(Autostart));

	if ( cfgDlg.exec() == QDialog::Accepted )
	{
	    per.time_ = cfgDlg.ui.timesSpinBox->value();
	    per.period_ = cfgDlg.ui.periodComboBox->currentIndex();
	    int chk_interval = toInterval(per);
	    if(    setParam(PathUpgrader,    cfgDlg.ui.pathLineEdit->text())
		|| setParam(CheckInterval,   chk_interval)
		|| setParam(ShowBroken,      cfgDlg.ui.showBrokenCheck->isChecked())
		|| setParam(IgnoreAptErrors, cfgDlg.ui.ignoreErrors->isChecked())
		|| setParam(Autostart,       cfgDlg.ui.autostartCheck->isChecked()))
		save();
	}
}
