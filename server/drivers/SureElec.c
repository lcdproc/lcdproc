/** \file server/drivers/SureElec.c
 *
 * Driver for the LCD modules (actually the controller board) available from
 * the 'SURE electronics' shop (http://www.sureelectronics.net).
 *
 * These devices are PIC based controlled, using a serial communication
 * protocol with the host. The actual connection to host is done through
 * USB through a serial <-> USB converter (CP2102 USB to UART Bridge)
 * integrated on the board.
 */

/*-
 * Copyright (C) 2009, Laurent Latil
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * It should be noted that the communication protocol, while somewhat
 * documented in the LCD user's manual, is not complete (ie. the
 * initialization sequence is not described), and inacurate on some points.
 * This driver is so based on the spying of serial communication using the
 * officially supported client softwares (LCD Smarties & SureLCD).
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "SureElec.h"
#include "adv_bignum.h"

#include "report.h"

/* API: variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "SureElec_";


typedef enum {
	SURE_ELEC_EDITION1 = 1,
	SURE_ELEC_EDITION2 = 2,
	SURE_ELEC_EDITION3 = 3,
}    SureElec_edition;

/** private data for the \c SureElec driver */
typedef struct SureElec_private_data {
	int fd;			/* Serial port file descriptor */

	int width, height;	/* Screen size */
	int cellwidth, cellheight;	/* Cell size */

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* definable characters */
	CGmode ccmode;

	int output_state;	/**< current output state */
	int contrast;		/**< current contrast */
	int brightness;
	int offbrightness;
	int backlight;

	SureElec_edition edition;

}      PrivateData;

static int open_port(Driver * drvthis, const char *device);
static void get_device_screen_size(Driver * drvthis, int *width, int *height);
static int write_(Driver * drvthis, const unsigned char *buf, size_t count);
static int read_(Driver * drvthis, void *buf, size_t count);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 *
 * \todo This function returns at several points without properly freeing
 *       file descriptors and allocated memory.
 */
