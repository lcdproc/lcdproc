/*
 * parse.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Handles input commands from clients, by splitting strings into tokens
 * and passing arguments to the appropriate handler.
 *
 * It works much like a command line.  Only the first token is used to
 * determine what function to call.
 *
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

/* This is a big function... TOO big.  How to trim....
 * TODO: Simplify... simplify...
 */

#define MAX_ARGUMENTS 40

int
parse_all_client_messages2 ()
{
	int i; /* int j, len;*/
	/*int newtoken, inquote;*/
	Client * c;
	char *str, *p, *q, *s;
/*   char *tok;*/
	int argc;
	char *argv[MAX_ARGUMENTS];
	/*char delimiters[] = " ";*/
	char leftquote[] = "\"'`([{";
	char rightquote[] = "\"'`)]}";
	char errmsg[256];
	int invalid = 0;
	int quoteindex;

	debug( RPT_DEBUG, "parse_all_client_messages()" );

	for (i = 0; i < MAX_ARGUMENTS; i++) {
		argv[i] = NULL;
	}

#define	SEPARATOR_CHAR	' '
#define	LINE_TERM_CHAR	'\0'
#define COMMENT_CHAR '#'

	for( c=clients_getfirst(); c; c=clients_getnext()) {

		/* And parse all its messages...*/
		/*debug(RPT_DEBUG, "parse: Getting messages...");*/
		for (str = client_get_message (c); str; str = client_get_message (c)) {

			debug (RPT_DEBUG, "parse: ...%s", str);
			/* Now, split up the string...*/
			argc = 0;
			i = 0;
			q = p = str;

			if (*p == COMMENT_CHAR) {
				continue; /* found a comment line - skip it...*/
			}

			debug (RPT_DEBUG, "starting string scan...");

			do {
				/* bypass initial white space...*/
				while ((*p == SEPARATOR_CHAR) && (*p)) {
					p++;
					q++;
				}

				/* If (*p) is null here, we reached the end of
				 * an empty parameter... so one of two things
				 * is true:
				 *
				 * 1. There is nothing but white space on this line (odd..)
				 * 2. This is trailing white space (odd... but allowable)
				 */

				if (*p == LINE_TERM_CHAR) {
					break;
					/* if there are no arguments, argc == 0 and will fail
					 * appropriately...
					 *
					 * if this is trailing white space, ignore the argc++ at the
					 * end and claim this as the end...
					 */
				}

				/* Handle quoted strings...*/
				if ((s = strchr(leftquote, *p)) != NULL) {
					quoteindex = s - leftquote;
					/*debug(RPT_DEBUG, "found <%c> at index [%d] = <%c>", *p, quoteindex, leftquote[quoteindex]);*/
					q = ++p; /* past open quote...*/
					while ((rightquote[quoteindex] != *p) && (*p != LINE_TERM_CHAR)) {
						p++;
					}
					if (*p == LINE_TERM_CHAR) {
						/* We just sucked up the rest of the command line: ERROR!!*/
						snprintf (errmsg, sizeof(errmsg), "huh? unterminated string! missing ending %c\n",
							rightquote[quoteindex]);
						sock_send_string (c->sock, errmsg);
						continue;
					} else {
						*p = LINE_TERM_CHAR;	/* terminate string*/
						p++;		/* bypass to next character*/
						/* Note that next character could be a EndOfLine (null)
						 * if the string was last on the line, or it could be
						 * something else... is it a blank?
						 */
						if (*p != SEPARATOR_CHAR && *p != LINE_TERM_CHAR) {
							sock_send_string (c->sock, "huh? improperly terminated string! (missing whitespace)\n");
							continue;
						}
					}

				/* Otherwise, normal string...*/
				} else {
					while (*p != SEPARATOR_CHAR && *p != LINE_TERM_CHAR)
						p++;
				}

				/* Not end of line?*/
				if (*p) {
					*p = LINE_TERM_CHAR;
					/*debug(RPT_DEBUG, "found new token: %s", q);*/
					argv[i++] = q;
					q = ++p;
				} else {
					/*debug(RPT_DEBUG, "found new token: %s", q);*/
					argv[i++] = q;
				}
				/* At the end of this statement,
				 * *p will be '\0' if end of input reached;
				 * otherwise, it is the first character of the
				 * next part of the string.
				 */


				argc++;
			} while (*p && i < MAX_ARGUMENTS - 1);

			/*debug(RPT_DEBUG, "exiting string scan...");*/

			argv[argc] = NULL;
			if (argc < 1)
				continue;

			/* Now find and call the appropriate function...*/
			invalid = 1;
			for (i = 0; commands[i].keyword; i++) {
				if (0 == strcmp (argv[0], commands[i].keyword)) {
					invalid = commands[i].function (c, argc, argv);
					break; /* found our function - don't continue on...*/
				}
			}

			if (invalid) {
				snprintf (errmsg, sizeof(errmsg), "huh? Invalid command \"%.40s\"\n", argv[0]);
				sock_send_string (c->sock, errmsg);
			}

			free (str);			  /* fixed memory leak?*/
		}							  /* end for(str...)*/
	}

	return 0;
}

