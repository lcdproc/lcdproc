/*
 * main.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *               2001, Rene Wagner
 *               2002, Mike Patnode
 *               2002, Guillaume Filion
 *
 *
 * Contains main(), plus signal callback functions and a help screen.
 *
 * Program init, command-line handling, and the main loop are
 * implemented here.  Also, minimal data about the program such as
 * the revision number.
 *
 * Some of this stuff should probably be move elsewhere eventually,
 * such as command-line handling and the main loop.  main() is supposed
 * to be "dumb".
 *
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
/* TODO: fill in what to include otherwise */

extern char *optarg;
extern int optind, optopt, opterr;

#include "shared/report.h"

#include "drivers.h"
#include "sock.h"
#include "clients.h"
#include "screenlist.h"
#include "screen.h"
#include "parse.h"
#include "render.h"
#include "serverscreens.h"
#include "menuscreens.h"
#include "input.h"
#include "shared/configfile.h"
#include "drivers.h"
#include "main.h"

#define DEFAULT_BIND_ADDR "127.0.0.1"
#define DEFAULT_BIND_PORT LCDPORT
#define DEFAULT_CONFIGFILE "/etc/LCDd.conf"
#define DEFAULT_USER "nobody"
#define DEFAULT_DRIVER "curses"
#define DEFAULT_DRIVER_PATH
#define MAX_DRIVERS 8
#define DEFAULT_FOREGROUND_MODE 0
#define DEFAULT_ROTATE_SERVER_SCREEN 1
#define DEFAULT_REPORTTOSYSLOG 0
#define DEFAULT_REPORTLEVEL RPT_WARNING

#define DEFAULT_SCREEN_DURATION 32
#define DEFAULT_HEARTBEAT HEARTBEAT_ON

/* Socket to bind to...

   Using loopback is much more secure; it means that this port is
   accessible only to programs running locally on the same host as LCDd.

   Using variables for these means that (later) we can select which port
   and which address to bind to at run time. */


/* Store some standard defines into vars... */
char *version = VERSION;
char *protocol_version = PROTOCOL_VERSION;
char *api_version = API_VERSION;
char *build_date = __DATE__;


/**** Configuration variables ****/
/* Some variables are settable on the command line. This are variables that
 * change the mode of operation. This includes settings that you can use to
 * enable debugging: driver selection, report settings, bind address etc.
 * These variables should be in main.h and main.c (below).
 *
 * All other settings do not need to be settable from the command line. They
 * also do not necesarily need to be read in main.c but can better be read in
 * in the file concerned.
 */

unsigned int bind_port = UNSET_INT;
char bind_addr[64];	/* Do not preinit these strings as they will occupy */
char configfile[256];	/* a lot of space in the executable. */
char user[64];		/* The values will be overwritten anyway... */

int foreground_mode = UNSET_INT;

static int report_level = UNSET_INT;
static int report_to_syslog = UNSET_INT;

/* The drivers and their driver parameters */
char *drivernames[MAX_DRIVERS];
int num_drivers = 0;

/* End of configuration variables */

/* Local variables */
int stored_argc;
char **stored_argv;
short got_reload_signal = 0;

/* Local exported variables */
long int timer = 0;

/**** Local functions ****/
void clear_settings();
int process_command_line (int argc, char **argv);
int process_configfile (char *cfgfile);
void set_default_settings();
void install_signal_handlers (int allow_reload);
void child_ok_func (int signal);
pid_t daemonize();
int wave_to_parent (pid_t parent_pid);
int init_drivers();
int drop_privs(char *user);
int init_screens();
void do_reload();
void do_mainloop();
void exit_program (int val);
void catch_reload_signal (int val);
int interpret_boolean_arg (char *s);
void output_help_screen ();
void output_GPL_notice();

#define CHAIN(e,f) { if( e>=0 ) { e=(f); }}
#define CHAIN_END(e,msg) { if( e<0 ) { report( RPT_CRIT,(msg)); exit(e); }}

