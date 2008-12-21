/** \file server/drivers/CFontz633.c
 * LCDd \c CFontz633 driver for CFA633 devices by CrystalFontz, Inc.
 */

/*  This is the LCDproc driver for CrystalFontz 633 devices
    (get yours from http://crystalfontz.com)

    Copyright (C) 2002 David GLAUDE

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


/*
 * Driver status
 * 04/04/2002: Working driver
 * 05/06/2002: Reading of return value
 * 02/09/2002: KeyPad handling and return string
 * 03/09/2002: New icon incorporated
 *
 * THINGS NOT DONE:
 * + No checking if right hardware is connected (firmware/hardware)
 * + No BigNum (but screen is too small ???)
 * + No support for multiple instance (require private structure)
 * + No cache of custom char usage (like in MtxOrb)
 *
 * THINGS DONE:
 * + Stopping the live reporting (of temperature)
 * + Stopping the reporting of temp and fan (is it necessary after reboot)
 * + Use of library for hbar and vbar (good but library could be better)
 * + Support for keypad (Using a KeyRing)
 *
 * THINGS TO DO:
 * + Make the caching at least for heartbeat icon
 * + Create and use the library (for custom char handling)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "CFontz633.h"
#include "CFontz633io.h"
#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#define CF633_KEY_UP		1
#define CF633_KEY_DOWN		2
#define CF633_KEY_LEFT		3
#define CF633_KEY_RIGHT		4
#define CF633_KEY_ENTER		5
#define CF633_KEY_ESCAPE	6


/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */

typedef enum {
	standard,	/* only char 0 is used for heartbeat */
	vbar,		/* vertical bars */
	hbar,		/* horizontal bars */
	custom,		/* custom settings */
	bignum,		/* big numbers */
	bigchar		/* big characters */
} CGmode;


/** private data for the \c CFonts633 driver */
typedef struct CFontz633_private_data {
	char device[200];

	int fd;

	int model;
	int newfirmware;
	int usb;
	int speed;

	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* defineable characters */
	CGmode ccmode;

	int contrast;
	int brightness;
	int offbrightness;
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "CFontz633_";

/* Internal functions */
/* static void CFontz633_linewrap (int on); */
/* static void CFontz633_autoscroll (int on);  */
static void CFontz633_hidecursor (Driver *drvthis);
static void CFontz633_reboot (Driver *drvthis);
static void CFontz633_no_live_report (Driver *drvthis);
static void CFontz633_hardware_clear (Driver *drvthis);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
CFontz633_init (Driver *drvthis)
{
	struct termios portset;
	int tmp, w, h;
	int reboot = 0;
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

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis);

	EmptyKeyRing(&keyring);
	EmptyReceiveBuffer(&receivebuffer);

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
		report(RPT_WARNING, "%s: cannot parse Size: %s; using default %s",
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
	if ((tmp != 1200) && (tmp != 2400) && (tmp != 9600) && (tmp != 19200) && (tmp != 115200)) {
		report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200; using default %d",
			drvthis->name, DEFAULT_SPEED);
		tmp = DEFAULT_SPEED;
	}
	if (tmp == 1200) p->speed = B1200;
	else if (tmp == 2400) p->speed = B2400;
	else if (tmp == 9600) p->speed = B9600;
	else if (tmp == 19200) p->speed = B19200;
	else if (tmp == 115200) p->speed = B115200;

	/* New firmware version?
	 * I will try to behave differently for firmware 0.6 or above.
	 * Currently this is not in use.
	 */
	p->newfirmware = drvthis->config_get_bool(drvthis->name, "NewFirmware", 0, 0);

	/* Reboot display? */
	reboot = drvthis->config_get_bool(drvthis->name, "Reboot", 0, 0);

	/* Am I USB or not? */
	p->usb = drvthis->config_get_bool(drvthis->name, "USB", 0, 0);

	/* Set up io port correctly, and open it... */
	debug(RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, (p->usb) ? (O_RDWR | O_NOCTTY) : (O_RDWR | O_NOCTTY | O_NDELAY));
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	/* We use RAW mode */
	if (p->usb) {
		// The USB way
		portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
					| INLCR | IGNCR | ICRNL | IXON );
		portset.c_oflag &= ~OPOST;
		portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
		portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
		portset.c_cflag |= CS8 | CREAD | CLOCAL;
		portset.c_cc[VMIN] = 0;
		portset.c_cc[VTIME] = 0;
	} else {
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
	}

	/* Set port speed */
	cfsetospeed(&portset, p->speed);
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

	/* make sure the framebuffer backing store is there... */
	p->backingstore = (unsigned char *) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->width * p->height);

	/* Set display-specific stuff.. */
	if (reboot) {
		report(RPT_INFO, "%s: rebooting LCD...", drvthis->name);
		CFontz633_reboot(drvthis);
		reboot = 0;
	}

	CFontz633_hidecursor(drvthis);

	CFontz633_set_contrast(drvthis, p->contrast);
	CFontz633_no_live_report(drvthis);
	CFontz633_hardware_clear(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz633_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->backingstore)
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
CFontz633_width (Driver *drvthis)
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
CFontz633_height (Driver *drvthis)
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
CFontz633_cellwidth (Driver *drvthis)
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
CFontz633_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz633_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

