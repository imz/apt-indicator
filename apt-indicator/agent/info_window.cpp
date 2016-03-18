
#include <QPushButton>

#include <info_window.h>

InfoWindow::InfoWindow(QWidget *parent):
    QWidget(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QPushButton *btn_auto = buttonBox->addButton(tr("Automatic upgrade"), QDialogButtonBox::YesRole);
    QPushButton *btn_noauto = buttonBox->addButton(tr("Run upgrade program"), QDialogButtonBox::YesRole);

    connect(btn_auto, &QPushButton::clicked, this, &InfoWindow::upgradeAuto);
    connect(btn_noauto, &QPushButton::clicked, this, &InfoWindow::upgradeNoauto);
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