int
main (int argc, char **argv)
{
	int e = 0;
	pid_t parent_pid = 0;

	stored_argc = argc;
	stored_argv = argv;

	/*
	 * Settings in order of preference:
	 *
	 * 1: Settings specified in command line options...
	 * 2: Settings specified in configuration file...
	 * 3: Default settings
	 *
	 * Because of this, and because one option (-c) specifies where
	 * the configuration file is, things are done in this order:
	 *
	 * 1. Read and set options.
	 * 2. Read configuration file; if option is read in configuration
	 *    file and not already set, then set it.
	 * 3. Having read configuration file, if parameter is not set,
	 *    set it to the default value.
	 *
	 * It is for this reason that the default values are **NOT** set
	 * in the variable declaration...
	 */

	/* Report that server is starting (report will be delayed) */
	report(RPT_NOTICE, "LCDd version %s starting", version );
	report(RPT_INFO, "Built on %s, protocol version %s, API version %s",
		build_date, protocol_version, api_version );

	clear_settings();

	/* Read command line*/
	CHAIN( e, process_command_line (argc, argv) );

	/* Read config file
	 * If config file was not given on command line use default */
	if (strcmp(configfile, UNSET_STR)==0)
		strncpy (configfile, DEFAULT_CONFIGFILE, sizeof(configfile));
	CHAIN( e, process_configfile (configfile) );

	/* Set default values*/
	set_default_settings();

	/* Set reporting settings (will also flush delayed reports) */
	set_reporting( "LCDd", report_level, (report_to_syslog?RPT_DEST_SYSLOG:RPT_DEST_STDERR) );
 	report( RPT_INFO, "Set report level to %d, output to %s", report_level, (report_to_syslog?"syslog":"stderr") );
	CHAIN_END( e, "Critical error while processing settings, abort." );

	/* Now, go into daemon mode (if we should)...
	 * We wait for the child to report it is running OK. This mechanism
	 * is used because forking after starting the drivers causes the
	 * child to loose the (LPT) port access. */
	if (!foreground_mode) {
		report(RPT_INFO, "Server forking to background");
		CHAIN( e, parent_pid = daemonize() );
	} else {
		output_GPL_notice();
		report(RPT_INFO, "Server running in foreground");
	}
	install_signal_handlers (!foreground_mode);
		/* Only catch SIGHUP if not in foreground mode */

	/* Startup the subparts of the server */
	CHAIN( e, sock_init() );
	CHAIN( e, screenlist_init() );
	CHAIN( e, init_drivers() );
	CHAIN( e, clients_init() );
	CHAIN( e, input_init() );
	CHAIN( e, menuscreens_init() );
	CHAIN( e, server_screen_init() );
	CHAIN_END( e, "Critical error while initializing, abort." );
	if (!foreground_mode) {
		/* Tell to parent that startup went OK. */
		wave_to_parent (parent_pid);
	}
	drop_privs(user); /* This can't be done before, because sending a
			signal to a process of a different user will fail */

	do_mainloop();
	/* This loop never stops; we'll get out only with a signal...*/

	return 0;
}


void
clear_settings ()
{
	int i;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	bind_port = UNSET_INT;
	strncpy( bind_addr, UNSET_STR, sizeof(bind_addr) );
	strncpy( configfile, UNSET_STR, sizeof(configfile) );
	strncpy( user, UNSET_STR, sizeof(user) );
	foreground_mode = UNSET_INT;
	rotate_server_screen = UNSET_INT;
	backlight = UNSET_INT;

	default_duration = UNSET_INT;
	report_to_syslog = UNSET_INT;
	report_level = UNSET_INT;

	for( i=0; i < num_drivers; i++ ) {
		free( drivernames[i] );
		drivernames[i] = NULL;
	}
	num_drivers = 0;
}


