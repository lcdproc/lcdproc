#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "debug.h"
#include "report.h"


// Variables
static char *framebuf = NULL;
static int width = LCD_DEFAULT_WIDTH;
static int height = LCD_DEFAULT_HEIGHT;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "debug_";



//////////////////////////////////////////////////////////////////////////
////////////////////// For Debugging Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO: somehow allow access to the driver->framebuffer to each
// function...

int
debug_init (Driver *drvthis, char *args)
{
	report (RPT_INFO, "debug_init()");

	framebuf = malloc (width * height);

	debug_clear (drvthis);

	// Set variables for server
	drvthis->api_version = api_version;
	drvthis->stay_in_foreground = &stay_in_foreground;
	drvthis->supports_multiple = &supports_multiple;

	// Set the functions the driver supports
	drvthis->clear = debug_clear;
	drvthis->string = debug_string;
	drvthis->chr = debug_chr;
	drvthis->vbar = debug_vbar;
	drvthis->hbar = debug_hbar;
	drvthis->init_num = debug_init_num;
	drvthis->num = debug_num;

	drvthis->init = debug_init;
	drvthis->close = debug_close;
	drvthis->width = debug_width;
	drvthis->height = debug_height;
	drvthis->flush = debug_flush;
	drvthis->set_contrast = debug_set_contrast;
	drvthis->backlight = debug_backlight;
	drvthis->set_char = debug_set_char;
	drvthis->icon = debug_icon;
	drvthis->init_vbar = debug_init_vbar;
	drvthis->init_hbar = debug_init_hbar;

	drvthis->get_key = debug_get_key;

	return 0;
}

/////////////////////////////////////////////////////////////////
// Closes the driver
//
MODULE_EXPORT void
debug_close (Driver *drvthis)
{
	report (RPT_INFO, "debug_close()");

	if(framebuf) free (framebuf);
	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
debug_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
debug_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
debug_clear (Driver *drvthis)
{
	report (RPT_INFO, "clear()");

	memset (framebuf, ' ', width * height);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
debug_flush (Driver *drvthis)
{
	int i, j;
	char out[LCD_MAX_WIDTH];

	report (RPT_INFO, "flush()");

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	//report (RPT_DEBUG, "+%s+", out);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			out[j] = framebuf[j + (i * width)];
		}
		out[width] = 0;
		//report (RPT_DEBUG, "|%s|", out);

	}

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	//report (RPT_DEBUG, "+%s+", out);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
debug_string (Driver *drvthis, int x, int y, char string[])
{

	int i;

	report(RPT_INFO, "string(%i,%i,%.40s)", x, y, string);

	y --; x --;  // Convert 1-based coords to 0-based...

	for (i = 0; string[i]; i++) {
		framebuf[(y * width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
debug_chr (Driver *drvthis, int x, int y, char c)
{
	report (RPT_DEBUG, "char(%i,%i,%c)", x, y, c);

	x--; y--;
	framebuf[(y * width) + x] = c;
}

MODULE_EXPORT void
debug_set_contrast (Driver *drvthis, int promille)
{
	report (RPT_INFO, "set_contrast(%i)", promille);
}

MODULE_EXPORT void
debug_backlight (Driver *drvthis, int on)
{
	report (RPT_INFO, "backlight(%i)", on);
}

MODULE_EXPORT void
debug_init_vbar (Driver *drvthis)
{
	report (RPT_INFO, "init_vbar()");
}

MODULE_EXPORT void
debug_init_hbar (Driver *drvthis)
{
	report (RPT_INFO, "init_hbar()");
}

MODULE_EXPORT void
debug_init_num (Driver *drvthis)
{
	report (RPT_INFO, "init_bignum()");
}

MODULE_EXPORT void
debug_num (Driver *drvthis, int x, int num)
{
	report (RPT_INFO, "big number(%i,%i)", x, num);
}

MODULE_EXPORT void
debug_set_char (Driver *drvthis, int n, char *dat)
{
	report (RPT_INFO, "set_char(%i,data)", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
MODULE_EXPORT void
debug_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	report (RPT_INFO, "vbar(%i,%i,%i,%i,%i)", x, y, len, promille, options);

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			debug_chr (drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
debug_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	report (RPT_INFO, "hbar(%i,%i,%i,%i,%i)", x, y, len, promille, options);

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			debug_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT void
debug_icon (Driver *drvthis, int x, int y, int icon)
{
	report (RPT_INFO, "icon(%i,%i,%i)", x, y, icon);
}

/////////////////////////////////////////////////////////////////
// Return a keypress
//
MODULE_EXPORT char *
debug_get_key (Driver *drvthis)
{
	report (RPT_INFO, "get_key()");
	return NULL;
}
