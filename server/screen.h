/*
 * screen.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#include "menu.h"
#include "menuitem.h"
#include "client.h"
/* These headers are placed here on purpose ! (circular references) */

#ifndef SCREEN_H
#define SCREEN_H

#include "shared/LL.h"
#include "client.h"

typedef struct Screen {
	char *id;
	char *name;
	int width, height;
	int duration;
	int timeout;
	short int priority;
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
Screen * screen_create (char * id, Client * client);

/* Destroys a screen */
int screen_destroy (Screen * s);

/* Add a widget to a screen */
int screen_add_widget (Screen * s, Widget * w);

/* Remove a widget from a screen (does not destroy it) */
int screen_remove_widget (Screen * s, Widget * w);

/* List functions */
static inline Widget * screen_getfirst_widget (Screen * s)
{
	return LL_GetFirst(s->widgetlist);
}

static inline Widget * screen_getnext_widget (Screen * s)
{
	return LL_GetNext(s->widgetlist);
}


/* Find a widget in a screen */
Widget *screen_find_widget (Screen * s, char *id);

#endif
