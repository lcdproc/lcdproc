/** \file clients/lcdexec/lcdexec.c
 * Main file for \c lcdexec, the program starter in the LCDproc suite.
 */

/* This file is part of lcdexec, an LCDproc client.
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
#include <strings.h>
#include <errno.h>
#include <sys/utsname.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "getopt.h"

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"


#if !defined(SYSCONFDIR)
# define SYSCONFDIR	"/etc"
#endif
#if !defined(PIDFILEDIR)
# define PIDFILEDIR	"/var/run"
#endif

#define DEFAULT_CONFIGFILE	SYSCONFDIR "/lcdexec.conf"
#define DEFAULT_PIDFILE		PIDFILEDIR "/lcdexec.pid"


/** information about a process started by lcdexec */
typedef struct ProcInfo {
	struct ProcInfo *next;	/**< pointer to the next ProcInfo entry */
	const MenuEntry *cmd;	/**< pointer to the corresponding menu entry */
	pid_t pid;		/**< PID the process was started with */
	time_t starttime;	/**< start time of the process */
	time_t endtime;		/**< finishing time of the process */
	int status;		/**< exit status of the process */
	int feedback;		/**< what info to show to the user */
	int shown;		/**< tell if the info has been shown to the user */
} ProcInfo;


char * help_text =
"lcdexec - LCDproc client to execute commands from the LCDd menu\n"
"\n"
"Copyright (c) 2002, Joris Robijn, 2006-2008 Peter Marschall.\n"
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
char *configfile = NULL;
char *address = NULL;
int port = UNSET_INT;
int foreground = FALSE;
static int report_level = UNSET_INT;
static int report_dest = UNSET_INT;
char *pidfile = NULL;
int pidfile_written = FALSE;
char *displayname = NULL;
char *default_shell = NULL;

/* Other global variables */
MenuEntry *main_menu = NULL;	/**< pointer to the main menu */
ProcInfo *proc_queue = NULL;	/**< pointer to the list of executed processes */

int lcd_wid = 0;		/**< LCD display width reported by the server */
int lcd_hgt = 0;		/**< LCD display height reported by the server */

int sock = -1;			/**< socket to connect to server */

int Quit = 0;			/**< indicate end of main loop */


/* Function prototypes */
static void exit_program(int val);
static void sigchld_handler(int signal);
static int process_command_line(int argc, char **argv);
static int process_configfile(char * configfile);
static int connect_and_setup(void);
static int process_response(char *str);
static int exec_command(MenuEntry *cmd);
static int show_procinfo_msg(ProcInfo *p);
static int main_loop(void);


#define CHAIN(e,f) { if (e>=0) { e=(f); }}
#define CHAIN_END(e) { if (e<0) { report(RPT_CRIT,"Critical error, abort"); exit(e); }}


int main(int argc, char **argv)
{
	int error = 0;
	struct sigaction sa;

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

	/* setup signal handler for children to avoid zombies */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sa.sa_handler = sigchld_handler;
	sigaction(SIGCHLD, &sa, NULL);

	main_loop();

	exit_program(EXIT_SUCCESS);

	/* NOTREACHED */
	return EXIT_SUCCESS;
}


static void exit_program(int val)
{
	//printf("exit program\n");
	Quit = 1;
	sock_close(sock);
	if ((foreground != TRUE) && (pidfile != NULL) && (pidfile_written == TRUE))
		unlink(pidfile);
	exit(val);
}


/* the grim reaper ;-) */
static void sigchld_handler(int signal)
{
	pid_t pid;
	int status;

	/* wait for the child that was signalled as finished */
	if ((pid = wait(&status)) != -1) {
		ProcInfo *p;

		/* fill the procinfo structure with the necessary information */
		for (p = proc_queue; p != NULL; p = p->next) {
			if (p->pid == pid) {
				p->status = status;
				p->endtime = time(NULL);
			}
		}
	}
}


