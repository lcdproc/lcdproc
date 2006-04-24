/*
 * lcdvc.c
 * This file is part of lcdvc, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
extern char *optarg;
extern int optind, optopt, opterr;

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"
#include "lcd_link.h"
#include "vc_link.h"
#include "lcdvc.h"

#define DEFAULT_CONFIGFILE	SYSCONFDIR "/lcdvc.conf"

char * help_text =
"lcdvc - LCDproc virtual console displayer.\n"
"Copyright (c) 2002, Joris Robijn\n"
"This file is released under the GNU General Public License. Refer to the\n"
"COPYING file distributed with this package.\n"
"Options:\n"
"  -c <file>\tSpecify a configfile to load ["DEFAULT_CONFIGFILE"]\n"
"  -a <address>\tDNS name or IP address of the LCDd server [localhost]\n"
"  -p <port>\tPort of the LCDd server [13666]\n"
"  -f <0|1>\tRun in 1=foreground or 0=background\n"
"  -r <level>\tSet reporting level (0-5) [2: errors and warnings]\n"
"  -s <0|1>\tReport to 1=syslog or 0=stderr\n"
"  -h\t\tShow this help\n";

char *progname = "lcdvc";
char *configfile = UNSET_STR;

/* Variables set by config */
int foreground_mode = UNSET_INT;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;
char *vcsa_device = UNSET_STR;
char *vcs_device = UNSET_STR;
char *keys[4];

/* Function prototypes */
int process_command_line(int argc, char ** argv);
int process_configfile(char * configfile);
int connect_and_setup();
int update_display();
int process_response(char * str);
int main_loop();

int main( int argc, char ** argv )
{
	int e = 0;

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
	CHAIN( e, connect_and_setup() );
	CHAIN_END( e );

	if (!foreground_mode) {
		if (daemon(1,1) != 0) {
			report(RPT_ERR, "Error: daemonize failed");
		}
	}

	main_loop();

	return 0;
}

int process_command_line( int argc, char ** argv )
{
	char c;
	char * p;
	int temp_int;
	int error = 0;

	/* No error output from getopt */
	opterr = 0;

	while ((c = getopt(argc, argv, "hc:a:p:f:r:s:")) > 0) {
		switch (c) {
		  case 'h':
			fprintf(stderr, "%s", help_text);
			exit( 0 );
		  case 'c':
			configfile = strdup(optarg);
			break;
		  case 'a':
			address = strdup(optarg);
			break;
		  case 'p':
			temp_int = strtol(optarg, &p, 0);
			if ( *optarg != 0 && *p == 0) {
				port = temp_int;
			} else {
				report(RPT_ERR, "Could not interpret value for -%c", c);
				error = -1;
			}
			break;
		  case 'f':
			temp_int = strtol(optarg, &p, 0);
			if (*optarg != 0 && *p == 0) {
				foreground_mode = temp_int;
			} else {
				report(RPT_ERR, "Could not interpret value for -%c", c);
				error = -1;
			}
			break;
		  case 'r':
			temp_int = strtol(optarg, &p, 0);
			if (*optarg != 0 && *p == 0 ) {
				report_level = temp_int;
			} else {
				report(RPT_ERR, "Could not interpret value for -%c", c);
				error = -1;
			}
			break;
		  case 's':
			temp_int = strtol( optarg, &p, 0 );
			if (*optarg != 0 && *p == 0 ) {
				report_dest = (temp_int?RPT_DEST_SYSLOG:RPT_DEST_STDERR);
			} else {
				report(RPT_ERR, "Could not interpret value for -%c", c);
				error = -1;
			}
			break;
		  case ':':
			report(RPT_ERR, "Missing option argument for %c", optopt);
			error = -1;
			break;
		  case '?':
			report(RPT_ERR, "Unknown option: %c", optopt);
			error = -1;
			break;
        	}
        }
	return error;
}

int process_configfile( char * configfile )
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
	if (foreground_mode == UNSET_INT) {
		foreground_mode = config_get_bool(progname, "Foreground", 0, 1);
	}
	vcs_device = strdup(config_get_string(progname, "vcsDevice", 0, "/dev/vcs"));
	vcsa_device = strdup(config_get_string(progname, "vcsaDevice", 0, "/dev/vcsa"));

	keys[0] = strdup( config_get_string( progname, "UpKey", 0, "Up" ));
	keys[1] = strdup( config_get_string( progname, "DownKey", 0, "Down" ));
	keys[2] = strdup( config_get_string( progname, "LeftKey", 0, "Left" ));
	keys[3] = strdup( config_get_string( progname, "RightKey", 0, "Right" ));

	return 0;
}

int split( char * str, char delim, char * parts[], int maxparts )
/* Splits a string into parts, to which pointers will be returned in &parts.
 * The return value is the number of parts.
 * maxparts is the maximum number of parts returned. If more parts exist
 * they are (unsplit) in the last part.
 * The parts are split at the character delim.
 * No new space will be allocated, the string str will be mutated !
 */
{
	char * p1 = str;
	char * p2;
	int part_nr = 0;

	/* Find the delim char to end the current part */
	while (part_nr < maxparts - 1 && (p2 = strchr(p1, delim))) {

		/* subsequent parts... */
		*p2 = 0;
		parts[part_nr] = p1;

		p1 = p2 + 1; /* Just after the delim char */
		part_nr ++;
	}
	/* and the last part... */
	parts[part_nr] = p1;
	part_nr ++;

	return part_nr;
}

int main_loop()
{
	int num_bytes;
	char buf[80];
	short w;

	/* Continuously check if we get a menu event... */

	while ((num_bytes = read_response(buf, sizeof(buf)-1)) >= 0) {
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

	report(RPT_WARNING, "Server disconnected %d", num_bytes);
	return 0;
}

