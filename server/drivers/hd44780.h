/** \file server/drivers/hd44780.h
 * Interface to the base driver module for Hitachi HD44780 based LCD displays.
 *
 * This is a modular driver that allows support for alternative HD44780
 * designs to be added in a flexible and maintainable manner.
 */

/* This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 */

#ifndef HD44780_H
#define HD44780_H

MODULE_EXPORT int  HD44780_init(Driver *drvthis);
MODULE_EXPORT void HD44780_close(Driver *drvthis);
MODULE_EXPORT int  HD44780_width(Driver *drvthis);
MODULE_EXPORT int  HD44780_height(Driver *drvthis);
MODULE_EXPORT int  HD44780_cellwidth(Driver *drvthis);
MODULE_EXPORT int  HD44780_cellheight(Driver *drvthis);
MODULE_EXPORT void HD44780_clear(Driver *drvthis);
MODULE_EXPORT void HD44780_flush(Driver *drvthis);
MODULE_EXPORT void HD44780_string(Driver *drvthis, int x, int y, const char s[]);
MODULE_EXPORT void HD44780_chr(Driver *drvthis, int x, int y, char ch);

MODULE_EXPORT void HD44780_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void HD44780_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void HD44780_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  HD44780_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void HD44780_set_char(Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int  HD44780_get_free_chars(Driver *drvthis);

MODULE_EXPORT int  HD44780_get_contrast(Driver *drvthis);
MODULE_EXPORT void HD44780_set_contrast(Driver *drvthis, int promille);
MODULE_EXPORT int  HD44780_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void HD44780_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void HD44780_backlight(Driver *drvthis, int on);
MODULE_EXPORT void HD44780_output(Driver *drvthis, int state);

MODULE_EXPORT const char *HD44780_get_key(Driver *drvthis);

#endif
