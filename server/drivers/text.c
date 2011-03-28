/** \file server/drivers/text.c
 * LCDd \c text driver for dump text mode terminals.
 * It displays the LCD screens, one below the other on the terminal,
 * and is this suitable for dump hard-copy terminals.
 */

/* Copyright (C) 1998-2004 The LCDproc Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "text.h"
#include "report.h"


/** private data for the \c text driver */
typedef struct text_private_data {
	int width;		/**< display width in characters */
	int height;		/**< display height in characters */
	char *framebuf;		/**< fram buffer */
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "text_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
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

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
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


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
text_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
text_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
text_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
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


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
text_string (Driver *drvthis, int x, int y, const char string[])
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


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
text_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--; x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}


/**
 * Change the display contrast.
 * Dumb text terminals do not support this, so we ignore it.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
text_set_contrast (Driver *drvthis, int promille)
{
	//PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Contrast: %d", promille);
}


/**
 * Turn the display backlight on or off.
 * Dumb text terminals do not support this, so we ignore it.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
text_backlight (Driver *drvthis, int on)
{
	//PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Backlight %s", (on) ? "ON" : "OFF");
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
text_get_info (Driver *drvthis)
{
	//PrivateData *p = drvthis->private_data;
        static char *info_string = "Text mode driver";

	return info_string;
}
