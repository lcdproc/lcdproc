#ifndef LCD_CURSES_H
#define LCD_CURSES_H

extern lcd_logical_driver *curses_drv;

int curses_drv_init (struct lcd_logical_driver *driver, char *args);
void curses_drv_backlight (int on);
void curses_drv_close ();
void curses_drv_clear ();
void curses_drv_flush ();
void curses_drv_string (int x, int y, char string[]);
void curses_drv_chr (int x, int y, char c);
void curses_drv_vbar (int x, int len);
void curses_drv_hbar (int x, int y, int len);
void curses_drv_icon (int which, char dest);
void curses_drv_flush ();
void curses_drv_flush_box (int lft, int top, int rgt, int bot);
void curses_drv_draw_frame (char *dat);
char curses_drv_getkey ();
void curses_drv_init_num ();
void curses_drv_num (int x, int num);
void curses_drv_heartbeat (int type);

/*Default settings for config file parsing*/
#define CONF_DEF_FOREGR "blue"
#define CONF_DEF_BACKGR "cyan"
#define CONF_DEF_BACKLIGHT "red"
#define CONF_DEF_SIZE "20x4"
#define CONF_DEF_TOP_LEFT_X 7
#define CONF_DEF_TOP_LEFT_Y 7

#endif
