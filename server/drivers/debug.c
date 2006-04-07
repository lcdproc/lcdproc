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

MODULE_EXPORT int
debug_init (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);

	framebuf = malloc(width * height);
	if (framebuf == NULL) {
		report(RPT_INFO, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}	

	debug_clear(drvthis);

	return 0;
}

/////////////////////////////////////////////////////////////////
// Closes the driver
//
MODULE_EXPORT void
debug_close (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);

	if (framebuf != NULL)
		free(framebuf);
	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
debug_width (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
	
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
debug_height (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
	
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
debug_clear (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);

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

	report(RPT_INFO, "%s()", __FUNCTION__);

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	//report(RPT_DEBUG, "+%s+", out);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			out[j] = framebuf[j + (i * width)];
		}
		out[width] = 0;
		//report(RPT_DEBUG, "|%s|", out);

	}

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	//report(RPT_DEBUG, "+%s+", out);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
debug_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	report(RPT_INFO, "%s(%i,%i,%.40s)", __FUNCTION__, x, y, string);

	y --; x --;  // Convert 1-based coords to 0-based...

	if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
		return;
		
	for (i = 0; string[i] != '\0'; i++) {
		if (x + i >= width)
			break;
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
	report(RPT_DEBUG, "%s(%i,%i,%c)", __FUNCTION__, x, y, c);

	x--; y--;
	
	if ((x >= 0) && (y >= 0) && (x < width) && (y < height))
		framebuf[(y * width) + x] = c;
}

MODULE_EXPORT void
debug_set_contrast (Driver *drvthis, int promille)
{
	report(RPT_INFO, "%s(%i)", __FUNCTION__, promille);
}

MODULE_EXPORT void
debug_backlight (Driver *drvthis, int on)
{
	report(RPT_INFO, "%s(%i)", __FUNCTION__, on);
}

MODULE_EXPORT void
debug_init_vbar (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
}

MODULE_EXPORT void
debug_init_hbar (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
}

MODULE_EXPORT void
debug_init_num (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
}

MODULE_EXPORT void
debug_num (Driver *drvthis, int x, int num)
{
	report(RPT_INFO, "%s(%i,%i)", __FUNCTION__, x, num);
}

MODULE_EXPORT void
debug_set_char (Driver *drvthis, int n, char *dat)
{
	report(RPT_INFO, "%s(%i,data)", __FUNCTION__, n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
MODULE_EXPORT void
debug_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	report(RPT_INFO, "%s(%i,%i,%i,%i,%i)", __FUNCTION__, x, y, len, promille, options);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			debug_chr(drvthis, x, y-pos, '|');
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

	report(RPT_INFO, "%s(%i,%i,%i,%i,%i)", __FUNCTION__, x, y, len, promille, options);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			debug_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT int
debug_icon (Driver *drvthis, int x, int y, int icon)
{
	report(RPT_INFO, "%s(%i,%i,%i)", __FUNCTION__, x, y, icon);

	return -1;	/* let the core do all the icon stuff */
}

/////////////////////////////////////////////////////////////////
// Return a keypress
//
MODULE_EXPORT const char *
debug_get_key (Driver *drvthis)
{
	report(RPT_INFO, "%s()", __FUNCTION__);
	return NULL;
}

