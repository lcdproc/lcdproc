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
#include "shared/defines.h"
#include "timing.h"

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "glcd_";

static char *defaultKeyMap[GLCD_KEYPAD_MAX] = {"Up", "Down", "Left", "Right", "Enter", "Escape"};

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
	int w, h, tmp;

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
	if ((p->glcd_functions = (struct glcdHwFcns *) calloc(1, sizeof(struct glcdHwFcns))) == NULL) {
		report(RPT_ERR, "%s: error mallocing", drvthis->name);
		return -1;
	}
	p->glcd_functions->drv_report = report;
	p->glcd_functions->drv_debug = debug;
	p->glcd_functions->blit = NULL;
	p->glcd_functions->close = NULL;
	p->glcd_functions->set_contrast = NULL;
	p->glcd_functions->set_backlight = NULL;
	p->glcd_functions->output = NULL;
	p->glcd_functions->poll_keys = NULL;

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
	p->framebuf.px_width = w;
	p->framebuf.px_height = h;
	p->framebuf.layout = FB_TYPE_LINEAR;
	p->framebuf.bytesPerLine = (p->framebuf.px_width + 7) / 8;
	p->framebuf.size = p->framebuf.bytesPerLine * p->framebuf.px_height;
	debug(RPT_INFO, "%s: size (first) = %d", drvthis->name, p->framebuf.size);

	/* Set contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, GLCD_DEFAULT_CONTRAST);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast must be between 0 and 1000; using default %d",
			drvthis->name, GLCD_DEFAULT_CONTRAST);
		tmp = GLCD_DEFAULT_CONTRAST;
	}
	p->contrast = tmp;

	/* Set brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, GLCD_DEFAULT_BRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, GLCD_DEFAULT_BRIGHTNESS);
		tmp = GLCD_DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Set backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, GLCD_DEFAULT_OFFBRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, GLCD_DEFAULT_OFFBRIGHTNESS);
		tmp = GLCD_DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

	/* Invalidate cached values */
	p->last_output_state = -1;
	p->backlightstate = -1;

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
	 * Check these values AFTER driver initialization, as the driver may
	 * update them.
	 */
	if ((p->framebuf.px_width > GLCD_MAX_WIDTH) || (p->framebuf.px_height > GLCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s: Size %dx%d set by ConnectionType is not supported",
		       drvthis->name, p->framebuf.px_width, p->framebuf.px_height);
		return -1;
	}

	/* Allocate framebuffer (re-calculate size before) */
	if (p->framebuf.layout == FB_TYPE_LINEAR) {
		p->framebuf.bytesPerLine = (p->framebuf.px_width + 7) / 8;
		p->framebuf.size = p->framebuf.bytesPerLine * p->framebuf.px_height;
	}
	else {
		p->framebuf.bytesPerLine = 0;
		p->framebuf.size = (p->framebuf.px_height + 7) / 8 * p->framebuf.px_width;
	}
	debug(RPT_INFO, "%s: size (final) = %d", drvthis->name, p->framebuf.size);

	p->framebuf.data = malloc(p->framebuf.size);
	if (p->framebuf.data == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf.data, 0x00, p->framebuf.size);

	/* Initialize renderer */
	if (glcd_render_init(drvthis) != 0)
		return -1;

	/* Cellwidth / height are set by the renderer */
	p->width = p->framebuf.px_width / p->cellwidth;
	p->height = p->framebuf.px_height / p->cellheight;
	debug(RPT_INFO, "%s: Screen size (final) = %dx%d", drvthis->name, p->width, p->height);

	/* Initialize key map */
	for (i = 0; i < GLCD_KEYPAD_MAX; i++) {
		char buf[40];

		/* First fill with default value */
		p->keyMap[i] = defaultKeyMap[i];

		/* Read config value */
		sprintf(buf, "KeyMap_%c", i + 'A');
		s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

		/* Was a key specified in the config file ? */
		if (s) {
			p->keyMap[i] = strdup(s);
			debug(RPT_INFO, "%s: Key '%c' = \"%s\"", drvthis->name, i + 'A', s);
		}
	}

	/* Initialize delay */
	if ((p->key_wait_time = malloc(sizeof(struct timeval))) == NULL) {
		report(RPT_ERR, "%s: error allocating memory", drvthis->name);
		return -1;
	}
	timerclear(p->key_wait_time);

	/* Get key auto repeat delay */
	tmp = drvthis->config_get_int(drvthis->name, "KeyRepeatDelay", 0, GLCD_DEFAULT_REPEAT_DELAY);
	if (tmp < 0 || tmp > 3000) {
		report(RPT_WARNING, "%s: KeyRepeatDelay must be between 0-3000; using default %d",
			drvthis->name, GLCD_DEFAULT_REPEAT_DELAY);
		tmp = GLCD_DEFAULT_REPEAT_DELAY;
	}
	p->key_repeat_delay = tmp;

	/* Get key auto repeat interval */
	tmp = drvthis->config_get_int(drvthis->name, "KeyRepeatInterval", 0, GLCD_DEFAULT_REPEAT_INTERVAL);
	if (tmp < 0 || tmp > 3000) {
		report(RPT_WARNING, "%s: KeyRepeatInterval must be between 0-3000; using default %d",
			drvthis->name, GLCD_DEFAULT_REPEAT_INTERVAL);
		tmp = GLCD_DEFAULT_REPEAT_INTERVAL;
	}
	p->key_repeat_interval = tmp;

	debug(RPT_INFO, "%s: Key repeat: delay = %d, interval = %d",
			drvthis->name, p->key_repeat_delay, p->key_repeat_interval);

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
		if (p->framebuf.data != NULL)
			free(p->framebuf.data);
		p->framebuf.data = NULL;
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

	memset(p->framebuf.data, 0x00, p->framebuf.size);

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
			glcd_render_char_unicode(drvthis, x, y, string[i] & 0xFF, 1, 1);
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
		glcd_render_char_unicode(drvthis, x, y, c & 0xFF, 1, 1);
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
			fb_draw_pixel(&(p->framebuf), col, row, 1);
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
			fb_draw_pixel(&(p->framebuf), col, row, 1);
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


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
glcd_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%i,%i)", __FUNCTION__, x, num);

	if (x < 1 || x > p->width || num < 0 || num > 10)
		return;

