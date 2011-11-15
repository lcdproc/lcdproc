/** \file server/drivers/CFontz.c
 * LCDd \c CFontz driver for the CFA632 & CFA634 displays by CrystalFontz, Inc.
 */

/*  This is the LCDproc driver for CrystalFontz devices (http://www.crystalfontz.com/)

    Copyright (C) 2001 ????

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

/* configfile support added by Rene Wagner (c) 2001 */
/* backlight support modified by Rene Wagner (c) 2001 */
/* block patch by Eddie Sheldrake (c) 2001 inserted by Rene Wagner */
/* big num patch by Luis Llorente (c) 2002 */
/* adaptions to other CFontz* drivers Peter Marschall (c) 2005/6 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "CFontz.h"
#include "report.h"
#include "lcd_lib.h"
#include "CFontz-charmap.h"
#include "adv_bignum.h"


/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */

/** private data for the \c CFontz driver */
typedef struct CFontz_private_data {
	char device[200];

	int fd;

	int model;
	int newfirmware;

	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* definable characters */
	CGmode ccmode;

	int contrast;
	int brightness;
	int offbrightness;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "CFontz_";


// Internal functions
static void CFontz_linewrap(Driver *drvthis, int on);
static void CFontz_autoscroll(Driver *drvthis, int on);
static void CFontz_hidecursor(Driver *drvthis);
static void CFontz_reboot(Driver *drvthis);
static void CFontz_cursor_goto(Driver *drvthis, int x, int y);
static void CFontz_raw_chr(Driver *drvthis, int x, int y, unsigned char c);


// TODO:  Get the frame buffers working right

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
CFontz_init(Driver *drvthis)
{
	struct termios portset;
	int tmp, w, h;
	int reboot = 0;
	int usb = 0;
	int speed = DEFAULT_SPEED;
	char size[200] = DEFAULT_SIZE;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */
	p->fd = -1;
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;
	p->ccmode = standard;

	debug(RPT_INFO, "CFontz: init(%p)", drvthis);

	/* Read config file */
	/* Which device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast must be between 0 and 1000; using default %d",
				drvthis->name, DEFAULT_CONTRAST);
		tmp = DEFAULT_CONTRAST;
	}
	p->contrast = tmp;

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
				drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
				drvthis->name, DEFAULT_OFFBRIGHTNESS);
		tmp = DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	if (tmp == 1200) speed = B1200;
	else if (tmp == 2400) speed = B2400;
	else if (tmp == 9600) speed = B9600;
	else if (tmp == 19200) speed = B19200;
	else if (tmp == 115200) speed = B115200;
	else {
		report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200; using default %d",
				drvthis->name, DEFAULT_SPEED);
		speed = DEFAULT_SPEED;
	}

	/* New firmware version? */
	p->newfirmware = drvthis->config_get_bool(drvthis->name, "NewFirmware", 0, 0);

	/* Reboot display? */
	reboot = drvthis->config_get_bool(drvthis->name, "Reboot", 0, 0);

	/* Am I USB or not? */
	usb = drvthis->config_get_bool(drvthis->name, "USB", 0, 0);

	/* Set up io port correctly, and open it... */
	debug(RPT_DEBUG, "CFontz: Opening device: %s", p->device);
	p->fd = open(p->device, (usb) ? (O_RDWR | O_NOCTTY) : (O_RDWR | O_NOCTTY | O_NDELAY));
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)",
				drvthis->name, p->device, strerror(errno));
		return -1;
	}

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
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	if (usb) {
		portset.c_cc[VMIN] = 1;
		portset.c_cc[VTIME] = 3;
	}

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);

	/* make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	// Set display-specific stuff..
	if (reboot) {
		report(RPT_INFO, "%s: rebooting LCD...", drvthis->name);
		CFontz_reboot(drvthis);
	}

#ifndef SEAMLESS_HBARS
	report(RPT_WARNING, "%s: driver built without SEAMLESS_HBARS. "
	       "Hbars will NOT display correctly!", drvthis->name);
#endif

	sleep (1);
	CFontz_hidecursor(drvthis);
	CFontz_linewrap(drvthis, 1);
	CFontz_autoscroll(drvthis, 0);
	//CFontz_backlight(drvthis, backlight_brightness);  // render.c variables should not be used in drivers !

	CFontz_set_contrast(drvthis, p->contrast);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

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
CFontz_width(Driver *drvthis)
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
CFontz_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
CFontz_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
CFontz_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	if (p->newfirmware) {
		unsigned char out[3 * LCD_MAX_WIDTH];

		for (i = 0; i < p->height; i++) {
			unsigned char *ptr = out;
			int j;

			/* move cursor to start of (i+1)'th line */
			CFontz_cursor_goto(drvthis, 1, i+1);

			for (j = 0; j < p->width; j++) {
				unsigned char c = p->framebuf[(i * p->width) + j];

				/* characters that need to be treated special */
				if ((c < 0x20) || ((c >= 0x80) && (c < 0x88))) {
					if (c < 0x08) {
						// custom chars are at position 0x80 - 0x87
						c += 0x80;
					}
					else {
						// send data directly to LCD
						*ptr++ = CFONTZ_Send_Data_Directly_To_LCD;
						*ptr++ = 0x01;
					}
				}
				*ptr++ = c;
			}
			write(p->fd, out, (ptr - out));
		}
	}
	else {
		// Custom characters start at 0x80, not at 0.
		for (i = 0; i < p->width * p->height; i++) {
			if (p->framebuf[i] < 32)
				p->framebuf[i] += 0x80;
		}

		for (i = 0; i < p->height; i++) {
			/* move cursor to start of (i+1)'th line */
			CFontz_cursor_goto(drvthis, 1, i+1);

			write(p->fd, p->framebuf + (p->width * i), p->width);
		}
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
CFontz_chr(Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = (p->newfirmware)
						  ? CFontz_charmap[(unsigned) c]
						  : c;
}


/**
 * Print a raw character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
static void
CFontz_raw_chr(Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}


/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille.
 */
MODULE_EXPORT int
CFontz_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change LCD contrast.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
CFontz_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Contrast_Control, 0 };

	// Check it
	if ((promille < 0) || (promille > 1000))
		return;

	// Store it
	p->contrast = promille;

	// And do it (converted from [0,1000] - > [0,100])
	out[1] = (unsigned char) (promille / 10);
	write(p->fd, out, 2);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