MODULE_EXPORT int
SureElec_init(Driver * drvthis)
{
	char device[256] = DEFAULT_DEVICE;
	char size[256] = DEFAULT_SIZE;
	int tmp;
	int user_width = -1, user_height = -1;
	int device_width = -1, device_height = -1;
	const char *sedition;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialise the PrivateData structure */
	p->fd = -1;
	p->edition = SURE_ELEC_EDITION2;	/* Assume an edition 2
						 * version */

	p->cellwidth = CELL_WIDTH;
	p->cellheight = CELL_HEIGHT;

	p->framebuf = NULL;
	p->backingstore = NULL;

	p->backlight = 0;

	debug(RPT_INFO, "SureElec driver: init(%p)", drvthis);

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	if (open_port(drvthis, device) == -1) {
		return -1;
	}

	/* Get edition version */
	sedition = drvthis->config_get_string(drvthis->name, "Edition", 0, "");

	if (strlen(sedition) != 0) {
		int n = sedition[0] - '0';
		if (strlen(sedition) != 1 || n < 1 || n > 3) {
			report(RPT_ERR, "%s: unknown edition version %s; must be one of 1, 2, or 3",
			       drvthis->name, sedition);
			return -1;
		}
		p->edition = (SureElec_edition) n;
	}

	/* Get the user-specified display size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, ""), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if (strlen(size) != 0) {
		if ((sscanf(size, "%dx%d", &user_width, &user_height) != 2)
		    || (user_width <= 0) || (user_width > LCD_MAX_WIDTH)
		  || (user_height <= 0) || (user_height > LCD_MAX_HEIGHT)) {
			report(RPT_ERR, "%s: invalid screen size: %s",
			       drvthis->name, size);
			return -1;
		}
	}
	if (p->edition != SURE_ELEC_EDITION1) {
		/* For edition >1 the device can report screen size */
		get_device_screen_size(drvthis, &device_width, &device_height);

		if (user_width != -1) {
			if (user_width != device_width || user_height != device_height) {
				report(RPT_WARNING, "%s: device and config settings disagree for screen size: %d x %d  <>  %d x %d", drvthis->name, device_width, device_height, user_width, user_height);
				report(RPT_WARNING, "%s: config specified size applied.", drvthis->name);
			}
			device_width = user_width;
			device_height = user_height;
		}
	} else {
		device_width = user_width;
		device_height = user_height;
	}

	if (device_width == -1) {
		report(RPT_ERR, "%s: screen size must be specified",
		       drvthis->name);
		return -1;
	}
	/* set screen size */
	p->width = device_width;
	p->height = device_height;
	report(RPT_INFO, "%s: screen size %d x %d.", drvthis->name, p->width, p->height);

	/* Get contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	debug(RPT_INFO, "%s: Contrast (in config) is '%d'", drvthis->name, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast value must be between 0 and 1000; using default %d",
		       drvthis->name, DEFAULT_CONTRAST);
		tmp = DEFAULT_CONTRAST;
	}
	SureElec_set_contrast(drvthis, tmp);

	/* Get backlight */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", drvthis->name, tmp);
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

	/* Allocate the frame buffer. */
	p->framebuf = (unsigned char *)calloc(p->width * p->height, 1);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* Allocate the backing store. */
	p->backingstore = (unsigned char *)malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, '#', p->width * p->height);

	SureElec_clear(drvthis);
	SureElec_flush(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
SureElec_close(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		/* Free allocated framebuffers */
		if (p->framebuf)
			free(p->framebuf);
		if (p->backingstore)
			free(p->backingstore);

		/* Free our private data */
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
	debug(RPT_DEBUG, "SureElec: closed");
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
SureElec_width(Driver * drvthis)
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
SureElec_height(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
SureElec_cellwidth(Driver * drvthis)
{
	return CELL_WIDTH;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
SureElec_cellheight(Driver * drvthis)
{
	return CELL_HEIGHT;
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
SureElec_string(Driver * drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	/* Check coord. validity */
	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		p->framebuf[(y * p->width) + x] = string[i];
	}
	debug(RPT_DEBUG, "SureElec: printed string at (%d,%d)", x, y);
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
SureElec_clear(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	/* set hbar/vbar/bignum mode back to normal character display */
	p->ccmode = standard;

	/* replace all chars in framebuf with spaces */
	memset(p->framebuf, ' ', (p->width * p->height));

	debug(RPT_DEBUG, "SureElec: cleared screen");
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
SureElec_chr(Driver * drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;

	debug(RPT_DEBUG, "SureElec: writing character %02X to position (%d,%d)", c, x, y);
}

/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
SureElec_flush(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int modified = 0;
	int i, j;
	unsigned char cmd[4] = {'\xFE', '\x47', '\x01', /* line */ 0};

	for (i = 0; i < p->height; i++) {
		/* Check if line content changed */
		for (j = 0; j < p->width && p->framebuf[p->width * i + j] ==
		     p->backingstore[p->width * i + j]; j++) {
			;
		}

		if (j < p->width) {
			/*
			 * Line content has been changed, need to output the
			 * line on screen
			 */
			cmd[3] = i + 1;
			if (write_(drvthis, cmd, sizeof(cmd)) == -1 ||
			    write_(drvthis, &(p->framebuf[p->width * i]), p->width) == -1) {
				return;
			}
			modified = 1;
		}
	}

	if (modified) {
		/* If something changed on screen, update the backingstore */
		memcpy(p->backingstore, p->framebuf, p->width * p->height);
	}
	debug(RPT_DEBUG, "SureElec: frame buffer flushed");
}

/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return         Stored contrast in promille.
 */

MODULE_EXPORT int
SureElec_get_contrast(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->contrast;
}


/**
 * Set the LCD contrast.
 * \param drvthis   Pointer to driver structure.
 * \param promille  New contrast value in promille.
 */
MODULE_EXPORT void
SureElec_set_contrast(Driver * drvthis, int promille)
{
	unsigned char cmd[3] = {'\xFE', '\x50', 0};
	PrivateData *p = drvthis->private_data;
	int real_contrast;

	/* Check it */
	if ((promille < 0) || (promille > 1000))
		return;

	/* Store it */
	p->contrast = promille;

	real_contrast = (int)((long)promille * MAX_CONTRAST / 1000);

	/* And do it */
	cmd[2] = real_contrast;

	write_(drvthis, cmd, sizeof(cmd));
}

/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return         Stored brightness in promille.
 */
MODULE_EXPORT int
SureElec_get_brightness(Driver * drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	return (state == BACKLIGHT_ON) ? p->brightness : 0;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
 */
MODULE_EXPORT void
SureElec_set_brightness(Driver * drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check value */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value */
	if (state == BACKLIGHT_ON)
		p->brightness = promille;
	else
		p->offbrightness = promille;
}

/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
SureElec_backlight(Driver * drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int promille;

	if (BACKLIGHT_ON == on) {
		promille = p->brightness;
	} else {
		promille = p->offbrightness;
	}

	if (promille == 0 && p->backlight) {
		/* Turn BL off */
		unsigned char cmd[2] = {'\xFE', '\x46'};
		if (write_(drvthis, cmd, sizeof(cmd)) == -1) {
			return;
		}
		report(RPT_INFO, "SureElec: BL turned off");
		p->backlight = 0;
	}
	if (promille > 0) {
		unsigned char cmd_bl[3] = {'\xFE', '\x42', '\x0'};
		unsigned char cmd_brightness[3] = {'\xFE', '\x98', 0};
		int level;

		/* Turn BL on & set brightness level */
		if (!p->backlight) {
			if (write_(drvthis, cmd_bl, sizeof(cmd_bl)) == -1) {
				return;
			}
			report(RPT_INFO, "SureElec: BL turned on");
			p->backlight = 1;
		}
		level = (int)((long)promille * MAX_BACKLIGHT / 1000);
		cmd_brightness[2] = level;

		write_(drvthis, cmd_brightness, sizeof(cmd_brightness));
	}
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
SureElec_vbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
			       drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			/* add pixel line per pixel line */
			vBar[p->cellheight - i] = 0xFF;
			SureElec_set_char(drvthis, i, vBar);
		}
	}
	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
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
SureElec_hbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
			       drvthis->name);
			return;
		}
		p->ccmode = hbar;

		memset(hBar, 0x00, sizeof(hBar));

		for (i = 1; i <= p->cellwidth; i++) {
			/* fill pixel columns from left to right. */
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			SureElec_set_char(drvthis, i, hBar);
		}
	}
	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
SureElec_num(Driver * drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	if (p->ccmode != bignum) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
			       drvthis->name);
			return;
		}
		p->ccmode = bignum;
		do_init = 1;
	}
	/* Lib_adv_bignum does everything needed to show the bignumbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
SureElec_get_free_chars(Driver * drvthis)
{
	return NUM_CC;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8(=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
SureElec_set_char(Driver * drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char cmd[11] = {'\xFE', '\x4E', 0, 0, 0, 0, 0, 0, 0, 0, 0};

	unsigned char mask = (1 << CELL_WIDTH) - 1;
	int row;

	if ((unsigned int) n >= NUM_CC)
		return;
	if (!dat)
		return;

	cmd[2] = n;		/* Custom char to define. */

	for (row = 0; row < p->cellheight; row++) {
		cmd[row + 3] = dat[row] & mask;
	}
	write_(drvthis, cmd, sizeof(cmd));
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
SureElec_icon(Driver * drvthis, int x, int y, int icon)
{
	static unsigned char heart_open[] =
	{
		b__XXXXX,
		b__X_X_X,
		b_______,
		b_______,
		b_______,
		b__X___X,
		b__XX_XX,
		b__XXXXX
	};

	static unsigned char heart_filled[] =
	{
		b__XXXXX,
		b__X_X_X,
		b___X_X_,
		b___XXX_,
		b___XXX_,
		b__X_X_X,
		b__XX_XX,
		b__XXXXX
	};

	static unsigned char arrow_up[] =
	{
		b____X__,
		b___XXX_,
		b__X_X_X,
		b____X__,
		b____X__,
		b____X__,
		b____X__,
		b_______
	};

	static unsigned char arrow_down[] =
	{
		b____X__,
		b____X__,
		b____X__,
		b____X__,
		b__X_X_X,
		b___XXX_,
		b____X__,
		b_______
	};

	static unsigned char checkbox_off[] =
	{
		b_______,
		b_______,
		b__XXXXX,
		b__X___X,
		b__X___X,
		b__X___X,
		b__XXXXX,
		b_______
	};

	static unsigned char checkbox_on[] =
	{
		b____X__,
		b____X__,
		b__XXX_X,
		b__X_XX_,
		b__X_X_X,
		b__X___X,
		b__XXXXX,
		b_______
	};

	static unsigned char checkbox_gray[] =
	{
		b_______,
		b_______,
		b__XXXXX,
		b__X_X_X,
		b__XX_XX,
		b__X_X_X,
		b__XXXXX,
		b_______
	};

	unsigned char *pattern = NULL;
	char charnum = 0;

	switch (icon) {
		case ICON_BLOCK_FILLED:
			charnum = 0xFF;
			break;
		case ICON_ARROW_LEFT:
			charnum = 0x7F;
			break;
		case ICON_ARROW_RIGHT:
			charnum = 0x7E;
			break;
		case ICON_ARROW_UP:
			charnum = 1;
			pattern = arrow_up;
			break;
		case ICON_ARROW_DOWN:
			charnum = 2;
			pattern = arrow_down;
			break;
		case ICON_HEART_FILLED:
			pattern = heart_filled;
			break;
		case ICON_HEART_OPEN:
			pattern = heart_open;
			break;
		case ICON_CHECKBOX_OFF:
			charnum = 3;
			pattern = checkbox_off;
			break;
		case ICON_CHECKBOX_ON:
			charnum = 4;
			pattern = checkbox_on;
			break;
		case ICON_CHECKBOX_GRAY:
			charnum = 5;
			pattern = checkbox_gray;
			break;

		default:
			return -1;
	}

	if (NULL != pattern) {
		SureElec_set_char(drvthis, charnum, pattern);
	}
	SureElec_chr(drvthis, x, y, charnum);
	return 0;
}

/**
 * Open the communication port and sends the initialization 'magic' sequence
 * to the device
 * \param drvthis  Pointer to driver structure.
 * \param device   Name of the device to use.
 * \retval 0       Success
 * \retval <0      Error opening port or writing init sequence
 */
static int
open_port(Driver * drvthis, const char *device)
{
	PrivateData *p = drvthis->private_data;
	struct termios portset;

	unsigned char cmd[3] = {'\xFE', '\x56', 0};
	unsigned char init_seq[7] = {'\x54', '\x58', '\x4B', '\x52', '\x44', '\x41', '\x60'};
	int i;

	/* Set up io port correctly, and open it... */
	p->fd = open(device, O_RDWR | O_NOCTTY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, device, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: %s device could not be opened", drvthis->name, device);
		return -1;
	}
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

	tcgetattr(p->fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
#else
	/* The hard way */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
			      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	/* Set timeouts */
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;

	/* Set port speed */
	cfsetospeed(&portset, B9600);
	cfsetispeed(&portset, B0);

	/* Do it... */
	if (tcsetattr(p->fd, TCSANOW, &portset) == -1) {
		report(RPT_ERR, "%s: failed to configure port (%s)", drvthis->name, strerror(errno));
		return -1;
	}

	/* Send initialization sequence */
	for (i = 1; i <= 8; i++) {
		cmd[2] = i;
		if (write_(drvthis, cmd, sizeof(cmd)) == -1)
			return -1;
	}

	for (i = 0; i < sizeof(init_seq); i++) {
		if (write_(drvthis, (const unsigned char *)"\x0FE", 1) == -1
		    || write_(drvthis, &(init_seq[i]), 1) == -1)
			return -1;
	}
	return 0;
}

/**
 * Retrieves the display size from the device.
 * \param drvthis  Pointer to driver structure.
 * \param width    Pointer to variable storing display width
 * \param height   Pointer to variable storing display height
 */
static void
get_device_screen_size(Driver * drvthis, int *width, int *height)
{
	unsigned char cmd[2] = {'\xFE', '\x76'};
	char result[11];

	if (write_(drvthis, cmd, sizeof(cmd)) == -1)
		return;

	if (read_(drvthis, result, sizeof(result)) != -1) {
		*width = (int)(result[0] - '0') * 10 + (int)(result[1] - '0');
		*height = (int)(result[2] - '0') * 10 + (int)(result[3] - '0');
	}
}

/**
 * Writes count bytes from a buffer to the device.
 * \param drvthis  Pointer to driver structure
 * \param buf      Pointer to the buffer
 * \param count    Number of bytes to write
 * \retval         -1 upon error
 */
static int
write_(Driver * drvthis, const unsigned char *buf, size_t count)
{
	int write_count = 0, written;
	PrivateData *p = drvthis->private_data;

	while (write_count < count) {
		written = write(p->fd, &buf[write_count], count - write_count);
		if (written == -1) {
			report(RPT_ERR, "SureElec: cannot write to port");
			return -1;
		}
		write_count += written;
	}
	return write_count;
}

/**
 * Reads bytes in a buffer from the device.
 * \param drvthis  Pointer to driver structure
 * \param buf      Pointer to the buffer
 * \param count    Number of bytes to read
 * \retval         -1 upon error
 */
static int
read_(Driver * drvthis, void *buf, size_t count)
{
	fd_set rfds;
	struct timeval tv;
	PrivateData *p = drvthis->private_data;
	int read_count;

	/* Watch fd to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(p->fd, &rfds);

	read_count = 0;
	while (read_count < count) {
		int retval;

		/*
		 * For compatibility, better to reset the timeout delay (may
		 * be modified in select() call).
		 */
		tv.tv_sec = 1;	/* seconds */
		tv.tv_usec = 0;	/* microseconds */

		retval = select(p->fd + 1, &rfds, NULL, NULL, &tv);
		if (retval) {
			int read_result = read(p->fd, ((char *)buf) + read_count, count - read_count);
			if (read_result < 0) {
				return -1;
			} else {
				read_count += read_result;
			}
		} else {
			report(RPT_ERR, "SureElec: No answer from device");
			return -1;
		}
	}
	return read_count;
}
