/*
  main.c for LCDd

  Contains main(), plus signal callback functions and a help screen.

  Program init, command-line handling, and the main loop are
  implemented here.  Also, minimal data about the program such as
  the revision number.

  Some of this stuff should probably be move elsewhere eventually,
  such as command-line handling and the main loop.  main() is supposed
  to be "dumb".

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

extern char *optarg;
extern int optind, optopt, opterr;

#include "shared/debug.h"

#include "drivers.h"
#include "sock.h"
#include "clients.h"
#include "screenlist.h"
#include "screen.h"
#include "parse.h"
#include "render.h"
#include "serverscreens.h"
#include "input.h"
#include "configfile.h"
#include "drivers.h"
#include "main.h"

#define MAX_TIMER 0x10000

#define DEFAULT_DEBUG_LEVEL 1
#define DEFAULT_LCD_PORT LCDPORT
#define DEFAULT_BIND_ADDR "127.0.0.1"
#define DEFAULT_CONFIGFILE "/etc/LCDd.conf"
#define DEFAULT_USER "nobody"
#define DEFAULT_DRIVER "curses"
#define DEFAULT_WAITTIME 8
#define MAX_DRIVERS 8
#define DEFAULT_DAEMON_MODE 1
#define DEFAULT_ENABLE_SERVER_SCREEN 0


/* Store some standard defines into vars... */
char *version = VERSION;
char *protocol_version = PROTOCOL_VERSION;
char *build_date = __DATE__;


/* Socket to bind to...

   Using loopback is much more secure; it means that this port is
   accessible only to programs running locally on the same host as LCDd.

   Using variables for these means that (later) we can select which port
   and which address to bind to at run time. */


/**** Configuration variables ****/

// All are set to 'unset' values
int debug_level = -1;
int lcd_port = -1;
char bind_addr[64] = "";
char configfile[256] = "";
char user[64] = "";

int daemon_mode = -1;
int enable_server_screen = -1;

// The drivers and their driver parameters
char *drivernames[MAX_DRIVERS];
char *driverfilenames[MAX_DRIVERS];
char *driverargs[MAX_DRIVERS];
int num_drivers = 0;


// The parameter structure and args[] should
// be removed when getopt(3) is implemented,
// as there won't be any need for them then.
//typedef struct parameter {
//	char *sh, *lg;	// short and long versions
//} parameter;

// This is currently only a list of available arguments, but doesn't
// really *do* anything.  It just helps to figure out which parameters
// go to the server, and which ones go to individual drivers...
//static parameter args[] = {
//	{"-h", "--help"},
//	{"-d", "--driver"},
//	{"-t", "--type"},
//	{"-f", "--foreground"},
//	{"-b", "--backlight"},
//	{"-i", "--serverinfo"},
//	{"-w", "--waittime"},
//	{NULL, NULL},
//};


/**** Local functions ****/
void exit_program (int val);
void HelpScreen ();

void clear_settings();
int process_command_line (int argc, char **argv);
int process_configfile (char *cfgfile);
void set_default_settings();
int daemonize();
int init_sockets();
int drop_privs(char *user);
int init_drivers();
int init_screens();
void do_mainloop();

#define ESSENTIAL(f) {int r; if( ( r=f )!=0 ) return r;}

int
main (int argc, char **argv)
{
	// FIXME: s is getting clobbered - in MANY places!!!
	//screen *s = NULL;
	//char buf[64];

	signal (SIGINT, exit_program);		// Ctrl-C will cause a clean exit...
	signal (SIGTERM, exit_program);		// and "kill"...
	signal (SIGHUP, exit_program);		// and "kill -HUP" (hangup)...
	signal (SIGKILL, exit_program);		// and just in case, "kill -KILL" (which cannot be trapped; but oh well)

	// If no paramaters given, give the help screen.
	//if (argc == 1)
	//	HelpScreen ();

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

	// Open syslog facility
	openlog("LCDd", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "server version %s starting up (protocol version %s)",
		version, protocol_version);
	syslog(LOG_NOTICE, "server built on %s",
		build_date);

	clear_settings();

	// Read command line
	ESSENTIAL( process_command_line (argc, argv) );

	// Read config file
	ESSENTIAL( process_configfile (configfile) );

	// Set default values
	set_default_settings();

	if (debug_level > 0)
		syslog(LOG_NOTICE, "debug level set to %d", debug_level);

	ESSENTIAL( init_sockets() );
	ESSENTIAL( drop_privs(user) );
	ESSENTIAL( init_drivers() );
	ESSENTIAL( init_screens() );

#ifndef DEBUG
	// Now, go into daemon mode...
	if (daemon_mode) {
		syslog(LOG_NOTICE, "server forking to background");
		ESSENTIAL( daemonize() );
	} else {
		syslog(LOG_NOTICE, "server running in foreground");
	}
#endif

	do_mainloop();
	// This loop never stops; we'll get out only with a signal...

	return 0;
}


