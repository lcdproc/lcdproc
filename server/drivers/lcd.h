/*
 * lcd.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
 *
 * This file defines the LCDd-driver API
 * It is written to facilitate loadable driver modules.
 * There should be no further interaction between driver and server core
 * other that via this API.
 *
 * DO NOT MIX DRIVER ALLOCATED AND CORE ALLOCATED MEMORY.
 * With this I mean that the server core should NEVER WRITE in memory
 * allocated by the driver, and vice versa. Also the driver resp. core
 * should free or realloc the memory that it has allocated. You can always
 * simply copy a string if its data space is not 'yours'.
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
#define BACKLIGHT_WARNING 2
#define BACKLIGHT_RED_ALERT 3

// Icons for icon function
#define ICON_BLOCK_FILLED 0
#define ICON_HEART_OPEN 8
#define ICON_HEART_FILLED 9

/* Heartbeat data, taken from render.h */
/* ??? What do all these mean ? */
#define HEART_OFF 1
#define HEART_ON 2
#define HEART_OPEN 3
#define HEARTBEAT_OFF HEART_OFF
#define HEARTBEAT_ON HEART_ON
#define HEARTBEAT_OPEN HEART_OPEN

/* Patterns for hbar / vbar */
#define BAR_POS			0x001 /* default */
#define BAR_NEG			0x002
#define BAR_POS_AND_NEG		0x003
#define BAR_PATTERN_FILLED	0x000
#define BAR_PATTERN_OPEN	0x010
#define BAR_PATTERN_STRIPED	0x020
#define BAR_WITH_PERCENTAGE	0x100

/* Cursor types */
#define CURSOR_OFF 0
#define CURSOR_DEFAULT_ON 1
#define CURSOR_BLOCK 4
#define CURSOR_UNDER 5

/* What does the shared module handle look like on the current platform? */
#define MODULE_HANDLE void*

/* And how do we define the exported functions */
#define MODULE_EXPORT

typedef struct lcd_logical_driver {

	/* Ancient variables */

	/* For explanation of variables and functions see docs/API-v0.5.txt */

	/******** Variables in the driver module ********/
	/* The driver loader will look for symbols with these names ! */

	char **api_version;
	int *stay_in_foreground;	/* Does this driver require to be in foreground ?   */
	int *supports_multiple;		/* Does this driver support multiple instances ?    */
	char **symbol_prefix;		/* What should alternatively be prepended to the function names ? */


	/******** Functions in the driver module ********/
	/* The driver loader will look for symbols with these names ! */

	/* Basic functions */
	int (*init)		(struct lcd_logical_driver* drvthis, char *args);
	void (*close)		(struct lcd_logical_driver* drvthis);
	int (*width)		(struct lcd_logical_driver* drvthis);
	int (*height)		(struct lcd_logical_driver* drvthis);
	void (*clear)		(struct lcd_logical_driver* drvthis);
	void (*flush)		(struct lcd_logical_driver* drvthis);
	void (*string)		(struct lcd_logical_driver* drvthis, int x, int y, char *str);
	void (*chr)		(struct lcd_logical_driver* drvthis, int x, int y, char c);

	/* Extended functions */
	void (*vbar)		(struct lcd_logical_driver* drvthis, int x, int y, int len, int promille, int pattern);
	void (*hbar)		(struct lcd_logical_driver* drvthis, int x, int y, int len, int promille, int pattern);
	void (*num)		(struct lcd_logical_driver* drvthis, int x, int num);
	void (*heartbeat)	(struct lcd_logical_driver* drvthis, int state);
	void (*icon)		(struct lcd_logical_driver* drvthis, int x, int y, int icon);
	void (*cursor)		(struct lcd_logical_driver* drvthis, int x, int y, int state);

	/* Userdef characters, are those still supported ? */
	void (*set_char)	(struct lcd_logical_driver* drvthis, int n, char *dat);
	int (*get_free_chars)	(struct lcd_logical_driver* drvthis);
	int (*cellwidth)	(struct lcd_logical_driver* drvthis);
	int (*cellheight)	(struct lcd_logical_driver* drvthis);

	/* Hardware functions */
	int (*get_contrast)	(struct lcd_logical_driver* drvthis);
	void (*set_contrast)	(struct lcd_logical_driver* drvthis, int promille);
	int (*get_brightness)	(struct lcd_logical_driver* drvthis, int state);
	void (*set_brightness)	(struct lcd_logical_driver* drvthis, int state, int promille);
	void (*backlight)	(struct lcd_logical_driver* drvthis, int on);
	void (*output)		(struct lcd_logical_driver* drvthis, int state);

	/* Key functions */
	char *(*get_key)	(struct lcd_logical_driver* drvthis);
				/* Returns a string. Server cannot modify
				  this string. */

	char * (*get_info) ();

	/* Returns 0 for "no key pressed", or (A-Z). */
	char (*getkey) ();
	/* TO BE REMOVED, IS RENAMED AND CHANGED */


	/******** Variables in server core available for drivers ********/

	char * name;		/* Name of this driver */
	char * filename;	/* Filename of the shared module */

	MODULE_HANDLE module_handle;	/* The handle of the loaded shared module
					   Is platform specific */

	void * private_data;	/* Filled by server by calling store_private_ptr()
				   Driver should cast this to it's own
				   private structure pointer */


	/******** Functions in server core available for drivers ********/

	int (*store_private_ptr) (struct lcd_logical_driver * driver, void * private_data);
	/* Store the driver's private data */

	/* Configfile functions */
	/* See configfile.h for descriptions and usage. */

	unsigned char (*config_get_bool)( char *sectionname, char *keyname, int skip, unsigned char default_value );
	long int (*config_get_int)	( char *sectionname, char *keyname, int skip, long int default_value );
	double (*config_get_float)	( char *sectionname, char *keyname, int skip, double default_value );
	char *( *config_get_string)	( char *sectionname, char *keyname, int skip, char *default_value );
	int (*config_has_section)	( char *sectionname );
	int (*config_has_key)		( char *sectionname, char *keyname );

	/* Reporting function */
	/* Easily usable by including drivers/report.h */
	void (*report)			( const int level, const char *format, .../*args*/ );

	/* Display properties functions (for drivers that adapt to other loaded drivers) */
	int (*request_display_width) ();
	int (*request_display_height) ();

} Driver;

#endif
