/*  This is the LCDproc driver for the VFD of the Medion MD8800 PC

    Copyright (C) 2006 Stefan Herdler in collaboration with Martin Møller.
    This source Code is based on the NoritakeVFD, the serialVFD and the
    CFontzPacket Driver of this package.

    2006-05-07 Version 0.1: mostly everything should work

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

#ifndef MD8800_H
#define MD8800_H

#define DEFAULT_CELL_WIDTH	5
#define DEFAULT_CELL_HEIGHT	7
#define DEFAULT_DEVICE		"/dev/ttyS1"
#define DEFAULT_SPEED		9600
#define DEFAULT_SIZE		"16x2"

MODULE_EXPORT int  MD8800_init (Driver *drvthis);
MODULE_EXPORT void MD8800_close (Driver *drvthis);
MODULE_EXPORT int  MD8800_width (Driver *drvthis);
MODULE_EXPORT int  MD8800_height (Driver *drvthis);
MODULE_EXPORT int  MD8800_cellwidth (Driver *drvthis);
MODULE_EXPORT int  MD8800_cellheight (Driver *drvthis);
MODULE_EXPORT void MD8800_clear (Driver *drvthis);
MODULE_EXPORT void MD8800_flush (Driver *drvthis);
MODULE_EXPORT void MD8800_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void MD8800_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void MD8800_output (Driver *drvthis, int on);
MODULE_EXPORT const char * MD8800_get_info(Driver *drvthis);
MODULE_EXPORT void MD8800_backlight (Driver *drvthis, int on);
MODULE_EXPORT void MD8800_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT int  MD8800_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT int MD8800_icon (Driver *drvthis, int x, int y, int icon);


#endif
