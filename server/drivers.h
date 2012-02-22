/** \file server/drivers.h
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 */

#ifndef DRIVERS_H
#define DRIVERS_H

#include "drivers/lcd.h"
#include "shared/LL.h"

typedef struct DisplayProps {
	int width, height;
	int cellwidth, cellheight;
} DisplayProps;

extern DisplayProps *display_props;

int
drivers_load_driver(const char *name);

int
drivers_unload_all(void);

const char *
drivers_get_info(void);

void
drivers_clear(void);

void
drivers_flush(void);

void
drivers_string(int x, int y, const char *string);

void
drivers_chr(int x, int y, char c);

void
drivers_vbar(int x, int y, int len, int promille, int pattern);

void
drivers_hbar(int x, int y, int len, int promille, int pattern);

void
drivers_num(int x, int num);

void
drivers_heartbeat(int state);

void
drivers_icon(int x, int y, int icon);

void
drivers_set_char(char ch, char *dat);

int
drivers_get_contrast(void);

void
drivers_set_contrast(int contrast);

void
drivers_cursor(int x, int y, int state);

void
drivers_backlight(int brightness);

void
drivers_output(int state);

const char *
drivers_get_key(void);


/* Please don't read this list except using the following functions */
extern LinkedList *loaded_drivers;

static inline Driver *drivers_getfirst(void)
{
	return LL_GetFirst(loaded_drivers);
}

static inline Driver *drivers_getnext(void)
{
	return LL_GetNext(loaded_drivers);
}

#endif
