/*
 * serverscreens.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef SRVSTATS_H
#define SRVSTATS_H

#include "screen.h"

extern Screen * server_screen;

extern int rotate_server_screen;

int server_screen_init ();
int server_screen_shutdown ();
int update_server_screen ();
int goodbye_screen ();

#endif
