/* Driver module for Hitachi HD44780 based Optrex DMC-20481 LCD display 
 * The module is operated in it's 4 bit-mode to be connected to a single
 * 8 bit-port
 *
 * Copyright (c) 1998 Richard Rognlie       GNU Public License  
 *                    <rrognlie@gamerz.net>
 *
 * Large quantities of this code lifted (nearly verbatim) from
 * the lcd4.c module of lcdtext.  Copyright (C) 1997 Matthias Prinke
 * <m.prinke@trashcan.mcnet.de> and covered by GNU's GPL.
 * In particular, this program is free software and comes WITHOUT
 * ANY WARRANTY.
 *
 * Matthias stole (er, adapted) the code from the package lcdtime by
 * Benjamin Tse (blt@mundil.cs.mu.oz.au), August/October 1995
 * which uses the LCD-controller's 8 bit-mode.
 * References: port.h             by <damianf@wpi.edu>
 *             Data Sheet LTN211, Philips
 *             Various FAQs and TXTs about Hitachi's LCD Controller HD44780 -
 *                www.paranoia.com/~filipg is a good starting point  ???   
 */

#ifndef HD44780_H
#define HD44780_H

#include "port.h"


extern lcd_logical_driver *hd44780;

int HD44780_init(struct lcd_logical_driver *driver, char *args);
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



#endif
