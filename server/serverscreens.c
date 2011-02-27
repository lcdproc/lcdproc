/** \file server/serverscreens.c
 * This file contains code to allow the server to generate its own screens.
 * Currently, the startup, goodbye and server status screen are provided. The
 * server status screen shows total number of connected clients, and the
 * combined total of screens they provide.
 *
 * It is interesting to note that the server creates a special screen
 * definition for its screens, but uses the same widget set made available
 * to clients.
 */

/*-
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 *               2007, Peter Marschall
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
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

/* file-local function declarations */
static int reset_server_screen(int rotate, int heartbeat, int title);


/**
 * Create the server screen and (optionally) print the hello message to it.
 * \return  -1 on error allocating the screen or one of its widgets,
 *           0 otherwise.
 */
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
	}

	/* set parameters for server_screen and it's widgets */
	reset_server_screen(rotate_server_screen, !has_hello_msg, !has_hello_msg);

	/* set the widgets depending on the Hello option in LCDd.conf */
	if (has_hello_msg) {		/* show whole Hello message */
		int i;

		for (i = 0; i < display_props->height; i++) {
	 		const char *line = config_get_string("Server", "Hello", i, "");
			char id[8];

			sprintf(id, "line%d", i+1);
			w = screen_find_widget(server_screen, id);
			if ((w != NULL) && (w->text != NULL)) {
				strncpy(w->text, line, LCD_MAX_WIDTH);
				w->text[LCD_MAX_WIDTH] = '\0';
			}
		}
	}

	/* And enqueue the screen */
	screenlist_add(server_screen);

	debug(RPT_DEBUG, "%s() done", __FUNCTION__);

	return 0;
}

int
server_screen_shutdown(void)
{
	if (server_screen == NULL)
		return -1;

	screenlist_remove(server_screen);
	screen_destroy(server_screen);
	return 0;
}


/**
 * Print the numbers of connected clients and screens on the server screen
 * unless screen is set to be blank. If a custom hello message has been set
 * it is shown until the first client connects.
 * \return  Always 0.
 */
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

	/* turn off the Hello message after the first client connected */
	if (has_hello_msg && !hello_done) {
		/* TODO:
		 * checking for num_clients is not really correct; we really
		 * want num_screens (see also comment in main.c).
		 * Unfortunately we do only get called if the server screen
		 * needs to be updated; therefore we get num_screen updated too
		 * late so that after a client disconnects to quickly (in its
		 * 1st round of screens showing) num_screens still is 0.
		 */
		if (num_clients != 0) {
			reset_server_screen(rotate_server_screen, 1, 1);
			hello_done = 1;
		}
		else {
			return 0;
		}
	}

	/* ... and screens */
	for (c = clients_getfirst(); c != NULL; c = clients_getnext()) {
		num_screens += client_screen_count(c);
	}

	/* update statistics if we do not only want to show a blank screen */
	if (rotate_server_screen != SERVERSCREEN_BLANK) {
		/* format strings for the appropriate display size ... */
		if (display_props->height >= 3) {	/* >2-line display */
			w = screen_find_widget(server_screen, "line2");
			if ((w != NULL) && (w->text != NULL)) {
				snprintf(w->text, LCD_MAX_WIDTH,
						"Clients: %i", num_clients);
			}

			w = screen_find_widget(server_screen, "line3");
			if ((w != NULL) && (w->text != NULL)) {
				snprintf(w->text, LCD_MAX_WIDTH,
						"Screens: %i", num_screens);
			}
		} else {				/* 2-line display */
			w = screen_find_widget(server_screen, "line2");
			if ((w != NULL) && (w->text != NULL)) {
				snprintf(w->text, LCD_MAX_WIDTH,
						((display_props->width >= 16)
						 ? "Cli: %i  Scr: %i"
						 : "C: %i  S: %i"),
						num_clients, num_screens);
			}
		}
	}
	return 0;
}


/**
 * Writes the default or a custom goodbye message defined in the config file
 * to the screen. Default message is centered on the screen while the custom
 * message has to be formatted by the user.
 * \return  Always 0.
 */
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


/**
 * Clear all text on the server screen and (optionally) reset the title. If
 * the screen is blank or off it is put in the background. If it is on, the
 * screen is shown as a regular screen (priority info).
 *
 * \param rotate     Server screen state (on/off/blank).
 * \param heartbeat  If true (1) show the heartbeat unless the screen is blank.
 * \param title      If true (1) and screen is not blank print the default title.
 * \return  -1 if no server screen has been created yet, 0 otherwise.
 */
static int
reset_server_screen(int rotate, int heartbeat, int title)
{
	int i;

	if (server_screen == NULL)
		return -1;

	server_screen->heartbeat = (heartbeat && (rotate != SERVERSCREEN_BLANK))
					? HEARTBEAT_OPEN : HEARTBEAT_OFF;
	server_screen->priority = (rotate == SERVERSCREEN_ON)
					? PRI_INFO : PRI_BACKGROUND;

	for (i = 0; i < display_props->height; i++) {
		char id[8];
		Widget *w;

		sprintf(id, "line%d", i+1);
		w = screen_find_widget(server_screen, id);
		if (w != NULL) {
			w->x = 1;
			w->y = i+1;
			w->type = ((i == 0) && (title) && (rotate != SERVERSCREEN_BLANK))
					? WID_TITLE : WID_STRING;

			if (w->text != NULL) {
				w->text[0] = '\0';
				if ((i == 0) && (title) && (rotate != SERVERSCREEN_BLANK)) {
					strncpy(w->text, "LCDproc Server", LCD_MAX_WIDTH);
					w->text[LCD_MAX_WIDTH] = '\0';
				}
			}
		}
	}
	return 0;
}
