/*  This is the LCDproc driver for MSI-6931 displays
	as found in the following 1U rack servers by MSI:
		MS-9202
		MS-9205
		MS-9211

    Copyright (C) 2003, Marcel Pommer <marsellus at users dot sourceforge dot net>

	The code is derived from the CFontz driver

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

#ifndef MS6931_H
#define MS6931_H

#define MS6931_DEF_DEVICE "/dev/ttyS1"
#define MS6931_DEF_SIZE "16x2"
#define MS6931_DEF_CELL_WIDTH 5
#define MS6931_DEF_CELL_HEIGHT 8


MODULE_EXPORT int  ms6931_init	(Driver *drvthis);
MODULE_EXPORT void ms6931_close	(Driver *drvthis);
MODULE_EXPORT int  ms6931_width	(Driver *drvthis);
MODULE_EXPORT int  ms6931_height(Driver *drvthis);
MODULE_EXPORT void ms6931_flush	(Driver *drvthis);

MODULE_EXPORT int  ms6931_get_contrast	(Driver *drvthis);
MODULE_EXPORT void ms6931_set_contrast	(Driver *drvthis, int promille);
MODULE_EXPORT void ms6931_backlight		(Driver *drvthis, int on);

MODULE_EXPORT void ms6931_chr		(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void ms6931_clear 	(Driver *drvthis);
MODULE_EXPORT void ms6931_string	(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT const char *ms6931_get_key	(Driver *drvthis);
MODULE_EXPORT void ms6931_hbar		(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void ms6931_heartbeat	(Driver *drvthis, int state);

#endif /* ndef MS6931_H */
