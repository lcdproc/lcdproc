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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

/* This driver is mostly based on the MtxOrb driver.
   See the file MtxOrb.c for copyright details */
/* The heartbeat workaround has been taken from the curses driver
   See the file curses_drv.c for copyright details */


#ifndef LCDM001_H
#define LCDM001_H

/********************************************************************
  lcdm001.h
******************************************************************/

// REMOVE: I don't thing this is actualy needed.
// extern lcd_logical_driver *lcdm001;

int lcdm001_init (struct lcd_logical_driver *driver, char *args);

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_CURSORBLINK	0

/*Heartbeat workaround
  set chars to be displayed instead of "normal" icons*/

#define OPEN_HEART ' '     //This combination is at least visible
#define FILLED_HEART '*'
#define PAD 255
#endif
