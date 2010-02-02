/** \file clients/lcdproc/main.h
 * Contains mode related defines and structures.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#ifndef MAIN_H
#define MAIN_H

#include "shared/defines.h"

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
#define LCDP_AC_UNKNOWN		0x03

extern int Quit;
extern int sock;
extern char *version;
extern char *build_date;

extern int lcd_wid;
extern int lcd_hgt;
extern int lcd_cellwid;
extern int lcd_cellhgt;

/** Screen data structure */
typedef struct _screen_mode
{
	char *longname;		/**< Which screen is it (long name)? */
	char which;		/**< Which screen is it (short name)? */
	int on_time;		/**< How often to update while visible? */
	int off_time;		/**< How often to get stats while not visible? */
	int show_invisible;	/**< Send stats while not visible? */
	int timer;		/**< Time since last update */
	int flags;		/**< See mode flags defines */
	int (*func)(int,int,int *);	/**< Pointer to init / update function */
} ScreenMode;

/* mode flags */
#define VISIBLE 	0x00000001	/**< currently visible */
#define ACTIVE 		0x00000002	/**< selected for display */
#define INITIALIZED	0x00000004	/**< screen had already been initialized */

#define BLINK_ON	0x10
#define BLINK_OFF	0x11
#define BACKLIGHT_OFF	0x20
#define BACKLIGHT_ON	0x21
#define HOLD_SCREEN	0x30
#define CONTINUE	0x31

#define LCD_MAX_WIDTH	80
#define LCD_MAX_HEIGHT	80

const char *get_hostname(void);
const char *get_sysname(void);
const char *get_sysrelease(void);

#endif
