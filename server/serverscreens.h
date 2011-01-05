/** \file server/serverscreens.h
 * Interface for the serverscreen implementation.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 */

#ifndef SRVSTATS_H
#define SRVSTATS_H

#include "screen.h"

/* server screen rotation states */
#define SERVERSCREEN_OFF	0	/**< Show server screen in rotation. */
#define SERVERSCREEN_ON		1	/**< Show server sreen only when there is no other screen. */
#define SERVERSCREEN_BLANK	2	/**< Don't rotate, and only show a blank screen. */

extern Screen *server_screen;

extern int rotate_server_screen;

int server_screen_init(void);
int server_screen_shutdown(void);
int update_server_screen(void);
int goodbye_screen(void);

#endif
