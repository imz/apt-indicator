#ifndef UTILS_H
#define UTILS_H

#include <climits>

namespace Utils {

struct update_period
{
	int time_; /**< how many times*/
	int period_; /**< what period */
};

Utils::update_period intervalToPeriod(int interval); /**< convert interval in sec's to period */
int periodToInterval(const Utils::update_period& per); /**< convert period to interval in sec's */

}

#endif
