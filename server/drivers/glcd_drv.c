/** \file server/drivers/glcd_drv.c
 * Unified driver for graphical displays.
 *
 * The \c glcd driver consists of a base driver module and sub-drivers. The
 * base driver implements the driver API and connects to sub-drivers called
 * ConnectionTypes (CT-drivers). This is very similar to the hd44780 driver.
 *
 * The base driver renders text received by the driver API into an internal
 * framebuffer and passes that on to CT-drivers on calls to \c glcd_flush().
 * The CT-driver is responsible to convert the framebuffer into a memory layout
 * understood by the display and to transmit the data to the display.
 *
 * The framebuffer is of linear type, storing a black and white image of the
 * screen. Each byte contains 8 pixels (1bpp).
 *
 * The base driver does not implement incremental updates. Instead the
 * CT-driver is responsible for this.
 *
 * Additionally the CT-driver must create a data structure holding any
 * required data (configuration data, runtime information) and store a pointer
 * to it in the base driver's PrivateData->ct_data field.
 *
 * The base driver consists of the following files:
 * \li  glcd_drv.c      This file
 * \li  glcd-drivers.h  CT-driver registry. Add a pointer to your CT-driver's
 *                      init() function here.
 * \li  glcd-low.h      Base driver's PrivateData and ConnectionType API.
 * \li  glcd-render.c   Render characters using FreeType 2 or standard font.
 * \li  glcd_font5x8.h  LCDproc's default fixed 5x8 font.
 */

/*-
 * Copyright (C) 2011, Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lcd.h"
#include "lcd_lib.h"
#include "glcd_drv.h"
#include "glcd-low.h"
#include "glcd-drivers.h"
#include "report.h"
#include "glcd-render.h"

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "glcd_";

/**
 * Initialize the driver. (Required)
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
glcd_init(Driver *drvthis)
{
	PrivateData *p;
	int i;
	int (*init_fn) (Driver *drvthis)= NULL;
	const char *s;
	char size[200];
	int w, h;

	report(RPT_DEBUG, "%s()", __FUNCTION__);

	/* Allocate and store private data */
	p = (PrivateData *)calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Get and search for the connection type */
	s = drvthis->config_get_string(drvthis->name, "ConnectionType", 0, "t6963");
	for (i = 0; (connectionMapping[i].name != NULL) &&
	     (strcasecmp(s, connectionMapping[i].name) != 0); i++);
	if (connectionMapping[i].name == NULL) {
		report(RPT_ERR, "%s: unknown ConnectionType: %s", drvthis->name, s);
		return -1;
	}
	else {
		/* save connection type's init function */
		init_fn = connectionMapping[i].init_fn;
		report(RPT_INFO, "%s: using ConnectionType: %s", drvthis->name, connectionMapping[i].name);
	}

	/*
	 * Set up low-level functions. These should be overwritten by the
	 * connection type's init function.
	 */
	if ((p->glcd_functions = (GLCD_functions *) calloc(1, sizeof(GLCD_functions))) == NULL) {
		report(RPT_ERR, "%s: error mallocing", drvthis->name);
		return -1;
	}
	p->glcd_functions->drv_report = report;
	p->glcd_functions->drv_debug = debug;
	p->glcd_functions->blit = NULL;
	p->glcd_functions->close = NULL;

	/* Read display size in pixels */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, GLCD_DEFAULT_SIZE), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > GLCD_MAX_WIDTH)
	    || (h <= 0) || (h > GLCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s, Using default %s",
		       drvthis->name, size, GLCD_DEFAULT_SIZE);
		sscanf(GLCD_DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->px_width = w;
	p->px_height = h;

	/* Do local (=connection type specific) display init */
	if (init_fn(drvthis) != 0)
		return -1;

	/* consistency check: fail if image transfer function was not defined */
	if (p->glcd_functions->blit == NULL) {
		report(RPT_ERR, "%s: incomplete functions for connection type",
		       drvthis->name);
		return -1;
	}

	/*
	 * Calculate these values AFTER driver initialization, as the driver
	 * may update them.
	 */
	if ((p->px_width > GLCD_MAX_WIDTH) || (p->px_height > GLCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s: Size %dx%d set by ConnectionType is not supported",
		       drvthis->name, p->px_width, p->px_height);
		return -1;
	}
	p->cellwidth = GLCD_DEFAULT_CELLWIDTH;
	p->cellheight = GLCD_DEFAULT_CELLHEIGHT;
	p->width = p->px_width / p->cellwidth;
	p->height = p->px_height / p->cellheight;

	/* Allocate framebuffer */
	p->framebuf = malloc(FB_BYTES_TOTAL);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, 0x00, FB_BYTES_TOTAL);

	/* Initialize renderer */
	if (glcd_render_init(drvthis) != 0)
		return -1;

	glcd_clear(drvthis);

	return 0;
}


