#ifndef MAIN_H
#define MAIN_H

extern int Quit;
extern int sock;
extern char *version;
extern char *build_date;

extern int lcd_wid;
extern int lcd_hgt;
extern int lcd_cellwid;
extern int lcd_cellhgt;

typedef struct mode {
   char which;			// Which screen is it?
   int on_time;			// How often to update while visible?
   int off_time;		// How often to get stats while not visible?
   int show_invisible;		// Send stats while not visible?
   int timer;			// Time since last update
   int visible;			// Can we be seen right now?
} mode;

#define BLINK_ON 0x10
#define BLINK_OFF 0x11
#define BACKLIGHT_OFF 0x20
#define BACKLIGHT_ON 0x21
#define HOLD_SCREEN 0x30
#define CONTINUE 0x31

#define LCD_MAX_WIDTH 80
#define LCD_MAX_HEIGHT 80

#endif
