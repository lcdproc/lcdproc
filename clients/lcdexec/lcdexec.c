/*
 * lcdexec.c
 * This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>

#include "getopt.h"

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"

#if !defined(SYSCONFDIR)
# define SYSCONFDIR	"/etc"
#endif

#define DEFAULT_CONFIGFILE	SYSCONFDIR "/lcdexec.conf"


char * help_text =
"lcdexec - LCDproc client to execute commands from the LCDd menu\n"
"\n"
"Copyright (c) 2002, Joris Robijn, 2006 Peter Marschall.\n"
"This program is released under the terms of the GNU General Public License.\n"
"\n"
"Usage: lcdexec [<options>]\n"
"  where <options> are:\n"
"    -c <file>           Specify configuration file ["DEFAULT_CONFIGFILE"]\n"
"    -a <address>        DNS name or IP address of the LCDd server [localhost]\n"
"    -p <port>           port of the LCDd server [13666]\n"
"    -f                  Run in foreground\n"
"    -r <level>          Set reporting level (0-5) [2: errors and warnings]\n"
"    -s <0|1>            Report to syslog (1) or stderr (0, default)\n"
"    -h                  Show this help\n";

char *progname = "lcdexec";

/* Variables set by config */
#define UNSET_INT -1
#define UNSET_STR "\01"
char * configfile = NULL;
char * address = NULL;
int port = UNSET_INT;
int foreground = FALSE;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;
char *displayname = NULL;
char *default_shell = NULL;

MenuEntry *main_menu;

/* Other variables */
int sock = -1;

/* Function prototypes */
int process_command_line(int argc, char **argv);
int process_configfile(char * configfile);
int connect_and_setup();
int process_response(char * str);
int exec_command(MenuEntry *cmd);
int main_loop();


#define CHAIN(e,f) { if (e>=0) { e=(f); }}
#define CHAIN_END(e) { if (e<0) { report(RPT_CRIT,"Critical error, abort"); exit(e); }}


int main(int argc, char **argv)
{
	int error = 0;

	CHAIN(error, process_command_line(argc, argv));
	if (configfile == NULL)
		configfile = DEFAULT_CONFIGFILE;
	CHAIN(error, process_configfile(configfile));

	if (report_dest == UNSET_INT || report_level == UNSET_INT) {
		report_dest = RPT_DEST_STDERR;
		report_level = RPT_ERR;
	}
	set_reporting(progname, report_level, report_dest);
	CHAIN_END(error);

	CHAIN(error, connect_and_setup());
	CHAIN_END(error);

	if(foreground != TRUE) {
		if (daemon(1,1) != 0) {
			report(RPT_ERR, "Error: daemonize failed");
		}
	}

	main_loop();

	return 0;
}

