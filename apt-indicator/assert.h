/* `apt-indicator' - simple applet for user notification about updates
   (c) 2004
   Stanislav Ievlev <inger@altlinux.org>
   License: GPL
*/

#ifndef ALT_UPDATE_ASSERT_H
#define ALT_UPDATE_ASSERT_H

#define SYSTEM_ASSERT(expr) \
if ( (expr) ) \
{ \
    result_ = QObject::tr(#expr); \
    sendEvent(); \
    return; \
}

#endif

