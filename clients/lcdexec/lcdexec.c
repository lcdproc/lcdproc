/*
 * lcdexec.c
 * This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *
 */

/*
 * How to use submenus in the config file:
 *	[lcdexec]
 *	MenuCommand="Shut down,shutdown -h now"
 *	SubMenu="Internet,inet"
 *	# The inet submenu is filled as follows:
 *	inet_MenuCommand="open,inet open"
 *	inet_MenuCommand="close,inet close"
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"

#define DEFAULT_CONFIGFILE "/etc/LCDclients.conf"

char * help_text =
"lcdexec - LCDproc client to execute commands from the LCDd menu.\n"
"Copyright (c) 2002, Joris Robijn\n"
"This file is released under the GNU General Public License. Refer to the\n"
"COPYING file distributed with this package.\n"
"Options:\n"
"  -c <file>\tSpecify configuration file ["DEFAULT_CONFIGFILE"]\n"
"  -a <address>\tDNS name or IP address of the LCDd server [localhost]\n"
"  -p <port>\tport of the LCDd server [13666]\n"
"  -f <0|1>\tRun in foreground (1) or background (0, default)\n"
"  -r <level>\tSet reporting level (0-5) [2: errors and warnings]\n"
"  -s <0|1>\tReport to syslog (1) or stderr (0, default)\n"
"  -h\t\tShow this help\n";

char * progname = "lcdexec";

extern char *optarg;
extern int optind, optopt, opterr;

/* Variables set by config */
#define UNSET_INT -1
#define UNSET_STR "\01"
char * configfile = UNSET_STR;
char * address = UNSET_STR;
int port = UNSET_INT;
int foreground_mode = UNSET_INT;
char * appname = UNSET_STR;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;

Menu * main_menu;

/* Other variables */
int sock;

/* Function prototypes */
int process_command_line( int argc, char **argv );
int process_configfile( char * configfile );
int split( char * str, char delim, char * parts[], int maxparts );
int connect_and_setup ();
int process_response( char * str );
int exec_command( char * command );
int main_loop ();

#define CHAIN(e,f) { if( e>=0 ) { e=(f); }}
#define CHAIN_END(e) { if( e<0 ) { report( RPT_CRIT,"Critical error, abort"); exit(e); }}

int main( int argc, char **argv )
{
	int error = 0;

	CHAIN( error, process_command_line( argc, argv ));
	if( configfile == NULL )
		configfile = DEFAULT_CONFIGFILE;
	CHAIN( error, process_configfile( configfile ));

	if( report_dest == UNSET_INT || report_level == UNSET_INT ) {
		report_dest = RPT_DEST_STDERR;
		report_level = RPT_ERR;
	}
	set_reporting( progname, report_level, report_dest );
	CHAIN_END( error );

	CHAIN( error, connect_and_setup() );
	CHAIN_END( error );

	if(!foreground_mode) {
		if (daemon(1,1)!=0) {
			report(RPT_ERR, "Error: daemonize failed");
		}
	}

	main_loop();

	return 0;
}

int process_command_line( int argc, char **argv )
{
	char c;
	char * p;
	int temp_int;
	int error = 0;

	/* No error output from getopt */
	opterr = 0;

	while(( c = getopt( argc, argv, "c:O:a:p:f:r:s:h" )) > 0) {
		switch( c ) {
		  case 'c':
			configfile = strdup( optarg );
			break;
		  case 'O':
			config_read_string( progname, optarg );
			break;
		  case 'a':
			address = strdup( optarg );
			break;
		  case 'p':
			temp_int = strtol( optarg, &p, 0 );
			if( *optarg != 0 && *p == 0 ) {
				port = temp_int;
			} else {
				report( RPT_ERR, "Could not interpret value for -%c", c );
				error = -1;
			}
			break;
		  case 'f':
			temp_int = strtol( optarg, &p, 0 );
			if( *optarg != 0 && *p == 0 ) {
				foreground_mode = temp_int;
			} else {
				report( RPT_ERR, "Could not interpret value for -%c", c );
				error = -1;
			}
			break;
		  case 'r':
			temp_int = strtol( optarg, &p, 0 );
			if( *optarg != 0 && *p == 0 ) {
				report_level = temp_int;
			} else {
				report( RPT_ERR, "Could not interpret value for -%c", c );
				error = -1;
			}
			break;
			break;
		  case 's':
			temp_int = strtol( optarg, &p, 0 );
			if( *optarg != 0 && *p == 0 ) {
				report_dest = (temp_int?RPT_DEST_SYSLOG:RPT_DEST_STDERR);
			} else {
				report( RPT_ERR, "Could not interpret value for -%c", c );
				error = -1;
			}
			break;
		  case 'h':
			fprintf( stderr, "%s", help_text );
			exit( 0 );
		  case ':':
			report( RPT_ERR, "Missing option argument for %c", optopt );
			error = -1;
			break;
		  case '?':
			report( RPT_ERR, "Unknown option: %c", optopt );
			error = -1;
			break;
        	}
        }
	return error;
}