void
clear_settings ()
{
	int i;

	debug_level = -1;
	lcd_port = 0;
	bind_addr[0] = 0;
	configfile[0] = 0;
	user[0] = 0;
	daemon_mode = -1;
	enable_server_screen = -1;

	default_duration = -1;

	for( i=0; i < num_drivers; i++ ) {
		free( drivernames[i] );
		free( driverfilenames[i] );
		free( driverargs[i] );
		drivernames[i] = NULL;
		driverfilenames[i] = NULL;
		driverargs[i] = NULL;
	}
	num_drivers = 0;
}


/* parses arguments given on command line */
int
process_command_line (int argc, char **argv)
{
	char  c;
	char  buf[64];

	// analyze options here..
	while ((c = getopt(argc, argv, "a:p:d:hfiw:c:u:")) > 0) {
		// FIXME: Setting of c in this loop clobbers s!
		// s is set equivalent to c.
		switch(c) {
	 		case 'd':
				// Add to a list of drivers to be initialized later...
				if (num_drivers < MAX_DRIVERS) {
					drivernames[num_drivers]	= malloc( strlen(optarg)+1 );
					driverfilenames[num_drivers]	= malloc( strlen(optarg)+1 );
					driverargs[num_drivers]		= malloc(1);

					strcpy( drivernames[num_drivers], optarg );
					strcpy( driverfilenames[num_drivers], optarg );
					strcpy( driverargs[num_drivers], "");
					num_drivers ++;
				} else
					fprintf(stderr, "too many drivers!");
				break;
			case 'p':
				lcd_port = atoi(optarg);
				break;
			case 'u':
				strncpy(user, optarg, sizeof(user));
				break;
			case 'a':
				strncpy(bind_addr, optarg, sizeof(bind_addr));
				break;
			case 'h':
				HelpScreen ();
				break;
			case 'f':
				daemon_mode = 0;
				break;
			case 'c':
				strncpy(configfile, optarg, sizeof(configfile));
				break;
			case 'i':
				enable_server_screen = 0;
				break;
			case 'w':
				default_duration = atoi(optarg);
				if ( default_duration < 16 || default_duration > 10000 ) {
					snprintf(buf, sizeof(buf),
						"wait time should be between 16 and 10000 (in 1/8ths of second), not %s\n", optarg);
					fprintf(stderr, "%s", buf);
					HelpScreen ();
				};
				break;
			case '?':
				fprintf(stderr, "unknown option: '%c'\n", optopt);
				HelpScreen();
				break;
			case ':':
				fprintf(stderr, "missing option argument!");
				HelpScreen();
				break;
		}
	}

	if (optind < argc)
		fprintf(stderr, "non-option arguments!! BAD...");

	return 0;
}


/* reads and parses configuration file */
int
process_configfile ( char *configfile )
{
	int i;
	char * s;
	//char buf[64];

	// Read server settings

	config_read_file( configfile );

	if( debug_level == -1 )
		debug_level = config_get_int( "server", "debug", 0, -1 );

	if( lcd_port == -1 )
		lcd_port = config_get_int( "server", "port", 0, -1 );

	if( bind_addr == "" )
		strncpy( bind_addr, config_get_string( "server", "bind", 0, "" ), sizeof(bind_addr));

	if( user == "" )
		strncpy( user, config_get_string( "server", "user", 0, "" ), sizeof(user));

	if( default_duration == -1 ) {
		default_duration = (config_get_float( "server", "waittime", 0, 0 ) * 1000000 / TIME_UNIT );
		if( default_duration == 0 )
			default_duration = -1;
		else if( default_duration < 16 ) {
			fprintf( stderr, "waittime should be at least 2 (seconds)\n" );
			return -1;
		}
	}

	if( daemon_mode == -1 )
		daemon_mode = ! config_get_bool( "server", "foreground", 0, -1 );

	if( enable_server_screen == -1 )
		enable_server_screen = config_get_bool( "server", "serverscreen", 0, -1 );

	// Read drivers

	// If drivers have been specified on the command line, then do not
	// use the driver list from the config file.
	if( num_drivers == 0 ) {
		// read the drivernames

		while( 1 ) {
			s = config_get_string( "server", "driver", num_drivers, "" );
			if( !s || s[0] == 0 )
				break;

			drivernames[num_drivers] = malloc(strlen(s)+1);
			driverfilenames[num_drivers] = malloc(strlen(s)+1);
			driverargs[num_drivers] = malloc(1);

			strcpy( drivernames[num_drivers], s );
			strcpy( driverfilenames[num_drivers], s );
			strcpy( driverargs[num_drivers], "" );

			num_drivers++;
		}
	}

	// Now read the driver options that the server needs
	// Drivers can read their own options later...
	for( i=0; i<num_drivers; i ++ ) {
		s = config_get_string( drivernames[i], "file", 0, "" );
		driverfilenames[i] = realloc( driverfilenames[i], strlen(s)+1 );
		strcpy (driverfilenames[i], s );

		s = config_get_string( drivernames[i], "arguments", 0, "" );
		driverargs[i] = realloc( driverargs[i], strlen(s)+1 );
		strcpy (driverargs[i], s );
	}

	return 0;
}


