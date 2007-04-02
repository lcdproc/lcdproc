/*
 * serverscreens.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
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
#include "shared/configfile.h"

#include "drivers/lcd.h"
#include "drivers.h"
#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "main.h"
#include "serverscreens.h"

#define UNSET_INT -1

#define MAX_SERVERSCREEN_WIDTH 40


Screen *server_screen;
int rotate_server_screen = UNSET_INT;


int
server_screen_init(void)
{
	Widget *w;
	int line;

	debug(RPT_DEBUG, "server_screen_init");

	/* Create the screen */
	server_screen = screen_create("_server_screen", NULL);
	if (!server_screen) {
		report(RPT_ERR, "server_screen_init: Error allocating screen");
		return -1;
	}
	server_screen->name = "Server screen";
	server_screen->duration = RENDER_FREQ; /* 1 second, instead of 4...*/

	if ((rotate_server_screen == UNSET_INT) || (rotate_server_screen == 1)) {
		server_screen->priority = PRI_INFO;
	} else {
		server_screen->priority = PRI_BACKGROUND;
	}

	/* Create all the widgets...*/
	for (line = 1; line <= 4; line++) {
		char id[8];
		sprintf(id, "line%d", line);

		w = widget_create(id, WID_STRING, server_screen);
		if (!w) {
			report(RPT_ERR, "server_screen_init: Can't create a widget");
			return -1;
		}
		screen_add_widget(server_screen, w);
		w->x = 1;
		w->y = line;
		w->text = malloc(MAX_SERVERSCREEN_WIDTH+1);
		if (line == 1) {
			w->type = WID_TITLE;
			strncpy(w->text, "LCDproc Server", MAX_SERVERSCREEN_WIDTH);
		} else {
			w->text[0] = '\0';
		}
	}

	/* And enqueue the screen*/
	screenlist_add(server_screen);

	debug(RPT_DEBUG, "server_screen_init done");

	return 0;
}

int
server_screen_shutdown(void)
{
	if (!server_screen)
		return -1;

	screenlist_remove(server_screen);
	screen_destroy(server_screen);
	return 0;
}

int
update_server_screen(void)
{
	Client *c;
	Widget *w;
	int num_clients;
	int num_screens;

	/* Now get info on the number of connected clients...*/
	num_clients = clients_client_count();

	/* ... and screens */
	num_screens = 0;
	for (c = clients_getfirst(); c != NULL; c = clients_getnext()) {
		num_screens += client_screen_count(c);
	}

	/* Format strings for the appropriate size display... */
	if (display_props->height >= 3) {
		w = screen_find_widget(server_screen, "line2");
		snprintf(w->text, MAX_SERVERSCREEN_WIDTH,
					"Clients: %i", num_clients);
		w = screen_find_widget(server_screen, "line3");
		snprintf(w->text, MAX_SERVERSCREEN_WIDTH,
					"Screens: %i", num_screens);
	} else {
		w = screen_find_widget(server_screen, "line2");
		snprintf(w->text, MAX_SERVERSCREEN_WIDTH,
				((display_props->width >= 16)
				 ? "Cli: %i  Scr: %i"
				 : "C: %i  S: %i"),
				num_clients, num_screens);
	}
	return 0;
}

int
goodbye_screen(void)
{
	if (!display_props)
		return 0;

	drivers_clear();

	if (config_has_key("Server", "GoodBye")) {	/* custom GoodBye */
		int i;
	   
		/* loop over all display lines to read config & display message */
		for (i = 0; i < display_props->height; i++) {
			const char *line = config_get_string("Server", "GoodBye", i, "");

			drivers_string(1, 1+i, line);
		}
	}		
	else {		/* default GoodBye */
		if ((display_props->height >= 2) && (display_props->width >= 16)) {
			int xoffs = (display_props->width - 16) / 2;
			int yoffs = (display_props->height - 2) / 2;

			char *top = "Thanks for using";
#ifdef LINUX
			char *low = "LCDproc & Linux!";
#else
			char *low = "    LCDproc!    ";
#endif

			drivers_string(1+xoffs, 1+yoffs, top);
			drivers_string(1+xoffs, 2+yoffs, low);
		}
	}

	drivers_cursor(1, 1, CURSOR_OFF);
	drivers_flush();

	return 0;
}
