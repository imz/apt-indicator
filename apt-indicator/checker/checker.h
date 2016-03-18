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

public Q_SLOTS:
    void startProgram();

private:
    DistUpgrade *dist_upgrade;
};

#endif
