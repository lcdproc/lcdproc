/*
 * screenlist.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2003, Joris Robijn
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

/*extern int screenlist_action;*/
extern bool autorotate;

int screenlist_init ();
	/* Initializes the screenlist. */

int screenlist_shutdown ();
	/* Shuts down the screenlist. */

int screenlist_add (Screen * s);
	/* Adds a screen to the screenlist. */

int screenlist_remove (Screen * s);
	/* Removes a screen from the screenlist. */

void screenlist_process ();
	/* Processes the screenlist. Decides if we need to switch to an other
	 * screen. */

void screenlist_switch (Screen * s);
	/* Switches to an other screen in the proper way. Informs clients of
	 * the switch. ALWAYS USE THIS FUNCTION TO SWITCH SCREENS. */

Screen *screenlist_current ();
	/* Returns the currently active screen. */

int screenlist_goto_next ();
	/* Moves on to the next screen. */

int screenlist_goto_prev ();
	/* Moves on to the previous screen. */

#endif
