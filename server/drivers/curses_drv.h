#ifndef LCD_CURSES_H
#define LCD_CURSES_H

#include "lcd.h"

              int  curses_drv_init (Driver * drvthis, char *args);
MODULE_EXPORT void curses_drv_close (Driver *drvthis);
MODULE_EXPORT int  curses_drv_width (Driver *drvthis);
MODULE_EXPORT int  curses_drv_height (Driver *drvthis);
MODULE_EXPORT void curses_drv_clear (Driver *drvthis);
MODULE_EXPORT void curses_drv_flush (Driver *drvthis);
MODULE_EXPORT void curses_drv_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void curses_drv_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void curses_drv_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void curses_drv_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void curses_drv_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void curses_drv_heartbeat (Driver *drvthis, int type);
MODULE_EXPORT void curses_drv_icon (Driver *drvthis, int which, char dest);

MODULE_EXPORT void curses_drv_backlight (Driver *drvthis, int on);

MODULE_EXPORT char curses_drv_getkey (Driver *drvthis);

MODULE_EXPORT void curses_drv_init_num (Driver *drvthis);

/*Default settings for config file parsing*/
#define CONF_DEF_FOREGR "blue"
#define CONF_DEF_BACKGR "cyan"
#define CONF_DEF_BACKLIGHT "red"
#define CONF_DEF_SIZE "20x4"  // currently not used, LCD_DEFAULT_WIDTH etc. is used
#define CONF_DEF_TOP_LEFT_X 7
#define CONF_DEF_TOP_LEFT_Y 7

#endif
