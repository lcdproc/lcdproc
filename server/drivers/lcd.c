/*
 * Driver master (lcd.c)
 *
 * This code actually sets up the drivers as requested,
 * initializes drivers, and allocates memory for drivers.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/LL.h"
#include "shared/debug.h"

#include "lcd.h"

static int lcd_drv_init (lcd_logical_driver * driver, char *args);
static void lcd_drv_close ();
static void lcd_drv_clear ();
static void lcd_drv_flush ();
static void lcd_drv_string (int x, int y, char *string);
static void lcd_drv_chr (int x, int y, char c);
static int lcd_drv_contrast (int contrast);
static void lcd_drv_backlight (int on);
static void lcd_drv_output (int on);
static void lcd_drv_init_vbar ();
static void lcd_drv_init_hbar ();
static void lcd_drv_init_num ();
static void lcd_drv_num (int x, int num);
static void lcd_drv_set_char (int n, char *dat);
static void lcd_drv_vbar (int x, int len);
static void lcd_drv_hbar (int x, int y, int len);
static void lcd_drv_icon (int which, char dest);
static void lcd_drv_flush_box (int lft, int top, int rgt, int bot);
static void lcd_drv_draw_frame ();
static char lcd_drv_getkey ();
static char *lcd_drv_getinfo ();

#ifdef MTXORB_DRV
#include "MtxOrb.h"
#endif

#ifdef CFONTZ_DRV
#include "CFontz.h"
#endif

#ifdef LB216_DRV
#include "lb216.h"
#endif

#ifdef TEXT_DRV
#include "text.h"
#endif

#ifdef DEBUG_DRV
#include "debug.h"
#endif

#ifdef CURSES_DRV
#include "curses_drv.h"
#endif

#ifdef HD44780_DRV
#include "hd44780.h"
#endif

#ifdef SLI_DRV
#include "wirz-sli.h"
#endif

#ifdef JOY_DRV
#include "joy.h"
#endif

#ifdef IRMANIN_DRV
#include "irmanin.h"
#endif

#ifdef LIRCIN_DRV
#include "lircin.h"
#endif

#ifdef BAYRAD_DRV
#include "bayrad.h"
#endif

#ifdef SED1520_DRV
#include "sed1520.h"
#endif

#ifdef STV5730_DRV
#include "stv5730.h"
#endif

#ifdef GLK_DRV
#include "glk.h"
#endif

// Make program more readable and understandable;
// hide details...
#define ResetList(a)		LL_Rewind(a)
#define GetDriverData(a)	((lcd_logical_driver *)LL_Get(a))
#define NextDriver(a)		(LL_Next(a))
#define MoreDrivers(a)		(LL_Next(a) == 0)
#define DriverPresent(a)	(a)
#define FunctionPresent(a)	((a) != 0)

// TODO: Make a Windows server, and clients...?

lcd_logical_driver lcd;

// TODO: Add multiple names for the same driver?
//
// Would allow the use of "MatrixOrbital" (for example) or
// "Joystick" or "CrystalFontz" or similar more memorable
// names, as well as older ones...
//
lcd_physical_driver drivers[] = {

#ifdef MTXORB_DRV
	{"MtxOrb", MtxOrb_init,},
#endif
#ifdef CFONTZ_DRV
	{"CFontz", CFontz_init,},
#endif
#ifdef HD44780_DRV
	{"HD44780", HD44780_init,},
#endif
#ifdef SLI_DRV
	{"sli", sli_init,},
#endif
#ifdef LB216_DRV
	{"LB216", LB216_init,},
#endif
#ifdef TEXT_DRV
	{"text", text_init,},
#endif
#ifdef DEBUG_DRV
	{"debug", debug_init,},
#endif
#ifdef CURSES_DRV
	{"curses", curses_drv_init,},
#endif
#ifdef JOY_DRV
	{"joy", joy_init,},
#endif
#ifdef IRMANIN_DRV
	{"irmanin", irmanin_init,},
#endif
#ifdef LIRCIN_DRV
	{"lircin", lircin_init,},
#endif
#ifdef BAYRAD_DRV
	{"BayRAD", bayrad_init,},
#endif
#ifdef GLK_DRV
	{"glk", glk_init,},
#endif
#ifdef SED1520_DRV
	{"sed1520", sed1520_init,},
#endif
#ifdef STV5730_DRV
	{"stv5730", stv5730_init,},
#endif
	{NULL, NULL,},

};

LL *list;

////////////////////////////////////////////////////////////
// This function initializes a few basics as well as the
// "base" array.  To initialize a specific driver, use the
// lcd_add_driver() function.
// 
// This was eliminated; everything
// done here is to be replaced by the use of lcd_add_driver()...
int
lcd_init (char *args)
{
	return 0;
}

// This sets up all of the "wrapper" driver functions
// which call all of the drivers in turn.
//
static int
lcd_drv_init (struct lcd_logical_driver *driver, char *args)
{
	driver->wid = LCD_STD_WIDTH;
	driver->hgt = LCD_STD_HEIGHT;

	driver->cellwid = LCD_STD_CELL_WIDTH;
	driver->cellhgt = LCD_STD_CELL_HEIGHT;

	driver->framebuf = NULL;

	// Set up these wrapper functions...
	driver->clear = lcd_drv_clear;
	driver->string = lcd_drv_string;
	driver->chr = lcd_drv_chr;
	driver->vbar = lcd_drv_vbar;
	driver->hbar = lcd_drv_hbar;
	driver->init_num = lcd_drv_init_num;
	driver->num = lcd_drv_num;

	driver->init = lcd_drv_init;
	driver->close = lcd_drv_close;
	driver->flush = lcd_drv_flush;
	driver->flush_box = lcd_drv_flush_box;
	driver->contrast = lcd_drv_contrast;
	driver->backlight = lcd_drv_backlight;
	driver->output = lcd_drv_output;
	driver->set_char = lcd_drv_set_char;
	driver->icon = lcd_drv_icon;
	driver->init_vbar = lcd_drv_init_vbar;
	driver->init_hbar = lcd_drv_init_hbar;
	driver->draw_frame = lcd_drv_draw_frame;

	driver->getkey = lcd_drv_getkey;
	driver->getinfo = lcd_drv_getinfo;

	return 1;						  // 1 is arbitrary.  (must be 1 or more)
}

#define ChkNull(a,b,c) if ((driver->a) == 0) { syslog(LOG_INFO, "warning: %s: null entries deprecated!", (c)); driver->a = (b); }
#define ChkBaseDrv(a,b,c) if ((driver->a) == (void *) -1) { syslog(LOG_ERR, "warning: %s: base driver has been REMOVED!", (c)); driver->a = (b); }

static int
lcd_drv_patch_init (struct lcd_logical_driver *driver)
{
	// These are to patch drivers that use "NULL" as a valid value...
	ChkNull(clear, lcd_drv_clear, "clear");
	ChkNull(string, lcd_drv_string, "string");
	ChkNull(chr, lcd_drv_chr, "chr");
	ChkNull(vbar, lcd_drv_vbar, "vbar");
	ChkNull(hbar, lcd_drv_hbar, "hbar");
	ChkNull(init_num, lcd_drv_init_num, "init_num");
	ChkNull(num, lcd_drv_num, "num");

	ChkNull(init, lcd_drv_init, "init");
	ChkNull(close, lcd_drv_close, "close");
	ChkNull(flush, lcd_drv_flush, "flush");
	ChkNull(flush_box, lcd_drv_flush_box, "flush_box");
	ChkNull(contrast, lcd_drv_contrast, "contrast");
	ChkNull(backlight, lcd_drv_backlight, "backlight");
	ChkNull(output, lcd_drv_output, "output");
	ChkNull(set_char, lcd_drv_set_char, "set_char");
	ChkNull(icon, lcd_drv_icon, "icon");
	ChkNull(init_vbar, lcd_drv_init_vbar, "init_vbar");
	ChkNull(init_hbar, lcd_drv_init_hbar, "init_hbar");
	ChkNull(draw_frame, lcd_drv_draw_frame, "draw_frame");

	// Now check for base driver entries...;
	ChkBaseDrv(getkey, lcd_drv_getkey, "getkey");
	ChkBaseDrv(getinfo, lcd_drv_getinfo, "getinfo");

	ChkBaseDrv(clear, lcd_drv_clear, "clear");
	ChkBaseDrv(string, lcd_drv_string, "string");
	ChkBaseDrv(chr, lcd_drv_chr, "chr");
	ChkBaseDrv(vbar, lcd_drv_vbar, "vbar");
	ChkBaseDrv(hbar, lcd_drv_hbar, "hbar");
	ChkBaseDrv(init_num, lcd_drv_init_num, "init_num");
	ChkBaseDrv(num, lcd_drv_num, "num");

	ChkBaseDrv(init, lcd_drv_init, "init");
	ChkBaseDrv(close, lcd_drv_close, "close");
	ChkBaseDrv(flush, lcd_drv_flush, "flush");
	ChkBaseDrv(flush_box, lcd_drv_flush_box, "flush_box");
	ChkBaseDrv(contrast, lcd_drv_contrast, "contrast");
	ChkBaseDrv(backlight, lcd_drv_backlight, "backlight");
	ChkBaseDrv(output, lcd_drv_output, "output");
	ChkBaseDrv(set_char, lcd_drv_set_char, "set_char");
	ChkBaseDrv(icon, lcd_drv_icon, "icon");
	ChkBaseDrv(init_vbar, lcd_drv_init_vbar, "init_vbar");
	ChkBaseDrv(init_hbar, lcd_drv_init_hbar, "init_hbar");
	ChkBaseDrv(draw_frame, lcd_drv_draw_frame, "draw_frame");

	ChkBaseDrv(getkey, lcd_drv_getkey, "getkey");
	ChkBaseDrv(getinfo, lcd_drv_getinfo, "getinfo");
}

/*
 * This function can be replaced later with something
 * that utilizes the results of dynamic library loading
 *
 */

