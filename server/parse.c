/** \file server/parse.c
 * Handles input commands from clients, by splitting strings into tokens
 * and passing arguments to the appropriate handler.
 *
 * It works much like a command line.  Only the first token is used to
 * determine what function to call.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2008, Peter Marschall
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/LL.h"
#include "shared/sockets.h"
#include "shared/report.h"
#include "clients.h"
#include "commands/command_list.h"
#include "parse.h"
#include "sock.h"

#define MAX_ARGUMENTS 40


static inline int is_whitespace(char x)	{
	return ((x == ' ') || (x == '\t') || (x == '\r'));
}

static inline int is_final(char x) {
	return ((x == '\n') || (x == '\0'));
}

static inline int is_opening_quote(char x, char q) {
	return ((q == '\0') && ((x == '\"') || (x == '{')));
}

static inline int is_closing_quote(char x, char q) {
	return (((q == '{') && (x == '}')) || ((q == '\"') && (x == '\"')));
}


static int parse_message(const char *str, Client *c)
{
	typedef enum { ST_INITIAL, ST_WHITESPACE, ST_ARGUMENT, ST_FINAL } State;
	State state = ST_INITIAL;

	int error = 0;
	char quote = '\0';	/* The quote used to open a quote string */
	int pos = 0;
	char *arg_space;
	int argc = 0;
	char *argv[MAX_ARGUMENTS];
	int argpos = 0;
	CommandFunc function = NULL;

	debug(RPT_DEBUG, "%s(str=\"%.120s\", client=[%d])", __FUNCTION__, str, c->sock);

	/* We will create a list of strings that is shorter or equally long as
	 * the original string str.
	 */
	arg_space = malloc(strlen(str)+1);
	if (arg_space == NULL) {
		report(RPT_ERR, "%s: Could not allocate memory", __FUNCTION__);
		sock_send_error(c->sock, "error allocating memory!\n");
	}

	argv[0] = arg_space;

	while ((state != ST_FINAL) && !error) {
		char ch = str[pos++];

		switch (state) {
		  case ST_INITIAL:
		  case ST_WHITESPACE:
			if (is_whitespace(ch))
				break;
			if (is_final(ch)) {
				state = ST_FINAL;
				break;
			}
			/* otherwise fall through */
			state = ST_ARGUMENT;
		  case ST_ARGUMENT:
			if (is_final(ch)) {
				if (quote)
					error = 2;
				if (argc >= MAX_ARGUMENTS-1) {
					error = 1;
				}
				else {
					argv[argc][argpos] = '\0';
					argv[argc+1] = argv[argc] + argpos + 1;
					argc++;
					argpos = 0;
				}
				state = ST_FINAL;
			}
			else if (ch == '\\') {
			 	if (str[pos]) {
			 		/* We solve quoted chars here right away */
					const char escape_chars[] = "nrt";
					const char escape_trans[] = "\n\r\t";
			 		char *p = strchr(escape_chars, str[pos]);

					/* Is it wise to have the characters \n, \r & \t expanded ?
					 * Can the displays deal with them ?
					 */
					if (p != NULL) {
						/* Insert a replacement for the code */
						argv[argc][argpos++] = escape_trans[p - escape_chars];
					}
					else {
						 /* Copy char literally */
						argv[argc][argpos++] = str[pos];
					}
					pos++;
			 	}
			 	else {
			 		error = 2;
					/* alternative: argv[argc][argpos++] = ch; */
					if (argc >= MAX_ARGUMENTS-1) {
						error = 1;
					}
					else {
						argv[argc][argpos] = '\0';
						argv[argc+1] = argv[argc] + argpos + 1;
						argc++;
						argpos = 0;
					}
			 		state = ST_FINAL;
			 	}
			}
			else if (is_opening_quote(ch, quote)) {
				quote = ch;
			}
			else if (is_closing_quote(ch, quote)) {
				quote = '\0';
				if (argc >= MAX_ARGUMENTS-1) {
					error = 1;
				}
				else {
					argv[argc][argpos] = '\0';
					argv[argc+1] = argv[argc] + argpos + 1;
					argc++;
					argpos = 0;
				}
				state = ST_WHITESPACE;
			}
			else if (is_whitespace(ch) && (quote == '\0')) {
				if (argc >= MAX_ARGUMENTS-1) {
					error = 1;
				}
				else {
					argv[argc][argpos] = '\0';
					argv[argc+1] = argv[argc] + argpos + 1;
					argc++;
					argpos = 0;
				}
				state = ST_WHITESPACE;
			}
			else {
				argv[argc][argpos++] = ch;
			}
			break;
		  case ST_FINAL:
		  	/* This will never be reached */
			break;
		}
	}
	if (argc < MAX_ARGUMENTS)
		argv[argc] = NULL;
	else
		error = 1;

	if (error) {
		sock_send_error(c->sock, "Could not parse command\n");
		free(arg_space);
		return 0;
	}

#if 0 /* show what we have parsed */
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s%c", argv[i], (i == argc-1) ? '\n' : ' ');
	}
#endif

	/* Now find and call the appropriate function...*/
	function = get_command_function(argv[0]);

	if (function != NULL) {
		error = function(c, argc, argv);
		if (error) {
			sock_printf_error(c->sock, "Function returned error \"%.40s\"\n", argv[0]);
			report(RPT_WARNING, "Command function returned an error after command from client on socket %d: %.40s", c->sock, str);
		}
	}
	else {
		sock_printf_error(c->sock, "Invalid command \"%.40s\"\n", argv[0]);
		report(RPT_WARNING, "Invalid command from client on socket %d: %.40s", c->sock, str);
	}

	free(arg_space);
	return 0;
}


int
parse_all_client_messages(void)
{
	Client *c;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	for (c = clients_getfirst(); c != NULL; c = clients_getnext()) {
		char *str;

		/* And parse all its messages...*/
		/*debug(RPT_DEBUG, "parse: Getting messages...");*/
		for (str = client_get_message(c); str != NULL; str = client_get_message(c)) {
			parse_message(str, c);
			free(str);

			if (c->state == GONE) {
				sock_destroy_client_socket(c);
				break;
			}
		}
	}
	return 0;
}


