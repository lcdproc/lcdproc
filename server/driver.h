/*
 * driver.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "drivers/lcd.h"

#define bool int


Driver *
driver_load( char * name, char * filename, char * args );

int
driver_unload( Driver * driver );

int
driver_bind_module( Driver * driver );

int
driver_unbind_module( Driver * driver );

bool
driver_does_output( Driver * driver );

bool
driver_does_input( Driver * driver );

bool
driver_support_multiple( Driver * driver );

bool
driver_stay_in_foreground( Driver * driver );

#endif
