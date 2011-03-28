/*  This is the LCDproc driver for Cwlinux devices (http://www.cwlinux.com)

    Copyright (C) 2002, Andrew Ip
                  2002, David Glaude
                  2003, David Glaude
                  2006, Peter Marschall

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

#ifndef CWLNX_H
#define CWLNX_H

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_BACKLIGHT	1
#define DEFAULT_BRIGHTNESS	700

#define DEFAULT_CELL_WIDTH_1602		5
#define DEFAULT_CELL_WIDTH_12232	6
#define DEFAULT_CELL_WIDTH_12832	6
#define DEFAULT_CELL_WIDTH		DEFAULT_CELL_WIDTH_12232

#define DEFAULT_CELL_HEIGHT_1602	8
#define DEFAULT_CELL_HEIGHT_12232	8
#define DEFAULT_CELL_HEIGHT_12832	8
#define DEFAULT_CELL_HEIGHT		DEFAULT_CELL_HEIGHT_12232

#define DEFAULT_SPEED_1602	19200
#define DEFAULT_SPEED_12232	19200
#define DEFAULT_SPEED_12832	19200
#define DEFAULT_SPEED		DEFAULT_SPEED_12232

#define DEFAULT_SIZE_1602	"16x2"
#define DEFAULT_SIZE_12232	"20x4"
#define DEFAULT_SIZE_12832	"21x4"
#define DEFAULT_SIZE		DEFAULT_SIZE_12232


MODULE_EXPORT int  CwLnx_init(Driver *drvthis);
MODULE_EXPORT void CwLnx_close(Driver *drvthis);
MODULE_EXPORT int  CwLnx_width(Driver *drvthis);
MODULE_EXPORT int  CwLnx_height(Driver *drvthis);
MODULE_EXPORT int  CwLnx_cellwidth(Driver *drvthis);
MODULE_EXPORT int  CwLnx_cellheight(Driver *drvthis);
MODULE_EXPORT void CwLnx_clear(Driver *drvthis);
MODULE_EXPORT void CwLnx_flush(Driver *drvthis);
MODULE_EXPORT void CwLnx_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void CwLnx_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *CwLnx_get_key(Driver *drvthis);

MODULE_EXPORT void CwLnx_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CwLnx_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CwLnx_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  CwLnx_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT int  CwLnx_get_free_chars(Driver *drvthis);
MODULE_EXPORT void CwLnx_set_char(Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT int  CwLnx_get_contrast(Driver *drvthis);
MODULE_EXPORT void CwLnx_set_contrast(Driver *drvthis, int contrast);
MODULE_EXPORT void CwLnx_backlight(Driver *drvthis, int on);

#endif

