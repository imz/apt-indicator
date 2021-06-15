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
#include <apt-pkg/version.h>

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QMap>

#include "dist_upgrade.h"
#include "../config.h"

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

		virtual void Start() override
		{
			pkgAcquireStatus::Start();
			//std::cerr << "Starting lists fetching\n";
		}

		virtual void Stop() override
		{
			pkgAcquireStatus::Stop();
		}

		//virtual bool MediaChange(string Media, string Drive)
		virtual bool MediaChange(const string &, const string &) override
		{
			return false;
		}

		virtual void Fetch(pkgAcquire::ItemDesc&) override
		{}

		virtual void Fail(pkgAcquire::ItemDesc&) override
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
		virtual void Done() override
		{ //do nothing
		}
	};
}

/**
 * @todo use mkdir -p (make_path)
 */
DistUpgrade::DistUpgrade(QObject *parent, const QString &homedir, bool broken,bool errors):
		QObject(parent),
		status_(Problem),     //problems by default
		show_broken_(broken),
		ignore_errors_(errors)
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
	pkgCacheFile Cache(false /* not WithLock */);
	OpTextStatus Prog;
	if( !Cache.BuildCaches(Prog) )
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
	pkgCacheFile Cache(false /* not WithLock */);
	OpTextStatus Prog;
	Cache.Open(Prog);

	if (pkgDistUpgrade(*Cache) == false && !ignore_errors_)
	{
		result_ = tr("DistUpgrade failed. Please, check your APT system");
		return ;
	}

	if (Cache->InstCount() != 0 || Cache->DelCount() != 0)
	{ //we have some programs for upgrade
		QMap<QString,QStringList> lst_replace;
		QStringList lst_remove;
		QStringList lst_upgrade;
		QStringList lst_install;
		QStringList lst_broken;

		status_ = Danger; //we need upgrade

		for (pkgCache::PkgIterator I = Cache->PkgBegin();I.end() != true; I++)
		{

			if (show_broken_ && (Cache[I].NowBroken() || Cache[I].InstBroken()))
			{
				lst_broken << I.Name();
			}

			//see details
			if (Cache[I].Delete() == true)
			{
        		    bool obsoleted = false;
        		    QStringList by;
        		    for( pkgCache::DepIterator D = I.RevDependsList(); D.end() == false; D++ )
        		    {
        			if( D->Type == pkgCache::Dep::Obsoletes &&
            			    Cache[D.ParentPkg()].Install() &&
            			    (pkgCache::Version*)D.ParentVer() == Cache[D.ParentPkg()].InstallVer &&
            			    Cache->VS().CheckDep(I.CurrentVer().VerStr(), D) == true )
        			{
            			    if( !obsoleted )
            				obsoleted = true;
                		    by.append(D.ParentPkg().Name());
        			}
        		    }
        		    if( obsoleted )
				lst_replace[I.Name()] = by;
        		    else
				lst_remove << I.Name();
			}

			if (Cache[I].NewInstall() == true)
			{
				lst_install << I.Name();
			}

			if (Cache[I].Upgrade() == true && Cache[I].NewInstall() == false)
			{
				lst_upgrade << I.Name();
			}
		}
		if (show_broken_ && !lst_broken.isEmpty())
		{
			result_ += QString("<strong>%1:</strong><br/>\n").arg(tr("Following packages have unmet dependencies"));
			lst_broken.sort();
			result_ += lst_broken.join(" ");
			result_ += "<br/>";
			status_ = Problem;
			return ;
		}
		if (!lst_remove.isEmpty())
		{
			result_ += QString("\n<font color='red'><strong>%1:</strong></font><br/>\n").arg(tr("Following packages will be removed"));
			lst_remove.sort();
			result_ += lst_remove.join(" ");
			result_ += "<br/>";
		}
		if (!lst_replace.isEmpty())
		{
			result_ += QString("\n<strong>%1:</strong><br/>\n").arg(tr("Following packages will be replaced"));
			QMapIterator<QString,QStringList> i(lst_replace);
			while(i.hasNext())
			{
			    i.next();
			    result_ += i.key();
			    result_ += "(";
			    result_ += i.value().join(",");
			    result_ += ") ";
			}
			result_ += "<br/>";
		}
		if (!lst_install.isEmpty())
		{
			result_ += QString("\n<strong>%1:</strong><br/>\n").arg(tr("Following packages will be installed"));
			lst_install.sort();
			result_ += lst_install.join(" ");
			result_ += "<br/>";
		}
		if (!lst_upgrade.isEmpty())
		{
			result_ += QString("\n<strong>%1:</strong><br/>\n").arg(tr("Following packages will be upgraded"));
			lst_upgrade.sort();
			result_ += lst_upgrade.join(" ");
			result_ += "<br/>";
		}
	}
	else
		status_ = Normal; //we don't need upgrade

}

/**
 * fork subprocess for apt
 * we had to create child 'cause APT is suxx, cannot unlock rpm database if we use it read-only
 */
void DistUpgrade::start()
{
	int fd = -1;
	do
	{
	    //lock
	    std::string lockfile = (workdir_.empty()) ? "/tmp/" : workdir_;
	    lockfile += ".agent_lock";

	    if( fd < 0 )
		fd = GetLock(lockfile);
	    if (fd < 0 ) {
		result_ = tr("unable to get exclusive lock");
	    } else {
		FileFd Lock(fd); //autoclose when exit this function
		if (!pkgInitConfig(*_config) || !pkgInitSystem(*_config, _system)) {
		    result_ = tr("Problems with APT initialization");
		} else {
		    switch( update() ) {
			case UpdTryAgain: { status_ = TryAgain; break; }
			case UpdProblem: { status_ = Problem; return; }
			case UpdNormal:
			default:
			    dist_upgrade(); //then dist-upgrade
		    }
		}
	    }

		if (status_ != TryAgain) {
		    break;
		} else {
		    sleep(RETRY_INTERVAL); //wait for the next retry
		}

	}
	while (status_ == TryAgain);
}
