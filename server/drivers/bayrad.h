/* Header file for BayRAD driver
 * for LCDproc LCD software
 * by Nathan Yawn, yawn@emacinc.com
 * 3/24/01
 */


#ifndef _BAYRAD_H
#define _BAYRAD_H

extern lcd_logical_driver *bayrad;

int bayrad_init(struct lcd_logical_driver *driver, char *args);
void bayrad_close();
void bayrad_clear();
void bayrad_flush();
void bayrad_string(int x, int y, char string[]);
void bayrad_chr(int x, int y, char c);
int bayrad_contrast(int contrast);
void bayrad_backlight(int on);
void bayrad_vbar(int x, int len);
void bayrad_init_vbar();
void bayrad_hbar(int x, int y, int len);
void bayrad_init_hbar();
void bayrad_set_char(int n, char *dat);
void bayrad_icon(int which, char dest);
void bayrad_flush_box(int lft, int top, int rgt, int bot);
void bayrad_draw_frame(char *dat);
char bayrad_getkey();

#endif
