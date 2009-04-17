/* `egg' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include <cstring>
#include <apt-pkg/init.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/cacheiterators.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/strutl.h>

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>

#include "dist_upgrade.h"
#include "../indicator/config.h"

#define DIST_UPGRADE_ASSERT(expr) \
if ( (expr) ) \
{ \
    result_ = tr(#expr); \
    emit endDistUpgrade(); \
    return; \
}

namespace
{

	inline
	void dbg_msg(const QString& str)
	{
#ifndef NDEBUG
		qDebug("debug: %s", qPrintable(str));
#else
		if( str.isEmpty() ) return;
#endif
	}

#define APT_UPDATE_ASSERT(expr) \
if ( (expr) ) \
{ \
    dbg_msg(tr(#expr)); \
    _config->Set("Dir::State", old_state_dir); \
    _config->Set("Dir::Cache", old_cache_dir); \
    return false ; \
}

#define APT_UPDATE_RESET \
{ \
    _config->Set("Dir::State", old_state_dir); \
    _config->Set("Dir::Cache", old_cache_dir); \
}

	/** progress class for Fetcher */
	class AcqTextStatus : public pkgAcquireStatus
	{
	public:
		AcqTextStatus(){}
		virtual ~AcqTextStatus() {};

		virtual void Start()
		{
			pkgAcquireStatus::Start();
			//std::cerr << "Starting lists fetching\n";
		}

		virtual void Stop()
		{
			pkgAcquireStatus::Stop();
		}

		virtual bool MediaChange(string Media, string Drive)
		{
			return false;
		}

		virtual void Fetch(pkgAcquire::ItemDesc &Itm)
		{}

		virtual void Fail(pkgAcquire::ItemDesc &Itm)
		{}
		//do nothing
	}
	;

	/** progress class for Cache */
	class OpTextStatus: public OpProgress
	{
	public:
		OpTextStatus()
		{}
		virtual ~OpTextStatus()
		{}
		virtual void Done()
		{ //do nothing
		}
	};
}

/**
 * @todo use mkdir -p (make_path)
 */
DistUpgrade::DistUpgrade(QObject *parent, const QString &homedir, bool broken,bool errors):
		QThread(parent),
		status_(Problem),     //problems by default
		show_broken_(broken),
		ignore_errors_(errors),
		child_pid_(-1)
{
	if( !homedir.isEmpty() )
	    workdir_ = homedir.toStdString();
	else
	    workdir_ = QDir::homePath().toStdString();

	workdir_ += "/.apt/";
	mkdir(workdir_.c_str(), 0755); //try to create work directory
}

DistUpgrade::~DistUpgrade()
{
    if (child_pid_ > 0) kill(child_pid_,SIGTERM);
}

DistUpgrade::Status DistUpgrade::status() const
{
	return status_;
}

QString DistUpgrade::result() const
{
	return result_;
}

/** apt-get update
 * first try in home dir, on problems rollback to system cache
 * @todo use mkdir -p (make_path)
 */
