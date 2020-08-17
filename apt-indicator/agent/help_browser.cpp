
#include <QProcess>
#include <QDesktopServices>
#include <QDir>
#include <QTextStream>

#include <help_browser.h>
#include "../config.h"

HelpBrowser::HelpBrowser(QWidget *parent):
    QDialog(parent)
{
    ui.setupUi(this);
    ui.backwardButton->setIcon(QIcon(":/pixmaps/light/go-previous.svg"));
    ui.backwardButton->setFlat(true);
    ui.forwardButton->setIcon(QIcon(":/pixmaps/light/go-next.svg"));
    ui.forwardButton->setFlat(true);
    ui.homeButton->setIcon(QIcon(":/pixmaps/light/go-home.svg"));
    ui.homeButton->setFlat(true);

    ui.helpText->setOpenLinks(true);
    ui.helpText->setOpenExternalLinks(false);
    connect( ui.helpText, &QTextBrowser::anchorClicked, this, &HelpBrowser::execLink );
    connect( ui.homeButton, &QPushButton::clicked, this, &HelpBrowser::loadIndex );

    const QString locale = QLocale::system().name();
    const int country_delim = locale.indexOf('_');
    QString country = (country_delim>0)?locale.left(country_delim):locale;

#ifdef NDEBUG
    help_dir = DATADIR"/doc/";
#else
    help_dir = QDir::currentPath() + "/doc/html/";
#endif
    if (!QDir(help_dir+country).exists("index.html")) country = "en";//default help is English
    help_dir.append(country);
    help_dir.append(QLatin1String("/"));
    ui.helpText->setSearchPaths(QStringList() << help_dir);

    loadIndex();
}

void HelpBrowser::loadIndex()
{
    QString helptext;
    QFile helpfile(QString().append(help_dir).append("index.html"));
    if( helpfile.open(QIODevice::ReadOnly) ) {
	QTextStream helpstream(&helpfile);
	helpstream.setCodec("UTF-8");
	helptext = helpstream.readAll();
	QTextDocument *helpdoc = new QTextDocument(ui.helpText);
	helpdoc->addResource(QTextDocument::ImageResource, QUrl("IMG_PACKAGE_INSTALLED_UPDATED"), QIcon(":/pixmaps/light/package-installed-updated.svg").pixmap(22));
	helpdoc->addResource(QTextDocument::ImageResource, QUrl("IMG_PACKAGE_INSTALLED_OUTDATED"), QIcon(":/pixmaps/light/package-installed-outdated.svg").pixmap(22));
	helpdoc->addResource(QTextDocument::ImageResource, QUrl("IMG_PACKAGE_BROKEN"), QIcon(":/pixmaps/light/package-broken.svg").pixmap(22));
	helpdoc->addResource(QTextDocument::ImageResource, QUrl("IMG_PACKAGE_UPGRADE"), QIcon(":/pixmaps/light/package-upgrade.svg").pixmap(22));
	helpdoc->setHtml(helptext);
	ui.helpText->setDocument(helpdoc);
	helpfile.close();
    } else {
	qWarning("Unable ro read file: %s", qPrintable(helpfile.fileName()));
    }
}

void HelpBrowser::execLink( const QUrl &url )
{
    if( url.scheme().isEmpty() || url.scheme() == "file" )
    {
	ui.helpText->setSource(url);
    }
    else
    {
	if( !QDesktopServices::openUrl(url) )
	{
	    QString prog_command;
	    prog_command = "url_handler.sh";

	    qDebug("Try to open \"%s\" via %s", qPrintable(url.toString()), qPrintable(prog_command));

	    QProcess *prog_proc = new QProcess(this);
	    QStringList prog_args = prog_command.split(" ", Qt_SkipEmptyParts);
	    prog_args << url.toString() ;
	    QString prog_name = prog_args.takeFirst();
	    prog_proc->start(prog_name, prog_args);
	}
	ui.helpText->reload();
    }
}