/* parses arguments given on command line */
int
process_command_line (int argc, char **argv)
{
	int c, b;
	int e = 0, help = 0;

	debug( RPT_DEBUG, "%s( argc=%d, argv=...)", __FUNCTION__, argc );

	/* Reset getopt */
	optind = 0;
	opterr = 0; /* Prevent some messages to stderr */

	/* Analyze options here.. (please try to keep list of options the
	 * same everywhere) */
	while ((c = getopt(argc, argv, "hc:d:f:a:p:u:w:s:r:i:" )) > 0) {
		switch(c) {
			case 'h':
				help = 1; /* Continue to process the other
					   * options */
				break;
			case 'c':
				strncpy(configfile, optarg, sizeof(configfile));
				configfile[sizeof(configfile)-1] = 0; /* Terminate string */
				break;
	 		case 'd':
				/* Add to a list of drivers to be initialized later...*/
				if (num_drivers < MAX_DRIVERS) {
					drivernames[num_drivers] = malloc( strlen(optarg)+1 );
					strcpy( drivernames[num_drivers], optarg );
					num_drivers ++;
				} else {
					report( RPT_ERR, "Too many drivers!" );
					e = -1;
				}
				break;
			case 'f':
				b = interpret_boolean_arg( optarg );
				if( b == -1 ) {
					report( RPT_ERR, "Not a boolean value: '%s'", optarg );
					e = -1;
				} else {
					foreground_mode = b;
				}
				break;
			case 'a':
				strncpy(bind_addr, optarg, sizeof(bind_addr));
				bind_addr[sizeof(bind_addr)-1] = 0; /* Terminate string */
				break;
			case 'p':
				bind_port = atoi(optarg);
				break;
			case 'u':
				strncpy(user, optarg, sizeof(user));
				user[sizeof(user)-1] = 0; /* Terminate string */
				break;
			case 'w':
				default_duration = (int) (atof(optarg) * 1e6 / TIME_UNIT);
				if ( default_duration * TIME_UNIT < 2e6 ) {
					report( RPT_ERR, "Waittime should be at least 2 (seconds), not %.8s", optarg );
					e = -1;
				}
				break;
			case 's':
				b = interpret_boolean_arg( optarg );
				if( b == -1 ) {
					report( RPT_ERR, "Not a boolean value: '%s'", optarg );
					e = -1;
				} else {
					report_to_syslog = b;
				}
				break;
			case 'r':
				report_level = atoi(optarg);
				break;
			case 'i':
				b = interpret_boolean_arg( optarg );
				if( b == -1 ) {
					report( RPT_ERR, "Not a boolean value: '%s'", optarg );
					e = -1;
				} else {
					rotate_server_screen = b;
				}
				break;
			case '?':
				/* For some reason getopt also returns an '?'
				 * when an option argument is mission... */
				report( RPT_ERR, "Unknown option: '%c'", optopt );
				e = -1;
				break;
			case ':':
				report( RPT_ERR, "Missing option argument!" );
				e = -1;
				break;
		}
	}

	if (optind < argc) {
		report( RPT_ERR, "Non-option arguments on the command line !");
		e = -1;
	}
	if( help ) {
		output_help_screen();
		e = -1;
	}
	return e;
}


/* reads and parses configuration file */
int
process_configfile ( char *configfile )
{
	char * s;
	/*char buf[64];*/

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	/* Read server settings*/

	if( config_read_file( configfile ) != 0 ) {
		report( RPT_WARNING, "Could not read config file: %s", configfile );
	}

	if( bind_port == UNSET_INT )
		bind_port = config_get_int( "server", "port", 0, UNSET_INT );

	if( strcmp( bind_addr, UNSET_STR ) == 0 )
		strncpy( bind_addr, config_get_string( "server", "bind", 0, UNSET_STR ), sizeof(bind_addr));

	if( strcmp( user, UNSET_STR ) == 0 )
		strncpy( user, config_get_string( "server", "user", 0, UNSET_STR ), sizeof(user));

	if( default_duration == UNSET_INT ) {
		default_duration = (config_get_float( "server", "waittime", 0, 0 ) * 1e6 / TIME_UNIT );
		if( default_duration == 0 )
			default_duration = UNSET_INT;
		else if( default_duration * TIME_UNIT < 2e6 ) {
			report( RPT_WARNING, "Waittime should be at least 2 (seconds). Set to 2 seconds." );
			default_duration = 2e6 / TIME_UNIT;
		}
	}

	if( foreground_mode == UNSET_INT ) {
		int fg;
		fg = config_get_bool( "server", "foreground", 0, UNSET_INT );
		if( fg != UNSET_INT )
			foreground_mode = fg;
	}

	if( rotate_server_screen == UNSET_INT ) {
		rotate_server_screen = config_get_bool( "server", "serverscreen", 0, UNSET_INT );
	}

	if( backlight == UNSET_INT ) {
		s = config_get_string( "server", "backlight", 0, UNSET_STR );
		if( strcmp( s, "on" ) == 0 ) {
			backlight = BACKLIGHT_ON;
		}
		else if( strcmp( s, "off" ) == 0 ) {
			backlight = BACKLIGHT_OFF;
		}
		else if( strcmp( s, "open" ) == 0 ) {
			backlight = BACKLIGHT_OPEN;
		}
		else if( strcmp( s, UNSET_STR ) != 0 ) {
			report( RPT_WARNING, "Backlight state should be on, off or open" );
		}
	}

	if( report_to_syslog == UNSET_INT ) {
		report_to_syslog = config_get_bool( "server", "reportToSyslog", 0, UNSET_INT );
	}
	if( report_level == UNSET_INT ) {
		report_level = config_get_int( "server", "reportLevel", 0, UNSET_INT );
	}


	/* Read drivers*/

	 /* If drivers have been specified on the command line, then do not
	 * use the driver list from the config file.
	 */
	if( num_drivers == 0 ) {
		/* read the drivernames*/

		while( 1 ) {
			s = config_get_string( "server", "driver", num_drivers, NULL );
			if( !s )
				break;
			if( s[0] != 0 ) {
				drivernames[num_drivers] = malloc(strlen(s)+1);
				strcpy( drivernames[num_drivers], s );
				num_drivers++;
			}
		}
	}

	return 0;
}