DistUpgrade::UpdateResult DistUpgrade::update()
{
	UpdateResult upd_result = UpdNormal;

	if (workdir_.empty())
	{
	    result_ = tr("Unable to determine user's home: skip update stage.");
	    return UpdProblem;
	}

	//listdir infrastructure
	mkdir(workdir_.c_str(), 0755);
	std::string listdir = workdir_ + "lists/";
	mkdir(listdir.c_str(), 0755);
	listdir += "partial/";
	mkdir(listdir.c_str(), 0755);

	//cachedir infrastructure
	std::string cachedir = workdir_ + "cache/";
	mkdir(cachedir.c_str(), 0755);
	std::string archivedir = cachedir + "archives/";
	mkdir(archivedir.c_str(), 0755);
	archivedir += "partial/";
	mkdir(archivedir.c_str(), 0755);

	//set lists directory into users home: we need to create this directories before
	std::string old_state_dir = _config->FindDir("Dir::State");
	_config->Set("Dir::State", workdir_);
	std::string old_cache_dir = _config->FindDir("Dir::Cache");
	_config->Set("Dir::Cache", cachedir);

	//read /etc/apt/sources.list for releases
	pkgSourceList List;
	if( !List.ReadMainList() )
	{
	    APT_UPDATE_RESET;
	    result_ = tr("Failed to read sources lists.");
	    return UpdProblem;
	}

	// Create the download object
	AcqTextStatus Stat;
	pkgAcquire Fetcher(&Stat);
	if( !List.GetReleases(&Fetcher) )
	{
	    APT_UPDATE_RESET;
	    result_ = tr("Failed to get package releases.");
	    return UpdProblem;
	}
	Fetcher.Run();

	for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
	{
		if ((*I)->Status == pkgAcquire::Item::StatDone)
			continue;
		(*I)->Finished();
		upd_result = UpdTryAgain;
		result_ = tr("Release files for some repositories could not be retrieved or authenticated.\n");
	}

	//read list for indexes
	if( !List.GetIndexes(&Fetcher) )
	{
	    APT_UPDATE_RESET;
	    result_ = tr("Failed to get package indexes");
	    return UpdProblem;
	}
	if( Fetcher.Run() == pkgAcquire::Failed )
	{
	    APT_UPDATE_RESET;
	    result_ = tr("Failed to fetch packages information");
	    return UpdProblem;
	}

	for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
	{
		if ((*I)->Status == pkgAcquire::Item::StatDone)
			continue;

		(*I)->Finished();

		QString desc_uri = QString::fromStdString((*I)->DescURI());
		if( desc_uri.startsWith("cdrom:") )
		    continue;

		result_ += tr("Failed to fetch %1 %2\n").arg(desc_uri).arg(QString::fromStdString((*I)->ErrorText));
		upd_result = UpdProblem;
	}
	if( upd_result == UpdProblem )
	    return UpdProblem;

	// Clean out any old list files
	if (!Fetcher.Clean(_config->FindDir("Dir::State::lists")) ||
	    !Fetcher.Clean(_config->FindDir("Dir::State::lists") + "partial/"))
	{
		dbg_msg(tr("nothing to clean\n"));
	}

	// Prepare the cache
	pkgCacheFile Cache;
	OpTextStatus Prog;
	if( !Cache.BuildCaches(Prog, false) )
	{
	    APT_UPDATE_RESET;
	    result_ = tr("Failed to build package caches.");
	    return UpdProblem;
	}

	return upd_result;
}

//apt-get dist-upgrade
void DistUpgrade::dist_upgrade()
{
	pkgCacheFile Cache;
	OpTextStatus Prog;
	Cache.Open(Prog, false);

	if (pkgDistUpgrade(*Cache) == false && !ignore_errors_)
	{
		result_ = tr("DistUpgrade failed. Please, check your APT system");
		return ;
	}

	if (Cache->InstCount() != 0 || Cache->DelCount() != 0)
	{ //we have some programs for upgrade
		QString to_remove;
		QString to_upgrade;
		QString to_install;
		QString broken;

		status_ = Danger; //we need upgrade

		for (pkgCache::PkgIterator I = Cache->PkgBegin();I.end() != true; I++)
		{

			if (show_broken_ && (Cache[I].NowBroken() || Cache[I].InstBroken()))
			{
				broken += I.Name();
				broken += " ";
			}

			//see details
			if (Cache[I].Delete() == true)
			{
				to_remove += I.Name();
				to_remove += " ";
			}
			if (Cache[I].NewInstall() == true)
			{
				to_install += I.Name();
				to_install += " ";
			}

			if (Cache[I].Upgrade() == true && Cache[I].NewInstall() == false)
			{
				to_upgrade += I.Name();
				to_upgrade += " ";
			}
		}
		if (!to_remove.isEmpty())
		{
			result_ += tr("\n<font color='red'><b>Following packages will be removed:</b></font><br/>\n");
			result_ += to_remove;
			result_ += "<br/>";
		}
		if (!to_install.isEmpty())
		{
			result_ += tr("\n<b>Following packages will be installed:</b><br/>\n");
			result_ += to_install;
			result_ += "<br/>";
		}
		if (show_broken_ && !broken.isEmpty())
		{
			result_ += tr("<b>Following packages have unmet dependencies:</b><br/>\n");
			result_ += broken;
			result_ += "<br/>";
			status_ = Problem;
			return ;
		}
		if (!to_upgrade.isEmpty())
		{
			result_ += tr("\n<b>Following packages will be upgraded:</b><br/>\n");
			result_ += to_upgrade;
			result_ += "<br/>";
		}
	}
	else
		status_ = Normal; //we don't need upgrade

}

