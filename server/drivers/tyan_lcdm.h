/*  This is the LCDproc driver header for tyan lcd module (tyan Barebone GS series)

    Author: yhlu@tyan.com

    Copyright (C) 2004 Tyan Corp

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
#ifndef TYAN_LCDM_H
#define TYAN_LCDM_H

#include "lcd.h"

#define DEFAULT_CELL_WIDTH 5
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_DEVICE "/dev/lcd"
#define DEFAULT_SPEED 9600
#define DEFAULT_SIZE "16x2"

MODULE_EXPORT int  tyan_lcdm_init (Driver * drvthis, char *device);
MODULE_EXPORT void tyan_lcdm_close (Driver * drvthis);
MODULE_EXPORT int  tyan_lcdm_width (Driver * drvthis);
MODULE_EXPORT int  tyan_lcdm_height (Driver * drvthis);
MODULE_EXPORT void tyan_lcdm_clear (Driver * drvthis);
MODULE_EXPORT void tyan_lcdm_flush (Driver * drvthis);
MODULE_EXPORT void tyan_lcdm_string (Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void tyan_lcdm_chr (Driver * drvthis, int x, int y, char c);

MODULE_EXPORT void tyan_lcdm_vbar (Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void tyan_lcdm_hbar (Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void tyan_lcdm_num (Driver * drvthis, int x, int num);
MODULE_EXPORT int  tyan_lcdm_icon(Driver * drvthis, int x, int y, int icon);

MODULE_EXPORT void tyan_lcdm_set_char (Driver * drvthis, int n, char *dat);

MODULE_EXPORT void tyan_lcdm_backlight (Driver * drvthis, int on);

#endif
