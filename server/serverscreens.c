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

screen *server_screen;
char *id = "ClientList";
char *name = "Client List";

char title[256] = "LCDproc Server";
char one[256] = "";
char two[256] = "";
char three[256] = "";

#define WidgetXPos(w,a) (w)->x = (a)
#define WidgetYPos(w,a) (w)->y = (a)
#define WidgetText(w,a) (w)->text = (a)
#define WidgetString(w,a,b,t)	{WidgetXPos(w,a);WidgetYPos(w,b);WidgetText(w,t);}

int
server_screen_init ()
{
	widget *w;

	debug (RPT_DEBUG, "server_screen_init");

	server_screen = screen_create ();

	if (!server_screen) {
		report (RPT_ERR, "server_screen_init: Error allocating screen");
		return -1;
	}

	server_screen->id = id;
	server_screen->name = name;
	server_screen->duration = 8; /* 1 second, instead of 4...*/

	if (widget_add (server_screen, "title", "title", NULL, 1) != 0) {
		report (RPT_ERR, "server_screen_init: internal error: could not add title widget");
	}
	if (widget_add (server_screen, "one", "string", NULL, 1) != 0) {
		report (RPT_ERR, "server_screen_init: internal error: could not add title widget");
	}
	if (widget_add (server_screen, "two", "string", NULL, 1) != 0) {
		report (RPT_ERR, "server_screen_init: internal error: could not add title widget");
	}
	if (widget_add (server_screen, "three", "string", NULL, 1) != 0) {
		report (RPT_ERR, "server_screen_init: internal error: could not add title widget");
	}

	/* Now, initialize all the widgets...*/
	if ((w = widget_find (server_screen, "title")) != NULL) {
		WidgetText(w,title);
	} else
		report (RPT_ERR, "server_screen_init: Can't find title");

	if ((w = widget_find (server_screen, "one")) != NULL)
		WidgetString(w,1,2,one)
	else
		report (RPT_ERR, "server_screen_init: Can't find widget one");

	if ((w = widget_find (server_screen, "two")) != NULL)
		WidgetString(w,1,3,two)
	else
		report (RPT_ERR, "server_screen_init: Can't find widget two");

	if ((w = widget_find (server_screen, "three")) != NULL)
		WidgetString(w,1,4,three)
	else
		report (RPT_ERR, "server_screen_init: Can't find widget three");

	/* And enqueue the screen*/
	screenlist_add (server_screen);

	debug (RPT_DEBUG, "server_screen_init done");

	return 0;
}

static int
screen_count (client *c) {
	int n;

	n = 0;
	LL_Rewind (c->data->screenlist);
	do {
		if (LL_Get (c->data->screenlist) != NULL)
			n++;
	} while (LL_Next (c->data->screenlist) == 0);

	return n;
}

int
update_server_screen (int timer)
{
	client *c;
	int num_clients;
	/*screen *s;*/
	int num_screens;

	/* Draw a title...*/
	/*strcpy(title, "LCDproc Server");*/

	/* Now get info on the number of connected clients...*/
	num_clients = 0;
	num_screens = 0;
	LL_Rewind (clients);
	do {
		c = LL_Get (clients);
		if (c) {
			num_clients++;
			num_screens += screen_count(c);
/*			LL_Rewind (c->data->screenlist); */
/*			do { */
/*				s = LL_Get (c->data->screenlist); */
/*				if (s) { */
/*					num_screens++; */
/*				} */
/*			} while (LL_Next (c->data->screenlist) == 0); */
		}
	} while (LL_Next (clients) == 0);

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
