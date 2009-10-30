
#include "../config.h"

#include "utils.h"

#define IPERIODS_SIZE 6

namespace Utils {

Utils::update_period intervalToPeriod(int interval)
{
    int iperiods[IPERIODS_SIZE];
    iperiods[0] = 1;
    iperiods[1] = 60;
    iperiods[2] = 3600;
    iperiods[3] = 86400;
    iperiods[4] = 604800;
    iperiods[5] = 2592000;

	update_period per;
	per.time_ = CHECK_INTERVAL_FIRST;
	for (int i = IPERIODS_SIZE - 1; i >= 0; i--)
		if ( iperiods[i] < interval)
		{
			per.time_ = interval / iperiods[i];
			per.period_ = i;
			break;
		};
	return per;
}

int periodToInterval(const update_period& per)
{
    int iperiods[IPERIODS_SIZE];
    iperiods[0] = 1;
    iperiods[1] = 60;
    iperiods[2] = 3600;
    iperiods[3] = 86400;
    iperiods[4] = 604800;
    iperiods[5] = 2592000;

	int interval = per.time_ * iperiods[per.period_];
	if ( interval < CHECK_INTERVAL_FIRST)
		interval = CHECK_INTERVAL_FIRST;
	if ( interval > INT_MAX / 1000)
		interval = INT_MAX / 1000;
	return interval;
}

}
