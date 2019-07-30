/** \file server/main.c
 * Contains main(), plus signal callback functions and a help screen.
 *
 * Program init, command-line handling, and the main loop are
 * implemented here.  Also, minimal data about the program such as
 * the revision number.
 *
 * Some of this stuff should probably be move elsewhere eventually,
 * such as command-line handling and the main loop.  main() is supposed
 * to be "dumb".
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2001, Joris Robijn
 *               2001, Rene Wagner
 *               2002, Mike Patnode
 *               2002, Guillaume Filion
 *               2005-2006, Peter Marschall (cleanup)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>

#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>

#include "getopt.h"

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
/* TODO: fill in what to include otherwise */

#include <shared/report.h>
#include <shared/defines.h>

#include "drivers.h"
#include "sock.h"
#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "parse.h"
#include "render.h"
#include "serverscreens.h"
#include "menuscreens.h"
#include "input.h"
#include "drivers.h"
#include "main.h"

#include "elektragen.h"

#if !defined(SYSCONFDIR)
# define SYSCONFDIR "/etc"
#endif

#define DEFAULT_BIND_ADDR		"127.0.0.1"
#define DEFAULT_BIND_PORT		LCDPORT
#define DEFAULT_CONFIGFILE		SYSCONFDIR "/LCDd.conf"
#define DEFAULT_USER			"nobody"
#define DEFAULT_DRIVER			"curses"
#define DEFAULT_DRIVER_PATH		""	/* not needed */
#define MAX_DRIVERS			8
#define DEFAULT_FOREGROUND_MODE		0
#define DEFAULT_ROTATE_SERVER_SCREEN	SERVERSCREEN_ON
#define DEFAULT_REPORTDEST		RPT_DEST_STDERR
#define DEFAULT_REPORTLEVEL		RPT_WARNING

#define DEFAULT_FRAME_INTERVAL		125000
#define DEFAULT_SCREEN_DURATION		32
#define DEFAULT_BACKLIGHT		BACKLIGHT_OPEN
#define DEFAULT_HEARTBEAT		HEARTBEAT_OPEN
#define DEFAULT_TITLESPEED		TITLESPEED_MAX
#define DEFAULT_AUTOROTATE		AUTOROTATE_ON

/* Socket to bind to...

   Using loopback is much more secure; it means that this port is
   accessible only to programs running locally on the same host as LCDd.

   Using variables for these means that (later) we can select which port
   and which address to bind to at run time. */


/* Store some standard defines into vars... */
char *version = VERSION;
char *protocol_version = PROTOCOL_VERSION;
char *api_version = API_VERSION;


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

int frame_interval = DEFAULT_FRAME_INTERVAL;

/* The drivers and their driver parameters */
int num_drivers = 0;

/* End of configuration variables */

/* Local variables */
static int foreground_mode = UNSET_INT;
static int report_dest = UNSET_INT;
static int report_level = UNSET_INT;

static int stored_argc;
static const char **stored_argv;
static volatile short got_reload_signal = 0;

static Elektra * elektra;

/* Local exported variables */
long timer = 0;

/**** Local functions ****/
static void clear_settings(void);
static int process_config();
static void install_signal_handlers(int allow_reload);
static void child_ok_func(int signal);
static pid_t daemonize(void);
static int wave_to_parent(pid_t parent_pid);
static int init_drivers(void);
static int drop_privs(char *user);
static void do_reload(void);
static void do_mainloop(void);
static void exit_program(int val);
static void catch_reload_signal(int val);
static void output_help_screen(void);
static void output_GPL_notice(void);

#define CHAIN(e,f) { if (e>=0) { e=(f); }}
#define CHAIN_END(e,msg) { if (e<0) { report(RPT_CRIT,(msg)); exit(EXIT_FAILURE); }}

static const char * help_prefix =
	"LCDd - LCDproc Server Daemon, "VERSION"\n\n"
	"Copyright (c) 1998-2017 Selene Scriven, William Ferrell, and misc. contributors.\n"
	"This program is released under the terms of the GNU General Public License.\n\n";


