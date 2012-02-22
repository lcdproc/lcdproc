/** \file server/commands/client_commands.c
 * Implements handlers for general client commands.
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "shared/report.h"
#include "shared/sockets.h"

#include "drivers.h"
#include "client.h"
#include "render.h"
#include "input.h"
#include "client_commands.h"


/**
 * Debugging only..  prints out a list of arguments it receives
 */
int
test_func_func(Client *c, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		report(RPT_INFO, "%s: %i -> %s", __FUNCTION__, i, argv[i]);
		sock_printf(c->sock, "%s:  %i -> %s\n", __FUNCTION__, i, argv[i]);
	}
	return 0;
}

/**
 * The client must say "hello" before doing anything else.
 *
 * It sends back a string of info about the server to the client.
 *
 *\verbatim
 * Usage: hello
 *\endverbatim
 *
 * \todo  Give \em real info about the server/lcd
 */
int
hello_func(Client *c, int argc, char **argv)
{
	if (argc > 1) {
		sock_send_error(c->sock, "extra parameters ignored\n");
	}

	debug(RPT_INFO, "Hello!");

	sock_printf(c->sock, "connect LCDproc %s protocol %s lcd wid %i hgt %i cellwid %i cellhgt %i\n",
		VERSION, PROTOCOL_VERSION,
		display_props->width, display_props->height,
		display_props->cellwidth, display_props->cellheight);

	/* make note that client has sent hello */
	c->state = ACTIVE;

	return 0;
}

/**
 * The client should say "bye" before disconnecting
 *
 * The function does not respond to the client: it simply cuts connection.
 *
 *\verbatim
 * Usage: bye
 *\endverbatim
 */
int
bye_func(Client *c, int argc, char **argv)
{
	if (c != NULL) {
		debug(RPT_INFO, "Bye, %s!", (c->name != NULL) ? c->name : "unknown client");

		c->state = GONE;
		//sock_send_error(c->sock, "\"bye\" is currently ignored\n");
	}
	return 0;
}

/**
 * Sets info about the client, such as its name
 *
 *\verbatim
 * Usage: client_set -name <id>
 *\endverbatim
 */
int
client_set_func(Client *c, int argc, char **argv)
{
	int i;

	if (c->state != ACTIVE)
		return 1;

	if (argc != 3) {
		sock_send_error(c->sock, "Usage: client_set -name <name>\n");
		return 0;
	}

	i = 1;
	do {
		char *p = argv[i];

		/* ignore leading '-' in options: we allow both forms */
		if (*p == '-')
			p++;

		/* Handle the "name" option */
		if (strcmp(p, "name") == 0) {
			i++;
			if (argv[i] == '\0') {
				sock_printf_error(c->sock, "internal error: no parameter #%d\n", i);
				continue;
			}

			debug(RPT_DEBUG, "client_set: name=\"%s\"", argv[i]);

			/* set the name...*/
			if (c->name != NULL)
				free(c->name);

			if ((c->name = strdup(argv[i])) == NULL) {
				sock_send_error(c->sock, "error allocating memory!\n");
			}
			else {
				sock_send_string(c->sock, "success\n");
				i++; /* bypass argument (name string)*/
			}
		}
		else {
			sock_printf_error(c->sock, "invalid parameter (%s)\n", p);
		}
	} while (++i < argc);

	return 0;
}

/**
 * Tells the server the client would like to accept keypresses
 * of a particular type
 *
 *\verbatim
 * Usage: client_add_key [-exclusively|-shared] {<key>}+
 *\endverbatim
 */
int
client_add_key_func(Client *c, int argc, char **argv)
{
	int exclusively = 0;
	int argnr;

	if (c->state != ACTIVE)
		return 1;

	if (argc < 2) {
		sock_send_error(c->sock, "Usage: client_add_key [-exclusively|-shared] {<key>}+\n");
		return 0;
	}

	argnr = 1;
	if (argv[argnr][0] == '-') {
		if (strcmp( argv[argnr], "-shared") == 0) {
			exclusively = 0;
		}
		else if(strcmp(argv[argnr], "-exclusively") == 0) {
			exclusively = 1;
		}
		else {
			sock_printf_error(c->sock, "Invalid option: %s\n", argv[argnr]);
		}
		argnr++;
	}
	for ( ; argnr < argc; argnr++) {
		if (input_reserve_key(argv[argnr], exclusively, c) < 0) {
			sock_printf_error(c->sock, "Could not reserve key \"%s\"\n", argv[argnr]);
		}
	}
	sock_send_string(c->sock, "success\n");

	return 0;
}

/**
 * Tells the server the client would NOT like to accept keypresses
 * of a particular type
 *
 *\verbatim
 * Usage: client_del_key {<key>}+
 *\endverbatim
 */
int
client_del_key_func(Client *c, int argc, char **argv)
{
	int argnr;

	if (c->state != ACTIVE)
		return 1;

	if (argc < 2) {
		sock_send_error(c->sock, "Usage: client_del_key {<key>}+\n");
		return 0;
	}

	for (argnr = 1; argnr < argc; argnr++) {
		input_release_key(argv[argnr], c);
	}
	sock_send_string(c->sock, "success\n");

	return 0;
}

/**
 * Toggles the backlight, if enabled.
 *
 *\verbatim
 * Usage: backlight {on|off|toggle|blink|flash}
 *\endverbatim
 */
int
backlight_func(Client *c, int argc, char **argv)
{
	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: backlight {on|off|toggle|blink|flash}\n");
		return 0;
	}

	debug(RPT_DEBUG, "backlight(%s)", argv[1]);


	//backlight = (backlight && 1);  /* only preserves ON/OFF bit*/

	if (strcmp ("on", argv[1]) == 0) {
		c->backlight = BACKLIGHT_ON;
	}
	else if (strcmp ("off", argv[1]) == 0) {
		c->backlight = BACKLIGHT_OFF;
	}
	else if (strcmp ("toggle", argv[1]) == 0) {
		if (c->backlight == BACKLIGHT_ON)
			c->backlight = BACKLIGHT_OFF;
		else if (c->backlight == BACKLIGHT_OFF)
			c->backlight = BACKLIGHT_ON;
	}
	else if (strcmp ("blink", argv[1]) == 0) {
		c->backlight |= BACKLIGHT_BLINK;
	}
	else if (strcmp ("flash", argv[1]) == 0) {
		c->backlight |= BACKLIGHT_FLASH;
	}

	sock_send_string(c->sock, "success\n");

	return 0;

}

/**
 * Sends back information about the loaded drivers.
 *
 *\verbatim
 * Usage: info
 *\endverbatim
 */
int
info_func(Client *c, int argc, char **argv)
{
	if (c->state != ACTIVE)
		return 1;

	if (argc > 1) {
		sock_send_error(c->sock, "Extra arguments ignored...\n");
	}

	sock_printf(c->sock, "%s\n", drivers_get_info());

	return 0;
}

