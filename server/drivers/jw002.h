/** \file server/drivers/jw002.h
 * Includes for jw002.c, the JW-002 driver.
 */

#ifndef JW002_H
#define JW002_H
/*
    Includes for jw002.c, the JW-002 driver.

    Copyright (C) 2008, 2009, 2019, 2020, Ethan Dicks

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

#include "lcd.h"

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		19200
#define JW002_DEFAULT_WIDTH      24
#define JW002_DEFAULT_HEIGHT     8
#define JW002_DEFAULT_CELLWIDTH  5
#define JW002_DEFAULT_CELLHEIGHT 8

#define JW002_DEFAULT_FONT      0
#define JW002_MAX_FONT          32

#define DEFAULT_SIZE		"24x8"
#define DEFAULT_XOFFSET         0
#define DEFAULT_YOFFSET         0

#define DEFAULT_BACKLIGHT	0


MODULE_EXPORT int  jw002_init (Driver *drvthis);
MODULE_EXPORT void jw002_close (Driver *drvthis);
MODULE_EXPORT int  jw002_width (Driver *drvthis);
MODULE_EXPORT int  jw002_height (Driver *drvthis);
MODULE_EXPORT int  jw002_cellwidth (Driver *drvthis);
MODULE_EXPORT int  jw002_cellheight (Driver *drvthis);
MODULE_EXPORT void jw002_clear (Driver *drvthis);
MODULE_EXPORT void jw002_flush (Driver *drvthis);
MODULE_EXPORT void jw002_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void jw002_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char * jw002_get_key (Driver *drvthis);

MODULE_EXPORT void jw002_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void jw002_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  jw002_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void jw002_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void jw002_cursor (Driver *drvthis, int x, int y, int state);

MODULE_EXPORT int  jw002_get_free_chars (Driver *drvthis);
MODULE_EXPORT void jw002_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT void jw002_output (Driver *drvthis, int state);
MODULE_EXPORT const char * jw002_get_info (Driver *drvthis);

#endif /* JW002_H */