void
set_default_settings()
{
	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	/* Set defaults into unfilled variables....*/

	if (bind_port == UNSET_INT)
		bind_port = DEFAULT_BIND_PORT;
	if (strcmp( bind_addr, UNSET_STR ) == 0)
		strncpy (bind_addr, DEFAULT_BIND_ADDR, sizeof(bind_addr));
	if (strcmp( user, UNSET_STR ) == 0)
		strncpy(user, DEFAULT_USER, sizeof(user));

	if (foreground_mode == UNSET_INT)
		foreground_mode = DEFAULT_FOREGROUND_MODE;
	if (rotate_server_screen == UNSET_INT)
		rotate_server_screen = DEFAULT_ROTATE_SERVER_SCREEN;

	if (default_duration == UNSET_INT)
		default_duration = DEFAULT_SCREEN_DURATION;
	if (backlight == UNSET_INT)
		backlight = BACKLIGHT_OPEN;

	if (report_to_syslog == UNSET_INT )
		report_to_syslog = DEFAULT_REPORTTOSYSLOG;
	if( report_level == UNSET_INT )
		report_level = DEFAULT_REPORTLEVEL;


	/* Use default driver*/
	if( num_drivers == 0 ) {
		drivernames[0] = malloc(strlen(DEFAULT_DRIVER)+1);
		strcpy(drivernames[0], DEFAULT_DRIVER);
		num_drivers = 1;
	}
}


void
install_signal_handlers (int allow_reload)
{
	/* Installs signal handlers so that the program does clean exit and
	 * can also receive a reload signal.
	 * sigaction() is favoured over signal() */

	struct sigaction sa;

	debug( RPT_DEBUG, "%s( allow_reload=%d )", __FUNCTION__, allow_reload );

	sa.sa_handler = exit_program;
	sigemptyset ( &(sa.sa_mask) );
	sa.sa_flags = SA_RESTART;

	sigaction (SIGINT, &sa, NULL);		/* Ctrl-C will cause a clean exit...*/
	sigaction (SIGTERM, &sa, NULL);		/* and "kill"...*/

	if (allow_reload) {
		sa.sa_handler = catch_reload_signal;
		/* On SIGHUP reread config and restart the drivers ! */
	}
	else {
		/* Treat this signal just like INT and TERM */
	}
	sigaction (SIGHUP, &sa, NULL);
}


void
child_ok_func (int signal) {
	/* We only catch this signal to be sure the child runs OK. */

	debug( RPT_INFO, "%s( signal=%d )", __FUNCTION__, signal );

	/* Exit now !    because of bug? in wait() */
	_exit( 0 ); /* Parent exits normally. */
}


