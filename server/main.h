/*
 * main.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
  contains a few things that other parts of the program might want
  to know about...
 */

extern char *version;
extern char *protocol_version;
extern char *build_date;

void exit_program (int val);

/* 1/8th second is a single time unit...*/
#define TIME_UNIT 125000
/* But I plan to double the framerate soon, or make it variable...*/
/*#define TIME_UNIT (125000/2)*/

#define DEFAULT_SCREEN_PRIORITY 128
#define DEFAULT_SCREEN_DURATION 32
#define DEFAULT_HEARTBEAT HEARTBEAT_ON
#define DEFAULT_ADDR "127.0.0.1"

#endif
