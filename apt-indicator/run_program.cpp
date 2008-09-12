/* `apt-indicator' - simple applet for user notification about updates
   (c) 2003
   Stanislav Ievlev <inger@altlinux.org>
   Sergey V Turchin <zerg@altlinux.org>
   License: GPL
*/

#include <unistd.h>
#include <sys/wait.h>

#include <QCoreApplication>
#include <QCustomEvent>

#include "run_program.h"
#include "config.h"

#define RUN_ASSERT(expr) \
if ( (expr) ) \
{ \
    result_ = QObject::tr(#expr); \
    emit endRun(); \
    return; \
}

RunProgram::RunProgram(QObject *parent, const QString &progname):
    QThread(parent),
    progname_(progname),
    status_(Failed)
{}

RunProgram::Status RunProgram::status() const
{
    return status_;
}

QString RunProgram::result() const
{
    return result_;
}

void RunProgram::run()
{
	RUN_ASSERT((child_pid_ = fork())<0);

	//run subprocess ...
	if (!child_pid_)
	{ //child
		char *argv[2] = {NULL, NULL};
		argv[0] = strdup(progname_.toLatin1());
		execv(progname_.toLatin1(), argv);
		_exit(Failed);
	}
	else
	{ //father
		int	child_status;
		
		RUN_ASSERT(waitpid (child_pid_, &child_status, 0) != child_pid_);

		//then process child retcode
		if (WIFEXITED (child_status))
		{
			status_ = static_cast<Status>(WEXITSTATUS (child_status)); //get status from child via exit code
			if (status_)
			{
			    if (status_ == Failed)
				result_ = QObject::tr("execution of program failed");
			    else
				result_ = QString(QObject::tr("child was exited with code %1")).arg(status_);
			}
		}

		if (WIFSIGNALED (child_status))
		{
			result_ = QString(QObject::tr("child was terminated with signal %1")).arg(WTERMSIG (child_status));
			status_ = Failed; //overwrite status
		}

		emit endRun(); // send event about end of work
		return ;
	}
}
