/*
 * client_functions.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
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

#include "main.h"
#include "render.h"
#include "clients.h"
#include "parse.h"
#include "screen.h"
#include "widget.h"
#include "client_functions.h"

int info_func (client * c, int argc, char **argv);
int sleep_func (client * c, int argc, char **argv);

client_function commands[] = {
	{"test_func", test_func_func},
	{"hello", hello_func},
	{"client_set", client_set_func},
	{"client_add_key", client_add_key_func},
	{"client_del_key", client_del_key_func},
	{"screen_add_key", screen_add_key_func},
	{"screen_del_key", screen_del_key_func},
	{"screen_add", screen_add_func},
	{"screen_del", screen_del_func},
	{"screen_set", screen_set_func},
	{"widget_add", widget_add_func},
	{"widget_del", widget_del_func},
	{"widget_set", widget_set_func},
	{"menu_add", menu_add_func},
	{"menu_del", menu_del_func},
	{"menu_set", menu_set_func},
	{"menu_add_item", menu_add_item_func},
	{"menu_del_item", menu_del_item_func},
	{"menu_set_item", menu_set_item_func},
	/* Misc stuff...*/
	{"backlight", backlight_func},
	{"output", output_func},
	{"noop", noop_func},
	{"info", info_func},
	{"sleep", sleep_func},
	{NULL, NULL},
};

/* Remember: argc counts all arguments, including the command name;
 *     so it will actually be one more than the number of arguments given
 *     to the command...*/

#define NoArguments		(argc == 1)
#define AnyArguments		(argc != 1)
#define MoreArgumentsThan(a)	(argc > (a))

/* TODO:  Maybe more error-checking for "->"'s?*/

/***************************************************************
 * Debugging only..  prints out a list of arguments it receives
 */
int
test_func_func (client * c, int argc, char **argv)
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
hello_func (client * c, int argc, char **argv)
{
	char str[256];

	/* TODO:  Give *real* info about the server/lcd...*/

	if (AnyArguments) {
		sock_send_string (c->sock, "huh? extra parameters ignored\n");
	}

	debug(RPT_INFO, "Hello!");

	memset(str, '\0', sizeof(str));
	snprintf (str, sizeof(str), "connect LCDproc %s protocol %s lcd wid %i hgt %i cellwid %i cellhgt %i\n",
		version, protocol_version, display_props->width, display_props->height, display_props->cellwidth, display_props->cellheight);

/*	lcdproc (client) depends on the above format...
 *	snprintf (str, sizeof(str), "connect LCDproc %s protocol %s LCD %ix%i with cells %ix%i\n",
 *		version, protocol_version, lcd.wid, lcd.hgt, lcd.cellwid, lcd.cellhgt);
 */

	sock_send_string (c->sock, str);

	if (c->data)
		c->data->ack = 1;

	return 0;
}

/***************************************************
 * sets info about the client, such as its name
 *
 * usage: client_set -name <id>
 */
