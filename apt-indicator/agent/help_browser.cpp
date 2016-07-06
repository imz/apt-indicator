
#include <QProcess>
#include <QDesktopServices>

#include <help_browser.h>

HelpBrowser::HelpBrowser(QWidget *parent):
    QDialog(parent)
{
    ui.setupUi(this);
    ui.helpText->setOpenLinks(true);
    ui.helpText->setOpenExternalLinks(false);
    connect( ui.helpText, &QTextBrowser::anchorClicked, this, &HelpBrowser::execLink );
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
	    QStringList prog_args = prog_command.split(" ", QString::SkipEmptyParts);
	    prog_args << url.toString() ;
	    QString prog_name = prog_args.takeFirst();
	    prog_proc->start(prog_name, prog_args);
	}
	ui.helpText->reload();
    }
}