void
set_default_settings()
{
	// Set defaults into unfilled variables....

	if (debug_level == -1)
		debug_level = DEFAULT_DEBUG_LEVEL;
	if (lcd_port == 0)
		lcd_port = DEFAULT_LCD_PORT;
	if (bind_addr[0] == 0)
		strncpy (bind_addr, DEFAULT_BIND_ADDR, sizeof(bind_addr));
	if (configfile[0] == 0)
		strncpy (configfile, DEFAULT_CONFIGFILE, sizeof(configfile));
	if (user[0] == '\0')
		strncpy(user, DEFAULT_USER, sizeof(user));
	if (daemon_mode == -1)
		daemon_mode = DEFAULT_DAEMON_MODE;
	if (enable_server_screen == -1)
		enable_server_screen = DEFAULT_ENABLE_SERVER_SCREEN;

	if (default_duration == -1)
		default_duration = DEFAULT_SCREEN_DURATION;

	// Use default driver
	if( num_drivers == 0 ) {
		drivernames[0] = malloc(strlen(DEFAULT_DRIVER)+1);
		strcpy(drivernames[0], DEFAULT_DRIVER);
		driverfilenames[0] = malloc(1);
		strcpy(driverfilenames[0], DEFAULT_DRIVER);
		driverargs[0] = malloc(1);
		strcpy(driverargs[0], "");
	}
}


int
daemonize()
{
	int child;

	switch ((child = fork ()) ) {
	  case -1:
		syslog(LOG_ERR, "could not fork");
		return 1;
	  case 0: // We are the child
		break;
	  default: // We are the parent
		usleep (1500000);		  // Wait for child to initialize
		exit (0);				  /* PARENT EXITS */
	}
	// This line removed because it eats error messages...
	//setsid();                                       /* RELEASE TTY */
	//
	// After this point, as a daemon, no error messages should
	// go to the console unless drastic; rather, they should go to syslog
	//
	// However, option processing is not yet done, nor is any initialization (!)
	// So we must wait until the main loop.
	return 0;
}


int
init_sockets ()
{

	if (sock_create_server (&bind_addr, lcd_port) <= 0) {
		syslog(LOG_ERR, "error opening socket");
		return 1;
	}

	// Now init a bunch of required stuff...

	if (client_init () < 0) {
		syslog(LOG_ERR, "error initializing client list");
		return 1;
	}
	return 0;
}


int
init_drivers()
{
	int i, res;
	char buf[64];

	int output_loaded = 0;

	// FIXME: This sets s equal to a value related to i
	// (bitshifted left?)  FIX FIX FIX ARGH....
	//
	// Go thru all drivers and initialize all of them
	for (i = 0; i < num_drivers; i++) {

		res = load_driver (drivernames[i], driverfilenames[i], driverargs[i]);
		if (res >= 0) {
			// Load went OK

			switch( res ) {
			  case 0: // Driver does input only
			  	break;
			  case 1: // Driver does output
			  	output_loaded = 1;
			  	break;
			  case 2: // Driver does output in foreground (don't daemonize)
			  	if ( !output_loaded ) {
			  		daemon_mode = 1;
			  	}
			  	output_loaded = 1;
			  	break;
			}
		} else {
			snprintf(buf, sizeof(buf), "Could not load driver %s\n", drivernames[i]);
			fprintf(stderr, buf);
		}
	}

	// Do we have a running output driver ?
	if ( output_loaded ) {
		return 0;
	} else {
		return -1;
	}
}


int drop_privs(char *user)
{
	struct passwd *pwent;

	if (getuid() == 0 || geteuid() == 0) {
		if ((pwent = getpwnam(user)) == NULL) {
			if (errno) {
				perror("LCDd: getpwnam");
				return 1;
			} else {
				fprintf(stderr, "user %s not a valid user!", user);
				return 1;
			}
		} else {
			if (setuid(pwent->pw_uid) < 0) {
				fprintf(stderr, "unable to switch to user %s\n", user);
				perror("LCDd: setuid");
				return 1;
			}
		}
	}
	return 0;
}


int
init_screens ()
{
	if (screenlist_init () < 0) {
		syslog(LOG_ERR, "error initializing screen list");
		return 1;
	}
	// Make sure the server screen shows up every once in a while..
	if (server_screen_init () < 0) {
		syslog(LOG_ERR, "error initializing server screens");
		return 1;
	} else if (!enable_server_screen) {
		server_screen->priority = 256;
	}
	return 0;
}