#if defined(CF635_FLUSH)
	int len = p->width * p->height;
	unsigned char out[3];

	for (i = 0; i < len; i++) {
		if (p->framebuf[i] != p->backingstore[i]) {
			out[0] = (unsigned char) (i % p->width);	// column
			out[1] = (unsigned char) (i / p->width);	// line
			out[2] = p->framebuf[i];			// character
			send_bytes_message(p->fd, CF633_Send_Data_to_LCD, 3, out);
			p->backingstore[i] = p->framebuf[i];
		}
	}
#else
/*
 * We don't use delta update yet.
 * It is possible but not easy, we can only update a line, full or begining.
 */
	unsigned char *xp = p->framebuf;
	unsigned char *xq = p->backingstore;

	for (i = 0; i < p->width; i++) {
		if (*xp++ != *xq++) {
			send_bytes_message(p->fd, CF633_Set_LCD_Contents_Line_One, 16, p->framebuf);
			memcpy(p->backingstore, p->framebuf, p->width);
			break;
		}
	}

	xp = p->framebuf + p->width;
	xq = p->backingstore + p->width;

	for (i = 0; i < p->width; i++) {
		if (*xp++ != *xq++) {
			send_bytes_message(p->fd, CF633_Set_LCD_Contents_Line_Two, 16, p->framebuf + p->width);
			memcpy(p->backingstore + p->width, p->framebuf + p->width, p->width);
			break;
		}
	}
#endif /* defined(CF635_FLUSH) */
}


/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
CFontz633_get_key (Driver *drvthis)
{
	// PrivateData *p = drvthis->private_data;
	unsigned char key = GetKeyFromKeyRing(&keyring);

	switch (key) {
		case CF633_KEY_LEFT:
			return "Left";
			break;
		case CF633_KEY_UP:
			return "Up";
			break;
		case CF633_KEY_DOWN:
			return "Down";
			break;
		case CF633_KEY_RIGHT:
			return "Right";
			break;
		case CF633_KEY_ENTER:
			return "Enter"; /* Is this correct ? */
			break;
		case CF633_KEY_ESCAPE:
			return "Escape";
			break;
		default:
			if (key != '\0')
				report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
			return NULL;
			break;
	}
	return NULL;
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
CFontz633_chr (Driver *drvthis, int x, int y, char c)
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
CFontz633_get_contrast (Driver *drvthis)
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
CFontz633_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	int hardware_contrast;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	p->contrast = promille;

	/* map range [0, 1000] -> [0, 50] that the hardware understands */
	hardware_contrast = p->contrast / 20;
	send_onebyte_message(p->fd, CF633_Set_LCD_Contrast, hardware_contrast);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
CFontz633_get_brightness(Driver *drvthis, int state)
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
CFontz633_set_brightness(Driver *drvthis, int state, int promille)
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
	//CFontz633_backlight(drvthis, state);
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
CFontz633_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->brightness
			     : p->offbrightness;

	/* map range [0, 1000] -> [0, 100] that the hardware understands */
	hardware_value /= 10;
	send_onebyte_message(p->fd, CF633_Set_LCD_And_Keypad_Backlight, hardware_value);
}


