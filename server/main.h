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

/* You should be able to modify the following freqencies... */
#define RENDER_FREQ 8
/* We want 8 frames per second */
#define PROCESS_FREQ 32
/* And 32 times per second processing of messages and keypresses. */
#define MAX_RENDER_LAG_FRAMES 16
/* Allow the rendering strokes to lag behind this many frames.
 * More lag will not be corrected, but will cause slow-down. */
#define TIME_UNIT (1e6/RENDER_FREQ)
/* Variable from stone age, still used a lot.  */

extern long int timer;
/* 32 bits at 8Hz will overflow in 2 ^ 29 = 5e8 seconds = 17 years.
 * If you get an overflow, please mail us and we will fix this personally
 * for you ! */

#endif