static int process_command_line(int argc, char **argv)
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
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
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
	if (pidfile == NULL) {
		pidfile = strdup(config_get_string(progname, "PidFile", 0, DEFAULT_PIDFILE));
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


static int connect_and_setup(void)
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


static int process_response(char *str)
{
	char *argv[20];
	int argc;
	char *str2 = strdup(str); /* get_args modifies str2 */

	report(RPT_DEBUG, "Server said: \"%s\"", str);

	/* Check what the server just said to us... */
	argc = get_args(argv, str2, sizeof(argv)/sizeof(argv[0]));
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
		if ((strcmp(argv[1], "select") == 0) ||
		    (strcmp(argv[1], "leave") == 0)) {
			MenuEntry *entry;

			if (argc < 3) {
				report(RPT_WARNING, "Server gave invalid response");
				free(str2);
				return -1;
			}

			/* Find the entry by id */
			entry = menu_find_by_id(main_menu, atoi(argv[2]));
			if (entry == NULL) {
				report(RPT_WARNING, "Could not find the item id given by the server");
				free(str2);
				return -1;
			}

			/* The id has been found.
			 * We trigger on the following conditions:
			 * - command entry without args
			 * - last arg of a command entry with args */
			if (((entry->type == MT_EXEC) && (entry->children == NULL)) ||
			    ((entry->type & MT_ARGUMENT) && (entry->next == NULL))) {

				// last arg => get parent entry
				if ((entry->type & MT_ARGUMENT) && (entry->next == NULL))
					entry = entry->parent;

				if (entry->type == MT_EXEC)
					exec_command(entry);
			}
		}
		else if ((strcmp(argv[1], "plus") == 0) ||
			 (strcmp(argv[1], "minus") == 0) ||
			 (strcmp(argv[1], "update") == 0)) {
			MenuEntry *entry;

			if (argc < 4) {
				report(RPT_WARNING, "Server gave invalid response");
				free(str2);
				return -1;
			}

			/* Find the entry by id */
			entry = menu_find_by_id(main_menu, atoi(argv[2]));
			if (entry == NULL) {
				report(RPT_WARNING, "Could not find the item id given by the server");
				free(str2);
				return -1;
			}

			switch (entry->type) {
				case MT_ARG_SLIDER:
					entry->data.slider.value = atoi(argv[3]);
					break;
				case MT_ARG_RING:
					entry->data.ring.value = atoi(argv[3]);
					break;
				case MT_ARG_NUMERIC:
					entry->data.numeric.value = atoi(argv[3]);
					break;
				case MT_ARG_ALPHA:
					entry->data.alpha.value = realloc(entry->data.alpha.value,
									  strlen(argv[3]));
					strcpy(entry->data.alpha.value, argv[3]);
					break;
				case MT_ARG_IP:
					entry->data.ip.value = realloc(entry->data.ip.value,
									strlen(argv[3]));
					strcpy(entry->data.ip.value, argv[3]);
					break;
				case MT_ARG_CHECKBOX:
					if ((entry->data.checkbox.allow_gray) &&
					    (strcasecmp(argv[3], "gray") == 0))
						entry->data.checkbox.value = 2;
					else if (strcasecmp(argv[3], "on") == 0)
						entry->data.checkbox.value = 1;
					else
						entry->data.checkbox.value = 0;
					break;
				default:
					report(RPT_WARNING, "Illegal menu entry type for event");
					free(str2);
					return -1;
			}
		}
		else {
			; /* Ignore other menuevents */
		}
	}
	else if (strcmp(argv[0], "connect") == 0) {
		int a;

		/* determine display height and width */
		for (a = 1; a < argc; a++) {
			if (strcmp(argv[a], "wid") == 0)
				lcd_wid = atoi(argv[++a]);
			else if (strcmp(argv[a], "hgt") == 0)
				lcd_hgt = atoi(argv[++a]);
		}
	}
	else if (strcmp(argv[0], "bye") == 0) {
		// TODO: make it better
		report(RPT_INFO, "Server said: \"%s\"", str);
		exit_program(EXIT_SUCCESS);
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


static int exec_command(MenuEntry *cmd)
{
	if ((cmd != NULL)  && (menu_command(cmd) != NULL)) {
		const char *command = menu_command(cmd);
		const char *argv[4];
		pid_t pid;
		ProcInfo *p;
		char *envp[cmd->numChildren+1];
		MenuEntry *arg;
		int i;

		/* set argument vector */
		argv[0] = default_shell;
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = NULL;

		/* set environment vector: allocate & fill contents */
		for (arg = cmd->children, i = 0; arg != NULL; arg = arg->next, i++) {
			char buf[1025];

			switch (arg->type) {
				case MT_ARG_SLIDER:
					snprintf(buf, sizeof(buf)-1, "%s=%d",
						 arg->name, arg->data.slider.value);
					break;
				case MT_ARG_RING:
					snprintf(buf, sizeof(buf)-1, "%s=%s", arg->name,
						 arg->data.ring.strings[arg->data.ring.value]);
					break;
				case MT_ARG_NUMERIC:
					snprintf(buf, sizeof(buf)-1, "%s=%d",
						 arg->name, arg->data.numeric.value);
					break;
				case MT_ARG_ALPHA:
					snprintf(buf, sizeof(buf)-1, "%s=%s",
						 arg->name, arg->data.alpha.value);
					break;
				case MT_ARG_IP:
					snprintf(buf, sizeof(buf)-1, "%s=%s",
						 arg->name, arg->data.ip.value);
					break;
				case MT_ARG_CHECKBOX:
					if (arg->data.checkbox.map[arg->data.checkbox.value] != NULL)
					    strncpy(buf, arg->data.checkbox.map[arg->data.checkbox.value],
					    	    sizeof(buf)-1);
					else
						snprintf(buf, sizeof(buf)-1, "%s=%d",
							 arg->name, arg->data.checkbox.value);
					break;
				default:
					/* error ? */
					break;
			}
			buf[sizeof(buf)-1] ='\0';
			envp[i] = strdup(buf);

			debug(RPT_DEBUG, "Environment: %s", envp[i]);
		}
		envp[cmd->numChildren] = NULL;

		debug(RPT_DEBUG, "Executing '%s' via Shell %s", command, default_shell);

		switch (pid = fork()) {
		  case 0:
			/* We're the child: execute the command */
			execve(argv[0], (char **) argv, envp);
			exit(EXIT_SUCCESS);
			break;
		  default:
			/* We're the parent: setup the ProcInfo structure */
			p = calloc(1, sizeof(ProcInfo));
			if (p != NULL) {
				p->cmd = cmd;
				p->pid = pid;
				p->starttime = time(NULL);
				p->feedback = cmd->data.exec.feedback;
				/* prepend it to existing queue atomically */
				p->next = proc_queue;
				proc_queue = p;
			}
        		break;
		  case -1:
			report(RPT_ERR, "Could not fork");
			return -1;
		}

		/* free envp's contents */
		for (i = 0; envp[i] != NULL; i++)
			free(envp[i]);

		return 0;
	}
	return -1;
}


static int show_procinfo_msg(ProcInfo *p)
{
	if ((p != NULL) && (lcd_wid > 0) && (lcd_hgt > 0)) {
		if (p->endtime > 0) {
			/* nothing to do => the quick way out (successful) */
			if ((p->shown) || (!p->feedback))
				return 1;

			sock_printf(sock, "screen_add [%u]\n", p->pid);
			sock_printf(sock, "screen_set [%u] -name {lcdexec [%u]}"
					  " -priority alert -timeout %d"
					  " -heartbeat off\n",
					p->pid, p->pid, 6*8);

			if (lcd_hgt > 2) {
				sock_printf(sock, "widget_add [%u] t title\n", p->pid);
				sock_printf(sock, "widget_set [%u] t {%s}\n", p->pid, p->cmd->displayname);
				sock_printf(sock, "widget_add [%u] s1 string\n", p->pid);
				sock_printf(sock, "widget_add [%u] s2 string\n", p->pid);
				sock_printf(sock, "widget_add [%u] s3 string\n", p->pid);

				sock_printf(sock, "widget_set [%u] s1 1 2 {[%u] finished%s}\n",
						p->pid, p->pid, (WIFSIGNALED(p->status) ? "," : ""));

				if (WIFEXITED(p->status)) {
					if (WEXITSTATUS(p->status) == EXIT_SUCCESS) {
						sock_printf(sock, "widget_set [%u] s2 1 3 {successfully.}\n",
								p->pid);
					}
					else {
						sock_printf(sock, "widget_set [%u] s2 1 3 {with code 0x%02X.}\n",
								p->pid, WEXITSTATUS(p->status));
					}
				}
				else if (WIFSIGNALED(p->status)) {
					sock_printf(sock, "widget_set [%u] s2 1 3 {killed by SIG %d.}\n",
						p->pid, WTERMSIG(p->status));
				}

				if (lcd_hgt > 3)
					sock_printf(sock, "widget_set [%u] s3 1 4 {Exec time: %lds}\n",
							p->pid, p->endtime - p->starttime);
			}
			else {
				sock_printf(sock, "widget_add [%u] s1 string\n", p->pid);
				sock_printf(sock, "widget_add [%u] s2 string\n", p->pid);
				sock_printf(sock, "widget_set [%u] s1 1 1 {%s}\n",
						p->pid, p->cmd->displayname);
				if (WIFEXITED(p->status)) {
					if (WEXITSTATUS(p->status) == EXIT_SUCCESS) {
						sock_printf(sock, "widget_set [%u] s2 1 2 {succeeded}\n",
								p->pid, p->status);
					}
					else {
						sock_printf(sock, "widget_set [%u] s2 1 2 {finished (0x%02X)}\n",
								p->pid, p->status);
					}
				}
				else if (WIFSIGNALED(p->status)) {
					sock_printf(sock, "widget_set [%u] s2 1 2 {killed by SIG %d}\n",
							p->pid, WTERMSIG(p->status));

				}
			}
			return 1;
		}
	}
	return 0;
}


static int main_loop(void)
{
	int num_bytes;
	char buf[100];
	int keepalive_delay = 0;
	int status_delay = 0;

	/* Continuously check if we get a menu event... */
	while (!Quit && ((num_bytes = sock_recv_string(sock, buf, sizeof(buf)-1)) >= 0)) {
		if (num_bytes == 0) {
			ProcInfo *p;

			/* wait for 1/10th of a second */
			usleep(100000);

			/* send an empty line every 3 seconds to make sure the server still exists */
			if (keepalive_delay++ >= 30) {
				keepalive_delay = 0;
				if (sock_send_string(sock, "\n") < 0) {
					break; /* Out of while loop */
				}
			}

			/* check for a screen to show and procinfo deletion every second */
			if (status_delay++ >= 10) {
				status_delay = 0;

				/* delete the ProcInfo from the queue */
				for (p = proc_queue; p != NULL; p = p->next) {
					ProcInfo *pn = p->next;

					if ((pn != NULL) && (pn->shown)) {
						p->next = pn->next;
						free(pn);
					}
				}
				/* deleting queue head is special */
				if ((proc_queue != NULL) && (proc_queue->shown)) {
					p = proc_queue;
					proc_queue = proc_queue->next;
					free(p);
				}

				/* look for a process to display, display it & mark it as shown */
				for (p = proc_queue; p != NULL; p = p->next) {
					p->shown |= show_procinfo_msg(p);
				}
			}
		}
		else {
			process_response(buf);
		}
	}

	if (!Quit)
		report(RPT_ERR, "Server disconnected (or connection error)");
	return 0;
}

/* EOF */