int
client_set_func (client * c, int argc, char **argv)
{
	int i;
	char str[16], buf[80];

	memset(str, '\0', sizeof(str));
	if (!c->data->ack)
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
				if (c->data->name)
					free (c->data->name);

				if ((c->data->name = strdup (str)) == NULL) {
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
client_add_key_func (client * c, int argc, char **argv)
{
	char *  keys ;

	if (!c->data->ack)
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

	if (!c->data->client_keys) {
		/* No keys list, create a new one*/
		c->data->client_keys = strdup( keys );
	} else {
		/* Add supplied keys to existing list
		 * NOTE: There could be duplicates in the resulting list
		 *    That's OK, it's the existence of the key in the list
		 *    that's important.  We'll be more careful in the delete
		 *    key function.
		 */
		char *  new ;
		int  new_len = strlen(c->data->client_keys) + strlen(keys) + 1 ;

		new = realloc( c->data->client_keys, new_len );
		if( new ) {
			c->data->client_keys = new ;
			strcat( new, keys );
		} else {
			sock_send_string(c->sock, "huh? could not allocate memory for new keys\n");
			return 0;
		}
	}

	if (c->data->client_keys) {
		sock_send_string(c->sock, "success\n");
	} else {
		sock_send_string(c->sock, "huh? failed\n");
	}

	return 0;
}

/*********************************************************************
 + Tells the server the client would NOT like to accept keypresses
 * of a particular type
 *
 * usage: client_del_key <keylist>
 */
int
client_del_key_func (client * c, int argc, char **argv)
{
	char *  keys ;

	if (!c->data->ack)
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

	if (c->data->client_keys) {
		/* Client has keys, remove keys from the list
		 * NOTE: We let malloc/realloc remember the length
		 *    of the allocated storage.  If keys are later
		 *    added, realloc (in add_key above) will make
		 *    sure there is enough space at c->data->client_keys
		 */
		char *  from ;
		char *  to ;

		to = from = c->data->client_keys ;
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

/**********************************************************************
 * Tells the server the client would like to accept keypresses
 * of a particular type when the given screen is active on the display
 *
 * screen_add_key <screenid> <keylist>
 */
int
screen_add_key_func (client * c, int argc, char **argv)
{
	/*widget *  w ;*/  /* Keys are stored on a WID_KEYS widget */
	screen *  s ;  /* Attached to a specific screen */
	char *  id ;  /* Screen ID */
	char *  keys ;  /* Keys wanted */

	if (!c->data->ack)
		return 1;

	if (argc != 3) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: screen_add_key <screenid> <keylist>\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh? You must specify a key list\n");
				break;
			default:
				sock_send_string (c->sock, "huh?  Too many parameters...\n");
				break;
		}
		return 0;
	}

	id = argv[1];
	keys = argv[2];
	debug(RPT_DEBUG, "screen_add_key: Adding key(s) %s to screen %s", keys, id);

	/* Find the screen*/
	s = screen_find (c, id);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}

	/* Save the keys*/
	if (!s->keys) {
		/* Save supplied key list*/
		s->keys = strdup( keys );
	} else {
		/* Add supplied keys to existing list
		 * NOTE: There could be duplicates in the resulting list
		 *    That's OK, it's the existence of the key in the list
		 *    that's important.  We'll be more careful in the delete
		 *    key function.
		 */
		char *  new ;
		new = realloc( s->keys, strlen(s->keys) + strlen(keys) +1 );
		if( new ) {
			s->keys = new ;
			strcat( new, keys );
		} else {
			sock_send_string(c->sock, "huh? could not allocate memory for new keys\n");
			return 0;
		}
	}

	if (!s->keys) {
		sock_send_string(c->sock, "huh? failed\n");
	} else
		sock_send_string(c->sock, "success\n");

	return 0;
}

/*************************************************************************
 * Tells the server the client would NOT like to accept keypresses
 * of a particular type when the given screen is active on the display
 *
 * usage: screen_del_key <screenid> <keylist>
 */
int
screen_del_key_func (client * c, int argc, char **argv)
{
	/*widget *  w ;*/  /* Keys are stored on a WID_KEYS widget */
	screen *  s ;  /* Attached to a specific screen */
	char *  id ;  /* Screen ID */
	char *  keys ;  /* Keys wanted */

	if (!c->data->ack)
		return 1;

	if (argc != 3) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: screen_del_key <screenid> <keylist>\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh? You must specify a key list\n");
				break;
			default:
				sock_send_string (c->sock, "huh? Too many parameters\n");
				break;
		}
		return 0;
	}

	id = argv[1] ;
	keys = argv[2] ;
	debug(RPT_DEBUG, "screen_del_key: Deleting key(s) %s from screen %s", keys, id);

	/* Find the screen*/
	s = screen_find (c, id);

	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}

	/* Do we have keys?*/
	if (s->keys) {
		/* Have keys, remove keys from the list
		 * NOTE: We let malloc/realloc remember the length
		 *    of the allocated storage.  If keys are later
		 *    added, realloc (in add_key above) will make
		 *    sure there is enough space at s->keys
		 */
		char *  from ;
		char *  to ;

		to = from = s->keys ;
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
		to = '\0';	/* terminates the new keys string...*/
	}

	sock_send_string(c->sock, "success\n");

	return 0;
}

/***************************************************************
 * Tells the server the client has another screen to offer
 *
 * usage: screen_add <id>
 */