static
void *
lcd_find_init (char *driver) {
	int i;

	for (i = 0; drivers[i].name; i++) {
		if (strcmp(driver, drivers[i].name) == 0) {
			return (*drivers[i].init);
		}
	}
	return NULL;
}

// TODO:  lcd_remove_driver()

// This initializes the specified driver and sends parameters to
// it.  This is the function which calls, for example,
// MtxOrb_init.  The specifics come from the drivers[] array.

int
lcd_add_driver (char *driver, char *args)
{
	int i;
	char buf[64];
	int (*init_driver) ();

	lcd_logical_driver *add;

	if ((init_driver = (void *) lcd_find_init(driver)) != NULL) {

		// This creates an instance of the lcd structure specific to the
		// driver... it is passed to the driver's init routine...

		//if ((add = malloc (sizeof (*add))) == NULL) {
		//	snprintf (buf, sizeof(buf), "couldn't allocate space for driver \"%s\"", driver);
		//	syslog (LOG_ERR, buf);
		//	return -1;
		//}

		snprintf(buf, sizeof(buf), "adding %s driver", driver);
		syslog(LOG_INFO, buf);

		add = &lcd;
		memset (add, 0, sizeof (add));

		// Default settings for the driver...
		lcd_drv_init(add, NULL);

		// Allocate space for a framebuffer...
		if ((add->framebuf = malloc (add->wid * add->hgt)) == NULL) {
			snprintf (buf, sizeof(buf), "couldn't allocate framebuffer for driver \"%s\"", driver);
			syslog (LOG_ERR, buf);
			// free (add);
			return -1;
		}
		memset (add->framebuf, ' ', (add->wid * add->hgt));

		i = init_driver (add, args);
		lcd_drv_patch_init (add);	// patches drivers that think NULL is okay...
		return i;
	} else {
		snprintf(buf, sizeof(buf), "invalid driver: %s", driver);
		syslog(LOG_ERR, buf);
	}
	return -1;
}