/**
 * Get rid of the blinking cursor.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 0);
}


/**
 * Stop live reporting of temperature.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_no_live_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[2] = { 0, 0 };

	for (out[0] = 0; out[0] < 8; out[0]++) {
		send_bytes_message(p->fd, CF633_Set_Up_Live_Fan_or_Temperature_Display, 2, out);
	}
}


/**
 * Stop the reporting of any fan.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_no_fan_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_onebyte_message(p->fd, CF633_Set_Up_Fan_Reporting, 0);
}


/**
 * Stop the reporting of any temperature.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_no_temp_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { 0, 0, 0, 0 };

	send_bytes_message(p->fd, CF633_Set_Up_Temperature_Reporting, 4, out);
}


/**
 * Reset the LCD display.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { 8, 18, 99 };

	send_bytes_message(p->fd, CF633_Reboot, 3, out);
	sleep(2);
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
CFontz633_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			vBar[p->cellheight - i] = 0xFF;
			CFontz633_set_char(drvthis, i, vBar);
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
CFontz633_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			CFontz633_set_char(drvthis, i, hBar);
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
CFontz633_num(Driver *drvthis, int x, int num)
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
CFontz633_get_free_chars (Driver *drvthis)
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
CFontz633_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[9];
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	out[0] = n;	/* Custom char to define. xxx */

	for (row = 0; row < p->cellheight; row++) {
		out[row+1] = dat[row] & mask;
	}
	send_bytes_message(p->fd, CF633_Set_LCD_Special_Character_Data, 9, out);
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
CFontz633_icon (Driver *drvthis, int x, int y, int icon)
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
	static unsigned char arrow_up[] = 
		{ b____X__,
		  b___XXX_,
		  b__X_X_X,
		  b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b_______ };
	static unsigned char arrow_down[] = 
		{ b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b__X_X_X,
		  b___XXX_,
		  b____X__,
		  b_______ };
	/*
	static unsigned char arrow_left[] = 
		{ b_______,
		  b____X__,
		  b___X___,
		  b__XXXXX,
		  b___X___,
		  b____X__,
		  b_______,
		  b_______ };
	static unsigned char arrow_right[] = 
		{ b_______,
		  b____X__,
		  b_____X_,
		  b__XXXXX,
		  b_____X_,
		  b____X__,
		  b_______,
		  b_______ };
	*/
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
	static unsigned char ellipsis[] = 
		{ b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b__X_X_X,
		  b_______ };
	static unsigned char block_filled[] = 
		{ b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };
	*/

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			CFontz633_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 0, heart_filled);
			CFontz633_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 0, heart_open);
			CFontz633_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 1, arrow_up);
			CFontz633_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 2, arrow_down);
			CFontz633_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_LEFT:
			CFontz633_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			CFontz633_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 3, checkbox_off);
			CFontz633_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 4, checkbox_on);
			CFontz633_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 5, checkbox_gray);
			CFontz633_chr(drvthis, x, y, 5);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontz633_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
}


/**
 * Clear the LCD using hardware commands.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontz633_hardware_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_zerobyte_message(p->fd, CF633_Clear_LCD_Screen);
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
CFontz633_string (Driver *drvthis, int x, int y, const char string[])
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
			p->framebuf[(y * p->width) + x] = string[i];
	}
}

