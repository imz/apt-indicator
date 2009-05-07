/* `apt-indicator' - simple applet for user notification about updates
(c) 2003
Stanislav Ievlev <inger@altlinux.org>
Sergey V Turchin <zerg@altlinux.org>
License: GPL
*/

#include <stdio.h>
#include <getopt.h>
#include <limits.h>

#include "../config.h"

extern const char *__progname;

void usage(FILE *out)
{
    fprintf(out, "Usage: %s [-hva] [--help] [--version] [--autostart]\n", __progname);
    fprintf(out, "This is a simple applet both for Gnome and KDE which\n");
    fprintf(out, " made notifications for users that newer packages are available\n");
    fprintf(out, "  -h, --help             display help screen\n");
    fprintf(out, "  -v, --version          display version information\n");
    fprintf(out, "  -a, --autostart	       for running by Window Manager's autostart\n\n");
    fprintf(out, "Report bugs to http://bugzilla.altlinux.ru\n");
}


int main( int argc, char **argv )
{

	int autostart = 0; /* run under autostart */

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
	char cfg_path[PATH_MAX];
    }

	return 0;
}
