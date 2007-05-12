/** \file serverscreens.c
 * Implement the serverscreens.
 */

/*
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *               2007, Peter Marschall
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

#include "drivers.h"
#include "clients.h"
#include "render.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "main.h"
#include "serverscreens.h"


/* global variables */
Screen *server_screen = NULL;
int rotate_server_screen = UNSET_INT;

/* file-local variables */
static int has_hello_msg = 0;

int
server_screen_init(void)
{
	Widget *w;
	int i;

	has_hello_msg = config_has_key("Server", "Hello");

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	/* Create the screen */
	server_screen = screen_create("_server_screen", NULL);
	if (!server_screen) {
		report(RPT_ERR, "server_screen_init: Error allocating screen");
		return -1;
	}
	server_screen->name = "Server screen";
	server_screen->duration = RENDER_FREQ; /* 1 second, instead of 4...*/
	server_screen->heartbeat = (has_hello_msg) ? HEARTBEAT_OFF : HEARTBEAT_OPEN;
	server_screen->priority = (rotate_server_screen == SERVERSCREEN_ON)
				  ? PRI_INFO : PRI_BACKGROUND;

	/* Create all the widgets...*/
	for (i = 0; i < display_props->height; i++) {
		char id[8];
		sprintf(id, "line%d", i+1);

		w = widget_create(id, WID_STRING, server_screen);
		if (w == NULL) {
			report(RPT_ERR, "server_screen_init: Can't create a widget");
			return -1;
		}
		screen_add_widget(server_screen, w);
		w->x = 1;
		w->y = i+1;
		w->text = calloc(LCD_MAX_WIDTH+1, 1);

		/* depending on Hello in LCDd.conf set the widgets */
		if (has_hello_msg) {
		 	const char *line = config_get_string("Server", "Hello", i, "");

			strncpy(w->text, line, LCD_MAX_WIDTH);
			w->text[LCD_MAX_WIDTH] = '\0';
		}
		else {
			if (i == 0) {
				w->type = WID_TITLE;
				strncpy(w->text, "LCDproc Server", LCD_MAX_WIDTH);
			} else {
				w->text[0] = '\0';
			}
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
	static int hello_done = 0;
	Client *c;
	Widget *w;
	int num_clients = 0;
	int num_screens = 0;

	/* get info on the number of connected clients...*/
	num_clients = clients_client_count();

	/* turn off the Hello message after the first client onnected */
	if (has_hello_msg && !hello_done) {
		if (num_clients != 0) {
			if (!hello_done) {
				server_screen->heartbeat = HEARTBEAT_OPEN;

				w = screen_find_widget(server_screen, "line1");
				w->type = WID_TITLE;
				strncpy(w->text, "LCDproc Server", LCD_MAX_WIDTH);
			}	
			hello_done = 1;
		}
		if (!hello_done)
			return 0;
	}

	/* ... and screens */
	for (c = clients_getfirst(); c != NULL; c = clients_getnext()) {
		num_screens += client_screen_count(c);
	}

	/* format strings for the appropriate display size ... */
	if (display_props->height >= 3) {	/* >2-line display */
		w = screen_find_widget(server_screen, "line2");
		if (w != NULL) {
			snprintf(w->text, LCD_MAX_WIDTH,
					"Clients: %i", num_clients);
		}			

		w = screen_find_widget(server_screen, "line3");
		if (w != NULL) {
			snprintf(w->text, LCD_MAX_WIDTH,
					"Screens: %i", num_screens);
		}			
	} else {				/* 2-line display */
		w = screen_find_widget(server_screen, "line2");
		if (w != NULL) {
			snprintf(w->text, LCD_MAX_WIDTH,
					((display_props->width >= 16)
					 ? "Cli: %i  Scr: %i"
					 : "C: %i  S: %i"),
					num_clients, num_screens);
		}		
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