int
main(int argc, const char **argv)
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
	report(RPT_NOTICE, "LCDd version %s starting", version);
	report(RPT_INFO, "Protocol version %s, API version %s",
		protocol_version, api_version);

	clear_settings();

	/* Read config */
	CHAIN(e, process_config());

	/* Set reporting settings (will also flush delayed reports) */
	set_reporting("LCDd", report_level, report_dest);
	report(RPT_INFO, "Set report level to %d, output to %s", report_level,
			((report_dest == RPT_DEST_SYSLOG) ? "syslog" : "stderr"));
	CHAIN_END(e, "Critical error while processing settings, abort.");

	/* Now, go into daemon mode (if we should)...
	 * We wait for the child to report it is running OK. This mechanism
	 * is used because forking after starting the drivers causes the
	 * child to loose the (LPT) port access. */
	if (!foreground_mode) {
		report(RPT_INFO, "Server forking to background");
		CHAIN(e, parent_pid = daemonize());
	} else {
		output_GPL_notice();
		report(RPT_INFO, "Server running in foreground");
	}
	install_signal_handlers(!foreground_mode);
		/* Only catch SIGHUP if not in foreground mode */

	/* Startup the subparts of the server */
	CHAIN(e, sock_init(bind_addr, bind_port));
	CHAIN(e, screenlist_init());
	CHAIN(e, init_drivers());
	CHAIN(e, clients_init());
	CHAIN(e, input_init(elektra));
	CHAIN(e, menuscreens_init(elektra));
	CHAIN(e, server_screen_init(elektra));
	CHAIN_END(e, "Critical error while initializing, abort.");
	if (!foreground_mode) {
		/* Tell to parent that startup went OK. */
		wave_to_parent(parent_pid);
	}
	drop_privs(user); /* This can't be done before, because sending a
			signal to a process of a different user will fail */

	do_mainloop();
	/* This loop never stops; we'll get out only with a signal...*/

	return 0;
}


static void
clear_settings(void)
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	bind_port = UNSET_INT;
	strncpy(bind_addr, UNSET_STR, sizeof(bind_addr));
	strncpy(configfile, UNSET_STR, sizeof(configfile));
	strncpy(user, UNSET_STR, sizeof(user));
	foreground_mode = UNSET_INT;
	rotate_server_screen = UNSET_INT;
	backlight = UNSET_INT;
	heartbeat = UNSET_INT;
	titlespeed = UNSET_INT;

	default_duration = UNSET_INT;
	report_dest = UNSET_INT;
	report_level = UNSET_INT;
	num_drivers = 0;
}

static void on_fatal_error(ElektraError * error) // TODO (kodebach): finalize method
{
	fprintf(stderr, "ERROR: %s\n", elektraErrorDescription(error));
	exit(EXIT_FAILURE);
}


/* reads and parses configuration */
static int
process_config()
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	ElektraError * error = NULL;
	elektra = NULL;
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
		printHelpMessage(elektra, NULL, help_prefix);
		elektraClose (elektra);
		return EXIT_SUCCESS;
	}

	elektraFatalErrorHandler(elektra, on_fatal_error);

	bind_port = elektraGet(elektra, ELEKTRA_TAG_SERVER_PORT);
	strncpy(bind_addr, elektraGet(elektra, ELEKTRA_TAG_SERVER_BIND), sizeof(bind_addr));
	strncpy(user, elektraGet(elektra, ELEKTRA_TAG_SERVER_USER), sizeof(user));

	frame_interval = elektraGet(elektra, ELEKTRA_TAG_SERVER_FRAMEINTERVAL);
	default_duration = elektraGet(elektra, ELEKTRA_TAG_SERVER_WAITTIME) * 1e6 / frame_interval;
	if (default_duration * frame_interval < 2e6) {
		report(RPT_WARNING, "Waittime should be at least 2 (seconds). Set to 2 seconds.");
		default_duration = 2e6 / frame_interval;
	}

	foreground_mode = elektraGet(elektra, ELEKTRA_TAG_SERVER_FOREGROUND);
	rotate_server_screen = elektraGet(elektra, ELEKTRA_TAG_SERVER_SERVERSCREEN);
	backlight = elektraGet(elektra, ELEKTRA_TAG_SERVER_BACKLIGHT);
	heartbeat = elektraGet(elektra, ELEKTRA_TAG_SERVER_HEARTBEAT);
	autorotate = elektraGet(elektra, ELEKTRA_TAG_SERVER_AUTOROTATE);

	titlespeed = elektraGet(elektra, ELEKTRA_TAG_SERVER_TITLESPEED);
	report_dest = (elektraGet(elektra, ELEKTRA_TAG_SERVER_REPORTTOSYSLOG)) ? RPT_DEST_SYSLOG : RPT_DEST_STDERR;

	report_level = elektraGet(elektra, ELEKTRA_TAG_SERVER_REPORTLEVEL);

	return 0;
}

static void
install_signal_handlers(int allow_reload)
{
	/* Installs signal handlers so that the program does clean exit and
	 * can also receive a reload signal.
	 * sigaction() is favoured over signal() */

	struct sigaction sa;

	debug(RPT_DEBUG, "%s(allow_reload=%d)", __FUNCTION__, allow_reload);

	sigemptyset(&(sa.sa_mask));

	/* Clients can cause SIGPIPE if they quit unexpectedly, and the
	 * default action is to kill the server.  Just ignore it. */
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	sa.sa_handler = exit_program;
#ifdef HAVE_SA_RESTART
	sa.sa_flags = SA_RESTART;
#endif

	sigaction(SIGINT, &sa, NULL);		/* Ctrl-C will cause a clean exit...*/
	sigaction(SIGTERM, &sa, NULL);		/* and "kill"...*/

	if (allow_reload) {
		sa.sa_handler = catch_reload_signal;
		/* On SIGHUP reread config and restart the drivers ! */
	}
	else {
		/* Treat this signal just like INT and TERM */
	}
	sigaction(SIGHUP, &sa, NULL);
}


