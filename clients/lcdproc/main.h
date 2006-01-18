#ifndef MAIN_H
#define MAIN_H

#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif

#define LCDP_BATT_HIGH		0x00
#define LCDP_BATT_LOW		0x01
#define LCDP_BATT_CRITICAL	0x02
#define LCDP_BATT_CHARGING	0x03
#define LCDP_BATT_ABSENT	0x04
#define LCDP_BATT_UNKNOWN	0xFF

#define LCDP_AC_OFF		0x00
#define LCDP_AC_ON		0x01
#define LCDP_AC_BACKUP		0x02
#define LCDP_AC_UNKNOWN		0xFF

extern int Quit;
extern int sock;
extern char *version;
extern char *build_date;

extern int lcd_wid;
extern int lcd_hgt;
extern int lcd_cellwid;
extern int lcd_cellhgt;

typedef struct mode
{
	char *longname;			  // Which screen is it?
	char which;			  // Which screen is it?
	int on_time;			  // How often to update while visible?
	int off_time;			  // How often to get stats while not visible?
	int show_invisible;		  // Send stats while not visible?
	int timer;			  // Time since last update
	int visible;			  // Can we be seen right now?
} mode;

#define BLINK_ON	0x10
#define BLINK_OFF	0x11
#define BACKLIGHT_OFF	0x20
#define BACKLIGHT_ON	0x21
#define HOLD_SCREEN	0x30
#define CONTINUE	0x31

#define LCD_MAX_WIDTH	80
#define LCD_MAX_HEIGHT	80

const char *get_hostname();
const char *get_sysname();
const char *get_sysrelease();

#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif
