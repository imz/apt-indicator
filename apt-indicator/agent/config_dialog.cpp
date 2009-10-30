
#include "utils.h"
#include "configuration.h"

#include "config_dialog.h"

ConfigDialog::ConfigDialog(Configuration *cfg):
    QDialog(0)
{
    cfg_ = cfg;
    setupUi(this);

    Utils::update_period per = Utils::intervalToPeriod(cfg_->getInt(Configuration::CheckInterval));
    timesSpinBox->setValue(per.time_);
    periodComboBox->setCurrentIndex(per.period_);
    pathLineEdit->setText(cfg_->getString(Configuration::UpgraderProfile));
    showBrokenCheck->setChecked(cfg_->getBool(Configuration::ShowBroken));
    ignoreErrors->setChecked(cfg_->getBool(Configuration::IgnoreAptErrors));
    autostartCheck->setChecked(cfg_->getBool(Configuration::Autostart));
    popupTrayCheck->setChecked(cfg_->getBool(Configuration::PopupTray));
    hideWhenSleepCheck->setChecked(cfg_->getBool(Configuration::HideWhenSleep));

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onClick(QAbstractButton*)));
}

ConfigDialog::~ConfigDialog()
{}

void ConfigDialog::onClick(QAbstractButton *btn)
{
    switch( buttonBox->standardButton(btn) )
    {
	case QDialogButtonBox::RestoreDefaults:
	{
	    // set defaults
	    break;
	}
	case QDialogButtonBox::Ok:
	{
	    Utils::update_period per_new;
	    per_new.time_ = timesSpinBox->value();
	    per_new.period_ = periodComboBox->currentIndex();
	    int chk_interval = Utils::periodToInterval(per_new);
	    bool changed = false;
	    changed = cfg_->setParam(Configuration::UpgraderProfile, pathLineEdit->text()) || changed;
	    changed = cfg_->setParam(Configuration::CheckInterval,   chk_interval) || changed;
	    changed = cfg_->setParam(Configuration::ShowBroken,      showBrokenCheck->isChecked()) || changed;
	    changed = cfg_->setParam(Configuration::IgnoreAptErrors, ignoreErrors->isChecked()) || changed;
	    changed = cfg_->setParam(Configuration::Autostart,       autostartCheck->isChecked()) || changed;
	    changed = cfg_->setParam(Configuration::PopupTray,       popupTrayCheck->isChecked()) || changed;
	    changed = cfg_->setParam(Configuration::HideWhenSleep,   hideWhenSleepCheck->isChecked()) || changed;
	    if( changed )
		cfg_->save();
	    accept();
	    break;
	}
	default:
	    break;
    }
}
