/** \file server/drivers/ax93304.h
 * Includes for ax93304.c, the AX93304 driver.
 */

#ifndef _AX93304_H
#define _AX93304_H
/*
    Includes for ax93304.c, the AX93304 driver.

    Copyright (C) 2026 Sebastian Stolz

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
*/

MODULE_EXPORT int  ax93304_init(Driver *drvthis);
MODULE_EXPORT void ax93304_close(Driver *drvthis);
MODULE_EXPORT int  ax93304_width(Driver *drvthis);
MODULE_EXPORT int  ax93304_height(Driver *drvthis);
MODULE_EXPORT int  ax93304_cellwidth(Driver *drvthis);
MODULE_EXPORT int  ax93304_cellheight(Driver *drvthis);
MODULE_EXPORT void ax93304_clear(Driver *drvthis);
MODULE_EXPORT void ax93304_flush(Driver *drvthis);
MODULE_EXPORT void ax93304_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void ax93304_chr(Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void ax93304_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void ax93304_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  ax93304_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void ax93304_set_char(Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT void ax93304_backlight(Driver *drvthis, int promille);

MODULE_EXPORT const char *ax93304_get_key(Driver *drvthis);

#endif
