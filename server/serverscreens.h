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

extern screen *server_screen;

int server_screen_init ();
int update_server_screen (int timer);
int goodbye_screen ();

#endif
