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
typedef struct driver_private_data {
	int width;
	int height;
	char *framebuf;
} PrivateData;

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
	PrivateData *p;
	char buf[256];

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */

	// Set display sizes
	if ((drvthis->request_display_width() > 0)
	    && (drvthis->request_display_height() > 0)) {
		// Use size from primary driver
		p->width = drvthis->request_display_width();
		p->height = drvthis->request_display_height();
	}
	else {
		/* Use our own size from config file */
		strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, TEXTDRV_DEFAULT_SIZE), sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		if ((sscanf(buf , "%dx%d", &p->width, &p->height) != 2)
		    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
		    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
					drvthis->name, buf, TEXTDRV_DEFAULT_SIZE);
			sscanf(TEXTDRV_DEFAULT_SIZE, "%dx%d", &p->width, &p->height);
		}
	}

	// Allocate the framebuffer
	p->framebuf = malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}

/////////////////////////////////////////////////////////////////
// Closes the device
//
MODULE_EXPORT void
text_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->framebuf != NULL)
			free(p->framebuf);

		free(p);
	}	
	drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
text_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
text_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
text_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
text_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[LCD_MAX_WIDTH];
	int i;

	memset(out, '-', p->width);
	out[p->width] = '\0';
	printf("+%s+\n", out);

	for (i = 0; i < p->height; i++) {
		memcpy(out, p->framebuf + (i * p->width), p->width);
		out[p->width] = '\0';
		printf("|%s|\n", out);
	}

	memset(out, '-', p->width);
	out[p->width] = '\0';
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
	PrivateData *p = drvthis->private_data;
	int i;

	x--; y--; // Convert 1-based coords to 0-based...

	if ((y < 0) || (y >= p->height))
                return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)	// no write left of left border
			p->framebuf[(y * p->width) + x] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
text_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--; x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets the contrast
//
MODULE_EXPORT void
text_set_contrast (Driver *drvthis, int promille)
{
	//PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Contrast: %d", promille);
}

/////////////////////////////////////////////////////////////////
// Sets the backlight brightness
//
MODULE_EXPORT void
text_backlight (Driver *drvthis, int on)
{
	//PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Backlight %s", (on) ? "ON" : "OFF");
}