pid_t
daemonize()
{
	pid_t child;
	pid_t parent;
	int child_status;
	struct sigaction sa;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	parent = getpid();
	debug( RPT_INFO, "parent = %d", parent );

	/* Install handler at parent for child's signal */
	/* sigaction should be more portable than signal, but it does not
	 * work for some reason. */
	sa.sa_handler = child_ok_func;
	sigemptyset ( &(sa.sa_mask) );
	sa.sa_flags = SA_RESTART;
	sigaction (SIGUSR1, &sa, NULL);

	/* Do the fork */
	switch ((child = fork ()) ) {
	  case -1:
		report( RPT_ERR, "Could not fork" );
		return -1;
	  case 0: /* We are the child */
		break;
	  default: /* We are the parent */
		debug( RPT_INFO, "child = %d", child );
		wait( &child_status );
		/* BUG? According to the man page wait() should also return
		 * when a signal comes in that is caught. Instead it
		 * continues to wait. */

		if( WIFEXITED(child_status) ) {
			/* Child exited normally, probably because of some
			 * error. */
			debug( RPT_INFO, "Child has terminated!" );
			exit( WEXITSTATUS(child_status) );
			/* Parent exits with same status as child did... */
		}
		/* Child is still running and has signalled it's OK.
		 * This means the parent can now rest in peace. */
		debug( RPT_INFO, "Got OK signal from child." );
		exit( 0 ); /* Parent exits normally. */
	}
	/* At this point we are always the child. */
	/* Reset signal handler */
	sa.sa_handler = SIG_DFL;
	sigaction (SIGUSR1, &sa, NULL);

	setsid();	/* Create a new session because otherwise we'll
			 * catch a SIGHUP when the shell is closed. */

	return parent;
}


int
wave_to_parent (pid_t parent_pid)
{
	debug( RPT_DEBUG, "%s( parent_pid=%d )", __FUNCTION__, parent_pid );

	kill( parent_pid, SIGUSR1 );

	return 0;
}


int
init_drivers()
{
	int i, res;

	int output_loaded = 0;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	for (i = 0; i < num_drivers; i++) {

		res = drivers_load_driver (drivernames[i]);
		if (res >= 0) {
			/* Load went OK */

			switch( res ) {
			  case 0: /* Driver does input only */
			  	break;
			  case 1: /* Driver does output */
			  	output_loaded = 1;
			  	break;
			  case 2: /* Driver does output in foreground (don't daemonize) */
			  	foreground_mode = 1;
			  	output_loaded = 1;
			  	break;
			}
		} else {
			report( RPT_ERR, "Could not load driver %.40s", drivernames[i] );
		}
	}

	/* Do we have a running output driver ?*/
	if ( output_loaded ) {
		return 0;
	} else {
		report( RPT_ERR, "There is no output driver" );
		return -1;
	}
}


int drop_privs(char *user)
{
	struct passwd *pwent;

	debug( RPT_DEBUG, "%s( user=\"%.40s\" )", __FUNCTION__, user );

	if (getuid() == 0 || geteuid() == 0) {
		if ((pwent = getpwnam(user)) == NULL) {
			report( RPT_ERR, "User %.40s not a valid user!", user );
			return -1;
		} else {
			if (setuid(pwent->pw_uid) < 0) {
				report( RPT_ERR, "Unable to switch to user %.40s", user );
				return -1;
			}
		}
	}
	return 0;
}


int
init_screens ()
{
	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	if (screenlist_init () < 0) {
		report(RPT_ERR, "Error initializing screen list");
		return -1;
	}

	/* Add the server screen */
	if (server_screen_init () < 0) {
		report(RPT_ERR, "Error initializing server screens");
		return -1;
	}
	if (!rotate_server_screen) {
		/* Display the server screen only when there are no other
		 * screens */
		/* server_screen->priority = PRI_BACKGROUND; */
	}
	return 0;
}

void
do_reload ()
{
	int e = 0;

	drivers_unload_all ();		/* Close all drivers */

	config_clear();
	clear_settings();

	/* Reread command line*/
	CHAIN( e, process_command_line (stored_argc, stored_argv) );

	/* Reread config file */
	if (strcmp(configfile, UNSET_STR)==0)
		strncpy (configfile, DEFAULT_CONFIGFILE, sizeof(configfile));
	CHAIN( e, process_configfile (configfile) );

	/* Set default values */
	CHAIN( e, ( set_default_settings(), 0 ));

	/* Set reporting values */
	CHAIN( e, set_reporting( "LCDd", report_level, (report_to_syslog?RPT_DEST_SYSLOG:RPT_DEST_STDERR) ) );
 	CHAIN( e, ( report( RPT_INFO, "Set report level to %d, output to %s", report_level, (report_to_syslog?"syslog":"stderr") ), 0 ));

	/* And restart the drivers */
	CHAIN( e, init_drivers() );
	CHAIN_END( e, "Critical error while reloading, abort." );
}