#ifdef HAVE_FT2
	if (p->use_ft2) {
		int y;
		int sc;

		sc = min(p->height, 3);
		y = p->height - (p->height - sc)/2;

		if (num == 10)
			glcd_render_char_unicode(drvthis, x, y, ':', sc, 1);
		else
			glcd_render_char_unicode(drvthis, x, y, '0' + num, sc, sc);
		return;
	}
#endif
	glcd_render_bignum(drvthis, x, num);
}


/**
 * Get current LCD contrast. This is only the locally stored contrast.
 * \param drvthis  Pointer to driver structure.
 * \return         Stored contrast in promille.
 */
MODULE_EXPORT int
glcd_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	return p->contrast;
}


/**
 * Change LCD contrast.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
glcd_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%i)", __FUNCTION__, promille);

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* Cache the value */
	p->contrast = promille;

	/* call local function */
	if (p->glcd_functions->set_contrast != NULL)
		p->glcd_functions->set_contrast(p, promille);
}


/**
 * Retrieve brightness. This only reads the locally stored values.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return         Stored brightness in promille.
 */
MODULE_EXPORT int
glcd_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%i)", __FUNCTION__, state);

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Set on/off brightness.
 * \param drvthis   Pointer to driver structure.
 * \param state     Brightness state (on/off) for which we want to store the value.
 * \param promille  New brightness in promille.
 */
MODULE_EXPORT void
glcd_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%i,%i)", __FUNCTION__, state, promille);

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
	}
	else {
		p->offbrightness = promille;
	}

	/* Force update on next render cycle */
	p->backlightstate = -1;
}

/**
 * Turn the backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
glcd_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	debug(RPT_DEBUG, "%s(%i)", __FUNCTION__, on);

	/* Immediately return if no change is necessary */
	if (p->backlightstate == on)
		return;

	if (p->glcd_functions->set_backlight != NULL)
		p->glcd_functions->set_backlight(p, on);

	p->backlightstate = on;
}


/**
 * Send out-of-band data to the device.
 * \param drvthis  Pointer to driver structure.
 * \param value    Integer. Meaning is specific to the device
 */
MODULE_EXPORT void
glcd_output(Driver *drvthis, int value)
{
	PrivateData *p = drvthis->private_data;
	debug(RPT_DEBUG, "%s(%i)", __FUNCTION__, value);

	/* return is no change */
	if (p->last_output_state == value)
		return;

	p->last_output_state = value;

	/* call output function only if it is defined for the commenction type */
	if (p->glcd_functions->output != NULL)
		p->glcd_functions->output(p, value);
}


/**
 * Get key from the key panel connected to the display.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
glcd_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char scancode;
	char *keystr = NULL;
	struct timeval current_time, delay_time;

	/* return "no key pressed" if required functions missing */
	if (p->glcd_functions->poll_keys == NULL)
		return NULL;

	/*
	 * scancode is either '0' (no key pressed) or an index into the key
	 * string mapping table ('1-26').
	 */
	scancode = p->glcd_functions->poll_keys(p);
	if (scancode != '\0') {
		if (scancode > GLCD_KEYPAD_MAX)
			return NULL;
		keystr = p->keyMap[scancode - 1];
	}

	/*
	 * If a key has been pressed and it is not the same as in the previous
	 * call to this function return that key string and start a timer. If
	 * it is the same, check if the timer has passed. If not (or the timer
	 * has been disabled) return no key string. Otherwise set the repeat
	 * interval timer and return that key.
	 */
	if (keystr != NULL) {
		if (keystr == p->pressed_key) {
			if (timerisset(p->key_wait_time)) {
				gettimeofday(&current_time, NULL);
				if (timercmp(&current_time, p->key_wait_time, >)) {
					/* Set timer for next key */
					delay_time.tv_sec  = p->key_repeat_interval / 1000;
					delay_time.tv_usec = (p->key_repeat_interval % 1000) * 1000;
					timeradd(&current_time, &delay_time, p->key_wait_time);
				}
				else {
					return NULL;
				}
			}
			else {
				return NULL;
			}
		}
		else {
			/* Set the time for repeated key press if enabled */
			if (p->key_repeat_delay > 0) {
				gettimeofday(&current_time, NULL);
				delay_time.tv_sec  = p->key_repeat_interval / 1000;
				delay_time.tv_usec = (p->key_repeat_interval % 1000) * 1000;
				timeradd(&current_time, &delay_time, p->key_wait_time);
			}
			report(RPT_DEBUG, "%s: New key pressed: %s", drvthis->name, keystr);
		}
	}

	p->pressed_key = keystr;

	return keystr;
}
