#ifndef LCD_TEXT_H
#define LCD_TEXT_H

extern lcd_logical_driver *text;

int text_init (struct lcd_logical_driver *driver, char *args);
void text_close ();
void text_clear ();
void text_flush ();
void text_string (int x, int y, char string[]);
void text_chr (int x, int y, char c);
int text_contrast (int contrast);
void text_backlight (int on);
void text_init_vbar ();
void text_init_hbar ();
void text_init_num ();
void text_vbar (int x, int len);
void text_hbar (int x, int y, int len);
void text_num (int x, int num);
void text_set_char (int n, char *dat);
void text_flush_box (int lft, int top, int rgt, int bot);
void text_draw_frame (char *dat);


#endif
