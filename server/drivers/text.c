/*
 * TextMode driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 * Copyright (C) 1998-2004 The LCDproc Team
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "lcd.h"
#include "text.h"
#include "report.h"
//#include "drv_base.h"


// Variables
static int width;
static int height;
static char * framebuf;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "text_";

//////////////////////////////////////////////////////////////////////////
////////////////////// For Text-Mode Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////


MODULE_EXPORT int
text_init (Driver *drvthis)
{
	char buf[256];

	// Set display sizes
	if ((drvthis->request_display_width() > 0)
	    && (drvthis->request_display_height() > 0)) {
		// Use size from primary driver
		width = drvthis->request_display_width();
		height = drvthis->request_display_height();
	}
	else {
		/* Use our own size from config file */
		strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, TEXTDRV_DEFAULT_SIZE), sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		if ((sscanf(buf , "%dx%d", &width, &height) != 2)
		    || (width <= 0) || (width > LCD_MAX_WIDTH)
		    || (height <= 0) || (height > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
					drvthis->name, buf, TEXTDRV_DEFAULT_SIZE);
			sscanf(TEXTDRV_DEFAULT_SIZE, "%dx%d", &width, &height);
		}
	}

	// Allocate the framebuffer
	framebuf = malloc(width * height);
	if (framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	
	memset(framebuf, ' ', width * height);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}

/////////////////////////////////////////////////////////////////
// Closes the device
//
MODULE_EXPORT void
text_close (Driver *drvthis)
{
	if (framebuf != NULL)
		free(framebuf);

	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
text_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
text_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
text_clear (Driver *drvthis)
{
	memset(framebuf, ' ', width * height);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
text_flush (Driver *drvthis)
{
	int i, j;

	char out[LCD_MAX_WIDTH];

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	printf("+%s+\n", out);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			out[j] = framebuf[j + (i * width)];
		}
		out[width] = 0;
		printf("|%s|\n", out);
	}

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	printf("+%s+\n", out);
        fflush(stdin);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
text_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	x--; y--; // Convert 1-based coords to 0-based...

	if ((y < 0) || (y >= height))
                return;

	for (i = 0; (string[i] != '\0') && (x < width); i++, x++) {
		if (x >= 0)	// no write left of left border
			framebuf[(y * width) + x] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
text_chr (Driver *drvthis, int x, int y, char c)
{
	y--; x--;

	if ((x >= 0) && (y >= 0) && (x < width) && (y < height))
		framebuf[(y * width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets the contrast
//
MODULE_EXPORT void
text_set_contrast (Driver *drvthis, int promille)
{
	debug(RPT_DEBUG, "Contrast: %d", promille);
}

/////////////////////////////////////////////////////////////////
// Sets the backlight brightness
//
MODULE_EXPORT void
text_backlight (Driver *drvthis, int on)
{
	//PrivateData * p = (PrivateData*) drvthis->private_data;

	debug(RPT_DEBUG, "Backlight %s", (on) ? "ON" : "OFF");
}

