/* `apt-indicator' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <QFile>

#include "configuration.h"
#include "config_dialog.h"
#include "../config.h"

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
    setParam(UpgraderProfile,    cfg.value("upgrader_profile", DEF_UPGRADER).toString());
    setParam(CheckInterval,   cfg.value("check_interval", DEF_CHECK_INTERVAL).toInt());
    setParam(ShowBroken,      cfg.value("show_broken", false).toBool());
    setParam(IgnoreAptErrors, cfg.value("ignore_apt_errors", false).toBool());
    setParam(Autostart,       cfg.value("autostart", true).toBool());
    setParam(PopupTray,       cfg.value("popup_systray", true).toBool());
}

void Configuration::save()
{
    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "ALT_Linux", PROGRAM_PKG_NAME, this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    cfg.setValue("upgrader_profile", getString(UpgraderProfile));
    cfg.setValue("check_interval", getInt(CheckInterval));
    cfg.setValue("show_broken", getBool(ShowBroken));
    cfg.setValue("ignore_apt_errors", getBool(IgnoreAptErrors));
    cfg.setValue("autostart", getBool(Autostart));
    cfg.setValue("popup_systray", getBool(PopupTray));
}

bool Configuration::setParam(Param param, const QString &value)
{
    bool changed = false;
    switch(param)
    {
	case UpgraderProfile: { changed = getString(param) != value; upgrader_profile_ = value; break; }
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
	case PopupTray: { changed = getBool(param) != value; popup_tray_ = value; break; }
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
	case UpgraderProfile:
	    return upgrader_profile_;
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
	case PopupTray:
	    return popup_tray_;
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
	cfgDlg.ui.pathLineEdit->setText(getString(UpgraderProfile));
	cfgDlg.ui.showBrokenCheck->setChecked(getBool(ShowBroken));
	cfgDlg.ui.ignoreErrors->setChecked(getBool(IgnoreAptErrors));
	cfgDlg.ui.autostartCheck->setChecked(getBool(Autostart));
	cfgDlg.ui.popupTrayCheck->setChecked(getBool(PopupTray));

	if ( cfgDlg.exec() == QDialog::Accepted )
	{
	    update_period per_new;
	    per_new.time_ = cfgDlg.ui.timesSpinBox->value();
	    per_new.period_ = cfgDlg.ui.periodComboBox->currentIndex();
	    int chk_interval = toInterval(per_new);
	    bool changed = false;
	    changed = setParam(UpgraderProfile,    cfgDlg.ui.pathLineEdit->text()) || changed;
	    changed = setParam(CheckInterval,   chk_interval) || changed;
	    changed = setParam(ShowBroken,      cfgDlg.ui.showBrokenCheck->isChecked()) || changed;
	    changed = setParam(IgnoreAptErrors, cfgDlg.ui.ignoreErrors->isChecked()) || changed;
	    changed = setParam(Autostart,       cfgDlg.ui.autostartCheck->isChecked()) || changed;
	    changed = setParam(PopupTray,       cfgDlg.ui.popupTrayCheck->isChecked()) || changed;
	    if( changed )
		save();
	}
}

QString Configuration::commandUprader(Cmd cmd)
{
	QString command;
	QStringList entries = getString(Configuration::UpgraderProfile).split(";", QString::SkipEmptyParts);
	foreach(QString entry, entries)
	{
	    QStringList entry_paths = entry.split(":", QString::KeepEmptyParts);
	    if( entry_paths.size() == 3 && !entry_paths[0].isEmpty() && !entry_paths[1].isEmpty() )
	    {
		bool check_exists = true;
		QStringList check_paths = entry_paths[2].split(",", QString::SkipEmptyParts);
		foreach(QString path,check_paths)
		{
		    if(!QFile::exists(path))
			check_exists = false;
		}
		if( check_exists )
		    command = entry_paths[cmd];
	    }
	}
	return command;
}
