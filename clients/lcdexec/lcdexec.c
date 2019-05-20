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

#include "elektragen.h"

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
	const Command *cmd;	/**< pointer to the corresponding menu entry */
	pid_t pid;		/**< PID the process was started with */
	time_t starttime;	/**< start time of the process */
	time_t endtime;		/**< finishing time of the process */
	int status;		/**< exit status of the process */
	int feedback;		/**< what info to show to the user */
	int shown;		/**< tell if the info has been shown to the user */
} ProcInfo;


char * help_prefix =
"lcdexec - LCDproc client to execute commands from the LCDd menu\n"
"\n"
"Copyright (c) 2002, Joris Robijn, 2006-2008 Peter Marschall.\n"
"This program is released under the terms of the GNU General Public License.\n"
"\n";

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
Menu *main_menu = NULL;	/**< pointer to the main menu */
ProcInfo *proc_queue = NULL;	/**< pointer to the list of executed processes */

int lcd_wid = 0;		/**< LCD display width reported by the server */
int lcd_hgt = 0;		/**< LCD display height reported by the server */

int sock = -1;			/**< socket to connect to server */

int Quit = 0;			/**< indicate end of main loop */


/* Function prototypes */
static void exit_program(int val);
static void sigchld_handler(int signal);
static int process_config();
static int connect_and_setup(void);
static int process_response(char *str);
static int exec_command(Command *cmd);
static int show_procinfo_msg(ProcInfo *p);
static int main_loop(void);


#define CHAIN(e,f) { if (e>=0) { e=(f); }}
#define CHAIN_END(e) { if (e<0) { report(RPT_CRIT,"Critical error, abort"); exit(e); }}


int main(int argc, const char **argv)
{
	// only returns, if not in specload mode
	doSpecloadCheck(argc, argv);

	int error = 0;
	struct sigaction sa;

	CHAIN(error, process_config());

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

static void on_fatal_error(ElektraError * error) // TODO (elektra): finalize method
{
	fprintf(stderr, "ERROR: %s\n", elektraErrorDescription(error));
	exit(EXIT_FAILURE);
}

static int process_config()
{
	ElektraError * error = NULL;
	Elektra * elektra;
	int rc = loadConfiguration(&elektra, &error);

	if (rc == -1)
	{
		fprintf(stderr, "An error occurred while initializing elektra: %s", elektraErrorDescription(error));
		elektraErrorReset(&error);
		return -1;
	}

	if (rc == 1)
	{
		// help mode
		printHelpMessage(NULL, help_prefix);
		return 0;
	}

	elektraFatalErrorHandler(elektra, on_fatal_error);

	address = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_ADDRESS));
	port = elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_PORT);
	report_level = elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_REPORTLEVEL);
	report_dest = elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_REPORTTOSYSLOG) ? RPT_DEST_SYSLOG : RPT_DEST_STDERR;
	foreground = elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_FOREGROUND);
	pidfile = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_PIDFILE));

	displayname = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_DISPLAYNAME));

	default_shell = strdup(elektraGet(elektra, ELEKTRA_TAG_LCDEXEC_SHELL));

	main_menu = elektraGet(elektra, ELEKTRA_TAG_MENU_MAIN);
	// fail on non-existent main menu;
	if (main_menu == NULL) {
		report(RPT_ERR, "no main menu found in configuration");
		return -1;
	}
	
	main_menu_process(main_menu);

#if defined(DEBUG)
	menu_dump(main_menu, 0);
#endif



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
	if (main_menu_sock_send(main_menu, sock) < 0) {
		return -1;
	}

	return 0;
}

/**
 * returns the parameter with the given id, if it is contained in the given command
 * returns NULL otherwise
 */
