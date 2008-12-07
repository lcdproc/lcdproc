/*  This is the LCDproc driver for the LCD on the Logitech G15 keyboard

    Copyright (C) 2006 Anthony J. Mirabella.

    2006-07-23 Version 1.0: Most functions should be implemented and working

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

#ifndef G15_H_
#define G15_H_

//#include <usb.h>
#include <libg15.h>
#include <libg15render.h>
#include "lcd.h"

/** private data for the \c g15 driver */
typedef struct g15_private_data {
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* file descriptor for g15daemon socket */
	int g15screen_fd;
	/* g15daemon version for compatibility checks */
	const char *g15d_ver;
	/* canvas for LCD contents */
	g15canvas *canvas;
	g15canvas *backingstore;
	/* status indicators */
	int backlight_state;
} PrivateData;

#define G15_OFFSET				32
#define G15_PX_WIDTH			160
#define G15_PX_HEIGHT			43
#define G15_CHAR_WIDTH				20
#define G15_CHAR_HEIGHT				5
#define G15_CELL_WIDTH			8
#define G15_CELL_HEIGHT			8
#define G15_LCD_WRITE_CMD 		0x03
#define G15_USB_ENDPT 			2
#define G15_ICON_HEART_FILLED	3
#define G15_ICON_HEART_OPEN		3
#define G15_ICON_ARROW_UP		24
#define G15_ICON_ARROW_DOWN		25
#define G15_ICON_ARROW_RIGHT	26
#define G15_ICON_ARROW_LEFT		27
#define G15_ICON_CHECKBOX_ON	7
#define G15_ICON_CHECKBOX_OFF	9
#define G15_ICON_CHECKBOX_GRAY	10
#define G15_ICON_STOP			254
#define G15_ICON_PAUSE			186
#define G15_ICON_PLAY			16
#define G15_ICON_PLAYR			17
#define G15_ICON_FF				175
#define G15_ICON_FR				174
#define G15_ICON_NEXT			242
#define G15_ICON_PREV			243
#define G15_ICON_REC			7
#define G15_BIGNUM_LEN			1032

extern short g15_bignum_data[11][G15_BIGNUM_LEN];

MODULE_EXPORT int  g15_init (Driver *drvthis);
MODULE_EXPORT void g15_close (Driver *drvthis);
MODULE_EXPORT int  g15_width (Driver *drvthis);
MODULE_EXPORT int  g15_height (Driver *drvthis);
MODULE_EXPORT int  g15_cellwidth (Driver *drvthis);
MODULE_EXPORT int  g15_cellheight (Driver *drvthis);
MODULE_EXPORT void g15_clear (Driver *drvthis);
MODULE_EXPORT void g15_flush (Driver *drvthis);
MODULE_EXPORT void g15_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void g15_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT int g15_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void g15_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void g15_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT const char * g15_get_key (Driver *drvthis);
MODULE_EXPORT void g15_backlight(Driver *drvthis, int on);
MODULE_EXPORT void g15_num(Driver *drvthis, int x, int num);

#endif /*G15_H_*/
