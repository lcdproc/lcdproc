#ifndef SED1520_H
#define SED1520_H

extern lcd_logical_driver *sed1520;

int sed1520_init (struct lcd_logical_driver *driver, char *args);
void sed1520_close ();
void sed1520_clear ();
void sed1520_flush ();
void sed1520_string (int x, int y, char string[]);
void sed1520_chr (int x, int y, char c);
void sed1520_vbar (int x, int len);
void sed1520_hbar (int x, int y, int len);
void sed1520_num (int x, int num);
void sed1520_set_char (int n, char *dat);
void sed1520_icon (int which, char dest);
void sed1520_flush_box (int lft, int top, int rgt, int bot);
void sed1520_draw_frame (char *dat);

#endif
