/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#ifndef ALT_UPDATE_RUN_PROGRAM_H
#define ALT_UPDATE_RUN_PROGRAM_H

#include <QObject>
#include <QThread>

class RunProgram: public QThread
{
public:
	typedef enum Status {Good = 0, Failed = 77};

	RunProgram(QObject *_receiver, const QString &progname);
	virtual void run();

	Status	status() const;
	QString	result() const;
private:
	void	sendEvent();

	QObject	*receiver_;
	QString progname_;
	Status	status_;
	QString	result_;
	pid_t	child_pid_;
};

#endif
