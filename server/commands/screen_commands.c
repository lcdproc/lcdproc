/** \file server/commands/screen_commands.c
 * Implements handlers for the client commands concerning screens.
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

#include "client.h"
#include "screen.h"
#include "render.h"
#include "screen_commands.h"

/**
 * Tells the server the client has another screen to offer
 *
 *\verbatim
 * Usage: screen_add <id>
 *\endverbatim
 */
int
screen_add_func(Client *c, int argc, char **argv)
{
	int err = 0;
	Screen *s;

	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: screen_add <screenid>\n");
		return 0;
	}

	debug(RPT_DEBUG, "screen_add: Adding screen %s", argv[1]);

	s = client_find_screen(c, argv[1]);
	if (s != NULL) {
		sock_send_error(c->sock, "Screen already exists\n");
		return 0;
	}

	s = screen_create(argv[1], c);
	if (s == NULL) {
		sock_send_error(c->sock, "failed to create screen\n");
		return 0;
	}

	err = client_add_screen(c, s);

	if (err == 0) {
		sock_send_string(c->sock, "success\n");
	} else {
		sock_send_error(c->sock, "failed to add screen\n");
	}
	report(RPT_INFO, "Client on socket %d added added screen \"%s\"", c->sock, s->id);
	return 0;
}

/**
 * The client requests that the server forget about a screen
 *
 *\verbatim
 * Usage: screen_del <screenid>
 *\endverbatim
 */
int
screen_del_func(Client *c, int argc, char **argv)
{
	int err = 0;
	Screen *s;

	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: screen_del <screenid>\n");
		return 0;
	}

	debug(RPT_DEBUG, "screen_del: Deleting screen %s", argv[1]);

	s = client_find_screen(c, argv[1]);
	if (s == NULL) {
		sock_send_error(c->sock, "Unknown screen id\n");
		return 0;
	}

	err = client_remove_screen(c, s);
	if (err == 0) {
		sock_send_string(c->sock, "success\n");
	}
	else if (err < 0) {
		sock_send_error(c->sock, "failed to remove screen\n");
	}
	else {
		sock_send_error(c->sock, "Unknown screen id\n");
	}

	report(RPT_INFO, "Client on socket %d removed screen \"%s\"", c->sock, s->id);

	screen_destroy(s);
	return 0;
}

/**
 * Configures info about a particular screen, such as its
 *  name, priority, or duration
 *
 *\verbatim
 * Usage: screen_set <id> [-name <name>] [-wid <width>] [-hgt <height>]
 *     [-priority <prio>] [-duration <int>] [-timeout <int>]
 *     [-heartbeat <type>] [-backlight <type>]
 *     [-cursor <type>] [-cursor_x <xpos>] [-cursor_y <ypos>]
 *\endverbatim
 */