static CommandParameter * find_parameter(Command * command, int id, CommandParameterType * type)
{
	for (kdb_long_long_t i = 0; i < command->parametersSize; i++)
	{
		switch (command->parameterTypes[i])
		{
		case COMMAND_PARAMETER_TYPE_ALPHA:
			if(command->parameters[i].alpha->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		case COMMAND_PARAMETER_TYPE_CHECKBOX:
			if(command->parameters[i].checkbox->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		case COMMAND_PARAMETER_TYPE_IP:
			if(command->parameters[i].ip->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		case COMMAND_PARAMETER_TYPE_NUMERIC:
			if(command->parameters[i].numeric->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		case COMMAND_PARAMETER_TYPE_RING:
			if(command->parameters[i].ring->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		case COMMAND_PARAMETER_TYPE_SLIDER:
			if(command->parameters[i].slider->id == id) {
				*type = command->parameterTypes[i];
				return &command->parameters[i];
			}
			break;
		}
	}
	return NULL;
}

/**
 * returns true and sets *command to the command, if id found and actually command
 * returns true and sets *command = NULL, if id found but not command
 * returns false and sets *command = NULL, if id not found
 */
static bool find_triggered_command(Menu * menu, int id, Command ** command)
{
	*command = NULL;
	for (kdb_long_long_t i = 0; i < menu->entriesSize; i++)
	{
		switch (menu->entryTypes[i])
		{
		case MENU_ENTRY_TYPE_COMMAND:
			if(menu->entries[i].command->id == id) {
				*command = menu->entries[i].command;
				return true;
			} else if(menu->entries[i].command->actionId == id) {
				return true;
			}
			CommandParameterType t;
			if(find_parameter(menu->entries[i].command, id, &t) != NULL)
			{
				return true;
			}
			break;
		case MENU_ENTRY_TYPE_MENU:
			if(menu->entries[i].menu->id == id)
			{
				return true;
			}
			if(find_triggered_command(menu->entries[i].menu, id, command))
			{
				return true;
			}
			break;
		}
	}
	
	return false;
}


/**
 * returns true and sets *parameter to the parameter and *type to the corresponding type, if id found and actually parameter
 * returns true and sets *parameter = NULL, if id found but not parameter
 * returns false and sets *parameter = NULL, if id not found
 */
static bool find_triggered_parameter(Menu * menu, int id, CommandParameter ** parameter, CommandParameterType * type)
{
	*parameter = NULL;
	for (kdb_long_long_t i = 0; i < menu->entriesSize; i++)
	{
		switch (menu->entryTypes[i])
		{
		case MENU_ENTRY_TYPE_COMMAND:
			if(menu->entries[i].command->id == id) {
				return true;
			} else if(menu->entries[i].command->actionId == id) {
				return true;
			}
			*parameter = find_parameter(menu->entries[i].command, id, type);
			if(*parameter != NULL)
			{
				return true;
			}
			break;
		case MENU_ENTRY_TYPE_MENU:
			if(menu->entries[i].menu->id == id)
			{
				return true;
			}
			if(find_triggered_parameter(menu->entries[i].menu, id, parameter, type))
			{
				return true;
			}
			break;
		}
	}
	
	return false;
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
				if (argc < 3) {
					report(RPT_WARNING, "Server gave invalid response");
					free(str2);
					return -1;
				}

				/* Find the entry by id */
				Command * command;
				if (!find_triggered_command(main_menu, atoi(argv[2]), &command)) {
					report(RPT_WARNING, "Could not find the item id given by the server");
					free(str2);
					return -1;
				}

				if(command == NULL) {
						report(RPT_WARNING, "Illegal menu entry type for event");
						free(str2);
						return -1;
				}

				exec_command(command);
		}
		else if ((strcmp(argv[1], "plus") == 0) ||
			 (strcmp(argv[1], "minus") == 0) ||
			 (strcmp(argv[1], "update") == 0)) {

			if (argc < 4) {
				report(RPT_WARNING, "Server gave invalid response");
				free(str2);
				return -1;
			}

			/* Find the entry by id */
			CommandParameter * parameter;
			CommandParameterType parameterType;
			if (!find_triggered_parameter(main_menu, atoi(argv[2]), &parameter, &parameterType)) {
				report(RPT_WARNING, "Could not find the item id given by the server");
				free(str2);
				return -1;
			}

			if(parameter == NULL) {
					report(RPT_WARNING, "Illegal menu entry type for event");
					free(str2);
					return -1;
			}

			switch (parameterType) {
				case COMMAND_PARAMETER_TYPE_SLIDER:
					parameter->slider->value = atoi(argv[3]);
					break;
				case COMMAND_PARAMETER_TYPE_RING:
					parameter->ring->value = atoi(argv[3]);
					break;
				case COMMAND_PARAMETER_TYPE_NUMERIC:
					parameter->numeric->value = atoi(argv[3]);
					break;
				case COMMAND_PARAMETER_TYPE_ALPHA:
					parameter->alpha->value = realloc(parameter->alpha->value,
									  strlen(argv[3]));
					strcpy(parameter->alpha->value, argv[3]);
					break;
				case COMMAND_PARAMETER_TYPE_IP:
					parameter->ip->value = realloc(parameter->ip->value,
									strlen(argv[3]));
					strcpy(parameter->ip->value, argv[3]);
					break;
				case COMMAND_PARAMETER_TYPE_CHECKBOX:
					if ((parameter->checkbox->allowgray) &&
					    (strcasecmp(argv[3], "gray") == 0))
						parameter->checkbox->value = 2;
					else if (strcasecmp(argv[3], "on") == 0)
						parameter->checkbox->value = 1;
					else
						parameter->checkbox->value = 0;
					break;					
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

static int exec_command(Command *cmd)
{
	if (cmd == NULL  || strlen(cmd->exec) == 0) {
		return -1;
	}

	const char *command = cmd->exec;
	const char *argv[4];
	pid_t pid;
	ProcInfo *p;
	char *envp[cmd->parametersSize+1];

	/* set argument vector */
	argv[0] = default_shell;
	argv[1] = "-c";
	argv[2] = command;
	argv[3] = NULL;

	/* set environment vector: allocate & fill contents */
	for (kdb_long_long_t i = 0; i < cmd->parametersSize; ++i) {
		char buf[1025];

		CommandParameter param = cmd->parameters[i];
		switch (cmd->parameterTypes[i]) {
			case COMMAND_PARAMETER_TYPE_SLIDER:
				envp[i] = elektraFormat(buf, "%s="ELEKTRA_LONG_F, param.slider->envname, param.slider->value);
				break;
			case COMMAND_PARAMETER_TYPE_RING:
				envp[i] = elektraFormat(buf, "%s=%s", param.ring->envname, param.ring->strings[param.ring->value]);
				break;
			case COMMAND_PARAMETER_TYPE_NUMERIC:
				envp[i] = elektraFormat(buf, "%s="ELEKTRA_LONG_F, param.numeric->envname, param.numeric->value);
				break;
			case COMMAND_PARAMETER_TYPE_ALPHA:
				envp[i] = elektraFormat(buf, "%s=%s", param.alpha->envname, param.alpha->value);
				break;
			case COMMAND_PARAMETER_TYPE_IP:
				envp[i] = elektraFormat(buf, "%s=%s", param.ip->envname, param.ip->value);
				break;
			case COMMAND_PARAMETER_TYPE_CHECKBOX:
				switch (param.checkbox->value)
				{
				case CHECKBOX_STATE_ON:
					if(strlen(param.checkbox->ontext) > 0) {
						envp[i] = elektraFormat(buf, "%s", param.checkbox->ontext);
					} else {
						envp[i] = elektraFormat(buf, "%s="ELEKTRA_LONG_F, param.checkbox->envname, param.checkbox->value);
					}
					break;
				case CHECKBOX_STATE_OFF:
					if(strlen(param.checkbox->offtext) > 0) {
						envp[i] = elektraFormat(buf, "%s", param.checkbox->offtext);
					} else {
						envp[i] = elektraFormat(buf, "%s="ELEKTRA_LONG_F, param.checkbox->envname, param.checkbox->value);
					}
					break;
				case CHECKBOX_STATE_GRAY:
					if(strlen(param.checkbox->graytext) > 0) {
						envp[i] = elektraFormat(buf, "%s", param.checkbox->graytext);
					} else {
						envp[i] = elektraFormat(buf, "%s="ELEKTRA_LONG_F, param.checkbox->envname, param.checkbox->value);
					}
					break;
				}
				break;
			default:
				/* error ? */
				break;
		}

		debug(RPT_DEBUG, "Environment: %s", envp[i]);
	}
	envp[cmd->parametersSize] = NULL;

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
			p->feedback = cmd->feedback;
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
	for (kdb_long_long_t i = 0; i < cmd->parametersSize; i++) {
		elektraFree(envp[i]);
	}

	return 0;
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
