/** \file server/drivers/lcd.h
 * This file defines the LCDd-driver API
 *
 * It is written to facilitate loadable driver modules.
 * There should be no further interaction between driver and server core
 * other that via this API.
 *
 * Driver writer notes
 * ~~~~~~~~~~~~~~~~~~~
 * See documentation in the docs/lcdproc-dev/ directory.
 *
 * DO NOT MIX DRIVER ALLOCATED AND CORE ALLOCATED MEMORY.
 * With this I mean that the server core should NEVER WRITE in memory
 * allocated by the driver, and vice versa. Also the driver resp. core
 * should free or realloc the memory that it has allocated. You can always
 * simply copy a string if its data space is not 'yours'.
 */

/*-
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2001, Joris Robijn
 */

#ifndef LCD_H
#define LCD_H

#include <stddef.h>

/* Maximum supported sizes */
#define LCD_MAX_WIDTH 256
#define LCD_MAX_HEIGHT 256

/* Standard supported sizes */
#define LCD_DEFAULT_WIDTH 20
#define LCD_DEFAULT_HEIGHT 4
#define LCD_DEFAULT_CELLWIDTH 5
#define LCD_DEFAULT_CELLHEIGHT 8

/* Backlight data */
#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON  1

/* Icons. If a driver does not support an icon, it can return -1 from the
 * icon function, and let the core place a replacement character.
 */
/* Icons below are one character wide */
#define ICON_BLOCK_FILLED	0x100
#define ICON_HEART_OPEN		0x108
#define ICON_HEART_FILLED	0x109
#define ICON_ARROW_UP		0x110
#define ICON_ARROW_DOWN		0x111
#define ICON_ARROW_LEFT		0x112
#define ICON_ARROW_RIGHT	0x113
#define ICON_CHECKBOX_OFF	0x120
#define ICON_CHECKBOX_ON	0x121
#define ICON_CHECKBOX_GRAY	0x122
#define ICON_SELECTOR_AT_LEFT	0x128
#define ICON_SELECTOR_AT_RIGHT	0x129
#define ICON_ELLIPSIS		0x130

/* Icons below are two characters wide */
#define ICON_STOP		0x200	/* should look like  []  */
#define ICON_PAUSE		0x201	/* should look like  ||  */
#define ICON_PLAY		0x202	/* should look like  >   */
#define ICON_PLAYR		0x203	/* should llok like  <   */
#define ICON_FF			0x204	/* should look like  >>  */
#define ICON_FR			0x205	/* should look like  <<  */
#define ICON_NEXT		0x206	/* should look like  >|  */
#define ICON_PREV		0x207	/* should look like  |<  */
#define ICON_REC		0x208	/* should look like  ()  */

/* Icon numbers from 0 to 0x0FF could be used for client defined chars.
 * However this is not implemented and there are no crystalized ideas
 * about how to do it. get_free_chars and set_char should be used, but a
 * lot of things in that area might need to be changed.
 */

/* Heartbeat data */
#define HEARTBEAT_OFF 0
#define HEARTBEAT_ON 1

/* Patterns for hbar / vbar, mostly (if not all) UNIMPLEMENTED */
#define BAR_POS		0x001 /* default
				 Promilles allowed: 0 to 1000
				 The zero-point is at the left or bottom */
#define BAR_NEG		0x002 /* the bar grows in negative direction
				 Promilles allowed: -1000 to 0
				 The zero-point is at the left or top */
#define BAR_POS_AND_NEG	0x003 /* the bars can grow in both directions
				 Promilles allowed: -1000 to 1000
				 The zero-point is in the center */
#define BAR_PATTERN_FILLED	0x000 /* default */
#define BAR_PATTERN_OPEN	0x010
#define BAR_PATTERN_STRIPED	0x020
#define BAR_WITH_PERCENTAGE	0x100

/* Cursor types */
#define CURSOR_OFF 0
#define CURSOR_DEFAULT_ON 1
#define CURSOR_BLOCK 4
#define CURSOR_UNDER 5

/* Type of characters currently stored in CGRAM. */
typedef enum {
	standard,		/* one char is used for heartbeat */
	vbar,			/* vertical bars */
	hbar,			/* horizontal bars */
	icons,			/* standard icons */
	custom,			/* custom settings */
	bignum,			/* big numbers */
} CGmode;

/* What does the shared module handle look like on the current platform? */
#define MODULE_HANDLE void*

/* And how do we define the exported functions */
#define MODULE_EXPORT

