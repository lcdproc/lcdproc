/*
 * drivers.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *
 */

#ifndef DRIVERS_H
#define DRIVERS_H

#include "drivers/lcd.h"
#include "shared/LL.h"

typedef struct DisplayProps {
	int width, height;
	int cellwidth, cellheight;
} DisplayProps;

extern DisplayProps * display_props;

#ifndef bool
# define bool short
# define true 1
# define false 0
#endif


int
drivers_load_driver( char * name );

int
drivers_unload_all();

const char *
drivers_get_info();

void
drivers_clear();

void
drivers_flush();

void
drivers_string( int x, int y, char * string );

void
drivers_chr( int x, int y, char c );

void
drivers_vbar( int x, int y, int len, int promille, int pattern );

void
drivers_hbar( int x, int y, int len, int promille, int pattern );

void
drivers_num( int x, int num );

void
drivers_heartbeat( int state );

void
drivers_icon( int x, int y, int icon );

void
drivers_set_char( char ch, char *dat );

int
drivers_get_contrast();

void
drivers_set_contrast( int contrast );

void
drivers_cursor( int x, int y, int state );

void
drivers_backlight( int brightness );

void
drivers_output( int state );

const char *
drivers_get_key();


/* Please don't read this list except using the following functions */
extern LinkedList * loaded_drivers;

static inline Driver * drivers_getfirst ()
{
	return LL_GetFirst(loaded_drivers);
}

static inline Driver * drivers_getnext ()
{
	return LL_GetNext(loaded_drivers);
}

#endif
