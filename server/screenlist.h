/*
 * screenlist.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef SCREENLIST_H
#define SCREENLIST_H

#include "screen.h"

#define SCR_HOLD 1
#define SCR_SKIP 2
#define SCR_BACK 3
#define RENDER_HOLD 11
#define RENDER_SKIP 12
#define RENDER_BACK 13

extern int screenlist_action;
extern int timer;

int screenlist_init ();
int screenlist_shutdown ();

LinkedList *screenlist_getlist ();
screen *screenlist_current ();

int screenlist_add (screen * s);
screen *screenlist_next ();
screen *screenlist_prev ();

int screenlist_remove (screen * s);
int screenlist_remove_all (screen * s);

#endif
