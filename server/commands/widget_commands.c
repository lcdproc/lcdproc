/** \file server/commands/widget_commands.c
 * Implements handlers for client commands concerning widgets.
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
#include "widget.h"
#include "drivers.h"
#include "widget_commands.h"


/**
 * Adds a widget to a screen, but doesn't give it a value
 *
 *\verbatim
 * Usage: widget_add <screenid> <widgetid> <widgettype> [-in <id>]
 *\endverbatim
 */
int
widget_add_func(Client *c, int argc, char **argv)
{
	int err;
	char *sid;
	char *wid;
	WidgetType wtype;
	Screen * s;
	Widget * w;

	if (c->state != ACTIVE)
		return 1;

	if ((argc < 4) || (argc > 6)) {
		sock_send_error(c->sock, "Usage: widget_add <screenid> <widgetid> <widgettype> [-in <id>]\n");
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	s = client_find_screen(c, sid);
	if (s == NULL) {
		sock_send_error(c->sock, "Invalid screen id\n");
		return 0;
	}

	/* Find widget type */
	wtype = widget_typename_to_type(argv[3]);
	if (wtype == WID_NONE) {
		sock_send_error(c->sock, "Invalid widget type\n");
		return 0;
	}

	/* Check for additional flags...*/
	if (argc > 4) {
		char *p = argv[4];

		/* ignore leading '-' in options: we allow both forms */
		if (*p == '-')
			p++;

		/* Handle the "in" flag to place widgets in a container...*/
		if (strcmp(p, "in") == 0) {
			Widget *frame;

			if (argc < 6) {
				sock_send_error(c->sock, "Specify a frame to place widget in\n");
				return 0;
			}

			/* Now we replace s with the framescreen.
			 * This way it will not be placed in the normal screen
			 * but in the framescreen.
			 */
			frame = screen_find_widget(s, argv[5]);
			if (frame == NULL) {
				sock_send_error(c->sock, "Error finding frame\n");
				return 0;
			}
			s = frame->frame_screen;
		}
	}

	/* Create the widget */
	w = widget_create(wid, wtype, s);
	if (w == NULL) {
		sock_send_error(c->sock, "Error adding widget\n");
		return 0;
	}

	/* Add the widget to the screen */
	err = screen_add_widget(s, w);
	if (err == 0)
		sock_send_string(c->sock, "success\n");
	else
		sock_send_error(c->sock, "Error adding widget\n");

	return 0;
}

/**
 * Removes a widget from a screen, and forgets about it
 *
 *\verbatim
 * Usage: widget_del <screenid> <widgetid>
 *\endverbatim
 */
int
widget_del_func(Client *c, int argc, char **argv)
{
	int err = 0;

	char *sid;
	char *wid;
	Screen *s;
	Widget *w;

	if (c->state != ACTIVE)
		return 1;

	if (argc != 3) {
		sock_send_error(c->sock, "Usage: widget_del <screenid> <widgetid>\n");
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	debug(RPT_DEBUG, "screen_del: Deleting widget %s.%s", sid, wid);

	s = client_find_screen(c, sid);
	if (s == NULL) {
		sock_send_error(c->sock, "Invalid screen id\n");
		return 0;
	}

	w = screen_find_widget(s, wid);
	if (w == NULL) {
		sock_send_error(c->sock, "Invalid widget id\n");
		return 0;
	}

	err = screen_remove_widget(s, w);
	if (err == 0)
		sock_send_string(c->sock, "success\n");
	else
		sock_send_error(c->sock, "Error removing widget\n");

	return 0;
}

static int not_direction(char c) {
	return c != 'h' && c != 'v';
}

/**
 * Configures information about a widget, such as its size, shape,
 * contents, position, speed, etc.
 *
 *\verbatim
 * widget_set <screenid> <widgetid> <widget-SPECIFIC-data>
 *\endverbatim
 */
int
widget_set_func(Client *c, int argc, char **argv)
{
	int i;
	char *wid;
	char *sid;

	Screen *s;
	Widget *w;

	if (c->state != ACTIVE)
		return 1;

	/* If there weren't enough parameters...
	 * We can't test for too many, since each widget may have a
	 * different number - plus, if the argument count is wrong, what ELSE
	 * could be wrong...?
	 */

	if (argc < 4) {
		sock_send_error(c->sock, "Usage: widget_set <screenid> <widgetid> <widget-SPECIFIC-data>\n");
		return 0;
	}

	/* Find screen */
	sid = argv[1];
	s = client_find_screen(c, sid);
	if (s == NULL) {
		sock_send_error(c->sock, "Unknown screen id\n");
		return 0;
	}
	/* Find widget */
	wid = argv[2];
	w = screen_find_widget(s, wid);
	if (w == NULL) {
		sock_send_error(c->sock, "Unknown widget id\n");
		/* Client Debugging...*/
		{
			int i;
			report(RPT_WARNING, "Unknown widget id (%s)", argv[2]);
			for (i = 0; i < argc; i++)
				report(RPT_WARNING, "    %.40s ", argv[i]);
		}
		return 0;
	}
	i = 3;
	switch (w->type) {
	case WID_STRING:		/* String takes "x y text" */
		if (argc != i + 3) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}

		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);
		free(w->text);
		w->text = strdup(argv[i + 2]);
		debug(RPT_DEBUG, "Widget %s set to %s", wid, w->text);

		break;
	case WID_HBAR:			/* Hbar takes "x y length" */
		if (argc != i + 3) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}

		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);
		w->length = atoi(argv[i + 2]);

		debug(RPT_DEBUG, "Widget %s set to %i", wid, w->length);

		break;
	case WID_VBAR:			/* Vbar takes "x y length" */
		if (argc != i + 3) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}
		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}

		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);
		w->length = atoi(argv[i + 2]);

		debug(RPT_DEBUG, "Widget %s set to %i", wid, w->length);

		break;
	case WID_PBAR:			/* Pbar takes "x y width promille [begin-label end-label]" */
		if (argc < i + 4 || argc > i + 6) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}
		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}
		free(w->begin_label);
		free(w->end_label);
		w->begin_label = NULL;
		w->end_label = NULL;
		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);
		w->width = atoi(argv[i + 2]);
		w->promille = atoi(argv[i + 3]);
		if (argc >= i + 5)
			w->begin_label = strdup(argv[i + 4]);
		if (argc >= i + 6)
			w->end_label = strdup(argv[i + 5]);
		debug(RPT_DEBUG, "Widget %s set to %i", wid, w->promille);

		break;
	case WID_ICON:			/* Icon takes "x y icon" */
		if (argc != i + 3) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}
		int icon;

		icon = widget_iconname_to_icon(argv[i + 2]);
		if (icon == -1) {
			sock_send_error(c->sock, "Invalid icon name\n");
			return 0;
		}

		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);
		w->length = icon;

		break;
	case WID_TITLE:			/* title takes "text" */
		if (argc != i + 1) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		free(w->text);
		w->text = strdup(argv[i]);
		/* Set width too */
		w->width = display_props->width;
		debug(RPT_DEBUG, "Widget %s set to %s", wid, w->text);

		break;
	case WID_SCROLLER:		/* Scroller takes "left top right bottom direction speed text" */
		if (argc != i + 7) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0])) ||
		    (!isdigit((unsigned int) argv[i + 2][0])) ||
		    (!isdigit((unsigned int) argv[i + 3][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}

		/* Direction must be m, v or h*/
		if (not_direction(argv[i + 4][0]) && argv[i + 4][0] != 'm') {
			sock_send_error(c->sock, "Invalid direction\n");
			return 0;
		}

		w->left = atoi(argv[i]);
		w->top = atoi(argv[i + 1]);
		w->right = atoi(argv[i + 2]);
		w->bottom = atoi(argv[i + 3]);
		w->length = argv[i + 4][0];
		w->speed = atoi(argv[i + 5]);
		free(w->text);
		w->text = strdup(argv[i + 6]);
		debug(RPT_DEBUG, "Widget %s set to %s", wid, w->text);

		break;
	case WID_FRAME:			/* Frame takes "left top right bottom wid hgt direction speed" */
		if (argc != i + 8) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if ((!isdigit((unsigned int) argv[i][0])) ||
		    (!isdigit((unsigned int) argv[i + 1][0])) ||
		    (!isdigit((unsigned int) argv[i + 2][0])) ||
		    (!isdigit((unsigned int) argv[i + 3][0])) ||
		    (!isdigit((unsigned int) argv[i + 4][0])) ||
		    (!isdigit((unsigned int) argv[i + 5][0]))) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}

		if (not_direction(argv[i + 6][0])) {
			sock_send_error(c->sock, "Invalid direction\n");
			return 0;
		}

		w->left = atoi(argv[i]);
		w->top = atoi(argv[i + 1]);
		w->right = atoi(argv[i + 2]);
		w->bottom = atoi(argv[i + 3]);
		w->width = atoi(argv[i + 4]);
		w->height = atoi(argv[i + 5]);
		w->length = argv[i + 6][0];
		w->speed = atoi(argv[i + 7]);
		debug(RPT_DEBUG, "Widget %s set to (%i,%i)-(%i,%i) %ix%i", wid, w->left, w->top, w->right, w->bottom, w->width, w->height);

		break;
	case WID_NUM:			/* Num takes "x num" */
		if (argc != i + 2) {
			sock_send_error(c->sock, "Wrong number of arguments\n");
			return 0;
		}

		if (!isdigit((unsigned int) argv[i][0])) {
			sock_send_error(c->sock, "Invalid coordinates\n");
			return 0;
		}
		if (!isdigit((unsigned int) argv[i + 1][0])) {
			sock_send_error(c->sock, "Invalid number\n");
			return 0;
		}

		w->x = atoi(argv[i]);
		w->y = atoi(argv[i + 1]);

		debug(RPT_DEBUG, "Widget %s set to %i", wid, w->y);

		break;
	case WID_NONE:
	default:
		sock_send_error(c->sock, "Widget has no type\n");
		return 0;
	}

	sock_send_string(c->sock, "success\n");
	return 0;
}

