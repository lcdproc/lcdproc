/*
 * serverscreens.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Implements the serverscreens
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/report.h"

#include "drivers/lcd.h"
#include "drivers.h"
#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "serverscreens.h"

Screen * server_screen;

#define MAX_SERVERSCREEN_WIDTH 40

int
server_screen_init ()
{
	Widget * w;
	int line;

	debug (RPT_DEBUG, "server_screen_init");

	/* Create the screen */
	server_screen = screen_create ("_server_screen", NULL);
	if (!server_screen) {
		report (RPT_ERR, "server_screen_init: Error allocating screen");
		return -1;
	}
	server_screen->name = "Server screen";
	server_screen->duration = 8; /* 1 second, instead of 4...*/
	server_screen->priority = -1; /* TODO: use priorities good */

	/* Create all the widgets...*/
	for (line=1; line<=4; line++) {
		char id[8];
		sprintf (id, "line%d", line);

		w = widget_create (id, WID_STRING, server_screen);
		if (!w) {
			report (RPT_ERR, "server_screen_init: Can't create a widget");
			return -1;
		}
		screen_add_widget (server_screen, w);
		w->x = 1;
		w->y = line;
		w->text = malloc (MAX_SERVERSCREEN_WIDTH+1);
		if (line == 1) {
			w->type = WID_TITLE;
			strncpy (w->text, "LCDproc Server", MAX_SERVERSCREEN_WIDTH);
		} else {
			w->text[0] = 0;
		}
	}

	/* And enqueue the screen*/
	screenlist_add (server_screen);

	debug (RPT_DEBUG, "server_screen_init done");

	return 0;
}

int
update_server_screen (int timer)
{
	Client * c;
	Widget * w;
	int num_clients;
	int num_screens;

	/* Now get info on the number of connected clients...*/
	num_clients = clients_client_count();

	/* ... and screens */
	num_screens = 0;
	for (c = clients_getfirst (); c; c = clients_getnext () ) {
		num_screens += client_screen_count(c);
	}

	/* Format strings for the appropriate size display... */
	if (display_props->height >= 3) {
		w = screen_find_widget (server_screen, "line2");
		snprintf (w->text, MAX_SERVERSCREEN_WIDTH,
					"Clients: %i", num_clients);
		w = screen_find_widget (server_screen, "line3");
		snprintf (w->text, MAX_SERVERSCREEN_WIDTH,
					"Screens: %i", num_screens);
	} else {
		w = screen_find_widget (server_screen, "line2");
		/*if (display_props->width >= 20)*/
		snprintf (w->text, MAX_SERVERSCREEN_WIDTH,
				"Cli: %i  Scr: %i",
				num_clients, num_screens);
		/*else					* 16x2 size */
	}
	return 0;
}

int
goodbye_screen ()
{
	char *b20 = "                    ";
	char *t20 = "  Thanks for using  ";
#ifdef LINUX
	char *l20 = " LCDproc and Linux! ";
#else
	char *l20 = "      LCDproc!      ";
#endif

	char *b16 = "                ";
	char *t16 = "Thanks for using";
#ifdef LINUX
	char *l16 = " LCDproc+Linux! ";
#else
	char *l16 = "    LCDproc!    ";
#endif

	if( !display_props )
		return 0;

	drivers_clear ();

	if (display_props->height >= 4) {
		if (display_props->width >= 20) {
			drivers_string (1, 1, b20);
			drivers_string (1, 2, t20);
			drivers_string (1, 3, l20);
			drivers_string (1, 4, b20);
		} else {
			drivers_string (1, 1, b16);
			drivers_string (1, 2, t16);
			drivers_string (1, 3, l16);
			drivers_string (1, 4, b16);
		}
	} else {
		if (display_props->width >= 20) {
			drivers_string (1, 1, t20);
			drivers_string (1, 2, l20);
		} else {
			drivers_string (1, 1, t16);
			drivers_string (1, 2, l16);
		}
	}

	drivers_flush ();

	return 0;
}
