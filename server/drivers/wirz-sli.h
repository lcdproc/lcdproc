/*	wirz-sli.h -- Header file for LCDproc Wirz SLI driver
	Copyright (C) 1999 Horizon Technologies-http://horizon.pair.com/
	Written by Bryan Rittmeyer <bryanr@pair.com> - Released under GPL
			
        LCD info: http://www.wirz.com/sli/                               */

#ifndef SLI_H
#define SLI_H

extern lcd_logical_driver *sli;

int sli_init (lcd_logical_driver * driver, char *device);
void sli_close ();
void sli_flush ();
void sli_flush_box (int lft, int top, int rgt, int bot);
void sli_chr (int x, int y, char c);
int sli_contrast (int contrast);
void sli_backlight (int on);
void sli_init_vbar ();
void sli_init_hbar ();
void sli_vbar (int x, int len);
void sli_hbar (int x, int y, int len);
void sli_init_num ();
void sli_num (int x, int num);
void sli_set_char (int n, char *dat);
void sli_icon (int which, char dest);
void sli_draw_frame (char *dat);
char sli_getkey ();

#endif
