/*
 * drivers.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
 */

int
load_driver( char * name, char * filename, char * args );
// returns:
// <0 error
//  0 ok, driver is an input driver only
//  1 ok, driver is an output driver
//  2 ok, driver is an output driver that needs to run in the foreground

int
unload_all_drivers();
