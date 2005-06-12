/*
 * render.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef RENDER_H
#define RENDER_H

#include "screen.h"

#define HEARTBEAT_OFF 0
#define HEARTBEAT_ON 1
#define HEARTBEAT_OPEN 2

#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON 1
#define BACKLIGHT_OPEN 2

#define BACKLIGHT_BLINK 0x100
#define BACKLIGHT_FLASH 0x200

#define CURSOR_OFF 0
#define CURSOR_DEFAULT_ON 1
#define CURSOR_BLOCK 4
#define CURSOR_UNDER 5

extern int heartbeat;
extern int backlight;
extern int output_state;

int render_screen (Screen * s, long int timer);
	/* Renders the given screen. */

int server_msg( const char * text, int expire );
	/* Displays a short message in a corner. Message must be shorter
	 * than 16 chars. */

#endif