int process_command_line(int argc, char **argv)
{
	int c;
	int error = 0;

	/* No error output from getopt */
	opterr = 0;

	while ((c = getopt(argc, argv, "c:a:p:fr:s:h")) > 0) {
		char *end;
		int temp_int;

		switch(c) {
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
		  case 'h':
			fprintf(stderr, "%s", help_text);
			exit(0);
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

int process_configfile(char *configfile)
{
	const char *tmp;

	if (configfile == NULL)
		configfile = DEFAULT_CONFIGFILE;

	if (config_read_file(configfile) < 0) {
		report(RPT_WARNING, "Could not read config file: %s", configfile);
	}

	if (address == NULL) {
		address = strdup(config_get_string(progname, "Address", 0, "localhost"));
	}
	if (port == UNSET_INT) {
		port = config_get_int(progname, "Port", 0, 13666);
	}
	if (report_level == UNSET_INT) {
		report_level = config_get_int(progname, "ReportLevel", 0, RPT_WARNING);
	}
	if (report_dest == UNSET_INT) {
		report_dest = (config_get_bool(progname, "ReportToSyslog", 0, 0))
				? RPT_DEST_SYSLOG
				: RPT_DEST_STDERR;
	}
	if (foreground != TRUE) {
		foreground = config_get_bool(progname, "Foreground", 0, FALSE);
	}

	if ((tmp = config_get_string(progname, "DisplayName", 0, NULL)) != NULL)
		displayname = strdup(tmp);

	/* try to find a shell that understands the -c COMMAND syntax */
	if ((tmp = config_get_string(progname, "Shell", 0, NULL)) != NULL)
		default_shell = strdup(tmp);
	else {
		/* 1st fallback: SHELL environment variable */
		report(RPT_WARNING, "Shell not set in configuration, falling back to variable SHELL");
		default_shell = getenv("SHELL");

		/* 2nd fallback: /bin/sh */
		if (default_shell == NULL) {
			report(RPT_WARNING, "variable SHELL not set, falling back to /bin/sh");
			default_shell = "/bin/sh";
		}
	}

	main_menu = menu_read(NULL, "MainMenu");
#if defined(DEBUG)
	menu_dump(main_menu);
#endif

	// fail on non-existent main menu;
	if (main_menu == NULL) {
		report(RPT_ERR, "no main menu found in configuration");
		return -1;
	}	

	return 0;
}

int connect_and_setup()
{
	report(RPT_INFO, "Connecting to %s:%d", address, port);

	sock = sock_connect(address, port);
	if (sock < 0) {
		return -1;
	}

	/* init connection and set client name */
	sock_send_string(sock, "hello\n");
	if (displayname != NULL) {
		sock_printf(sock, "client_set -name {%s}\n", displayname);
	}
	else {
		struct utsname unamebuf;
	
		if (uname(&unamebuf) == 0)
			sock_printf(sock, "client_set -name {%s %s}\n", progname, unamebuf.nodename);
		else		
			sock_printf(sock, "client_set -name {%s}\n", progname);
	}	

	/* Create our menu */
	if (menu_sock_send(main_menu, NULL, sock) < 0) {
		return -1;
	}

	return 0;
}

int process_response(char *str)
{
	char *argv[15];
	int argc;
	char *str2 = strdup(str); /* get_args modifies str2 */

	report(RPT_DEBUG, "Server said: \"%s\"", str);

	/* Check what the server just said to us... */
	argc = get_args(argv, str2, 10);
	if (argc < 1) {
		free(str2);
		return 0;
	}

	if (strcmp(argv[0], "menuevent") == 0) {
		/* Ah, this is what we were waiting for ! */

		if (argc < 2) {
			report(RPT_WARNING, "Server gave invalid response");
			free(str2);
			return -1;
		}
		if (strcmp(argv[1], "select") == 0) {
			MenuEntry *exec;
			
			if (argc < 3) {
				report(RPT_WARNING, "Server gave invalid response");
				free(str2);
				return -1;
			}

			/* Find the id */
			exec = menu_find_by_id(main_menu, atoi(argv[2]));
			if (exec == NULL) {
				report(RPT_WARNING, "Could not find the item id given by the server");
				free(str2);
				return -1;
			}
			/* The id has been found */
			exec_command(exec);
		}
		else {
			; /* Ignore other menuevents */
		}
	}
	else if (strcmp(argv[0], "huh?") == 0) {
		/* Report errors */
		report(RPT_WARNING, "Server said: \"%s\"", str);
	}
	else {
		; /* Ignore all other responses */
	}
	free(str2);
	return 0;
}

int exec_command(MenuEntry *cmd)
{
	if ((cmd != NULL)  && (menu_command(cmd) != NULL)) {
		const char *command = menu_command(cmd);
		const char *argv[4];

		report(RPT_NOTICE, "Executing: %s", command);

		argv[0] = default_shell;
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = NULL;

		switch (fork()) {
		  case 0:
			/* We're the child. Execute the command. */
			execv(argv[0], (char **) argv);
			exit(0);
			break;
		  case -1:
			report(RPT_ERR, "Could not fork");
			return -1;
		  default:
			/* We're the parent */
	        	break;
		}
		return 0;
	}
	return -1;
}

int main_loop ()
{
	int num_bytes;
	char buf[100];
	int w = 0;

	/* Continuously check if we get a menu event... */

	while ((num_bytes = sock_recv_string(sock, buf, sizeof(buf)-1)) >= 0) {
		if (num_bytes == 0) {
			usleep(100000);

			/* Send an empty line every 3 seconds to make sure the server still exists */
			if (w++ >= 30) {
				w = 0;
				if (sock_send_string(sock, "\n") < 0) {
					break; /* Out of while loop */
				}
			}
		}
		else {
			process_response (buf);
		}
	}

	report(RPT_ERR, "Server disconnected (or connection error)");
	return 0;
}
