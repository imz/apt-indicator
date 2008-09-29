
#include <QProcess>

#include <help_browser.h>

HelpBrowser::HelpBrowser(QWidget *parent):
    QDialog(parent)
{
    ui.setupUi(this);
    connect( ui.helpText, SIGNAL( anchorClicked(const QUrl&) ), this, SLOT( execLink( const QUrl& ) ) );
}

void HelpBrowser::execLink( const QUrl &url )
{
    qDebug("HelpBrowser::execLink");
    QString prog_command;
    if( url.scheme() == "http" )
    {
	prog_command = getenv("HELP_BROWSER");
	if( prog_command.isEmpty() )
	{
	    qWarning("%s", qPrintable(tr("HELP_BROWSER environment variable is empty")));
	    prog_command = getenv("BROWSER");
	    if( prog_command.isEmpty() )
		qWarning("%s", qPrintable(tr("BROWSER environment variable is empty")));
	}
    }
    else if( url.scheme() == "mailto" )
    {
	prog_command = getenv("MAILER");
	if( prog_command.isEmpty() )
	    qWarning("%s", qPrintable(tr("MAILER environment variable is empty")));
    }
    else
    {
	return;
    }
    ui.helpText->reload();
    
    if( prog_command.isEmpty() )
    {
	prog_command = "url_handler.sh";
	qDebug("Set prog_command to %s", qPrintable(prog_command));
    }
    
    QProcess *prog_proc = new QProcess(this);
    QStringList prog_args = prog_command.split(" ", QString::SkipEmptyParts);
    prog_args << url.toString() ;
    QString prog_name = prog_args.takeFirst();
    prog_proc->start(prog_name, prog_args);
}
