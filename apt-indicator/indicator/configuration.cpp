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
    cfg_ = new QSettings(QSettings::IniFormat, QSettings::UserScope, "ALT_Linux", PROGRAM_NAME, this);
    cfg_->setFallbacksEnabled(false);
    //cfg_->beginGroup("");
}

Configuration::~Configuration()
{}

QString Configuration::pathToUpgrader() const
{
    return cfg_->value("path_to_upgrader", DEF_UPGRADER_PATH).toString();
}

int Configuration::checkInterval() const
{
	return cfg_->value("check_interval", DEF_CHECK_INTERVAL).toInt();
}

bool Configuration::showBroken() const
{
	return cfg_->value("show_broken", false).toBool();
}

bool Configuration::ignoreErrors() const
{
	return cfg_->value("ingore_apt_errors", false).toBool();
}

bool Configuration::skipAutostart() const
{
	return cfg_->value("skip_autostart", false).toBool();
}

void Configuration::setSkipAutostart(bool value)
{
	cfg_->setValue("skip_autostart", value);
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
	update_period per = toPeriod(checkInterval());

	cfgDlg.ui.timesSpinBox->setValue(per.time_);
	cfgDlg.ui.periodComboBox->setCurrentIndex(per.period_);
	cfgDlg.ui.pathLineEdit->setText(pathToUpgrader());
	cfgDlg.ui.showBrokenCheck->setChecked(showBroken());
	cfgDlg.ui.ignoreErrors->setChecked(ignoreErrors());

	if ( cfgDlg.exec() == QDialog::Accepted )
	{
		per.time_ = cfgDlg.ui.timesSpinBox->value();
		per.period_ = cfgDlg.ui.periodComboBox->currentIndex();
		cfg_->setValue("path_to_upgrader", cfgDlg.ui.pathLineEdit->text());
		cfg_->setValue("check_interval", toInterval(per));
		cfg_->setValue("show_broken", (bool)cfgDlg.ui.showBrokenCheck->isChecked());
		cfg_->setValue("ingore_apt_errors", (bool)cfgDlg.ui.ignoreErrors->isChecked());
	}
}