int
screen_add_func (client * c, int argc, char **argv)
{
	int err = 0;
	char scr[128];

	if (!c->data->ack)
		return 1;

	if (argc != 2) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh?  usage: screen_add <screenid>\n");
				break;
			default:
				sock_send_string (c->sock, "huh?  Too many parameters...\n");
				break;
		}
		return 0;
	}

	debug(RPT_DEBUG, "screen_add: Adding screen %s", argv[1]);

	memset(scr, '\0', sizeof(scr));
	strncpy(scr, argv[1], sizeof(scr) - 1);

	err = screen_add (c, scr);
	if (err == 0)
		sock_send_string(c->sock, "success\n");
	else if (err < 0) {
		report(RPT_WARNING, "screen_add_func:  Error adding screen");
		sock_send_string (c->sock, "huh? failed to add screen id#\n");
	} else
		sock_send_string (c->sock, "huh? You already have a screen with that id#\n");

	report(RPT_NOTICE, "added a screen (%s) to the display", scr);
	return 0;
}

/****************************************************************
 * Client requests that the server forget about a screen
 *
 * usage: screen_del <screenid>
 */
int
screen_del_func (client * c, int argc, char **argv)
{
	int err = 0;
	char scr[128];

	if (!c->data->ack)
		return 1;

	if (argc != 2) {
		if (NoArguments)
			sock_send_string (c->sock, "huh? usage: screen_del <screenid>\n");
		else
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	debug (RPT_DEBUG, "screen_del: Deleting screen %s", argv[1]);

	/* Enforce bounds limits on argv[1]*/
	memset(scr, '\0', sizeof(scr));
	strncpy(scr, argv[1], sizeof(scr) - 1);

	err = screen_remove (c, scr);
	if ( err == 0 )
		sock_send_string(c->sock, "success\n");
	else if (err < 0) {
		report(RPT_WARNING, "screen_del_func:  Error removing screen");
		sock_send_string(c->sock, "huh? failed to remove screen\n");
	} else
		sock_send_string (c->sock, "huh? You don't have a screen with that id#\n");

	report(RPT_NOTICE, "removed a screen (%s) from the display", scr);
	return 0;
}

/***************************************************************
 * Configures info about a particular screen, such as its
 *  name, priority, or duration
 *
 * usage: screen_set <id> [ -priority <int> ] [ -name <name> ] [ -duration <int> ]
 *     [ -wid <width> ] [ -hgt <height> ] [ -heartbeat <type> ] [ -cursor <type> ]
 *     [ -cursor_x <xpos> ] [ -cursor_y <ypos> ]
 */
int
screen_set_func (client * c, int argc, char **argv)
{
	int i;

	int number;
	char *id;
	screen *s;

	if (!c->data->ack)
		return 1;

	if (NoArguments) {
		sock_send_string (c->sock, "huh?  usage: screen_set <id> [ -priority <int> ] [ -name <name> ] [ -duration <int> ] [ -wid <width> ] [ -hgt <height> ] [ -heartbeat <type> ]\n");
		return 0;
	} else if (argc == 2) {
		sock_send_string (c->sock, "huh? What do you want to set?\n");
		return 0;
	}

	id = argv[1];
	s = screen_find (c, id);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	/* Handle the rest of the parameters*/
	for (i = 2; i < argc; i++) {
		char *p;

		/* The following code allows us to use p for comparisions,
		 * ignoring a (valid) single leading '-' - reduces string comparisons
		 * by half.
		 */

		p = argv[i];
		if (*p == '-')
			p++;

		/* Handle the "name" parameter*/
		if (strcmp (p, "name") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: name=\"%s\"", argv[i]);

				/* set the name...*/
				if (s->name)
					free (s->name);
				s->name = strdup (argv[i]);
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -name requires a parameter\n");
			}
		}
		/* Handle the "priority" parameter*/
		else if (strcmp (p, "priority") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: priority=\"%s\"", argv[i]);

				/* set the priority...*/
				number = atoi (argv[i]);
				if (number > 0)
					s->priority = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -priority requires a parameter\n");
			}
		}
		/* Handle the "duration" parameter*/
		else if (strcmp (p, "duration") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: duration=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi (argv[i]);
				if (number > 0)
					s->duration = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -duration requires a parameter\n");
			}
		}
		/* Handle the "heartbeat" parameter*/
		else if (strcmp (p, "heartbeat") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: heartbeat=\"%s\"", argv[i]);

				/* set the heartbeat type...*/
				if (0 == strcmp (argv[i], "on"))
					s->heartbeat = HEARTBEAT_ON;
				if (0 == strcmp (argv[i], "heart"))
					s->heartbeat = HEARTBEAT_ON;
				if (0 == strcmp (argv[i], "normal"))
					s->heartbeat = HEARTBEAT_ON;
				if (0 == strcmp (argv[i], "default"))
					s->heartbeat = HEARTBEAT_ON;
				if (0 == strcmp (argv[i], "off"))
					s->heartbeat = HEARTBEAT_OFF;
				if (0 == strcmp (argv[i], "none"))
					s->heartbeat = HEARTBEAT_OFF;
				if (0 == strcmp (argv[i], "slash"))
					s->heartbeat = HEARTBEAT_SLASH;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -heartbeat requires a parameter\n");
			}
		}
		/* Handle the "wid" parameter*/
		else if (strcmp (p, "wid") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: wid=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi (argv[i]);
				if (number > 0)
					s->wid = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -wid requires a parameter\n");
			}

		}
		/* Handle the "hgt" parameter*/
		else if (strcmp (p, "hgt") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: hgt=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi (argv[i]);
				if (number > 0)
					s->hgt = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -hgt requires a parameter\n");
			}
		}
		/* Handle the "timeout" parameter*/
		else if (strcmp (p, "timeout") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: timeout=\"%s\"", argv[i]);
				/* set the duration...*/
				number = atoi (argv[i]);
				/* Add the timeout value (count of TIME_UNITS)
				 *  to struct,  TIME_UNIT is 1/8th of a second
				 */
				if (number > 0) {
					s->timeout = number;
					report(RPT_NOTICE, "Timeout set.");
				}
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -timeout requires a parameter\n");
			}
		}
		/* Handle the "backlight" parameter*/
		else if (strcmp (argv[i], "backlight") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: backlight=\"%s\"", argv[i]);
				/* set the backlight status based on what the client has set*/
				switch(c->backlight_state) {
					case BACKLIGHT_OPEN:
						if (strcmp ("on", argv[i]) == 0)
							s->backlight_state = BACKLIGHT_ON;

						if (strcmp ("off", argv[i]) == 0)
							s->backlight_state = BACKLIGHT_OFF;

						if (strcmp ("toggle", argv[i]) == 0) {
							if (s->backlight_state == BACKLIGHT_ON)
								s->backlight_state = BACKLIGHT_OFF;
							else if (s-backlight_state == BACKLIGHT_OFF)
								s->backlight_state = BACKLIGHT_ON;
						}

						if (strcmp ("blink", argv[i]) == 0)
							s->backlight_state  |= BACKLIGHT_BLINK;

						if (strcmp ("flash", argv[i]) == 0)
							s->backlight_state |= BACKLIGHT_FLASH;
					break;
					default:
						/*If the backlight is not OPEN then inherit its state*/
						s->backlight_state = c->backlight_state;
					break;
				}
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -backlight requires a parameter\n");
			}
		}
		/* Handle the "cursor" parameter */
		else if (strcmp (argv[i], "cursor") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: cursor=\"%s\"", argv[i]);

				/* set the heartbeat type...*/
				if (0 == strcmp (argv[i], "off"))
					s->heartbeat = CURSOR_OFF;
				if (0 == strcmp (argv[i], "on"))
					s->cursor = CURSOR_DEFAULT_ON;
				if (0 == strcmp (argv[i], "under"))
					s->cursor = CURSOR_UNDER;
				if (0 == strcmp (argv[i], "block"))
					s->cursor = CURSOR_BLOCK;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -cursor requires a parameter\n");
			}
		}
		/* Handle the "cursor_x" parameter */
		else if (strcmp (p, "cursor_x") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: cursor_x=\"%s\"", argv[i]);

				/* set the position...*/
				number = atoi (argv[i]);
				if (number > 0 && number <= s->wid) {
					s->cursor_x = number;
					sock_send_string(c->sock, "success\n");
				}
				else {
					sock_send_string(c->sock, "huh? cursor position outside screen\n");
				}
			} else {
				sock_send_string (c->sock, "huh? -cursor_x requires a parameter\n");
			}
		}
		/* Handle the "cursor_y" parameter */
		else if (strcmp (p, "cursor_y") == 0) {
			if (argc > i + 1) {
				i++;
				debug (RPT_DEBUG, "screen_set: cursor_y=\"%s\"", argv[i]);

				/* set the position...*/
				number = atoi (argv[i]);
				if (number > 0 && number <= s->hgt) {
					s->cursor_y = number;
					sock_send_string(c->sock, "success\n");
				}
				else {
					sock_send_string(c->sock, "huh? cursor position outside screen\n");
				}
			} else {
				sock_send_string (c->sock, "huh? -cursor_y requires a parameter\n");
			}
		}

		else sock_send_string (c->sock, "huh? invalid parameter\n");
	}/* done checking argv*/
	return 0;
}

