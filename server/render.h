/** \file server/render.h
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 */

#ifndef RENDER_H
#define RENDER_H

#define HEARTBEAT_OFF		0
#define HEARTBEAT_ON		1
#define HEARTBEAT_OPEN		2

#define BACKLIGHT_OFF		0
#define BACKLIGHT_ON		1
#define BACKLIGHT_OPEN		2

#define BACKLIGHT_BLINK		0x100
#define BACKLIGHT_FLASH		0x200

#define CURSOR_OFF		0
#define CURSOR_DEFAULT_ON	1
#define CURSOR_BLOCK		4
#define CURSOR_UNDER		5

#define TITLESPEED_NO		0	/* needs to be (TITLESPEED_MIN - 1) */
#define TITLESPEED_MIN		1
#define TITLESPEED_MAX		10

extern int heartbeat;
extern int backlight;
extern int titlespeed;
extern int output_state;

/* Render the given screen. */
int render_screen(Screen *s, long timer);

/* Display a short message, which must be shorter than 16 chars, in a corner */
int server_msg(const char *text, int expire);

#endif
