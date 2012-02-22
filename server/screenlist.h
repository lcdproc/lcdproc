/** \file server/screenlist.h
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2003, Joris Robijn
 */

#ifndef SCREENLIST_H
#define SCREENLIST_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif
#include "shared/defines.h"

#define AUTOROTATE_OFF		0
#define AUTOROTATE_ON		1

extern bool autorotate;		/**< If enabled, screens will rotate */

int screenlist_init(void);
	/* Initializes the screenlist. */

int screenlist_shutdown(void);
	/* Shuts down the screenlist. */

int screenlist_add(Screen *s);
	/* Adds a screen to the screenlist. */

int screenlist_remove(Screen *s);
	/* Removes a screen from the screenlist. */

void screenlist_process(void);
	/* Processes the screenlist. Decides if we need to switch to an other
	 * screen. */

void screenlist_switch(Screen *s);
	/* Switches to an other screen in the proper way. Informs clients of
	 * the switch. ALWAYS USE THIS FUNCTION TO SWITCH SCREENS. */

Screen *screenlist_current(void);
	/* Returns the currently active screen. */

int screenlist_goto_next(void);
	/* Moves on to the next screen. */

int screenlist_goto_prev(void);
	/* Moves on to the previous screen. */

#endif