void
do_mainloop ()
{
	screen *s = NULL;
	//char buf[64];

	// FIXME: s should still be null from initialization.... what's happening here?!
	while (1) {
		sock_poll_clients ();		// poll clients for input
		parse_all_client_messages ();	// analyze input from network clients
		handle_input ();		// handle key input from devices

		// TODO:  Move this code to screenlist.c...
		// ... it should just say "handle_screens();"
		// Timer gets reset by screenlist_next()

		timer++;

		//if (s == NULL)
		//	s = screenlist_current();

		// this is here because s is getting overwritten...
		//if (s != screenlist_current()) {
		//	syslog(LOG_DEBUG, "internal error! s was found overwritten at main.c:637");
		//	s = screenlist_current();
		//}
		//
		//TODO: THIS MUST BE FIXED..... WHY is s getting overwritten?
		// s is a local, it is never passed or assigned to anywhere.
		// So SOMETHING is going haywire and clobbering memory....

		if (s && (timer >= s->duration))
			screenlist_next ();

		// Just in case it gets out of hand...
		if (timer >= MAX_TIMER)
			timer = 0;

		// Update server screen with the right number
		// of clients and screens...
		//
		// TODO: Move this call to every client connection
		//       and every screen add...

		update_server_screen (timer);

		// draw the current scren

		if ((s = screenlist_current ()) != NULL)

			draw_screen (s, timer);
		else
			no_screen_screen (timer);

		usleep (TIME_UNIT);
	}

	// Quit!
	exit_program (0);
}

void
exit_program (int val)
{
	char buf[64];

	// TODO: These things shouldn't be so interdependent.  The order
	// things are shut down in shouldn't matter...

	strcpy(buf, "server shutting down on ");
	switch(val) {
		case 1: strcat(buf, "SIGHUP"); break;
		case 2: strcat(buf, "SIGINT"); break;
		case 15: strcat(buf, "SIGTERM"); break;
		default: snprintf(buf, sizeof(buf), "server shutting down on signal %d", val); break;
			 // Other values should not be seen, but just in case..
	}

	syslog(LOG_NOTICE, buf);	// send message to syslog


	goodbye_screen ();		// display goodbye screen on LCD display
	unload_all_drivers ();		// release driver memory and file descriptors

	client_shutdown ();		// shutdown clients (must come first)
	screenlist_shutdown ();		// shutdown screens (must come after client_shutdown)
	sock_close_all ();		// close all open sockets (must come after client_shutdown)


	exit (0);
}


void
HelpScreen ()
{

	printf ("\nLCDd Server Daemon (part of lcdproc), %s\n", version);
	printf ("Copyright (c) 1999 Scott Scriven, William Ferrell, and misc contributors\n");
	printf ("This program is freely redistributable under the terms of the GNU Public License\n\n");
	printf ("Usage: LCDd [ -hfiw ] [ -c <config> ] [ -d <driver> ] [ -a <addr> ] \\\n\t[ -p <port> ] [ -u <user> ] [ -w <time> ]\n\n");
	printf ("Available options are:\n");

	printf ("\t-h\t\tDisplay this help screen\n");
	printf ("\t-c <config>\tUse a configuration file other than %s\n", DEFAULT_CONFIGFILE);
	//printf ("\t-t\t\tSelect an LCD size (20x4, 16x2, etc...)\n");
	printf ("\t-d <driver>\tAdd a driver to use (output only to first)\n");
	//printf ("\t\t\tCFontz, curses, HD44780, irmanin, joy,\n\t\t\tMtxOrb, LB216, text\n");
	//printf ("\t\t\t(args will be passed to the driver for init)\n");
	printf ("\t-f\t\tRun in the foreground\n");
	//printf ("\t-b\t--backlight <mode>\n\t\t\tSet backlight mode (on, off, open)\n");
	printf ("\t-i\t\tDisable showing of the main LCDproc server screen\n");
	printf ("\t-w <waittime>\tTime to pause at each screen (in 1/8s of a second)\n");
	printf ("\t-a <addr>\tNetwork (IP) address to bind to\n");
	printf ("\t-p <port>\tNetwork port to listen for connections on\n");
	printf ("\t-u <user>\tUser to run as\n");

	printf ("\nCurrently available drivers:\n");
	lcd_list_drivers();

	//printf ("\tHelp on each driver's parameters are obtained upon request:\n\t\t\"LCDd -d driver --help\"\n");
	//printf ("Example:\n");
	//printf ("\tLCDd -d MtxOrb \"--device /dev/lcd --contrast 200\" -d joy\n");
	printf ("\n");

	exit (0);
}
