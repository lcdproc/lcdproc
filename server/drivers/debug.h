#ifndef LCD_DEBUG_H
#define LCD_DEBUG_H


int debug_init (struct lcd_logical_driver *driver, char *args);
void debug_close ();
void debug_clear ();
void debug_flush ();
void debug_string (int x, int y, char string[]);
void debug_chr (int x, int y, char c);
void debug_contrast (int contrast);
void debug_backlight (int on);
void debug_init_vbar ();
void debug_init_hbar ();
void debug_init_num ();
void debug_vbar (int x, int len);
void debug_hbar (int x, int y, int len);
void debug_num (int x, int num);
void debug_set_char (int n, char *dat);
void debug_icon (int which, char dest);
void debug_flush_box (int lft, int top, int rgt, int bot);
void debug_draw_frame (char *dat);
char debug_getkey ();


#endif
