/*  This is the LCDproc driver for Noritake VFD Device CU20045SCPB-T28A

    Copyright (C) 2005 Simon Funke
    This source Code is based on CFontz Driver of this package.

    2005-08-01 Version 0.1: mostly everything should work (vbar, hbar never tested)

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

#ifndef NORITAKEVFD_H
#define NORITAKEVFD_H

#define DEFAULT_CELL_WIDTH	5
#define DEFAULT_CELL_HEIGHT	7
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		9600
#define DEFAULT_BRIGHTNESS	140
#define DEFAULT_OFFBRIGHTNESS	100
#define DEFAULT_SIZE		"20x4"
#define DEFAULT_PARITY		0

MODULE_EXPORT int  NoritakeVFD_init (Driver *drvthis);
MODULE_EXPORT void NoritakeVFD_close (Driver *drvthis);
MODULE_EXPORT int  NoritakeVFD_width (Driver *drvthis);
MODULE_EXPORT int  NoritakeVFD_height (Driver *drvthis);
MODULE_EXPORT int  NoritakeVFD_cellwidth (Driver *drvthis);
MODULE_EXPORT int  NoritakeVFD_cellheight (Driver *drvthis);
MODULE_EXPORT void NoritakeVFD_clear (Driver *drvthis);
MODULE_EXPORT void NoritakeVFD_flush (Driver *drvthis);
MODULE_EXPORT void NoritakeVFD_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void NoritakeVFD_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void NoritakeVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void NoritakeVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void NoritakeVFD_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  NoritakeVFD_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void NoritakeVFD_cursor (Driver *drvthis, int x, int y, int state);

MODULE_EXPORT int  NoritakeVFD_get_free_chars (Driver *drvthis);
MODULE_EXPORT void NoritakeVFD_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT int  NoritakeVFD_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void NoritakeVFD_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void NoritakeVFD_backlight (Driver *drvthis, int on);

#endif