// TODO: Put lcd_shutdown in the shutdown function...
int
lcd_shutdown ()
{
	lcd_logical_driver *driver;

	lcd.close ();

	return 0;
}

//////////////////////////////////////////////////////////////////////
// All functions below here call their respective driver functions...
//
// The way it works is this:
//   It loops through the list of drivers, calling each one to keep
//      them all synchronized.  During this loop...
//   - First, set the framebuffer to point to the drivers' framebuffer.
//       (this ensures that whatever driver gets called will operate on
//        the correct buffer)
//   - Then, it checks to see what sort of call to make.
//       driver->func() > 0 means it should call the driver function.
//       driver->func() == NULL means it should not make a call.
//       driver->func() == -1 means it should call the generic driver.
//////////////////////////////////////////////////////////////////////

/*
 * The functions below are wrapper functions for the actual driver
 * functions.
 *
 */

/*
 * TODO: Convert these functions to either null functions (output)
 * or to use a new interface for a linked list (input).
 *
 */

// lcd_drv_close ()
// lcd_drv_clear ()
// lcd_drv_flush ()
// lcd_drv_string (int x, int y, char string[])
// lcd_drv_chr (int x, int y, char c)
// lcd_drv_contrast (int contrast)
// lcd_drv_backlight (int on)
// lcd_drv_output (int on)
// lcd_drv_init_vbar ()
// lcd_drv_init_hbar ()
// lcd_drv_init_num ()
// lcd_drv_num (int x, int num)
// lcd_drv_set_char (int n, char *dat)
// lcd_drv_vbar (int x, int len)
// lcd_drv_hbar (int x, int y, int len)
// lcd_drv_icon (int which, char dest)
// lcd_drv_flush_box (int lft, int top, int rgt, int bot)
// lcd_drv_draw_frame (char *dat)
// lcd_drv_getkey ()
// lcd_drv_getinfo ()

