#ifndef APT_INDICATOR_CHECKER_H
#define APT_INDICATOR_CHECKER_H

#include <QObject>

#include "../dist_upgrade.h"

class Checker: public QObject
{
Q_OBJECT
public:
    Checker(QObject *parent = 0);
    ~Checker();

public slots:
    void startProgram();
    void onEndDistUpgrade();

private:
    DistUpgrade *dist_upgrade;
};

#endif
