/** \file server/driver.h
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "drivers/lcd.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif
#include "shared/defines.h"

Driver *
driver_load(const char *name, const char *filename);

int
driver_unload(Driver *driver);

int
driver_bind_module(Driver *driver);

int
driver_unbind_module(Driver *driver);

bool
driver_does_output(Driver *driver);

bool
driver_does_input(Driver *driver);

bool
driver_support_multiple(Driver *driver);

bool
driver_stay_in_foreground(Driver *driver);


/* Alternative functions for all extended functions */

void driver_alt_vbar(Driver *drv, int x, int y, int len, int promille, int pattern);

void driver_alt_hbar(Driver *drv, int x, int y, int len, int promille, int pattern);

void driver_alt_num(Driver *drv, int x, int num);

void driver_alt_heartbeat(Driver *drv, int state);

void driver_alt_icon(Driver *drv, int x, int y, int icon);

void driver_alt_cursor(Driver *drv, int x, int y, int state);

#endif
