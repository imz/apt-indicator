/* `egg' - simple applet for user notification about updates
(c) 2003,2004
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#ifndef ALT_UPDATE_DIST_UPGRADE_H
#define ALT_UPDATE_DIST_UPGRADE_H

#include <QThread>
#include <QObject>

#if defined(QT_NO_THREAD)
#  error Thread support not enabled.
#endif


/**
 * @todo make kill at the end of thread
 * @repeat fetch lists until network stop
 */
class DistUpgrade : public QThread
{
Q_OBJECT
public:
	enum Status {Working, Normal, Danger, Problem, TryAgain};

	explicit
	DistUpgrade(QObject *parent, const QString &homedir, bool show_broken,bool ignore_errors);
	virtual ~DistUpgrade();
	virtual void run();

	Status status() const; /**< read dist-upgrade status */
	QString result() const; /**< read dist-upgrade results */
signals:
	void endDistUpgrade();
	
private:
	void doChild();
	void doFather();

	bool update();
	void dist_upgrade();

	std::string	workdir_;
	Status status_;
	QString result_; /**< dist-upgrade result for Information window*/
	bool show_broken_;
	bool ignore_errors_;
	pid_t child_pid_; /**< pid of the child process */

	int pipes[2]; /**< connection pipes */
};

#endif