void
do_mainloop ()
{
	Screen * s;
	struct timeval t;
	struct timeval last_t;
	int sleeptime;
	long int process_lag = 0;
	long int render_lag = 0;
	long int t_diff;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	gettimeofday (&t, NULL); /* Get initial time */

	while (1) {
		/* Get current time */
		last_t = t;
		gettimeofday (&t, NULL);
		t_diff = ((t.tv_sec - last_t.tv_sec) * 1e6 + (t.tv_usec - last_t.tv_usec));

		process_lag += t_diff;
		if (process_lag > 0) {
			/* Time for a processing stroke */
			sock_poll_clients ();		/* poll clients for input*/
			parse_all_client_messages ();	/* analyze input from network clients*/
			handle_input ();		/* handle key input from devices*/

			/* We've done the job... */
			process_lag = 0 - (1e6/PROCESS_FREQ);
			/* Note : this does not make a fixed frequency */
		}

		render_lag += t_diff;
		if (render_lag > 0) {
			/* Time for a rendering stroke */
			timer ++;
			screenlist_process ();
			s = screenlist_current ();

			/* TODO: Move this call to every client connection
			 *       and every screen add...
			 */
			if( s == server_screen ) {
				update_server_screen (timer);
			}
			render_screen (s, timer);

			/* We've done the job... */
			if( render_lag > (1e6/RENDER_FREQ) * MAX_RENDER_LAG_FRAMES ) {
				/* Cause rendering slowdown because too much lag */
				render_lag = (1e6/RENDER_FREQ) * MAX_RENDER_LAG_FRAMES;
			}
			render_lag -= (1e6/RENDER_FREQ);
			/* Note: this DOES make a fixed frequency (except with slowdown) */
		}

		/* Sleep just as long as needed */
		sleeptime = min (0-process_lag, 0-render_lag);
		if( sleeptime > 0 ) {
			usleep (sleeptime);
		}

		/* Check if a SIGHUP has been caught */
		if( got_reload_signal ) {
			got_reload_signal = 0;
			do_reload ();
		}
	}

	/* Quit! */
	exit_program (0);
}

void
exit_program (int val)
{
	char buf[64];

	debug( RPT_DEBUG, "%s( val=%d )", __FUNCTION__, val );

	/* TODO: These things shouldn't be so interdependent.  The order
	 * things are shut down in shouldn't matter...
	 */

	if( val > 0 ) {
		strncpy(buf, "Server shutting down on ", sizeof (buf) );
		switch(val) {
			case 1: strcat(buf, "SIGHUP"); break;
			case 2: strcat(buf, "SIGINT"); break;
			case 15: strcat(buf, "SIGTERM"); break;
			default: snprintf(buf, sizeof(buf), "Server shutting down on signal %d", val); break;
				 /* Other values should not be seen, but just in case.. */
		}
		report(RPT_NOTICE, buf);	/* report it */
	}

	/* Set emergency reporting and flush all messages if not done already. */
	if( report_level == UNSET_INT )
		report_level = DEFAULT_REPORTLEVEL;
	if( report_to_syslog == UNSET_INT )
		report_to_syslog = DEFAULT_REPORTTOSYSLOG;
	set_reporting( "LCDd", report_level, (report_to_syslog?RPT_DEST_SYSLOG:RPT_DEST_STDERR) );

	goodbye_screen ();		/* display goodbye screen on LCD display */
	drivers_unload_all ();		/* release driver memory and file descriptors */

	/* Shutdown things if server start was complete */
	clients_shutdown ();		/* shutdown clients (must come first) */
	menuscreens_shutdown ();
	screenlist_shutdown ();		/* shutdown screens (must come after client_shutdown) */
	input_shutdown ();		/* shutdown key input part */

	report( RPT_INFO, "Exiting." );
	_exit (0);
}

