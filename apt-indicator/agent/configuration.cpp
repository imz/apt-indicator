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
    autostart_ = true;
    popup_tray_ = true;
    hide_when_sleep_ = true;

    load();
}

Configuration::~Configuration()
{
}

void Configuration::load()
{
    QSettings cfg(this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    setParam(UpgraderProfile,    cfg.value("upgrader_profile", DEF_UPGRADER).toString());
    setParam(CheckInterval,   cfg.value("check_interval", DEF_CHECK_INTERVAL).toInt());
    setParam(ShowBroken,      cfg.value("show_broken", DEF_SHOW_BROKEN).toBool());
    setParam(IgnoreAptErrors, cfg.value("ignore_apt_errors", DEF_IGNORE_APT_ERRORS).toBool());
    setParam(Autostart,       cfg.value("autostart", DEF_AUTOSTART).toBool());
    setParam(PopupTray,       cfg.value("popup_systray", DEF_POPUP_TRAY).toBool());
    setParam(HideWhenSleep,   cfg.value("hide_when_sleep", DEF_HIDE_WHEN_SLEEP).toBool());
}

void Configuration::save()
{
    QSettings cfg(this);
    cfg.setFallbacksEnabled(false);
    //cfg_.beginGroup("");
    cfg.setValue("upgrader_profile", getString(UpgraderProfile));
    cfg.setValue("check_interval", getInt(CheckInterval));
    cfg.setValue("show_broken", getBool(ShowBroken));
    cfg.setValue("ignore_apt_errors", getBool(IgnoreAptErrors));
    cfg.setValue("autostart", getBool(Autostart));
    cfg.setValue("popup_systray", getBool(PopupTray));
    cfg.setValue("hide_when_sleep", getBool(HideWhenSleep));
}

bool Configuration::setParam(Param param, const char *value)
{
    setParam(param, QString(value));
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
	case HideWhenSleep: { changed = getBool(param) != value; hide_when_sleep_ = value; break; }
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
	case HideWhenSleep:
	    return hide_when_sleep_;
	default:
	    { qDebug("Configuration::getBool(unknown)"); break; }
    }
    return false;
}

void Configuration::showDialog()
{
	ConfigDialog cfgDlg(this);
	cfgDlg.exec();
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
