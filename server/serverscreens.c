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

#include "drivers.h"
#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "serverscreens.h"

Screen * server_screen;

char title[256] = "LCDproc Server";
char one[256] = "";
char two[256] = "";
char three[256] = "";

int
server_screen_init ()
{
	Widget * w;

	debug (RPT_DEBUG, "server_screen_init");

	server_screen = screen_create ("_server_screen", NULL);
	if (!server_screen) {
		report (RPT_ERR, "server_screen_init: Error allocating screen");
		return -1;
	}

	server_screen->name = "Server screen";
	server_screen->duration = 8; /* 1 second, instead of 4...*/
	server_screen->priority = -1; /* TODO: use priorities good */

	if (	(w = widget_create ("title", WID_TITLE, server_screen)) == NULL
	||	screen_add_widget (server_screen, w) != 0
	||	(w = widget_create ("one", WID_STRING, server_screen)) == NULL
	||	screen_add_widget (server_screen, w) != 0
	||	(w = widget_create ("two", WID_STRING, server_screen)) == NULL
	||	screen_add_widget (server_screen, w) != 0
	||	(w = widget_create ("three", WID_STRING, server_screen)) == NULL
	||	screen_add_widget (server_screen, w) != 0
	) {
		report (RPT_ERR, "server_screen_init: Can't create widgets");
		return -1;
	}

	/* Now, initialize all the widgets...*/
	w = screen_find_widget (server_screen, "title");
	w->text = strdup (title);

	w = screen_find_widget (server_screen, "one");
	w->x = 1;
	w->y = 2;
	w->text = strdup (one);

	w = screen_find_widget (server_screen, "two");
	w->x = 1;
	w->y = 3;
	w->text = strdup (two);

	w = screen_find_widget (server_screen, "three");
	w->x = 1;
	w->y = 4;
	w->text = strdup (three);

	/* And enqueue the screen*/
	screenlist_add (server_screen);

	debug (RPT_DEBUG, "server_screen_init done");

	return 0;
}

int
update_server_screen (int timer)
{
	Client * c;
	int num_clients;
	/*screen *s;*/
	int num_screens;

	/* Draw a title...*/
	/*strcpy(title, "LCDproc Server");*/

	/* Now get info on the number of connected clients...*/
	num_clients = clients_client_count();

	/* ... and screens */
	num_screens = 0;
	for (c = clients_getfirst (); c; c = clients_getnext () ) {
		num_screens += client_screen_count(c);
	}

	/* Format strings for the appropriate size display... */
	if (display_props->height >= 3) {
		snprintf (one, sizeof(one), "Clients: %i", num_clients);
		snprintf (two, sizeof(two), "Screens: %i", num_screens);
	} else {
		if (display_props->width >= 20)
			snprintf (one, sizeof(one), "%i Client%s, %i Screen%s", num_clients,
				(num_clients == 1) ? "" : "s", num_screens,
				(num_screens == 1) ? "" : "s");
		else							  /* 16x2 size*/
			snprintf (one, sizeof(one), "%i Cli%s, %i Scr%s", num_clients,
				(num_clients == 1) ? "" : "s", num_screens,
				(num_screens == 1) ? "" : "s");
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