void
catch_reload_signal (int val)
{
	debug( RPT_DEBUG, "%s( val=%d )", __FUNCTION__, val );

	got_reload_signal = 1;
}

int
interpret_boolean_arg (char *s)
{
	if( strcmp( s, "on" ) == 0 || strcmp( s, "yes" ) == 0
	|| strcmp( s, "true" ) == 0 || strcmp( s, "1" ) == 0 ) {
		return 1;
	}
	if( strcmp( s, "off" ) == 0 || strcmp( s, "no" ) == 0
	|| strcmp( s, "false" ) == 0 || strcmp( s, "0" ) == 0 ) {
		return 0;
	}
	return -1;
}

void
output_GPL_notice()
{
	/* This will only be invoked when running in foreground
	 * So, directly output to stderr
	 */
	fprintf (stderr, "LCDd %s, LCDproc Protocol %s\n", VERSION, PROTOCOL_VERSION);
	fprintf (stderr, "Part of LCDproc\n");
	fprintf (stderr, "Copyright (C) 1998-2003 William Ferrell, Scott Scriven\n");
	fprintf (stderr, "                        and many other contributors\n\n");
	fprintf (stderr, "This program is free software; you can redistribute it and/or\n");
	fprintf (stderr, "modify it under the terms of the GNU General Public License\n");
	fprintf (stderr, "as published by the Free Software Foundation; either version 2\n");
	fprintf (stderr, "of the License, or (at your option) any later version.\n\n");

	fprintf (stderr, "This program is distributed in the hope that it will be useful,\n");
	fprintf (stderr, "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	fprintf (stderr, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	fprintf (stderr, "GNU General Public License for more details.\n\n");

	fprintf (stderr, "The file COPYING contains the GNU General Public License.\n");
	fprintf (stderr, "You should have received a copy of the GNU General Public License\n");
	fprintf (stderr, "along with this program; if not, write to the Free Software\n");
	fprintf (stderr, "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n\n");
}


void
output_help_screen ()
{
	/* Help screen is printed to stdout on purpose. No reason to have
	 * this in syslog...
	 */
	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	fprintf (stdout, "LCDd: LCDproc Server Daemon, %s\n", version);
	fprintf (stdout, "Copyright (c) 1999-2003 Scott Scriven, William Ferrell, and misc contributors\n");
	fprintf (stdout, "This program is freely redistributable under the terms of the GNU Public License\n\n");
	fprintf (stdout, "Usage: LCDd [ -h ] [ -c <config> ] [ -d <driver> ] [ -f <bool> ] \\\n");
	fprintf (stdout, "\t[-a <addr> ] [ -p <port> ] [ -u <user> ] [ -w <time> ]\\\n");
	fprintf (stdout, "\t[ -r <level> ] [ -s <bool> ] [ -i <bool> ]\n\n");
	fprintf (stdout, "Available options are:\n");
	fprintf (stdout, "\t-h\t\tDisplay this help screen\n");
	fprintf (stdout, "\t-c <config>\tUse a configuration file other than %s\n", DEFAULT_CONFIGFILE);
	fprintf (stdout, "\t-d <driver>\tAdd a driver to use (overrides drivers in config file) [%s]\n", DEFAULT_DRIVER);
	fprintf (stdout, "\t-f <bool>\tWhether to run in the foreground\n");
	fprintf (stdout, "\t-a <addr>\tNetwork (IP) address to bind to [%s]\n", DEFAULT_BIND_ADDR);
	fprintf (stdout, "\t-p <port>\tNetwork port to listen for connections on [%i]\n", DEFAULT_BIND_PORT);
	fprintf (stdout, "\t-u <user>\tUser to run as [%s]\n", DEFAULT_USER);
	fprintf (stdout, "\t-w <waittime>\tTime to pause at each screen (in seconds) [%d]\n", DEFAULT_SCREEN_DURATION);
	fprintf (stdout, "\t-s <bool>\tIf set, reporting will be done using syslog\n");
	fprintf (stdout, "\t-r <level>\tReport level [%d]\n", DEFAULT_REPORTLEVEL);
	fprintf (stdout, "\t-i <bool>\tWhether to rotate the server info screen\n");
	fprintf (stdout, "\n");

	/* Error messages will be flushed to the configured output after this
	 * help message.
	 */
}
