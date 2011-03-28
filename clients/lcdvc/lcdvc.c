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

#if !defined(SYSCONFDIR)
# define SYSCONFDIR	"/etc"
#endif
#if !defined(PIDFILEDIR)
# define PIDFILEDIR	"/var/run"
#endif

#define DEFAULT_CONFIGFILE	SYSCONFDIR "/lcdvc.conf"
#define DEFAULT_PIDFILE		PIDFILEDIR "/lcdvc.pid"


char *help_text =
"lcdvc - LCDproc virtual console\n"
"\n"
"Copyright (c) 2002, Joris Robijn, 2006-2008 Peter Marschall.\n"
"This program is released under the terms of the GNU General Public License.\n"
"\n"
"Usage: lcdvc [<options>]\n"
"  where <options> are:\n"
"    -c <file>           Specify configuration file ["DEFAULT_CONFIGFILE"]\n"
"    -a <address>        DNS name or IP address of the LCDd server [localhost]\n"
"    -p <port>           port of the LCDd server [13666]\n"
"    -f                  Run in foreground\n"
"    -r <level>          Set reporting level (0-5) [2: errors and warnings]\n"
"    -s <0|1>            Report to syslog (1) or stderr (0, default)\n"
"    -h                  Show this help\n";

/* Variables set by config */
int foreground = FALSE;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;
char *vcsa_device = UNSET_STR;
char *vcs_device = UNSET_STR;
char *keys[4];

/* Other global variables */
char *progname = "lcdvc";
char *configfile = UNSET_STR;
char *pidfile = NULL;
int pidfile_written = FALSE;

int Quit = 0;			/**< indicate end of main loop */

/* Function prototypes */
static void exit_program(int val);
static int process_command_line(int argc, char **argv);
static int process_configfile(char *configfile);
static int main_loop(void);


int main(int argc, char **argv)
{
	int e = 0;
	struct sigaction sa;

	CHAIN( e, process_command_line( argc, argv ));
	if (strcmp( configfile, UNSET_STR ) == 0) {
		configfile = DEFAULT_CONFIGFILE;
	}
	CHAIN( e, process_configfile( configfile ));
	if (report_dest == UNSET_INT || report_level == UNSET_INT) {
		report_dest = RPT_DEST_STDERR;
		report_level = RPT_ERR;
	}
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


static int process_command_line(int argc, char **argv)
{
	int c;
	int error = 0;

	/* No error output from getopt */
	opterr = 0;

	while ((c = getopt(argc, argv, "hc:a:p:fr:s:")) > 0) {
		char *end;
		int temp_int;

		switch (c) {
		  case 'h':
			fprintf(stderr, "%s", help_text);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		  case 'c':
			configfile = strdup(optarg);
			break;
		  case 'a':
			address = strdup(optarg);
			break;
		  case 'p':
			temp_int = strtol(optarg, &end, 0);
			if ((*optarg != '\0') && (*end == '\0') &&
			     (temp_int > 0) && (temp_int <= 0xFFFF)) {
				port = temp_int;
			} else {
				report(RPT_ERR, "Illegal port value %s", optarg);
				error = -1;
			}
			break;
		  case 'f':
			foreground = TRUE;
			break;
		  case 'r':
			temp_int = strtol(optarg, &end, 0);
			if ((*optarg != '\0') && (*end == '\0') && (temp_int >= 0)) {
				report_level = temp_int;
			} else {
				report(RPT_ERR, "Illegal report level value %s", optarg);
				error = -1;
			}
			break;
		  case 's':
			temp_int = strtol(optarg, &end, 0);
			if ((*optarg != '\0') && (*end == '\0') && (temp_int >= 0)) {
				report_dest = (temp_int ? RPT_DEST_SYSLOG : RPT_DEST_STDERR);
			} else {
				report(RPT_ERR, "Illegal log destination value %s", optarg);
				error = -1;
			}
			break;
		  case ':':
			report(RPT_ERR, "Missing option argument for %c", optopt);
			error = -1;
			break;
		  case '?':
		  default:
			report(RPT_ERR, "Unknown option: %c", optopt);
			error = -1;
			break;
        	}
        }
	return error;
}


static int process_configfile(char *configfile)
{
	if (strcmp(configfile, UNSET_STR) == 0) {
		configfile = DEFAULT_CONFIGFILE;
	}
	if (config_read_file( configfile ) < 0) {
		report( RPT_WARNING, "Could not read config file: %s", configfile);
	}

	if (strcmp(address, UNSET_STR ) == 0) {
		address = strdup(config_get_string( progname, "Address", 0, "localhost"));
	}
	if (port == UNSET_INT) {
		port = config_get_int(progname, "Port", 0, 13666);
	}
	if (report_level == UNSET_INT ) {
		report_level = config_get_int(progname, "ReportLevel", 0, RPT_WARNING);
	}
	if (report_dest == UNSET_INT) {
		if (config_get_bool(progname, "ReportToSyslog", 0, 0)) {
			report_dest = RPT_DEST_SYSLOG;
		} else {
			report_dest = RPT_DEST_STDERR;
		}
	}
	if (foreground != TRUE) {
		foreground = config_get_bool(progname, "Foreground", 0, FALSE);
	}
	if (pidfile == NULL) {
		pidfile = strdup(config_get_string(progname, "PidFile", 0, DEFAULT_PIDFILE));
	}

	vcs_device = strdup(config_get_string(progname, "vcsDevice", 0, "/dev/vcs"));
	vcsa_device = strdup(config_get_string(progname, "vcsaDevice", 0, "/dev/vcsa"));

	keys[0] = strdup(config_get_string( progname, "UpKey", 0, "Up"));
	keys[1] = strdup(config_get_string( progname, "DownKey", 0, "Down"));
	keys[2] = strdup(config_get_string( progname, "LeftKey", 0, "Left"));
	keys[3] = strdup(config_get_string( progname, "RightKey", 0, "Right"));

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