CFontz_get_brightness(Driver *drvthis, int state)
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
CFontz_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

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
	//CFontz_backlight(drvthis, state);
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
CFontz_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { CFONTZ_Backlight_Control, 0 };
	int promille = (on == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	/* map range [0, 1000] -> [0, 100] that the hardware understands */
	out[1] = (unsigned char) (promille / 10);
	write(p->fd, out, 2);
}


/**
 * Turn the LCD's built-in linewrapping feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
CFontz_linewrap(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	out[0] = (on) ? CFONTZ_Wrap_On : CFONTZ_Wrap_Off;
	write(p->fd, out, 1);
}


/**
 * Turn the LCD's built-in automatic scrolling feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
CFontz_autoscroll(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	out[0] = (on) ? CFONTZ_Scroll_On : CFONTZ_Scroll_Off;
	write(p->fd, out, 1);
}


/**
 * Get rid of the blinking cursor.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz_hidecursor(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Hide_Cursor };

	write(p->fd, out, 1);
}


/**
 * Reset the LCD display.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz_reboot(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Reboot, CFONTZ_Reboot };

	write(p->fd, out, 2);
	sleep(4);
}


/**
 * Move cursor to position (x,y).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 */
static void
CFontz_cursor_goto(Driver *drvthis, int x, int y)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { CFONTZ_Set_Cursor_Position, 0, 0 };

	/* set cursor position */
	if ((x > 0) && (x <= p->width))
		out[1] = (unsigned char) (x - 1);
	if ((y > 0) && (y <= p->height))
		out[2] = (unsigned char) (y - 1);
	write(p->fd, out, 3);
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
CFontz_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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
			// add pixel line per pixel line ...
			// NOTE: cellwidth != bar width: 0x1F = 0xFF & ((1 << (p->cellwidth - 1)) - 1)
			vBar[p->cellheight - i] = 0x1F;
			CFontz_set_char(drvthis, i, vBar);
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
CFontz_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar)-1);
			CFontz_set_char(drvthis, i, hBar);
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
CFontz_num(Driver *drvthis, int x, int num)
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

	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
