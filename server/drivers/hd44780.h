/*
 * Base driver module for Hitachi HD44780 based LCD displays. This is
 * a modular driver that allows support for alternative HD44780
 * designs to be added in a flexible and maintainable manner.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *
 1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 */

#ifndef HD44780_H
#define HD44780_H

extern char have_keypad;	   // non-zero if the keypad code is activated

int HD44780_init (struct lcd_logical_driver *driver, char *args);
/* The following methods can all be hidden. They are used through function ptrs
void HD44780_close();
void HD44780_flush();
void HD44780_flush_box(int lft, int top, int rgt, int bot);
int HD44780_contrast(int contrast);
void HD44780_backlight(int on);
void HD44780_init_vbar();
void HD44780_init_hbar();
void HD44780_vbar(int x, int len);
void HD44780_hbar(int x, int y, int len);
void HD44780_init_num();
void HD44780_num(int x, int num);
void HD44780_set_char(int n, char *dat);
void HD44780_icon(int which, char dest);
void HD44780_draw_frame(char *dat);
*/

#endif