/*************************************************************************
 * Adds a widget to a screen, but doesn't give it a value
 *
 * usage: widget_add <screenid> <widgetid> <widgettype> [ -in <id> ]
 */
int
widget_add_func (client * c, int argc, char **argv)
{
	int err;

	char *type;
	char *sid;
	char *wid;
	char *in = NULL;
	screen *s;

	if (!c->data->ack)
		return 1;

	if ((argc < 4) || (argc > 6)) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: widget_add <screenid> <widgetid> <widgettype> [ -in <id> ]\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh? Widget ID and Widget Type not specified!\n");
				break;
			case 3:
				sock_send_string (c->sock, "huh? Widget Type not specified!\n");
				break;
			default:
				sock_send_string (c->sock, "huh? Too many parameters\n");
				break;
		}
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}

	type = argv[3];

	/* Check for additional flags...*/
	if (argc > 4) {
		char *p;

		p = argv[4];
		if (*p == '-')
			p++;

		/* Handle the "in" flag to place widgets in a container...*/
		if (strcmp (p, "in")) {
			if (argc < 6) {
				sock_send_string (c->sock, "huh? Specify a frame to place widget in\n");
				return 0;
			}
			in = argv[5];
		}
	}

	/* Add the widget and set its type...*/
	err = widget_add (s, wid, type, in, c->sock);
	if (err == 0)
		sock_send_string(c->sock, "success\n");
	else {
		report(RPT_WARNING, "widget_add_func:  Error adding widget");
		sock_send_string(c->sock, "huh? failed\n");
	}

	return 0;
}

