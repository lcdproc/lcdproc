#ifndef STV5730_H
#define STV5730_H

extern lcd_logical_driver *stv5730;

int stv5730_init (struct lcd_logical_driver *driver, char *args);
void stv5730_close ();
void stv5730_clear ();
void stv5730_flush ();
void stv5730_string (int x, int y, char string[]);
void stv5730_chr (int x, int y, char c);
void stv5730_vbar (int x, int len);
void stv5730_hbar (int x, int y, int len);
void stv5730_num (int x, int num);
void stv5730_icon (int which, char dest);
void stv5730_flush_box (int lft, int top, int rgt, int bot);
void stv5730_draw_frame (char *dat);

#endif
