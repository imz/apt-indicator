/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include "unistd.h"
#include <sys/syscall.h>

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QDebug>
#include <QTextStream>
#include <QLocale>
#include <QTranslator>

#include "../config.h"
#include "checker.h"

#define IOPRIO_CLASS_SHIFT      (13)
#define IOPRIO_PRIO_MASK        ((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask) ((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)  ((mask) & IOPRIO_PRIO_MASK)
#define IOPRIO_PRIO_VALUE(class, data)  (((class) << IOPRIO_CLASS_SHIFT) | data)

int main( int argc, char **argv )
{
	if( nice(20) == -1) {
	   // qWarning("Unable to nice.");
	}
	syscall(SYS_ioprio_set, 1, getpid(), IOPRIO_PRIO_VALUE(3, 7));

	QCoreApplication app( argc, argv );
	Checker checker;

	QTranslator translator(&app);
	QString lang = QLocale::system().name();
#ifndef NDEBUG
	translator.load( "apt_indicator_checker_" + lang, "./translations" );
#else
	translator.load( "apt_indicator_checker_" + lang, DATADIR"/translations" );
#endif
	app.installTranslator( &translator );

	return app.exec();
}