typedef struct lcd_logical_driver {

	/* For explanation of variables and functions see docs/API-v0.5.txt */

	/******** Variables in the driver module ********/
	/* The driver loader will look for symbols with these names ! */

	char **api_version;
	int *stay_in_foreground;	/* Does this driver require to be in foreground ?   */
	int *supports_multiple;		/* Does this driver support multiple instances ?    */
	char **symbol_prefix;		/* What should alternatively be prepended to the function names ? */


	/******** Functions in the driver module ********/
	/* The driver loader will look for symbols with these names ! */

	/* mandatory functions (necessary for all drivers) */
	int (*init)		(struct lcd_logical_driver *drvthis);
	void (*close)		(struct lcd_logical_driver *drvthis);

	/* essential output functions (necessary for output drivers) */
	int (*width)		(struct lcd_logical_driver *drvthis);
	int (*height)		(struct lcd_logical_driver *drvthis);
	void (*clear)		(struct lcd_logical_driver *drvthis);
	void (*flush)		(struct lcd_logical_driver *drvthis);
	void (*string)		(struct lcd_logical_driver *drvthis, int x, int y, const char *str);
	void (*chr)		(struct lcd_logical_driver *drvthis, int x, int y, char c);

	/* essential input functions (necessary for all input drivers) */
	const char *(*get_key)	(struct lcd_logical_driver *drvthis);

	/* extended output functions (optional; core provides alternatives) */
	void (*vbar)		(struct lcd_logical_driver *drvthis, int x, int y, int len, int promille, int pattern);
	void (*hbar)		(struct lcd_logical_driver *drvthis, int x, int y, int len, int promille, int pattern);
	void (*num)		(struct lcd_logical_driver *drvthis, int x, int num);
	void (*heartbeat)	(struct lcd_logical_driver *drvthis, int state);
	int (*icon)		(struct lcd_logical_driver *drvthis, int x, int y, int icon);
	void (*cursor)		(struct lcd_logical_driver *drvthis, int x, int y, int type);

	/* user-defined character functions, are those still supported ? */
	void (*set_char)	(struct lcd_logical_driver *drvthis, int n, unsigned char *dat);
	int (*get_free_chars)	(struct lcd_logical_driver *drvthis);
	int (*cellwidth)	(struct lcd_logical_driver *drvthis);
	int (*cellheight)	(struct lcd_logical_driver *drvthis);

	/* Hardware functions */
	int (*get_contrast)	(struct lcd_logical_driver *drvthis);
	void (*set_contrast)	(struct lcd_logical_driver *drvthis, int promille);
	int (*get_brightness)	(struct lcd_logical_driver *drvthis, int state);
	void (*set_brightness)	(struct lcd_logical_driver *drvthis, int state, int promille);
	void (*backlight)	(struct lcd_logical_driver *drvthis, int on);
	void (*output)		(struct lcd_logical_driver *drvthis, int state);

	/* informational functions */
	const char * (*get_info) (struct lcd_logical_driver *drvthis);


	/******** Variables in server core available for drivers ********/

	char *name;		/* Name of this driver */
	char *filename;		/* Filename of the shared module */

	MODULE_HANDLE module_handle;	/* The handle of the loaded shared module
					   Is platform specific */

	void *private_data;	/* Filled by server by calling store_private_ptr()
				   Driver should cast this to it's own
				   private structure pointer */


	/******** Functions in server core available for drivers ********/

	int (*store_private_ptr) (struct lcd_logical_driver * driver, void * private_data);
	/* Store the driver's private data */

	/* Configfile functions */
	/* See configfile.h for descriptions and usage. */

	short (*config_get_bool)	(const char *sectionname, const char *keyname, int skip, short default_value);
	long int (*config_get_int)	(const char *sectionname, const char *keyname, int skip, long int default_value);
	double (*config_get_float)	(const char *sectionname, const char *keyname, int skip, double default_value);
	const char *( *config_get_string)(const char *sectionname, const char *keyname, int skip, const char *default_value);
	int (*config_has_section)	(const char *sectionname);
	int (*config_has_key)		(const char *sectionname, const char *keyname);

	/* Reporting function */
	/* Easily usable by including drivers/report.h */
	void (*report)			(const int level, const char *format, .../*args*/ );

	/* Display properties functions (for drivers that adapt to other loaded drivers) */
	int (*request_display_width) ();
	int (*request_display_height) ();

} Driver;

#endif
