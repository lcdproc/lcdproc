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

int
parse_all_client_messages ()
{
	int i, j;
	int newtoken, inquote;
	client *c;
	char *str;
//   char *tok;
	int argc;
	char *argv[256];
	char delimiters[] = " \0";
	char leftquote[] = "\0\"'`([{\0";
	char rightquote[] = "\0\"'`)]}\0";
	char errmsg[256];
	int invalid = 0;

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
				//len = strlen(str);
				argc = 0;
				newtoken = 1;
				inquote = 0;
				for (i = 0; str[i]; i++) {
					if (inquote)	  // Scan for the end of the quote
					{
						if (str[i] == rightquote[inquote]) {	// Found the end of the quote
							inquote = 0;
							str[i] = 0;
							newtoken = 1;
						}
					} else			  // Normal operation; split at delimiters
					{
						for (j = 1; leftquote[j]; j++) {
							// Found the beginning of a new quote...
							if (str[i] == leftquote[j]) {
								inquote = j;
								str[i] = 0;
								continue;
							}
						}
						for (j = 0; delimiters[j]; j++) {
							// Break into a new string...
							if (str[i] == delimiters[j]) {
								str[i] = 0;
								newtoken = 1;
								continue;
							}
						}
					}
					if (newtoken && str[i]) {
						newtoken = 0;
						argv[argc] = str + i;
						argc++;
					} else {
					}
				}
				if (inquote) {
					sprintf (errmsg, "huh? Unterminated string: missing %c\n", rightquote[inquote]);
					sock_send_string (c->sock, errmsg);
					continue;
				}
/*
	    for(tok = strtok(str, delimiters);
		tok;
		tok=strtok(NULL, delimiters))
	    {
	       argv[argc] = tok;
	       argc++;
	    }
*/
				argv[argc] = NULL;
				if (argc < 1)
					continue;

				// Now find and call the appropriate function...
//          debug("parse: Finding function...\n");
				invalid = 1;
				for (i = 0; commands[i].keyword; i++) {
//             debug("(checking %s)\n", commands[i].keyword);
					if (0 == strcmp (argv[0], commands[i].keyword)) {
//                debug("(FOUND %s)\n", commands[i].keyword);
						invalid = commands[i].function (c, argc, argv);
//                debug("parse: Returned %i...\n", err);
					}
				}
				if (invalid) {
					// FIXME:  Check for buffer overflows here...
					sprintf (errmsg, "huh? Invalid command \"%s\"\n", argv[0]);
					sock_send_string (c->sock, errmsg);
				}

				free (str);			  // fixed memory leak?
			}							  // end for(str...)
		}								  // end if (c)
	} while (LL_Next (clients) == 0);

	return 0;
}
