/*
  parse.c

  Handles input commands from clients, by splitting strings into tokens
  and passing arguments to the appropriate handler.

  It works much like a command line.  Only the first token is used to
  determine what function to call.
  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/LL.h"
#include "shared/sockets.h"
#include "shared/debug.h"
#include "clients.h"
#include "client_functions.h"
#include "parse.h"

// This is a big function... TOO big.  How to trim....
// TODO: Simplify... simplify...
int
parse_all_client_messages ()
{
	int i; // int j, len;
	//int newtoken, inquote;
	client *c;
	char *str, *p, *q, *s;
//   char *tok;
	int argc;
	char *argv[256];
	//char delimiters[] = " ";
	char leftquote[] = "\"'`([{";
	char rightquote[] = "\"'`)]}";
	char errmsg[256];
	int invalid = 0;
	int quoteindex;

	for (i = 0; i <= 256; i++) {
		argv[i] = NULL;
	}

#define	SEPARATOR_CHAR	' '
#define	LINE_TERM_CHAR	'\0'
#define COMMENT_CHAR '#'

	//debug("parse: Rewinding list...\n");
	LL_Rewind (clients);
	do {
		// Get the next client...
		//debug("parse: Getting client...\n");
		c = LL_Get (clients);
		if (c) {
			// And parse all its messages...
			//debug("parse: Getting messages...\n");
			for (str = client_get_message (c); str; str = client_get_message (c)) {
				debug ("parse: ...%s\n", str);
				// Now, split up the string...
				argc = 0;
				i = 0;
				q = p = str;

				if (*p == COMMENT_CHAR) {
					continue; // found a comment line - skip it...
				}

				//fprintf(stderr, "starting string scan...\n");

				do {
					// bypass initial white space...
					while ((*p == SEPARATOR_CHAR) && (*p)) {
						p++;
						q++;
					}

					// If (*p) is null here, we reached the end of
					// an empty parameter... so one of two things
					// is true:
					//
					// 1. There is nothing but white space on this line (odd..)
					// 2. This is trailing white space (odd... but allowable)

					if (*p == LINE_TERM_CHAR) {
						break;
						// if there are no arguments, argc == 0 and will fail
						// appropriately...
						//
						// if this is trailing white space, ignore the argc++ at the
						// end and claim this as the end...
					}

					// Handle quoted strings...
					if ((s = strchr(leftquote, *p)) != NULL) {
						quoteindex = s - leftquote;
						//fprintf(stderr, "found <%c> at index [%d] = <%c>\n", *p, quoteindex, leftquote[quoteindex]);
						q = ++p; // past open quote...
						while ((rightquote[quoteindex] != *p) && (*p != LINE_TERM_CHAR)) {
							p++;
						}
						if (*p == LINE_TERM_CHAR) {
							// We just sucked up the rest of the command line: ERROR!!
							snprintf (errmsg, sizeof(errmsg), "huh? unterminated string! missing ending %c\n",
								rightquote[quoteindex]);
							sock_send_string (c->sock, errmsg);
							continue;
						} else {
							*p = LINE_TERM_CHAR;	// terminate string
							p++;		// bypass to next character
							// Note that next character could be a EndOfLine (null)
							// if the string was last on the line, or it could be
							// something else... is it a blank?
							if (*p != SEPARATOR_CHAR && *p != LINE_TERM_CHAR) {
								sock_send_string (c->sock, "huh? improperly terminated string! (missing whitespace)\n");
								continue;
							}
						}

					// Otherwise, normal string...
					} else {
						while (*p != SEPARATOR_CHAR && *p != LINE_TERM_CHAR)
							p++;
					}

					// Not end of line?
					if (*p) {
						*p = LINE_TERM_CHAR;
						//fprintf(stderr, "found new token: %s\n", q);
						argv[i++] = q;
						q = ++p;
					} else {
						//fprintf(stderr, "found new token: %s\n", q);
						argv[i++] = q;
					}
					// At the end of this statement,
					// *p will be '\0' if end of input reached;
					// otherwise, it is the first character of the
					// next part of the string.


					argc++;
				} while (*p);

				//fprintf(stderr, "exiting string scan...\n");

				argv[argc] = NULL;
				if (argc < 1)
					continue;

				// Now find and call the appropriate function...
				invalid = 1;
				for (i = 0; commands[i].keyword; i++) {
					if (0 == strcmp (argv[0], commands[i].keyword)) {
						invalid = commands[i].function (c, argc, argv);
						break; // found our function - don't continue on...
					}
				}

				if (invalid) {
					snprintf (errmsg, sizeof(errmsg), "huh? Invalid command \"%s\"\n", argv[0]);
					sock_send_string (c->sock, errmsg);
				}

				free (str);			  // fixed memory leak?
			}							  // end for(str...)
		}								  // end if (c)
	} while (LL_Next (clients) == 0);

	return 0;
}
