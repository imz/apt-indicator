/* `apt-indicator' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/
#ifndef ALT_UPDATE_LOCAL_H
#define ALT_UPDATE_LOCAL_H

#define VERSION "0.1.1"

#define PROGRAM_NAME "APT Indicator" /**< name of the program */
#define PROGRAM_PKG_NAME "apt-indicator" /**< internal name of the program */
#define DEF_UPGRADER_PATH "/usr/bin/synaptic" /**< default path to the synaptic program */
#define DATADIR "/usr/share/apt-indicator" /**< default data dir */

#ifndef NDEBUG
#define DEF_CHECK_INTERVAL 30 /**< default interval between checkings */
#else
#define DEF_CHECK_INTERVAL 86400 /**< default interval between checkings */
#endif

#define CHECK_INTERVAL_FIRST 5 /**< interval between program run and first check  */

#define RETRY_INTERVAL 180 /**< interval between unsuccessful upgrades */

#define EVENT_END_UPGRADE  1001
#define EVENT_END_RUN  1002

#endif
