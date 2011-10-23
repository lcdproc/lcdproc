#ifndef LCD_CURSES_H
#define LCD_CURSES_H

MODULE_EXPORT int  curses_init (Driver *drvthis);
MODULE_EXPORT void curses_close (Driver *drvthis);
MODULE_EXPORT int  curses_width (Driver *drvthis);
MODULE_EXPORT int  curses_height (Driver *drvthis);
MODULE_EXPORT void curses_clear (Driver *drvthis);
MODULE_EXPORT void curses_flush (Driver *drvthis);
MODULE_EXPORT void curses_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void curses_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void curses_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void curses_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void curses_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void curses_heartbeat (Driver *drvthis, int type);
MODULE_EXPORT int  curses_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void curses_backlight (Driver *drvthis, int on);

MODULE_EXPORT const char *curses_get_key (Driver *drvthis);
MODULE_EXPORT const char *curses_get_info(Driver *drvthis);

/* Default settings for config file parsing */
#define CONF_DEF_FOREGR		"blue"
#define CONF_DEF_BACKGR		"cyan"
#define CONF_DEF_BACKLIGHT	"red"
#define CONF_DEF_SIZE		"20x4"
#define CONF_DEF_TOP_LEFT_X	7
#define CONF_DEF_TOP_LEFT_Y	7
#define CONF_DEF_USEACS		0
#define CONF_DEF_DRAWBORDER	1

#endif