CFontz_get_free_chars(Driver *drvthis)
{
//PrivateData *p = drvthis->private_data;

  return NUM_CCs;
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
CFontz_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4 + p->cellheight];
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	/* define the n'th custom character */
	out[0] = CFONTZ_Set_Custom_Char;
	out[1] = n;

	for (row = 0; row < p->cellheight; row++) {
		out[2+row] = dat[row] & mask;
	}
	write(p->fd, out, 2 + p->cellheight);
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
CFontz_icon(Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;

	static unsigned char heart_open[] =
		{ b__XXXXX,
		  b__X_X_X,
		  b_______,
		  b_______,
		  b_______,
		  b__X___X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char heart_filled[] =
		{ b__XXXXX,
		  b__X_X_X,
		  b___X_X_,
		  b___XXX_,
		  b___XXX_,
		  b__X_X_X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char checkbox_off[] =
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X___X,
		  b__X___X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_on[] =
		{ b____X__,
		  b____X__,
		  b__XXX_X,
		  b__X_XX_,
		  b__X_X_X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_gray[] =
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X_X_X,
		  b__XX_XX,
		  b__X_X_X,
		  b__XXXXX,
		  b_______ };
	/*
	static unsigned char selector_left[] =
		{ b___X___,
		  b___XX__,
		  b___XXX_,
		  b___XXXX,
		  b___XXX_,
		  b___XX__,
		  b___X___,
		  b_______ };
	static unsigned char selector_right[] =
		{ b_____X_,
		  b____XX_,
		  b___XXX_,
		  b__XXXX_,
		  b___XXX_,
		  b____XX_,
		  b_____X_,
		  b_______ };
	*/

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			CFontz_raw_chr(drvthis, x, y, (p->newfirmware) ? 0x1F : 255);
			break;
		case ICON_HEART_FILLED:
			CFontz_set_char(drvthis, 0, heart_filled);
			CFontz_raw_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			CFontz_set_char(drvthis, 0, heart_open);
			CFontz_raw_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
			CFontz_raw_chr(drvthis, x, y, 0xDE);
			break;
		case ICON_ARROW_DOWN:
			CFontz_raw_chr(drvthis, x, y, 0xE0);
			break;
		case ICON_ARROW_LEFT:
			CFontz_raw_chr(drvthis, x, y, 0xE1);
			break;
		case ICON_ARROW_RIGHT:
			CFontz_raw_chr(drvthis, x, y, 0xDF);
			break;
		case ICON_CHECKBOX_OFF:
			CFontz_set_char(drvthis, 3, checkbox_off);
			CFontz_raw_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			CFontz_set_char(drvthis, 4, checkbox_on);
			CFontz_raw_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			CFontz_set_char(drvthis, 5, checkbox_gray);
			CFontz_raw_chr(drvthis, x, y, 5);
			break;
		case ICON_SELECTOR_AT_LEFT:
			if (!p->newfirmware)
				return -1;
			CFontz_raw_chr(drvthis, x, y, 0x10);
			break;
		case ICON_SELECTOR_AT_RIGHT:
			if (!p->newfirmware)
				return -1;
			CFontz_raw_chr(drvthis, x, y, 0x11);
			break;
		default:
			return -1; /* Let the core do other icons */
	}

	return 0;
}


/**
 * Set cursor position and state.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal cursor position (column).
 * \param y        Vertical cursor position (row).
 * \param state    New cursor state.
 */
MODULE_EXPORT void
CFontz_cursor(Driver *drvthis, int x, int y, int state)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char stylecmd[1];

	/* set cursor state */
	switch (state) {
		case CURSOR_OFF:	// no cursor
			stylecmd[0] = CFONTZ_Hide_Cursor;
			break;
		case CURSOR_UNDER:	// underline cursor
			stylecmd[0] = CFONTZ_Show_Underline_Cursor;
			break;
		case CURSOR_BLOCK:	// inverting blinking block
			stylecmd[0] = CFONTZ_Show_Inverting_Block_Cursor;
			break;
		case CURSOR_DEFAULT_ON:	// blinking block
		default:
			stylecmd[0] = CFONTZ_Show_Block_Cursor;
			break;
	}
	write(p->fd, stylecmd, 1);

	/* set cursor position */
	CFontz_cursor_goto(drvthis, x, y);
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
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
CFontz_string(Driver *drvthis, int x, int y, unsigned char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		/* Check for buffer overflows... */
		if (x >= 0)
			p->framebuf[(y * p->width) + x] = (p->newfirmware)
							  ? CFontz_charmap[(unsigned) string[i]]
							  : string[i];
	}
}