//child's work
void DistUpgrade::doChild()
{
	//close other end of the pipe
	close(pipes[0]);

	//lock
	std::string lockfile = (workdir_.empty()) ? "/tmp/" : workdir_;
	lockfile += ".agent_lock";

	int fd = GetLock(lockfile);
	if (fd < 0 )
	{
		result_ = tr("unable to get exclusive lock");
		return ;
	}

	FileFd Lock(fd); //autoclose when exit this function

	if (!pkgInitConfig(*_config) || !pkgInitSystem(*_config, _system))
	{
		result_ = tr("Problems with APT initialization");
		return ;
	}

	switch( update() )
	{
	    case UpdTryAgain: { status_ = TryAgain; break; }
	    case UpdProblem: { status_ = Problem; return; }
	    case UpdNormal:
	    default:
		dist_upgrade(); //then dist-upgrade
	}
}

/**
 * father's work
 * read messages from child
 */
void DistUpgrade::doFather()
{
	char buf[BUFSIZ];

	//close one end of the pipes
	close(pipes[1]);
	int n;
	while ((n = TEMP_FAILURE_RETRY(read(pipes[0], buf, sizeof(buf) - 1 * sizeof(char)))) > 0 )
	{
		buf[n + 1] = '\000';
		result_ += QString::fromLocal8Bit(buf); //append to result string
		memset(buf, 0, sizeof(buf));
	};
	close(pipes[0]);
}

/**
 * fork subprocess for apt
 * we had to create child 'cause APT is suxx, cannot unlock rpm database if we use it read-only
 */
void DistUpgrade::run()
{
	do
	{
		DIST_UPGRADE_ASSERT(pipe(pipes) < 0); //create connection pipe
		DIST_UPGRADE_ASSERT((child_pid_ = fork()) < 0) //fork subprocess

		if (!child_pid_) //run subprocess ...
		{ //child
			result_ = "";

			nice(15); //low dist uprade process priority
			doChild();

			//send result to father, if we have
			if (!result_.isEmpty())
			{
				FILE *f = fdopen(pipes[1], "w");
				QTextStream s(f, QIODevice::WriteOnly);
				s << result_;
				s.flush();
			}

			close(pipes[1]);

			//hack: when use exit() sometime child cannot exit (fork under thread problems)
			_exit(status_); //return status to parent process
		}
		else
		{ //father
			int	child_status;

			doFather(); //read pipes here

			//wait when child finish it's work
			DIST_UPGRADE_ASSERT(waitpid(child_pid_, &child_status, 0) != child_pid_);

			//get status from child via exit code
			if ((WIFEXITED (child_status)))
				status_ = static_cast<Status>(WEXITSTATUS (child_status));

			if (WIFSIGNALED (child_status))
			{
				result_ = QString(tr("child was terminated with signal %1")).arg(WTERMSIG (child_status));
				status_ = Problem; //overwrite status
			}
			if (status_ != TryAgain)
			{
			    emit endDistUpgrade(); //send notification about end of work
			    quit();
			}
			else
				sleep(RETRY_INTERVAL); //wait for the next retry
		
			child_pid_ = -1; //child process doesn't exists at this point
		}

	}
	while (status_ == TryAgain);
}
