/*  This is the LCDproc driver for Seetron devices (http://www.seetron.com)

    Copyright (C) 1999, William Ferrell and Scott Scriven
    		  2001, Philip Pokorny
		  2001, David Douthitt
		  2001, David Glaude
		  2001, Joris Robijn
		  2001, Eddie Sheldrake
		  2001, Rene Wagner
		  2002, Harald Milz

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

#ifndef SGX120_H
#define SGX120_H

extern lcd_logical_driver *SGX120;

int SGX120_init (lcd_logical_driver * driver, char *device);
void SGX120_close ();
void SGX120_flush ();
void SGX120_flush_box (int lft, int top, int rgt, int bot);
void SGX120_chr (int x, int y, char c);
int SGX120_contrast (int contrast);
void SGX120_backlight (int on);
void SGX120_init_vbar ();
void SGX120_init_hbar ();
void SGX120_vbar (int x, int len);
void SGX120_hbar (int x, int y, int len);
void SGX120_init_num ();
void SGX120_num (int x, int num);
void SGX120_set_char (int n, char *dat);
void SGX120_icon (int which, char dest);
void SGX120_draw_frame (char *dat);
void SGX120_clear (void);
void SGX120_string (int x, int y, char string[]);

#define SGX120_DEF_CELL_WIDTH 6
#define SGX120_DEF_CELL_HEIGHT 8
#define SGX120_DEF_CONTRAST 140
#define SGX120_DEF_DEVICE "/dev/lcd"
#define SGX120_DEF_SPEED B9600
#define SGX120_DEF_SIZE "20x4"

#endif
