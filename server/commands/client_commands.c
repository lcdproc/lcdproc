/*
 * client_commands.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 *
 * This particular file defines actions concerning clients.
 *
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
#include "render.h"
#include "client.h"

/***************************************************************
 * Debugging only..  prints out a list of arguments it receives
 */
int
test_func_func (Client * c, int argc, char **argv)
{
	int i;
	char str[256];

	for (i = 0; i < argc; i++) {
		snprintf (str, sizeof(str), "test_func_func:  %i -> %s\n", i, argv[i]);
		report (RPT_INFO, str);
		sock_send_string (c->sock, str);
	}
	return 0;
}

/***************************************************************
 * The client must say "hello" before doing anything else.
 *
 * It returns a string of info about the server to the client
 *
 * usage: hello
 */
int
hello_func (Client * c, int argc, char **argv)
{
	char str[256];

	/* TODO:  Give *real* info about the server/lcd...*/

	if (argc > 1) {
		sock_send_string (c->sock, "huh? extra parameters ignored\n");
	}

	debug(RPT_INFO, "Hello!");

	memset(str, '\0', sizeof(str));
	snprintf (str, sizeof(str), "connect LCDproc %s protocol %s lcd wid %i hgt %i cellwid %i cellhgt %i\n",
		VERSION, PROTOCOL_VERSION, display_props->width, display_props->height, display_props->cellwidth, display_props->cellheight);

/*	lcdproc (client) depends on the above format...
 *	snprintf (str, sizeof(str), "connect LCDproc %s protocol %s LCD %ix%i with cells %ix%i\n",
 *		version, protocol_version, lcd.wid, lcd.hgt, lcd.cellwid, lcd.cellhgt);
 */

	sock_send_string (c->sock, str);

	c->ack = 1;

	return 0;
}

/***************************************************
 * sets info about the client, such as its name
 *
 * usage: client_set -name <id>
 */
int
client_set_func (Client * c, int argc, char **argv)
{
	int i;
	char str[16], buf[80];

	memset(str, '\0', sizeof(str));
	if (!c->ack)
		return 1;

	if (argc != 3) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: client_set -name <name>\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh? Not enough parameters\n");
				break;
			default:
				sock_send_string (c->sock, "huh? Too many parameters\n");
				break;
		}
		return 0;
	}

	i = 1;
	do {
		char *p;

		/* This bit of code means that "-name" is the same as "name"...*/
		p = argv[i];
		if (*p == '-')
			p++;

		/* Handle the "name" parameter*/
		if (strcmp (p, "name") == 0) {
			i++;
			if (argv[i] == '\0') {
				snprintf (buf, sizeof(buf), "huh? internal error: no parameter #%d\n", i);
				sock_send_string (c->sock, buf);
				continue;
			}

			if (strlen(argv[i]) > sizeof(str) -1) {
				sock_send_string (c->sock, "huh? name too long\n");
			} else {
				strncpy(str, argv[i], sizeof(str) - 1);

				debug(RPT_DEBUG, "client_set: name=\"%s\"", argv[i]);

				/* set the name...*/
				if (c->name)
					free (c->name);

				if ((c->name = strdup (str)) == NULL) {
					sock_send_string(c->sock, "huh? error allocating memory!\n");
				} else {
					sock_send_string(c->sock, "success\n");
					i++; /* bypass argument (name string)*/
				}
			}
		} else {
			snprintf (buf, sizeof(buf), "huh? invalid parameter (%s)\n", p);
			sock_send_string (c->sock, buf);
		}
	} while (++i < argc);

	return 0;
}

/******************************************************************
 * Tells the server the client would like to accept keypresses
 * of a particular type
 *
 * usage: client_add_key <keylist>
 */