int parse_message (char * str, Client * c);

int
parse_all_client_messages ()
{
	Client * c;
	char * str;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	for( c=clients_getfirst(); c; c=clients_getnext()) {

		/* And parse all its messages...*/
		/*debug(RPT_DEBUG, "parse: Getting messages...");*/
		for (str = client_get_message (c); str; str = client_get_message (c)) {
			parse_message (str, c);
			free (str);
		}
	}
	return 0;
}

int parse_message (char * str, Client * c)
{
	typedef enum { ST_INITIAL, ST_WHITESPACE, ST_IGNORE, ST_ARGUMENT, ST_FINAL } State;
	State state = ST_INITIAL;

	char escape_chars[] = "nrt";
	char escape_trans[] = "\n\r\t";
	char errmsg[256];
	int error = 0;
	char used_quote = 0;	/* The quote used to open a quote string */
	int pos = 0;
	char ch;
	int i;
	char * arg_space;
	int argc =0 ;
	char *argv[MAX_ARGUMENTS];
	int argpos = 0;

	void close_arg () {
		if( argc == MAX_ARGUMENTS-1 ) {
			error = 1;
		}
		else {
			argv[argc][argpos] = 0;
			argv[argc+1] = argv[argc] + argpos + 1;
			argc ++;
			argpos = 0;
		}
	}

	debug( RPT_DEBUG, "%s( str=\"%.120s\", client=[%d] )", __FUNCTION__, str, c->sock );

	arg_space = malloc(strlen(str)+1);
	argv[0] = arg_space;
	/* We will create a new string that is shorter or equally long as
	 * the original string str.
	 */

	while( state != ST_FINAL && !error ) {
		ch = str[pos++];
		switch( state ) {

		  case ST_INITIAL:
		  case ST_WHITESPACE:
			switch( ch ) {
			  case '\r':
			  case '\t':
			  case ' ':
				break;
			  case '\n':
			  case 0:
				state = ST_FINAL;
				break;
			  default:
				state = ST_ARGUMENT;
				pos --; /* Rescan current char */
			}
			break;
		  case ST_IGNORE:
			switch( ch ) {
			  case '\n':
				state = ST_FINAL;
			  	break;
			}
			break;
		  case ST_ARGUMENT:
			switch( ch ) {
			  case '\r':
			  case '\t':
			  case ' ':
				if (used_quote) {
					/* We're in a quoted string, add it */
					argv[argc][argpos++] = ch;
				}
				else {
					close_arg();
					state = ST_WHITESPACE;
				}
				break;
			  case '\n':
			  case 0:
				if (used_quote) {
					error = 2;
				}
				close_arg();
				state = ST_FINAL;
				break;
			  case '\\':
			 	if (str[pos]) {
			 		/* We solve quoted chars here right away */
			 		char * p;
					p = strchr( escape_chars, str[pos] );
					if (p != NULL) {
						/* Insert a replacement for the code */
						argv[argc][argpos++] = escape_trans[(int)*p];
					}
					else {
						 /* Copy char litterally */
						argv[argc][argpos++] = str[pos];
					}
					pos++;
			 	}
			 	else {
			 		close_arg();
			 		error = 2;
			 		state = ST_FINAL;
			 	}
			 	break;
			  case '\"':
			  case '{':
				if (!used_quote) {
					used_quote = ch;
					break;
				}
				/* else fall through to default... */
			  default:
				if (used_quote) {
					/* Have we reached the end of the
					 * quote already ?
					 */
					if ((used_quote == '{' && ch == '}')
					|| (used_quote == '\"' && ch == '\"')) {
						used_quote = 0;
						break;
					}
				}
				argv[argc][argpos++] = ch;
			}
			break;
		  case ST_FINAL:
		  	/* This will never be reached */
			break;
		}
	}
	argv[argc] = NULL;
	if (error) {
		snprintf (errmsg, sizeof(errmsg), "huh? Could not parse command\n");
		report( RPT_WARNING, "Could not parse command from client on socket %d: %.40s", c->sock, str );
		sock_send_string (c->sock, errmsg);
		free( arg_space  );
		return 0;
	}

	/* Now find and call the appropriate function...*/
	error = 1;
	for (i = 0; commands[i].keyword; i++) {
		if (0 == strcmp (argv[0], commands[i].keyword)) {
			error = commands[i].function (c, argc, argv);
			break; /* found our function - don't continue on...*/
		}
	}

	if (error == 1) {
		snprintf (errmsg, sizeof(errmsg), "huh? Invalid command \"%.40s\"\n", argv[0]);
		sock_send_string (c->sock, errmsg);
		report( RPT_WARNING, "Invalid command from client on socket %d: %.40s", c->sock, str );
	}
	else if (error) {
		snprintf (errmsg, sizeof(errmsg), "huh? Function returned error \"%.40s\"\n", argv[0]);
		sock_send_string (c->sock, errmsg);
		report( RPT_WARNING, "Command function returned an error after command from client on socket %d: %.40s", c->sock, str );
	}

	free( arg_space );
	return 0;
}
