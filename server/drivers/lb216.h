#ifndef LB216_H
#define LB216_H


extern lcd_logical_driver *LB216;

int LB216_init(lcd_logical_driver *driver, char *device);
void LB216_clear ();
void LB216_close();
void LB216_string (int x, int y, char string[]);
void LB216_flush();
void LB216_flush_box(int lft, int top, int rgt, int bot);
void LB216_chr(int x, int y, char c) ;
void LB216_backlight(int on);
void LB216_init_vbar();
void LB216_init_hbar();
void LB216_vbar(int x, int len);
void LB216_hbar(int x, int y, int len);
void LB216_init_num();
void LB216_num(int x, int num);
void LB216_set_char(int n, char *dat);
void LB216_icon(int which, char dest);
void LB216_draw_frame(char *dat);

#endif
