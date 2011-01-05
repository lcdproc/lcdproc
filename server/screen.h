/** \file server/screen.h
 * Public interface to the screen management methods.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 * 		 2003, Joris Robijn
 */

#include "menu.h"
#include "menuitem.h"
#include "client.h"
/* These headers are placed here on purpose ! (circular references) */

#ifndef SCREEN_H
#define SCREEN_H

#include "shared/LL.h"
#include "client.h"

typedef enum {	PRI_HIDDEN, PRI_BACKGROUND, PRI_INFO, PRI_FOREGROUND,
		PRI_ALERT, PRI_INPUT
} Priority;

typedef struct Screen {
	char *id;
	char *name;
	int width, height;
	int duration;
	int timeout;
	Priority priority;
	short int heartbeat;
	short int backlight;
	short int cursor;
	short int cursor_x;
	short int cursor_y;
	char *keys;
	LinkedList *widgetlist;
	struct Client *client;
} Screen;

#include "widget.h"


extern int  default_duration ;
extern int  default_priority ;

#include "client.h"

/* Creates a new screen */
Screen *screen_create(char *id, Client *client);

/* Destroys a screen */
int screen_destroy(Screen *s);

/* Add a widget to a screen */
int screen_add_widget(Screen *s, Widget *w);

/* Remove a widget from a screen (does not destroy it) */
int screen_remove_widget(Screen *s, Widget *w);

/* List functions */
static inline Widget *screen_getfirst_widget(Screen *s)
{
	return (Widget *) ((s != NULL)
			   ? LL_GetFirst(s->widgetlist)
			   : NULL);
}

static inline Widget *screen_getnext_widget(Screen *s)
{
	return (Widget *) ((s != NULL)
			   ? LL_GetNext(s->widgetlist)
			   : NULL);
}


/* Find a widget in a screen */
Widget *screen_find_widget(Screen *s, char *id);

/* Convert priority names to priority and vv */
Priority screen_pri_name_to_pri(char *pri_name);
char *screen_pri_to_pri_name(Priority pri);

#endif
