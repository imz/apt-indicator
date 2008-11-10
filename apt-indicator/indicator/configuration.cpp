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
    check_interval_ = 0;
    show_broken_ = false;
    ignore_apt_errors_ = false;
    autostart_ = false;

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
    setParam(PathUpgrader,    cfg.value("path_upgrader", DEF_UPGRADER_PATH).toString());
    setParam(CheckInterval,   cfg.value("check_interval", DEF_CHECK_INTERVAL).toInt());
    setParam(ShowBroken,      cfg.value("show_broken", false).toBool());
    setParam(IgnoreAptErrors, cfg.value("ignore_apt_errors", false).toBool());
    setParam(Autostart,       cfg.value("autostart", true).toBool());
}

void Configuration::save()
{
    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "ALT_Linux", PROGRAM_PKG_NAME, this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    cfg.setValue("path_upgrader", getString(PathUpgrader));
    cfg.setValue("check_interval", getInt(CheckInterval));
    cfg.setValue("show_broken", getBool(ShowBroken));
    cfg.setValue("ignore_apt_errors", getBool(IgnoreAptErrors));
    cfg.setValue("autostart", getBool(Autostart));
}

bool Configuration::setParam(Param param, const QString &value)
{
    bool changed = false;
    switch(param)
    {
	case PathUpgrader: { changed = getString(param) != value; path_upgrader_ = value; break; }
	default: { changed = false; break; }
    }
    return changed;
}

bool Configuration::setParam(Param param, bool value)
{
    bool changed = false;
    switch(param)
    {
	case ShowBroken: { changed = getBool(param) != value; show_broken_ = value; break; }
	case IgnoreAptErrors: { changed = getBool(param) != value; ignore_apt_errors_ = value; break; }
	case Autostart: { changed = getBool(param) != value; autostart_ = value; break; }
	default: { changed = false; break; }
    }
    return changed;
}

bool Configuration::setParam(Param param, int value)
{
    bool changed = false;
    switch(param)
    {
	case CheckInterval: { changed = getInt(param) != value; check_interval_ = value; break; }
	default: { changed = false; break; }
    }
    return changed;
}

QString Configuration::getString(Param param)
{
    switch(param)
    {
	case PathUpgrader:
	    return path_upgrader_;
	default:
	    { qDebug("Configuration::getString(unknown)"); break; }
    }
    return "";
}

int Configuration::getInt(Param param)
{
    switch(param)
    {
	case CheckInterval:
	    return check_interval_;
	default:
	    { qDebug("Configuration::getInt(unknown)"); break; }
    }
    return 0;
}

bool Configuration::getBool(Param param)
{
    switch(param)
    {
	case ShowBroken:
	    return show_broken_;
	case IgnoreAptErrors:
	    return ignore_apt_errors_;
	case Autostart:
	    return autostart_;
	default:
	    { qDebug("Configuration::getBool(unknown)"); break; }
    }
    return false;
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
	    update_period per_new;
	    per_new.time_ = cfgDlg.ui.timesSpinBox->value();
	    per_new.period_ = cfgDlg.ui.periodComboBox->currentIndex();
	    int chk_interval = toInterval(per_new);
	    bool changed;
	    changed = setParam(PathUpgrader,    cfgDlg.ui.pathLineEdit->text()) || changed;
	    changed = setParam(CheckInterval,   chk_interval) || changed;
	    changed = setParam(ShowBroken,      cfgDlg.ui.showBrokenCheck->isChecked()) || changed;
	    changed = setParam(IgnoreAptErrors, cfgDlg.ui.ignoreErrors->isChecked()) || changed;
	    changed = setParam(Autostart,       cfgDlg.ui.autostartCheck->isChecked()) || changed;
	    if( changed )
		save();
	}
}
