/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <stdio.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "../config.h"

extern const char *__progname;

void usage(FILE *out)
{
    fprintf(out, "Usage: %s [-hva] [--help] [--version] [--autostart]\n", __progname);
    fprintf(out, "This is a simple applet both for Gnome and KDE which\n");
    fprintf(out, " made notifications for users that newer packages are available\n");
    fprintf(out, "  -h, --help             display help screen\n");
    fprintf(out, "  -v, --version          display version information\n");
    fprintf(out, "  -a, --autostart        for running by Window Manager's autostart\n\n");
    fprintf(out, "Report bugs to http://bugzilla.altlinux.ru\n");
}


int main( int argc, char **argv )
{

	int autostart = 0; /* run under autostart */
	int startup_agent = 1;

	/* process args */
	while (1)
	{
		static struct option long_options[] =
			{
				{"help", no_argument, 0, 'h'},
				{"version", no_argument, 0, 'v'},
				{"autostart", no_argument, 0, 'a'},
				{0, 0, 0, 0}
			};
		int c = getopt_long(argc, argv, "hva", long_options, NULL);
		if (c == -1)
			break;
		switch (c)
		{
		case 'h':
			usage(stdout);
			return EXIT_SUCCESS;
			break;
		case 'v':
			fprintf(stdout, "%s %s\n", __progname, VERSION);
			fprintf(stdout, "Written by Stanislav Ievlev and Sergey V Turchin\n");
			fprintf(stdout, "Copyright (C) 2003-2004 ALT Linux Team\n");
			return (EXIT_SUCCESS);
		case 'a':
			autostart = 1;
			break;
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}

    if( autostart )
    {
	FILE *cfg;
	char cfg_path[PATH_MAX];
	strcat(cfg_path, getenv("HOME"));
	strcat(cfg_path, "/.config/");
	strcat(cfg_path, ORGANISATION_DOMAIN);
	strcat(cfg_path, "/");
	strcat(cfg_path, PROGRAM_NAME);
	strcat(cfg_path, ".conf");
	cfg = fopen(cfg_path, "r");
	if( cfg )
	{
	    while( !feof(cfg) )
	    {
		char buffer[512];
		char *line;
		memset(buffer, '\0', 512);
		line = fgets(buffer, 511, cfg);
		if( line )
		{
		    if( strncmp("autostart=false", line, 14) == 0)
			startup_agent = 0;
		}
	    }
	    fclose(cfg);
	}
    }
    if( startup_agent )
        execvp(PROGRAM_NAME_BIN"-agent", NULL);
    return 0;
}