int
screen_set_func(Client *c, int argc, char **argv)
{
	int i;

	int number;
	char *id;
	Screen * s;

	if (c->state != ACTIVE)
		return 1;

	if (argc == 1) {
		sock_send_error(c->sock, "Usage: screen_set <id> [-name <name>]"
				" [-wid <width>] [-hgt <height>] [-priority <prio>]"
				" [-duration <int>] [-timeout <int>]"
				" [-heartbeat <type>] [-backlight <type>]"
				" [-cursor <type>]"
				" [-cursor_x <xpos>] [-cursor_y <ypos>]\n");
		return 0;
	}
	else if (argc == 2) {
		sock_send_error(c->sock, "What do you want to set?\n");
		return 0;
	}

	id = argv[1];
	s = client_find_screen(c, id);
	if (s == NULL) {
		sock_send_error(c->sock, "Unknown screen id\n");
		return 0;
	}
	/* Handle the rest of the parameters*/
	for (i = 2; i < argc; i++) {
		char *p = argv[i];

		/* ignore leading '-' in options: we allow both forms */
		if (*p == '-')
			p++;

		/* Handle the "name" parameter*/
		if (strcmp(p, "name") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: name=\"%s\"", argv[i]);

				/* set the name...*/
				if (s->name != NULL)
					free(s->name);
				s->name = strdup(argv[i]);
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-name requires a parameter\n");
			}
		}
		/* Handle the "priority" parameter*/
		else if (strcmp(p, "priority") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: priority=\"%s\"", argv[i]);

				/* first try to interpret it as a number */
				number = atoi(argv[i]);
				if (number > 0) {
					if (number <= 64)
						number = PRI_FOREGROUND;
					else if (number < 192)
						number = PRI_INFO;
					else
						number = PRI_BACKGROUND;
				}
				else {
					/* Try if it is a priority class */
					number = screen_pri_name_to_pri(argv[i]);
				}
				if (number >= 0) {
					s->priority = number;
					sock_send_string(c->sock, "success\n");
				}
				else {
					sock_send_error(c->sock, "invalid argument at -priority\n");
				}
			}
			else {
				sock_send_error(c->sock, "-priority requires a parameter\n");
			}
		}
		/* Handle the "duration" parameter*/
		else if (strcmp(p, "duration") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: duration=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi(argv[i]);
				if (number > 0)
					s->duration = number;
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-duration requires a parameter\n");
			}
		}
		/* Handle the "heartbeat" parameter*/
		else if (strcmp(p, "heartbeat") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: heartbeat=\"%s\"", argv[i]);

				/* set the heartbeat type...*/
				if (0 == strcmp(argv[i], "on"))
					s->heartbeat = HEARTBEAT_ON;
				else if (0 == strcmp(argv[i], "off"))
					s->heartbeat = HEARTBEAT_OFF;
				else if (0 == strcmp(argv[i], "open"))
					s->heartbeat = HEARTBEAT_OPEN;
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-heartbeat requires a parameter\n");
			}
		}
		/* Handle the "wid" parameter*/
		else if (strcmp(p, "wid") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: wid=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi(argv[i]);
				if (number > 0)
					s->width = number;
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-wid requires a parameter\n");
			}

		}
		/* Handle the "hgt" parameter*/
		else if (strcmp(p, "hgt") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: hgt=\"%s\"", argv[i]);

				/* set the duration...*/
				number = atoi(argv[i]);
				if (number > 0)
					s->height = number;
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-hgt requires a parameter\n");
			}
		}
		/* Handle the "timeout" parameter*/
		else if (strcmp(p, "timeout") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: timeout=\"%s\"", argv[i]);
				/* set the duration...*/
				number = atoi(argv[i]);
				/* Add the timeout value (count of TIME_UNITS)
				 *  to struct,  TIME_UNIT is 1/8th of a second
				 */
				if (number > 0) {
					s->timeout = number;
					report(RPT_NOTICE, "Timeout set.");
				}
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-timeout requires a parameter\n");
			}
		}
		/* Handle the "backlight" parameter*/
		else if (strcmp(p, "backlight") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: backlight=\"%s\"", argv[i]);
				if (strcmp("on", argv[i]) == 0)
					s->backlight = BACKLIGHT_ON;

				else if (strcmp("off", argv[i]) == 0)
					s->backlight = BACKLIGHT_OFF;

				else if (strcmp("toggle", argv[i]) == 0) {
					if (s->backlight == BACKLIGHT_ON)
						s->backlight = BACKLIGHT_OFF;
					else if (s-backlight == BACKLIGHT_OFF)
						s->backlight = BACKLIGHT_ON;
				}

				else if (strcmp("blink", argv[i]) == 0)
					s->backlight  |= BACKLIGHT_BLINK;

				else if (strcmp("flash", argv[i]) == 0)
					s->backlight |= BACKLIGHT_FLASH;

				else if (strcmp("open", argv[i]) == 0)
					s->backlight = BACKLIGHT_OPEN;

				else
					sock_send_error(c->sock, "unknown backlight mode\n");

				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-backlight requires a parameter\n");
			}
		}
		/* Handle the "cursor" parameter */
		else if (strcmp(p, "cursor") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: cursor=\"%s\"", argv[i]);

				/* set the heartbeat type...*/
				if (0 == strcmp(argv[i], "off"))
					s->cursor = CURSOR_OFF;
				if (0 == strcmp(argv[i], "on"))
					s->cursor = CURSOR_DEFAULT_ON;
				if (0 == strcmp(argv[i], "under"))
					s->cursor = CURSOR_UNDER;
				if (0 == strcmp(argv[i], "block"))
					s->cursor = CURSOR_BLOCK;
				sock_send_string(c->sock, "success\n");
			}
			else {
				sock_send_error(c->sock, "-cursor requires a parameter\n");
			}
		}
		/* Handle the "cursor_x" parameter */
		else if (strcmp(p, "cursor_x") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: cursor_x=\"%s\"", argv[i]);

				/* set the position...*/
				number = atoi(argv[i]);
				if (number > 0 && number <= s->width) {
					s->cursor_x = number;
					sock_send_string(c->sock, "success\n");
				}
				else {
					sock_send_error(c->sock, "Cursor position outside screen\n");
				}
			}
			else {
				sock_send_error(c->sock, "-cursor_x requires a parameter\n");
			}
		}
		/* Handle the "cursor_y" parameter */
		else if (strcmp(p, "cursor_y") == 0) {
			if (argc > i + 1) {
				i++;
				debug(RPT_DEBUG, "screen_set: cursor_y=\"%s\"", argv[i]);

				/* set the position...*/
				number = atoi(argv[i]);
				if (number > 0 && number <= s->height) {
					s->cursor_y = number;
					sock_send_string(c->sock, "success\n");
				}
				else {
					sock_send_error(c->sock, "Cursor position outside screen\n");
				}
			}
			else {
				sock_send_error(c->sock, "-cursor_y requires a parameter\n");
			}
		}

		else sock_send_error(c->sock, "invalid parameter\n");
	}/* done checking argv*/
	return 0;
}

