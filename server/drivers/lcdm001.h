/*  This is the header file of the LCDproc driver for
    the "LCDM001" device from kernelconcepts.de

    Copyright (C) 2001  Rene Wagner <reenoo@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */

/* This driver is mostly based on the MtxOrb driver.
   See the file MtxOrb.c for copyright details */
/* The heartbeat workaround has been taken from the curses driver
   See the file curses_drv.c for copyright details */


#ifndef LCDM001_H
#define LCDM001_H

/********************************************************************
  lcdm001.h
******************************************************************/

/* REMOVE: I don't thing this is actualy needed. */
/* extern lcd_logical_driver *lcdm001; */

MODULE_EXPORT int  lcdm001_init (Driver *drvthis);
MODULE_EXPORT void lcdm001_close (Driver *drvthis);
MODULE_EXPORT int  lcdm001_width (Driver *drvthis);
MODULE_EXPORT int  lcdm001_height (Driver *drvthis);
MODULE_EXPORT void lcdm001_clear (Driver *drvthis);
MODULE_EXPORT void lcdm001_flush (Driver *drvthis);
MODULE_EXPORT void lcdm001_string (Driver *drvthis, int x, int y, char *string);
MODULE_EXPORT void lcdm001_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void lcdm001_old_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void lcdm001_old_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void lcdm001_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void lcdm001_old_icon (Driver *drvthis, int which, char dest);

MODULE_EXPORT void lcdm001_output (Driver *drvthis, int on);

MODULE_EXPORT char lcdm001_getkey (Driver *drvthis);

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_CURSORBLINK	0

/*Heartbeat workaround
  set chars to be displayed instead of "normal" icons*/

#define OPEN_HEART ' '     /* This combination is at least visible */
#define FILLED_HEART '*'
#define PAD 255

#define LEFT_KEY '1'
#define RIGHT_KEY '2'
#define UP_KEY '3'
#define DOWN_KEY '4'

#endif
