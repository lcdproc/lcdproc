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

#define HEART_OFF 1
#define HEART_ON 2
#define HEART_OPEN 3

#define HEARTBEAT_OFF HEART_OFF
#define HEARTBEAT_ON HEART_ON
#define HEARTBEAT_OPEN HEART_OPEN

#define SERVER_SCREEN_NEVER 0
#define SERVER_SCREEN_NOSCREEN 1
#define SERVER_SCREEN_ALWAYS 2

#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON 1
#define BACKLIGHT_OPEN 2
#define BACKLIGHT_NOTSET 3

#define BACKLIGHT_BLINK 0x100
#define BACKLIGHT_FLASH 0x200

extern int heartbeat;
extern int backlight;
extern int backlight_state;
extern int backlight_brightness;
extern int backlight_off_brightness;
extern int output_state;
int draw_screen (screen * s, int timer);

#endif
