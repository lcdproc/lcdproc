/*  This is the LCDproc driver for xosdlib

      Copyright(C) 2005 Peter Marschall <peter@adpm.de>

   based on GPL'ed code:

   * misc. files from LCDproc source tree

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */


#include <unistd.h>
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

/* Debug mode: un-comment to turn on debugging messages in the server */
/* #define DEBUG 1 */

#include "lcd.h"
#include "report.h"
#include "xosdlib_drv.h"


static char icon_char = '@';

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "xosdlib_drv_";


/**
 * Init driver
 */
MODULE_EXPORT int
xosdlib_drv_init (Driver *drvthis)
{
	char size[LCD_MAX_WIDTH+1] = DEFAULT_SIZE;
	int tmp;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */

	p->contrast = DEFAULT_CONTRAST;
	p->brightness = DEFAULT_BRIGHTNESS;
	p->offbrightness = DEFAULT_OFFBRIGHTNESS;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	/* Read config file */

	/* Which size */
	if (drvthis->config_has_key(drvthis->name, "Size")) {
		int w;
		int h;
		
		strncpy(size, drvthis->config_get_string(drvthis->name, "Size",
							 0, DEFAULT_SIZE), sizeof(size));
		size[sizeof(size) - 1] = '\0';
		debug(RPT_INFO, "%s: Size (in config) is '%s'", __FUNCTION__, size);
		if ((sscanf(size, "%dx%d", &w, &h) != 2) ||
		    (w <= 0) || (w > LCD_MAX_WIDTH) ||
		    (h <= 0) || (h > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s. using default %s",
					drvthis->name, size, DEFAULT_SIZE);
			sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
		}
		p->width = w;
		p->height = h;
	}
	else {
		/* Determine the size of the screen */
		p->width = drvthis->request_display_width();
		p->height = drvthis->request_display_height();
		if ((p->width <= 0) || (p->width >= LCD_MAX_WIDTH) ||
		    (p->height <= 0) || (p->height >= LCD_MAX_HEIGHT)) {
			p->width = LCD_DEFAULT_WIDTH;
			p->height = LCD_DEFAULT_HEIGHT;
		}
	}
	report(RPT_INFO, "%s: using size %dx%d", drvthis->name, p->width, p->height);
		
	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_OFFBRIGHTNESS);
		tmp = DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

	/* which font */
	strncpy(p->font, drvthis->config_get_string(drvthis->name, "Font",
						    0, DEFAULT_FONT), sizeof(p->font));
	p->font[sizeof(p->font) - 1] = '\0';
	debug(RPT_INFO, "%s: Font (in config) is '%s'", __FUNCTION__, p->font);

	/* initialize xosdlib library */
	p->osd = xosd_create(p->height);
	if (p->osd == NULL) {
		report(RPT_ERR, "%s: xosd_create() failed", drvthis->name);
		return -1;
	}	

	/* set font */
	if (xosd_set_font(p->osd, p->font) == -1) {
		report(RPT_ERR, "%s: xosd_set_font() failed", drvthis->name);
		return -1;
	}	

        /* make sure the frame buffer is there... */
        p->framebuf = (unsigned char *) malloc(p->width * p->height);
        if (p->framebuf == NULL) {
                report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
                return -1;
        }
        memset(p->framebuf, ' ', p->width * p->height);

        /* make sure the framebuffer backing store is there... */
        p->backingstore = (unsigned char *) malloc(p->width * p->height);
        if (p->backingstore == NULL) {
                report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
                return -1;
        }
        memset(p->backingstore, ' ', p->width * p->height);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close down driver
 */
MODULE_EXPORT void
xosdlib_drv_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	if (p != NULL) {
		if (p->osd)
			xosd_destroy(p->osd);
		p->osd = NULL;
		
		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->backingstore != NULL)
			free(p->backingstore);
		p->backingstore = NULL;

		free(p);
	}	
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return width
 */
MODULE_EXPORT int
xosdlib_drv_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return height
 */
MODULE_EXPORT int
xosdlib_drv_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear screen
 */
MODULE_EXPORT void
xosdlib_drv_clear (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Flush framebuffer to screen
 */
MODULE_EXPORT void
xosdlib_drv_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;
	char buffer[LCD_MAX_WIDTH];

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	for (i = 0; i < p->height; i++) {
		memcpy(buffer, p->framebuf + (i * p->width), p->width);
		buffer[p->width] = '\0';
		
		debug(RPT_DEBUG, "xosd: flushed string \"%s\" at (%d,%d)", buffer, 0, i);
		xosd_display(p->osd, i, XOSD_string, buffer);
	}	
}


/**
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (p->width,p->height).
 */
MODULE_EXPORT void
xosdlib_drv_string (Driver *drvthis, int x, int y, char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;
	
	debug(RPT_DEBUG, "%s(%p, %d, %d, \"%s\")", __FUNCTION__, drvthis, x, y, string);

	x--;
	y--;

	for (i = 0; string[i] != '\0'; i++) {
		unsigned char c = (unsigned) string[i];

		if (c == 255)
			c = '#';
		p->framebuf[(y * p->width) + x++] = c;
	}	
}


/**
 * Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (p->width,p->height).
 */
MODULE_EXPORT void
xosdlib_drv_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%p, %d, %d, \'%c\')", __FUNCTION__, drvthis, x, y, c);

	x--;
	y--;

	switch ((unsigned) c) {
		case '\0':
			c = icon_char;
			break;
		case 255:
			c = '#';
			break;
	}
	p->framebuf[(y * p->width) + x] = c;
}


/**
 * Writes a big number, but not.  A bit like the curses driver.
 */
MODULE_EXPORT void
xosdlib_drv_old_num (Driver *drvthis, int x, int num)
{
	int y, dx;
	char c;

	debug(RPT_DEBUG, "%s(%p, %d, %d)", __FUNCTION__, drvthis, x, num);

	if ((num < 0) || (num > 10))
		return;

	c = (num >= 10) ? ':' : ('0' + num);

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			xosdlib_drv_chr(drvthis, x + dx, y, c);
}


/**
 * Draws a vertical bar; erases entire column onscreen.
 */
MODULE_EXPORT void
xosdlib_drv_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, pattern);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			xosdlib_drv_chr (drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
xosdlib_drv_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, pattern);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			xosdlib_drv_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Returns current contrast (in promille)
 * This is only the locally stored contrast.
 */
MODULE_EXPORT int
xosdlib_drv_get_contrast (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

        return p->contrast;
}


/**
 *  Changes screen contrast (in promille)
 */
MODULE_EXPORT void
xosdlib_drv_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	int contrast;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	p->contrast = promille;

	/* map range [0, 1000] to [0, 255] */
	contrast = (p->contrast * 255) / 1000;

	/* What to do with it ? */
}


/**
 * Retrieves brightness (in promille)
 */
MODULE_EXPORT int
xosdlib_drv_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Sets on/off brightness (in promille)
 */
MODULE_EXPORT void
xosdlib_drv_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is no get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
	}
	else {
		p->offbrightness = promille;
	}
}


/**
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 */
MODULE_EXPORT void
xosdlib_drv_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int value = (on == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	/* map range [0, 1000] -> [1, 255] */
	value = value * 255 / 1000;
	if (value <= 0)
		value = 1;

	/* set font color */
	//xosd_set_colour (p->osd, "#001122");
}