static void
child_ok_func(int signal)
{
	/* We only catch this signal to be sure the child runs OK. */

	debug(RPT_INFO, "%s(signal=%d)", __FUNCTION__, signal);

	/* Exit now !    because of bug? in wait() */
	_exit(EXIT_SUCCESS); /* Parent exits normally. */
}


static pid_t
daemonize(void)
{
	pid_t child;
	pid_t parent;
	int child_status;
	struct sigaction sa;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	parent = getpid();
	debug(RPT_INFO, "parent = %d", parent);

	/* Install handler at parent for child's signal */
	/* sigaction should be more portable than signal, but it does not
	 * work for some reason. */
	sa.sa_handler = child_ok_func;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);

	/* Do the fork */
	switch ((child = fork())) {
	  case -1:
		report(RPT_ERR, "Could not fork");
		return -1;
	  case 0: /* We are the child */
		break;
	  default: /* We are the parent */
		debug(RPT_INFO, "child = %d", child);
		wait(&child_status);
		/* BUG? According to the man page wait() should also return
		 * when a signal comes in that is caught. Instead it
		 * continues to wait. */

		if (WIFEXITED(child_status)) {
			/* Child exited normally, probably because of some
			 * error. */
			debug(RPT_INFO, "Child has terminated!");
			exit(WEXITSTATUS(child_status));
			/* Parent exits with same status as child did... */
		}
		/* Child is still running and has signalled it's OK.
		 * This means the parent can now rest in peace. */
		debug(RPT_INFO, "Got OK signal from child.");
		exit(EXIT_SUCCESS); /* Parent exits normally. */
	}
	/* At this point we are always the child. */
	/* Reset signal handler */
	sa.sa_handler = SIG_DFL;
	sigaction(SIGUSR1, &sa, NULL);

	setsid();	/* Create a new session because otherwise we'll
			 * catch a SIGHUP when the shell is closed. */

	return parent;
}


static int
wave_to_parent(pid_t parent_pid)
{
	debug(RPT_DEBUG, "%s(parent_pid=%d)", __FUNCTION__, parent_pid);

	kill(parent_pid, SIGUSR1);

	return 0;
}


static int
init_drivers(void)
{
	int res;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	/* Read drivers */
	num_drivers = elektraSize(elektra, ELEKTRA_TAG_SERVER_DRIVERS);
	for (kdb_long_long_t i = 0; i < num_drivers; ++i) {
		if(i > MAX_DRIVERS)
		{
			report(RPT_ERR, "too many driver; max = %d", MAX_DRIVERS);
			exit(EXIT_FAILURE);
		}

		res = drivers_load_driver(elektra, i);
		if (res >= 0) {
			/* Load went OK */

			if (res == 2)
				foreground_mode = 1;
		} else {
			report(RPT_ERR, "Could not load driver #"ELEKTRA_LONG_LONG_F, i);
		}
	}

	/* Do we have a running output driver ?*/
	if (output_driver)
		return 0;

	report(RPT_ERR, "There is no output driver");
	return -1;
}


static int
drop_privs(char *user)
{
	struct passwd *pwent;

	debug(RPT_DEBUG, "%s(user=\"%.40s\")", __FUNCTION__, user);

	if (getuid() == 0 || geteuid() == 0) {
		if ((pwent = getpwnam(user)) == NULL) {
			report(RPT_ERR, "User %.40s not a valid user!", user);
			return -1;
		} else {
			if (setuid(pwent->pw_uid) < 0) {
				report(RPT_ERR, "Unable to switch to user %.40s", user);
				return -1;
			}
		}
	}

	return 0;
}


static void
do_reload(void)
{
	int e = 0;

	drivers_unload_all();		/* Close all drivers */

	clear_settings();

	elektraClose(elektra);
	/* Reread config */
	CHAIN(e, process_config());

	/* Set reporting values */
	CHAIN(e, set_reporting("LCDd", report_level, report_dest));
	CHAIN(e, (report(RPT_INFO, "Set report level to %d, output to %s", report_level,
			((report_dest == RPT_DEST_SYSLOG) ? "syslog" : "stderr")), 0));

	/* And restart the drivers */
	CHAIN(e, init_drivers());
	CHAIN_END(e, "Critical error while reloading, abort.");
}


