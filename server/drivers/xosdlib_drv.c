/** \file server/drivers/xosdlib_drv.c
 * LCDd \c xosd driver for on screen display on X windows.
 */

/*
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Debug mode: un-comment to turn on debugging messages in the server */
/* #define DEBUG 1 */

#include "lcd.h"
#include "report.h"
#include "xosdlib_drv.h"
#include "adv_bignum.h"


static char icon_char = '@';

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "xosdlib_drv_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
xosdlib_drv_init (Driver *drvthis)
{
	const char *size;
	const char *offset;
	int x, y;
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

		size = drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE);
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

	/* Which x/y offsets */
	offset = drvthis->config_get_string(drvthis->name, "Offset", 0, DEFAULT_OFFSET);
	debug(RPT_INFO, "%s: Offset (in config) is '%s'", __FUNCTION__, offset);
	if (sscanf(offset, "%dx%d", &x, &y) != 2) {
		report(RPT_WARNING, "%s: cannot read Offset: %s. using default %s",
				drvthis->name, offset, DEFAULT_OFFSET);
		sscanf(DEFAULT_OFFSET, "%dx%d", &x, &y);
	}
	p->xoffs= x;
	p->yoffs = y;

	report(RPT_INFO, "%s: using offset %dx%d", drvthis->name, p->xoffs, p->yoffs);


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
	if (xosd_set_font(p->osd, p->font) != 0) {
		report(RPT_ERR, "%s: xosd_set_font() failed", drvthis->name);
		return -1;
	}

	/* set x/y offsets */
	if (xosd_set_horizontal_offset(p->osd, p->xoffs) != 0) {
		report(RPT_ERR, "%s: xosd_set_horizontal_offset() failed", drvthis->name);
		return -1;
	}
	if (xosd_set_vertical_offset(p->osd, p->yoffs) != 0) {
		report(RPT_ERR, "%s: xosd_set_vertical_offset() failed", drvthis->name);
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
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
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
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
xosdlib_drv_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
xosdlib_drv_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
xosdlib_drv_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
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
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
xosdlib_drv_string (Driver *drvthis, int x, int y, const char string[])
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
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
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
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
xosdlib_drv_get_free_chars (Driver *drvthis)
{
	//PrivateData *p = drvthis->private_data;

	  return 0;
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
xosdlib_drv_num(Driver *drvthis, int x, int num)
{
	//PrivateData *p = drvthis->private_data;
	int do_init = 1;

        if ((num < 0) || (num > 10))
                return;

        // Lib_adv_bignum does everything needed to show the bignumbers.
        lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
xosdlib_drv_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, options);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			xosdlib_drv_chr (drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
xosdlib_drv_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, options);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			xosdlib_drv_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Get current display contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the display.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille.
 */
MODULE_EXPORT int
xosdlib_drv_get_contrast (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

        return p->contrast;
}


/**
 * Change display contrast.
 * \note
 * This is currently not implemented.
 *
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
xosdlib_drv_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	int contrast;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is no way to get it from the display */
	p->contrast = promille;

	/* map range [0, 1000] to [0, 255] */
	contrast = (p->contrast * 255) / 1000;

	/* What to do with it ? */
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
xosdlib_drv_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
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
 * Turn the display backlight on or off.
 * \note
 * This is currently not implemented.
 *
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
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