/*******************************************************************
 * Removes a widget from a screen, and forgets about it
 *
 * usage: widget_del <screenid> <widgetid>
 */
int
widget_del_func (client * c, int argc, char **argv)
{
	int err = 0;

	char *sid;
	char *wid;
	screen *s;

	if (!c->data->ack)
		return 1;

	if (argc != 3) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh?  usage: widget_del <screenid> <widgetid>\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh?  Specify a widget #id\n");
				break;
			default:
				sock_send_string (c->sock, "huh?  Too many parameters...\n");
				break;
		}
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	debug (RPT_DEBUG, "screen_del: Deleting widget %s.%s", sid, wid);

	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
	}

	err = widget_remove (s, wid, c->sock);
	if (err == 0)
		sock_send_string(c->sock, "success\n");
	else {
		report( RPT_WARNING, "widget_del_func:  Error removing widget");
		sock_send_string(c->sock, "huh? failed\n");
	}

	return 0;
}

/********************************************************************
 * Configures information about a widget, such as its size, shape,
 *  contents, position, speed, etc...
 *
 * Ack!  This is long!
 *
 * widget_set <screenid> <widgetid> <widget-SPECIFIC-data>
 */
int
widget_set_func (client * c, int argc, char **argv)
{
	int i;

	int x, y;
	int left, top, right, bottom;
	int length, direction;
	int width, height;
	int speed;
	char *sid, *wid;
	screen *s;
	widget *w;

	if (!c->data->ack)
		return 1;

	/* If there weren't enough parameters...
	 * We can't test for too many, since each widget may have a
	 * different number - plus, if the argument count is wrong, what ELSE
	 * could be wrong...?
	 */

	if (argc < 4) {
		switch (argc) {
			case 1:
				sock_send_string (c->sock, "huh? usage: widget_set <screenid> <widgetid> <widget-SPECIFIC-data>\n");
				break;
			case 2:
				sock_send_string (c->sock, "huh? You must specify a widget id\n");
				break;
			case 3:
				sock_send_string (c->sock, "huh? You must send some widget data\n");
				break;
		}
		return 0;
	}

	sid = argv[1];
	wid = argv[2];
	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	w = widget_find (s, wid);
	if (!w) {
		sock_send_string (c->sock, "huh? Invalid widget id\n");
		/* Client Debugging...*/
		{
			int i;
			report( RPT_WARNING, "huh? Invalid widget id (%s)", wid);
			for (i = 0; i < argc; i++)
				report( RPT_WARNING, "    %.40s ", argv[i]);
		}
		return 0;
	}
	/* FIXME?  Shouldn't this be handled in widget.c?*/
	i = 3;
	switch (w->type) {
	case WID_STRING:				  /* String takes "x y text"*/
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else					  /* Set all the data...*/
			{
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
				if (w->text)
					free (w->text);
				w->text = strdup (argv[i + 2]);
				if (!w->text) {
					report( RPT_WARNING, "widget_set_func: Error allocating string");
					return -1;
				}
				debug (RPT_DEBUG, "Widget %s set to %s", wid, w->text);
				sock_send_string(c->sock, "success\n");
			}
		}
		break;
	case WID_HBAR:				  /* Hbar takes "x y length"*/
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				length = atoi (argv[i + 2]);
				w->x = x;
				w->y = y;
				w->length = length;
			}
			debug (RPT_DEBUG, "Widget %s set to %i", wid, w->length);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_VBAR:				  /* Vbar takes "x y length"*/
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				length = atoi (argv[i + 2]);
				w->x = x;
				w->y = y;
				w->length = length;
			}
			debug (RPT_DEBUG, "Widget %s set to %i", wid, w->length);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_ICON:				  /* Icon takes "x y binary_data"*/
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
				/* TODO:  Parse binary data and copy it to widget's data...*/
			}
		}
		sock_send_string (c->sock, "huh? Widget type not yet implemented\n");
		break;
	case WID_TITLE:				  /* title takes "text"*/
		if (argc != i + 1)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if (w->text)
				free (w->text);
			w->text = strdup (argv[i]);
			if (!w->text) {
				report( RPT_WARNING, "widget_set_func: Error allocating string");
				return -1;
			}
			debug (RPT_DEBUG, "Widget %s set to %s", wid, w->text);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_SCROLLER:			  /* Scroller takes "left top right bottom direction speed text"*/
		if (argc != i + 7) {
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		} else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0])) ||
			    (!isdigit ((unsigned int) argv[i + 2][0])) ||
			    (!isdigit ((unsigned int) argv[i + 3][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				left = atoi (argv[i]);
				/*debug("left: %d",left);*/
				top = atoi (argv[i + 1]);
				/*debug("top: %d",top);*/
				right = atoi (argv[i + 2]);
				/*debug("right: %d",right);*/
				bottom = atoi (argv[i + 3]);
				/*debug("bottom: %d",bottom);*/
				direction = (int) (argv[i + 4][0]);
				/*debug("dir: %c",(char)direction);*/
				speed = atoi (argv[i + 5]);
				/*debug("speed: %d",speed);*/
				/* Direction must be v or h*/
				if (((char) direction != 'h') && ((char) direction != 'v')) {
					sock_send_string (c->sock, "huh? Invalid direction\n");
				} else {
					w->left = left;
					w->top = top;
					w->right = right;
					w->bottom = bottom;
					w->length = direction;
					w->speed = speed;
					if (w->text)
						free (w->text);
					w->text = strdup (argv[i + 6]);
					if (!w->text) {
						sock_send_string(c->sock, "huh? out of memory\n");
						report( RPT_WARNING, "widget_set_func: Error allocating string");
						return -1;
					}
					debug (RPT_DEBUG, "Widget %s set to %s", wid, w->text);
					sock_send_string(c->sock, "success\n");
				}
			}
		}
		break;
	case WID_FRAME:				  /* Frame takes "left top right bottom wid hgt direction speed"*/
		if (argc != i + 8) {
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		} else {
			if ((!isdigit ((unsigned int) argv[i][0])) ||
			    (!isdigit ((unsigned int) argv[i + 1][0])) ||
			    (!isdigit ((unsigned int) argv[i + 2][0])) ||
			    (!isdigit ((unsigned int) argv[i + 3][0])) ||
			    (!isdigit ((unsigned int) argv[i + 4][0])) ||
			    (!isdigit ((unsigned int) argv[i + 5][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				left = atoi (argv[i]);
				/*debug("left: %d",left);*/
				top = atoi (argv[i + 1]);
				/*debug("top: %d",top);*/
				right = atoi (argv[i + 2]);
				/*debug("right: %d",right);*/
				bottom = atoi (argv[i + 3]);
				/*debug("bottom: %d",bottom);*/
				width = atoi (argv[i + 4]);
				/*debug("right: %d",right);*/
				height = atoi (argv[i + 5]);
				/*debug("bottom: %d",bottom);*/
				direction = (int) (argv[i + 6][0]);
				/*debug("dir: %c",(char)direction);*/
				speed = atoi (argv[i + 7]);
				/*debug("speed: %d",speed);*/
				/* Direction must be v or h*/
				if (((char) direction != 'h') && ((char) direction != 'v')) {
					sock_send_string (c->sock, "huh? Invalid direction\n");
				} else {
					w->left = left;
					w->top = top;
					w->right = right;
					w->bottom = bottom;
					w->wid = width;
					w->hgt = height;
					w->length = direction;
					w->speed = speed;
					debug (RPT_DEBUG, "Widget %s set to (%i,%i)-(%i,%i) %ix%i", wid, left, top, right, bottom, width, height);
					sock_send_string(c->sock, "success\n");
				}
			}
		}
		break;
	case WID_NUM:					  /* Num takes "x num"*/
		if (argc != i + 2)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if (!isdigit ((unsigned int) argv[i][0])) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else if (!isdigit ((unsigned int) argv[i + 1][0])) {
				sock_send_string (c->sock, "huh? Invalid number\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
			}
			debug (RPT_DEBUG, "Widget %s set to %i", wid, w->y);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_NONE:
	default:
		sock_send_string (c->sock, "huh? Widget has no type\n");
		break;
	}

	return 0;
}

/*************************************************************
 * Adds a menu to the client; handled by the server...
 *
 * usage: menu_add ...?
 */
int
menu_add_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/****************************************************************
 * Removes a client's menu and all contents from the server
 *
 * usage: menu_del ...?
 */
int
menu_del_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/***********************************************************************
 * Sets info about a menu, but not its items
 *
 * For example, should the menu be top-level, or buried somewhere?
 *
 * usage: menu_set ...?
 */
int
menu_set_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/*************************************************************************
 * Adds an item to a menu
 *
 * usage: menu_add_item ...?
 */
int
menu_add_item_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/*************************************************************************
 * Deletes an item from a menu
 *
 + usage: menu_del_item ...?
 */
int
menu_del_item_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/**************************************************************************
 * Sets the info about a menu item
 *
 * For example, text displayed, widget type, value, etc...
 *
 * usage: menu_set_item ...?
 */
int
menu_set_item_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/***************************************************************************
 * Toggles the backlight, if enabled.
 *
 * usage: backlight <on|off|toggle|blink|flash>
 */
int
backlight_func (client * c, int argc, char **argv)
{
	if (!c->data->ack)
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


	backlight = (backlight && 1);  /* only preserves ON/OFF bit*/

	if (strcmp ("on", argv[1]) == 0) {
		c->backlight_state = BACKLIGHT_ON;

	} else if (strcmp ("off", argv[1]) == 0) {
		c->backlight_state = BACKLIGHT_OFF;

	} else if (strcmp ("toggle", argv[1]) == 0) {
		if (c->backlight_state == BACKLIGHT_ON)
			c->backlight_state = BACKLIGHT_OFF;
		else if (c->backlight_state == BACKLIGHT_OFF)
			c->backlight_state = BACKLIGHT_ON;

	} else if (strcmp ("blink", argv[1]) == 0) {
		c->backlight_state |= BACKLIGHT_BLINK;

	} else if (strcmp ("flash", argv[1]) == 0) {
		c->backlight_state |= BACKLIGHT_FLASH;
	}

	sock_send_string(c->sock, "success\n");

	return 0;

}

/****************************************************************************
 * Sets the state of the output port (such as on MtxOrb LCDs)
 *
 * usage: output <on|off|int>
 */
#define ALL_OUTPUTS_ON -1
#define ALL_OUTPUTS_OFF 0

int
output_func (client * c, int argc, char **argv)
{
	/*int rc = 0;*/
	char str[128];

	if (argc != 2) {
		if (NoArguments)
			sock_send_string (c->sock, "huh?  usage: output <on|off|num> -- num may be decimal, hex, or octal\n");
		else
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	if (0 == strcmp (argv[1], "on"))
		output_state = ALL_OUTPUTS_ON;
	else if (0 == strcmp (argv[1], "off"))
		output_state = ALL_OUTPUTS_OFF;
	else {
		long out;
		char *endptr, *p;

		/* Note that there is no valid range set for
		 * output_state; thus a value in the 12 digits
		 * is not considered out of range.
		 */

		errno = 0;

		/* errno is set here, because if strtol does not result in
		 * ERANGE (out of range error) it will not set errno (!).
		 * At least, this is the case with glibc 2.1.3 ...
		 */

		p = argv[1];
		out = strtol(p, &endptr, 0);

		/* From the man page for strtol(3)
		 *
		 * In particular, if *nptr is not `\0' but **endptr is
		 * `\0' on return, the entire string is valid.
		 *
		 * In this case, argv[1] is *nptr, and &endptr is **endptr.
		 */

		if (errno) {
			int space;

			strcat(str, "huh? number argument: ");
			space = sizeof(str) - 3 - strlen(str);

			strncat(str, strerror(errno), space);
			strcat(str, "\n");

			sock_send_string (c->sock, str);
			return 0;
		} else if (*p != '\0' && *endptr == '\0') {
			output_state = out;
		} else {
			sock_send_string (c->sock, "huh?  invalid parameter...\n");
			return 0;
		}
	}

	sock_send_string(c->sock, "success\n");

	/* Makes sense to me to set the output immediately;
	 * however, the outputs are currently set in
	 * draw_screen(screen * s, int timer)
	 * Whatever for? */

	/* drivers_output (output_state); */

	report(RPT_NOTICE, "output states changed");
	return 0;
}

/****************************************************************************
 * info_func
 *
 * usage: info
 */
int
info_func (client * c, int argc, char **argv)
{
	char str[1024];

	if (! NoArguments) {
		sock_send_string (c->sock, "huh?  Extra arguments ignored...\n");
	}

	memset(str, '\0', sizeof(str));
	snprintf (str, sizeof(str)-1, (char*) drivers_get_info());

	sock_send_string (c->sock, str);

	return 0;
}

/*******************************************************************************
 * sleep_func
 *
 * usage: sleep <seconds>
 */
int
sleep_func (client * c, int argc, char **argv)
{
	int secs;
	long out;
	char *endptr, *p;
	char str[120];

#define MAX_SECS 60
#define MIN_SECS 1

	if (argc != 2) {
		if (NoArguments)
			sock_send_string (c->sock, "huh?  usage: sleep <secs>\n");
		else
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	errno = 0;

	/* errno is set here, because if strtol does not result in
	 * ERANGE (out of range error) it will not set errno (!).
	 * At least, this is the case with glibc 2.1.3 ...
	 */

	p = argv[1];
	out = strtol(p, &endptr, 0);

	/* From the man page for strtol(3)
	 *
	 * In particular, if *nptr is not `\0' but **endptr is
	 * `\0' on return, the entire string is valid.
	 *
	 * In this case, argv[1] is *nptr, and &endptr is **endptr.
	 */

	if (errno) {
		int space;

		strcat(str, "huh? number argument: ");
		space = sizeof(str) - 3 - strlen(str);

		strncat(str, strerror(errno), space);
		strcat(str, "\n");

		sock_send_string (c->sock, str);
		return 0;
	} else if (*p != '\0' && *endptr == '\0') {
		secs = out;
		out = out > MAX_SECS ? MAX_SECS : out;
		out = out < MIN_SECS ? MIN_SECS : out;
	} else {
		sock_send_string (c->sock, "huh?  invalid parameter...\n");
		return 0;
	}

	/* Repeat until no more remains - should normally be zero
	 * on exit the first time...*/
	snprintf(str, sizeof(str), "sleeping %d seconds\n", secs);
	sock_send_string (c->sock, str);

	/* whoops.... if this takes place as planned, ALL screens
	 * will "freeze" for the alloted time...
	 *
	 * while ((secs = sleep(secs)) > 0)
	 */	;

	sock_send_string (c->sock, "huh? ignored (not fully implemented)\n");
	return 0;
}

/*****************************************************************************
 * Does nothing, returns "noop complete" message
 *
 * This is useful for shell scripts or programs that want to talk
 *    with LCDproc and not get deadlocked.  Send a noop after each
 *    command and look for the "noop complete" message.
 */
int
noop_func (client * c, int argc, char **argv)
{
	sock_send_string (c->sock, "noop complete\n");
	return 0;
}

/*end of client_functions.c*/
