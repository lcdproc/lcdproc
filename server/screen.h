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

#ifndef SCREEN_H
#define SCREEN_H

#include "shared/LL.h"
#include "clients.h"

typedef struct screen {
	char *id;
	char *name;
	int wid, hgt;
	int priority;
	int duration;
	int heartbeat;
	int timeout;
	int backlight_state;
	int cursor;
	int cursor_x;
	int cursor_y;
	char *keys;
	LinkedList *widgets;
	client *parent;
} screen;

extern int  default_duration ;
extern int  default_priority ;

screen *screen_create ();
int screen_destroy (screen * s);

screen *screen_find (client * c, char *id);

int screen_add (client * c, char *id);
int screen_remove (client * c, char *id);

#endif
