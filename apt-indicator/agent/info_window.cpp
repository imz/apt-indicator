
#include <QPushButton>

#include <info_window.h>

InfoWindow::InfoWindow(QWidget *parent):
    QWidget(parent)
{
    setupUi(this);

    QPushButton *btn_auto = buttonBox->addButton(tr("Automatic upgrade"), QDialogButtonBox::YesRole);
    QPushButton *btn_noauto = buttonBox->addButton(tr("Start upgrade program"), QDialogButtonBox::YesRole);

    connect(btn_auto, SIGNAL(clicked()), this, SIGNAL(upgradeAuto()));
    connect(btn_noauto, SIGNAL(clicked()), this, SIGNAL(upgradeNoauto()));
}

InfoWindow::~InfoWindow()
{
}

void InfoWindow::setText(const QString &txt)
{
    infoText->setText(txt);
}

void InfoWindow::setButtonsVisible(bool vis)
{
    buttonBox->setVisible(vis);
}
