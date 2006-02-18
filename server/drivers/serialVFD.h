/* 	This is the LCDproc driver for various serial VFD Devices

	Copyright (C) 2006 Stefan Herdler

	This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
	driver.
	It may contain parts of other drivers of this package too.

	2006-02-13 Version 0.2: everything should work (not all hardware tested!)

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

#ifndef SERIALVFD_H
#define SERIALVFD_H
#include "lcd.h"
#include "adv_bignum.h"

#define CCMODE_STANDARD 0 /* only char 0 is used for heartbeat */
#define CCMODE_VBAR 1
#define CCMODE_HBAR 2
#define CCMODE_BIGNUM 3
#define CCMODE_BIGCHAR 4
#define CCMODE_CUSTOM 5


#define DEFAULT_CELL_WIDTH	5
#define DEFAULT_CELL_HEIGHT	7
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		9600
#define DEFAULT_BRIGHTNESS	140
#define DEFAULT_SIZE		"20x2"
#define DEFAULT_DISPLAYTYPE 	0


MODULE_EXPORT int  serialVFD_init (Driver *drvthis);
MODULE_EXPORT void serialVFD_close (Driver *drvthis);
MODULE_EXPORT int  serialVFD_width (Driver *drvthis);
MODULE_EXPORT int  serialVFD_height (Driver *drvthis);
MODULE_EXPORT int  serialVFD_cellwidth (Driver *drvthis);
MODULE_EXPORT int  serialVFD_cellheight (Driver *drvthis);
MODULE_EXPORT void serialVFD_clear (Driver *drvthis);
MODULE_EXPORT void serialVFD_flush (Driver *drvthis);
MODULE_EXPORT void serialVFD_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void serialVFD_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void serialVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void serialVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  serialVFD_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void serialVFD_set_char (Driver *drvthis, int n, char *dat);

MODULE_EXPORT int  serialVFD_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void serialVFD_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void serialVFD_output (Driver *drvthis, int state);
MODULE_EXPORT void serialVFD_num (Driver * drvthis, int x, int num);
MODULE_EXPORT int serialVFD_get_free_chars (Driver *drvthis);
MODULE_EXPORT char * serialVFD_get_info( Driver *drvthis );
#endif