static void
do_mainloop(void)
{
	Screen *s;
	struct timeval t;
	struct timeval last_t;
	int sleeptime;
	long int process_lag = 0;
	long int render_lag = 0;
	long int t_diff;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	gettimeofday(&t, NULL); /* Get initial time */

	while (1) {
		/* Get current time */
		last_t = t;
		gettimeofday(&t, NULL);
		t_diff = t.tv_sec - last_t.tv_sec;
		if ( ((t_diff + 1) > (LONG_MAX / 1e6)) || (t_diff < 0) ) {
			/* We're going to overflow the calculation - probably been to sleep, fudge the values */
			t_diff = 0;
			process_lag = 1;
			render_lag = frame_interval;
		} else {
			t_diff *= 1e6;
			t_diff += t.tv_usec - last_t.tv_usec;
		}
                process_lag += t_diff;
		if (process_lag > 0) {
			/* Time for a processing stroke */
			sock_poll_clients();		/* poll clients for input*/
			parse_all_client_messages();	/* analyze input from network clients*/
			handle_input();		/* handle key input from devices*/

			/* We've done the job... */
			process_lag = 0 - (1e6/PROCESS_FREQ);
			/* Note : this does not make a fixed frequency */
		}

		render_lag += t_diff;
		if (render_lag > 0) {
			/* Time for a rendering stroke */
			timer ++;
			screenlist_process();
			s = screenlist_current();

			/* TODO: Move this call to every client connection
			 *       and every screen add...
			 */
			if (s == server_screen) {
				update_server_screen();
			}
			render_screen(s, timer);

			/* We've done the job... */
			if (render_lag > frame_interval * MAX_RENDER_LAG_FRAMES) {
				/* Cause rendering slowdown because too much lag */
				render_lag = frame_interval * MAX_RENDER_LAG_FRAMES;
			}
			render_lag -= frame_interval;
			/* Note: this DOES make a fixed frequency (except with slowdown) */
		}

		/* Sleep just as long as needed */
		sleeptime = min(0-process_lag, 0-render_lag);
		if (sleeptime > 0) {
			usleep(sleeptime);
		}

		/* Check if a SIGHUP has been caught */
		if (got_reload_signal) {
			got_reload_signal = 0;
			do_reload();
		}
	}

	/* Quit! */
	exit_program(0);
}


static void
exit_program(int val)
{
	char buf[64];

	debug(RPT_DEBUG, "%s(val=%d)", __FUNCTION__, val);

	/* TODO: These things shouldn't be so interdependent.  The order
	 * things are shut down in shouldn't matter...
	 */

	if (val > 0) {
		strncpy(buf, "Server shutting down on ", sizeof(buf));
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
	if (report_level == UNSET_INT)
		report_level = DEFAULT_REPORTLEVEL;
	if (report_dest == UNSET_INT)
		report_dest = DEFAULT_REPORTDEST;
	set_reporting("LCDd", report_level, report_dest);

	goodbye_screen(elektra);		/* display goodbye screen on LCD display */
	drivers_unload_all();		/* release driver memory and file descriptors */

	/* Shutdown things if server start was complete */
	clients_shutdown();		/* shutdown clients (must come first) */
	menuscreens_shutdown();
	screenlist_shutdown();		/* shutdown screens (must come after client_shutdown) */
	input_shutdown();		/* shutdown key input part */
        sock_shutdown();                /* shutdown the sockets server */

	elektraClose(elektra);

	report(RPT_INFO, "Exiting.");
	_exit(EXIT_SUCCESS);
}


static void
catch_reload_signal(int val)
{
	debug(RPT_DEBUG, "%s(val=%d)", __FUNCTION__, val);

	got_reload_signal = 1;
}


static void
output_GPL_notice(void)
{
	/* This will only be invoked when running in foreground
	 * So, directly output to stderr
	 */
	fprintf(stderr, "LCDd %s, LCDproc Protocol %s\n", VERSION, PROTOCOL_VERSION);
	fprintf(stderr, "Part of the LCDproc suite\n");
	fprintf(stderr, "Copyright (C) 1998-2017 William Ferrell, Selene Scriven\n"
	                "                        and many other contributors\n\n");

	fprintf(stderr, "This program is free software; you can redistribute it and/or\n"
	                "modify it under the terms of the GNU General Public License\n"
	                "as published by the Free Software Foundation; either version 2\n"
	                "of the License, or (at your option) any later version.\n\n");

	fprintf(stderr, "This program is distributed in the hope that it will be useful,\n"
	                "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	                "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	                "GNU General Public License for more details.\n\n");

	fprintf(stderr, "You should have received a copy of the GNU General Public License\n"
	                "along with this program; if not, write to the Free Software Foundation,\n"
	                "Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n\n");
}