static void
lcd_drv_close ()
{
	;
}

static void
lcd_drv_clear ()
{
	;
}

static void
lcd_drv_flush ()
{
	;
}

static void
lcd_drv_string (int x, int y, char *string)
{
	;
}

static void
lcd_drv_chr (int x, int y, char c)
{
	;
}

static int
lcd_drv_contrast (int contrast)
{
	return -1;
}

static void
lcd_drv_backlight (int on)
{
	;
}

static void
lcd_drv_output (int on)
{
	;
}

static void
lcd_drv_init_vbar ()
{
	;
}

static void
lcd_drv_init_hbar ()
{
	;
}

static void
lcd_drv_init_num ()
{
	;
}

static void
lcd_drv_num (int x, int num)
{
	;
}

static void
lcd_drv_set_char (int n, char *dat)
{
	;
}

static void
lcd_drv_vbar (int x, int len)
{
	;
}

static void
lcd_drv_hbar (int x, int y, int len)
{
	;
}

static void
lcd_drv_icon (int which, char dest)
{
	;
}

static void
lcd_drv_flush_box (int lft, int top, int rgt, int bot)
{
	;
}

// TODO:  Check whether lcd.draw_frame() should really take a framebuffer
// TODO:   as an argument, or if it should always use lcd.framebuf
static void
lcd_drv_draw_frame (char *dat)
{
	;
}

static char *
lcd_drv_getinfo ()
{
	return NULL;
}

// Input functions: may come from multiple sources...

static char
lcd_drv_getkey ()
{
	return 0;
}

// char
// lcd_drv_getkey ()
// {
// 	lcd_logical_driver *driver;
// 
// 	char key;
// 
// 	LL_Rewind (list);
// 	do {
// 		driver = (lcd_logical_driver *) LL_Get (list);
// 		if (driver) {
// 			lcd.framebuf = driver->framebuf;
// 
// 			if ((int) driver->getkey > 0) {
// 				key = driver->getkey ();
// 				if (key)
// 					return key;
// 			} else if ((int) driver->getkey == -1) {
// 				key = drv_base->getkey ();
// 				if (key)
// 					return key;
// 			}
// 		}
// 	} while (LL_Next (list) == 0);
// 
// 	return 0;
// }