int
client_add_key_func (Client * c, int argc, char **argv)
{
	char *  keys ;

	if (!c->ack)
		return 1;

	if (argc != 2) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: client_add_key <keylist>\n");
				break;
			default:
				sock_send_string (c->sock, "huh?  Too many parameters...\n");
				break;
		}
		return 0;
	}

	keys = argv[1];
	debug(RPT_DEBUG, "client_add_key: current client will handle key(s) %s", keys);

	if (!c->client_keys) {
		/* No keys list, create a new one*/
		c->client_keys = strdup( keys );
	} else {
		/* Add supplied keys to existing list
		 * NOTE: There could be duplicates in the resulting list
		 *    That's OK, it's the existence of the key in the list
		 *    that's important.  We'll be more careful in the delete
		 *    key function.
		 */
		char *  new ;
		int  new_len = strlen(c->client_keys) + strlen(keys) + 1 ;

		new = realloc( c->client_keys, new_len );
		if( new ) {
			c->client_keys = new ;
			strcat( new, keys );
		} else {
			sock_send_string(c->sock, "huh? could not allocate memory for new keys\n");
			return 0;
		}
	}

	if (c->client_keys) {
		sock_send_string(c->sock, "success\n");
	} else {
		sock_send_string(c->sock, "huh? failed\n");
	}

	return 0;
}

/*********************************************************************
 * Tells the server the client would NOT like to accept keypresses
 * of a particular type
 *
 * usage: client_del_key <keylist>
 */
int
client_del_key_func (Client * c, int argc, char **argv)
{
	char *  keys ;

	if (!c->ack)
		return 1;

	if (argc != 2) {
		if (argc == 1) {
			sock_send_string (c->sock, "huh? usage: client_del_key <keylist>\n");
			return 0;
		} else {
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
			return 0;
		}
	}

	keys = argv[1] ;
	debug(RPT_DEBUG, "client_del_key: Deleting key(s) %s from client_keys", keys);

	if (c->client_keys) {
		/* Client has keys, remove keys from the list
		 * NOTE: We let malloc/realloc remember the length
		 *    of the allocated storage.  If keys are later
		 *    added, realloc (in add_key above) will make
		 *    sure there is enough space at c->data->client_keys
		 */
		char *  from ;
		char *  to ;

		to = from = c->client_keys ;
		while( *from ) {
			/*  Is this key to be deleted from the list?*/
			if( strchr( keys, *from ) ) {
				/* Yes, skip it*/
				++from ;
			} else {
				/* No, save it*/
				*to++ = *from++ ;
			}
		}
	}

	sock_send_string(c->sock, "success\n");

	return 0;
}

/***************************************************************************
 * Toggles the backlight, if enabled.
 *
 * usage: backlight <on|off|toggle|blink|flash>
 */
int
backlight_func (Client * c, int argc, char **argv)
{
	if (!c->ack)
		return 1;

	if (argc != 2) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh?  usage: backlight <on|off|toggle|blink|flash>\n");
				break;
			default:
				sock_send_string (c->sock, "huh?  Too many parameters...\n");
				break;
		}
		return 0;
	}

	debug (RPT_DEBUG, "backlight(%s)", argv[1]);


	//backlight = (backlight && 1);  /* only preserves ON/OFF bit*/

	if (strcmp ("on", argv[1]) == 0) {
		c->backlight = BACKLIGHT_ON;

	} else if (strcmp ("off", argv[1]) == 0) {
		c->backlight = BACKLIGHT_OFF;

	} else if (strcmp ("toggle", argv[1]) == 0) {
		if (c->backlight == BACKLIGHT_ON)
			c->backlight = BACKLIGHT_OFF;
		else if (c->backlight == BACKLIGHT_OFF)
			c->backlight = BACKLIGHT_ON;

	} else if (strcmp ("blink", argv[1]) == 0) {
		c->backlight |= BACKLIGHT_BLINK;

	} else if (strcmp ("flash", argv[1]) == 0) {
		c->backlight |= BACKLIGHT_FLASH;
	}

	sock_send_string(c->sock, "success\n");

	return 0;

}

/****************************************************************************
 * info_func
 *
 * usage: info
 */
int
info_func (Client * c, int argc, char **argv)
{
	char str[1024];

	if (argc > 1) {
		sock_send_string (c->sock, "huh?  Extra arguments ignored...\n");
	}

	memset(str, '\0', sizeof(str));
	snprintf (str, sizeof(str)-1, (char*) drivers_get_info());

	sock_send_string (c->sock, str);

	return 0;
}

