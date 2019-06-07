/** \file clients/lcdvc/lcdvc.c
 * Main file for \c lcdvc, the LCDproc virtual console.
 */

/*-
 * This file is part of lcdvc, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *               2006-2008, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "getopt.h"

#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"
#include "lcd_link.h"
#include "vc_link.h"
#include "lcdvc.h"

#include "elektragen.h"

char * help_prefix = 
"lcdvc - LCDproc virtual console\n"
"\n"
"Copyright (c) 2002, Joris Robijn, 2006-2008 Peter Marschall.\n"
"This program is released under the terms of the GNU General Public License.\n"
"\n";

/* Variables set by config */
int foreground = FALSE;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;
char *vcsa_device = UNSET_STR;
char *vcs_device = UNSET_STR;
char *keys[4];

/* Other global variables */
char *progname = "lcdvc";
char *pidfile = NULL;
int pidfile_written = FALSE;

int Quit = 0;			/**< indicate end of main loop */

/* Function prototypes */
static void exit_program(int val);
static int process_command_line(int argc, char **argv);
static int process_config ();
static int main_loop(void);


int main(int argc, const char **argv)
{
	// only returns, if not in specload mode
	doSpecloadCheck (argc, argv);

	int e = 0;
	struct sigaction sa;

	CHAIN( e, process_config ());
	set_reporting( progname, report_level, report_dest );

	CHAIN( e, open_vcs() );
	CHAIN( e, setup_connection() );
	CHAIN_END( e );

	if (foreground != TRUE) {
		if (daemon(1,1) != 0) {
			report(RPT_ERR, "Error: daemonize failed");
		}

		if (pidfile != NULL) {
			FILE *pidf = fopen(pidfile, "w");

			if (pidf) {
				fprintf(pidf, "%d\n", (int) getpid());
				fclose(pidf);
				pidfile_written = TRUE;
			} else {
				fprintf(stderr, "Error creating pidfile %s: %s\n",
					pidfile, strerror(errno));
				return(EXIT_FAILURE);
			}
		}
	}

	/* setup signal handlers for common signals */
	sigemptyset(&sa.sa_mask);
#ifdef HAVE_SA_RESTART
	sa.sa_flags = SA_RESTART;
#endif
	sa.sa_handler = exit_program;
	sigaction(SIGINT, &sa, NULL);	// Ctrl-C
	sigaction(SIGTERM, &sa, NULL);	// "regular" kill
	sigaction(SIGHUP, &sa, NULL);	// kill -HUP
	sigaction(SIGPIPE, &sa, NULL);	// write to closed socket
	sigaction(SIGKILL, &sa, NULL);	// kill -9 [cannot be trapped; but ...]

	main_loop();

	exit_program(EXIT_SUCCESS);

	/* NOTREACHED */
	return EXIT_SUCCESS;
}


static void exit_program(int val)
{
	//printf("exit program\n");
	Quit = 1;
	teardown_connection();
	if ((foreground != TRUE) && (pidfile != NULL) && (pidfile_written == TRUE))
		unlink(pidfile);
	exit(val);
}

static void on_fatal_error(ElektraError * error) // TODO (elektra): finalize method
{
	fprintf(stderr, "ERROR: %s\n", elektraErrorDescription(error));
	exit(EXIT_FAILURE);
}

static int process_config()
{
	ElektraError * error = NULL;
	Elektra * elektra = NULL;
	int rc = loadConfiguration(&elektra, &error);

	if (rc == -1)
	{
		fprintf(stderr, "An error occurred while initializing elektra: %s", elektraErrorDescription(error));
		elektraErrorReset(&error);
		return EXIT_FAILURE;
	}

	if (rc == 1)
	{
		// help mode
		printHelpMessage(NULL, help_prefix);
		return EXIT_SUCCESS;
	}

	elektraFatalErrorHandler(elektra, on_fatal_error);

	address = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_ADDRESS));
	port = elektraGet(elektra, ELEKTRA_TAG_LCDVC_PORT);
	report_level = elektraGet(elektra, ELEKTRA_TAG_LCDVC_REPORTLEVEL);
	report_dest = elektraGet(elektra, ELEKTRA_TAG_LCDVC_REPORTTOSYSLOG) ? RPT_DEST_SYSLOG : RPT_DEST_STDERR;
	foreground = elektraGet(elektra, ELEKTRA_TAG_LCDVC_FOREGROUND);
	pidfile = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_PIDFILE));
	
	vcs_device = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_VCSDEVICE));
	vcsa_device = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_VCSADEVICE));

	keys[0] = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_UPKEY));
	keys[1] = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_DOWNKEY));
	keys[2] = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_LEFTKEY));
	keys[3] = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDVC_RIGHTKEY));

	elektraClose(elektra);

	return 0;
}


static int main_loop(void)
{
	int num_bytes;
	char buf[80];
	short w = 0;

	/* Continuously check if we get a menu event... */

	while (!Quit && ((num_bytes = read_response(buf, sizeof(buf)-1)) >= 0)) {
		if (num_bytes != 0) {
			process_response(buf);
		}
		else {
			read_vcdata();
			update_display();

			/* Send an empty line every 3 seconds to make sure the server still exists */
			if (w++ >= 60) {
				w = 0;
				if (send_nop() < 0) {
					break; /* Out of while loop */
				}
			}
			usleep(50000);
		}
	}

	if (!Quit)
		report(RPT_WARNING, "Server disconnected %d", num_bytes);
	return 0;
}