int process_configfile( char * configfile )
{
	if( strcmp( configfile, UNSET_STR ) == 0 ) {
		configfile = DEFAULT_CONFIGFILE;
	}
	if( config_read_file( configfile ) < 0 ) {
		report( RPT_WARNING, "Could not read config file: %s", configfile );
	}

	if( strcmp( address, UNSET_STR ) == 0 ) {
		address = strdup( config_get_string( progname, "Address", 0, "localhost" ));
	}
	if( port == UNSET_INT ) {
		port = config_get_int( progname, "Port", 0, 13666 );
	}
	if( report_level == UNSET_INT ) {
		report_level = config_get_int( progname, "ReportLevel", 0, RPT_WARNING );
	}
	if( report_dest == UNSET_INT ) {
		if( config_get_bool( progname, "ReportToSyslog", 0, 0 )) {
			report_dest = RPT_DEST_SYSLOG;
		} else {
			report_dest = RPT_DEST_STDERR;
		}
	}
	if( foreground_mode == UNSET_INT ) {
		foreground_mode = config_get_bool( progname, "Foreground", 0, 1 );
	}

	main_menu = menu_read( "", progname );

	if( main_menu->num_menucmds == 0 && main_menu->num_submenus == 0 ) {
		main_menu->menucmd_name[0] = "echo test";
		main_menu->menucmd_exec[0] = "echo \"This is a test using the echo command.\"";
		main_menu->num_menucmds ++;
	}
	return 0;
}

int connect_and_setup ()
{
	char buf[200];

	report( RPT_INFO, "Connecting to %s:%d", address, port );

	sock = sock_connect( address, port );
	if( sock <= 0 ) {
		return -1;
	}

	/* Create our menu */
	sock_send_string( sock, "hello\n" );
	snprintf( buf, sizeof(buf)-1, "client_set -name \"%s\"\n", progname );
	sock_send_string( sock, buf );

	if( menu_send_to_LCDd( main_menu, "", sock ) < 0 ) {
		return -1;
	}

	return 0;
}

int process_response( char * str )
{
	char *argv[10];
	int argc;
	char * str2 = strdup( str ); /* get_args modifies str2 */

	report( RPT_DEBUG, "Server said: \"%s\"", str );

	/* Check what the server just said to us... */
	argc = get_args( argv, str2, 10 );
	if( argc < 1 ) {
		free( str2 );
		return 0;
	}

	if( strcmp( argv[0], "menuevent" ) == 0 ) {
		/* Ah, this is what we were waiting for ! */

		if( argc < 2 ) {
			report( RPT_WARNING, "Server gave invalid response" );
			free( str2 );
			return -1;
		}
		if( strcmp( argv[1], "select" ) == 0 ) {
			char * exec;
			if( argc < 3 ) {
				report( RPT_WARNING, "Server gave invalid response" );
				free( str2 );
				return -1;
			}

			/* Find the id */
			exec = menu_find_cmd_of_id( main_menu, argv[2] );
			if( !exec ) {
				report( RPT_WARNING, "Could not find the item id given by the server" );
				free( str2 );
				return -1;
			}
			/* The id has been found */
			exec_command( exec );
		}
		else {
			; /* Ignore other menuevents */
		}
	}
	else if( strcmp( argv[0], "huh?" ) == 0 ) {
		/* Report errors */
		report( RPT_WARNING, "Server said: \"%s\"", str );
	}
	else {
		; /* Ignore all other responses */
	}
	free( str2 );
	return 0;
}

int exec_command( char * command )
{
	char *argv[4];

	report( RPT_NOTICE, "Executing: %s", command );

	argv[0] = getenv( "SHELL" );
	argv[1] = "-c";
	argv[2] = command;
	argv[3] = NULL;

	if( !argv[0] ) {
		report( RPT_ERR, "SHELL environment variable not set." );
		return -1;
	}

	switch( fork() ) {
	  case 0:
		/* We're the child. Execute the command. */
		execv( argv[0], argv );
		exit(0);
		break;
	  case -1:
		report( RPT_ERR, "Could not fork" );
		return -1;
	  default:
		/* We're the parent */
	}
	return 0;
}

int main_loop ()
{
	int num_bytes;
	char buf[80];
	int w = 0;

	/* Continuously check if we get a menu event... */

	while(( num_bytes = sock_recv_string( sock, buf, sizeof(buf)-1)) >= 0 ) {
		if( num_bytes == 0 ) {
			usleep( 100000 );

			/* Send an empty line every 3 seconds to make sure the server still exists */
			if( w++ >= 30 ) {
				w = 0;
				if( sock_send_string( sock, "\n" ) < 0 ) {
					break; /* Out of while loop */
				}
			}
		}
		else {
			process_response (buf);
		}
	}

	report( RPT_ERR, "Server disconnected (or connection error)" );
	return 0;
}