/**
 * Close the driver (do necessary clean-up). (Required)
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glcd_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	if (p != NULL) {
		if (p->glcd_functions->close != NULL)
			p->glcd_functions->close(p);
		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;
		glcd_render_close(drvthis);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters. (Required for output)
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
glcd_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return p->width;
}


/**
 * Return the display height in characters. (Required for output)
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
glcd_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return p->height;
}


/**
 * Return the width of a character in pixels. (Optional)
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
glcd_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels. (Optional)
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
glcd_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return p->cellheight;
}


/**
 * Clear the screen. (Required for output)
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glcd_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	memset(p->framebuf, 0x00, FB_BYTES_TOTAL);

}


/**
 * Flush data on screen to the display. (Optional)
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glcd_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	p->glcd_functions->blit(p);

}


/**
 * Print a string on the screen at position (x,y). (Required for output)
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 *
 * \see glcd_flush
 */
MODULE_EXPORT void
glcd_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	debug(RPT_DEBUG, "%s(%i,%i,%.40s)", __FUNCTION__, x, y, string);

	if ((y < 1) || (y > p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x <= p->width); i++, x++) {
#ifdef HAVE_FT2
		if (p->use_ft2)
			glcd_render_char_unicode(drvthis, x, y, string[i] & 0xFF);
		else
#endif
			glcd_render_char(drvthis, x, y, string[i]);
	}
}


/**
 * Print a character on the screen at position (x,y). (Required for output)
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 *
 * \note Keep in mind that character 0x00 may be a valid character on your
 * display and it is used by the bignum library. Avoid using string handling
 * functions here!
 *
 * \see glcd_flush
 */
MODULE_EXPORT void
glcd_chr(Driver *drvthis, int x, int y, char c)
{
#ifdef HAVE_FT2
	PrivateData *p = drvthis->private_data;
#endif
	debug(RPT_DEBUG, "%s(%i,%i,%c)", __FUNCTION__, x, y, c);

#ifdef HAVE_FT2
	if (p->use_ft2)
		glcd_render_char_unicode(drvthis, x, y, c & 0xFF);
	else
#endif
		glcd_render_char(drvthis, x, y, c);
}


/**
 * API: Place an icon on the screen.
 */
MODULE_EXPORT int
glcd_icon(Driver *drvthis, int x, int y, int icon)
{
	debug(RPT_DEBUG, "%s(%i,%i,%i)", __FUNCTION__, x, y, icon);

	return (glcd_render_icon(drvthis, x, y, icon));
}

/**
 * API: Draws a vertical bar, from the bottom of the screen up.
 */
MODULE_EXPORT void
glcd_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int xstart, xend, ystart, yend;
	int col, row;

	debug(RPT_DEBUG, "%s(%i,%i,%i,%i,%i)", __FUNCTION__, x, y, len, promille, options);

	/* leave first column and top row empty */
	xstart = (x - 1) * p->cellwidth + 1;
	xend = xstart + p->cellwidth - 1;
	ystart = y * p->cellheight;
	yend = ystart - (((long) 2 * len * p->cellheight) * promille / 2000) + 1;

	for (col = xstart; col < xend; col++) {
		for (row = ystart; row > yend; row--) {
			fb_draw_pixel(p, col, row, 1);
		}
	}
}


/**
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
glcd_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int xstart, xend, ystart, yend;
	int col, row;

	debug(RPT_DEBUG, "%s(%i,%i,%i,%i,%i)", __FUNCTION__, x, y, len, promille, options);

	/* leave first column and top row empty */
	xstart = (x - 1) * p->cellwidth + 1;
	xend = xstart + (((long) 2 * len * p->cellwidth) * promille / 2000) - 1;
	ystart = (y - 1) * p->cellheight + 1;
	yend = ystart + p->cellheight - 1;

	for (row = ystart; row < yend; row++) {
		for (col = xstart; col < xend; col++) {
			fb_draw_pixel(p, col, row, 1);
		}
	}

}


/**
 * Provide some information about this driver. (Optional)
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
glcd_get_info(Driver *drvthis)
{
	static char *info_string = "Unified driver for graphical displays";

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return info_string;
}
