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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */


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


typedef struct cgram_cache {
	char cache[DEFAULT_CELL_HEIGHT];
	int clean;
} CGram;

typedef struct driver_private_data {
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
	CGram cc[NUM_CCs];
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
static void CFontz633_init_vbar (Driver *drvthis);
static void CFontz633_init_hbar (Driver *drvthis);
static void CFontz633_no_live_report (Driver *drvthis);
static void CFontz633_hardware_clear (Driver *drvthis);


/*
 * Opens com port and sets baud correctly...
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


/*
 * Clean-up
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


/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
CFontz633_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
CFontz633_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
CFontz633_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
CFontz633_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/*
 * Flushes all output to the lcd...
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


/*
 * Return one char from the KeyRing
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


/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,2).
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


/*
 * Returns current contrast (in promille)
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 */
MODULE_EXPORT int
CFontz633_get_contrast (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/*
 *  Changes screen contrast (in promille)
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


/*
 * Retrieves brightness (in promille)
 */
MODULE_EXPORT int
CFontz633_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/*
 * Sets on/off brightness (in promille)
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
		//CFontz633_backlight(drvthis, BACKLIGHT_ON);
	}
	else {
		p->offbrightness = promille;
		//CFontz633_backlight(drvthis, BACKLIGHT_OFF);
	}
}


/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
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


/*
 * Get rid of the blinking cursor
 */
static void
CFontz633_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 0);
}


/*
 * Stop live reporting of temperature.
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


/*
 * Stop the reporting of any fan.
 */
static void
CFontz633_no_fan_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_onebyte_message(p->fd, CF633_Set_Up_Fan_Reporting, 0);
}


/*
 * Stop the reporting of any temperature.
 */
static void
CFontz633_no_temp_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { 0, 0, 0, 0 };

	send_bytes_message(p->fd, CF633_Set_Up_Temperature_Reporting, 4, out);
}


/*
 * Reset the display bios
 */
static void
CFontz633_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { 8, 18, 99 };

	send_bytes_message(p->fd, CF633_Reboot, 3, out);
	sleep(2);
}


/*
 * Sets up for vertical bars.
 */
static void
CFontz633_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};

	if (p->ccmode != vbar) {
		//if (p->ccmode != standard) {
		//	/* Not supported(yet) */
		//	report(RPT_WARNING, "%s: init_vbar: cannot combine two modes using user defined characters"
		//		drvthis->name);
		//	return;
		//}
		p->ccmode = vbar;

		CFontz633_set_char(drvthis, 1, a);
		CFontz633_set_char(drvthis, 2, b);
		CFontz633_set_char(drvthis, 3, c);
		CFontz633_set_char(drvthis, 4, d);
		CFontz633_set_char(drvthis, 5, e);
		CFontz633_set_char(drvthis, 6, f);
		CFontz633_set_char(drvthis, 7, g);
	}
}


/*
 * Inits horizontal bars...
 */
static void
CFontz633_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
	};

	if (p->ccmode != hbar) {
		//if (p->ccmode != standard) {
		//	/* Not supported(yet) */
		//	report(RPT_WARNING, "%s: init_hbar: Cannot combine two modes using user defined characters",
		//		drvthis->name);
		//	return;
		//}
		p->ccmode = hbar;

		CFontz633_set_char(drvthis, 1, a);
		CFontz633_set_char(drvthis, 2, b);
		CFontz633_set_char(drvthis, 3, c);
		CFontz633_set_char(drvthis, 4, d);
		CFontz633_set_char(drvthis, 5, e);
		CFontz633_set_char(drvthis, 6, f);
	}
}


/*
 * Draws a vertical bar...
 */
MODULE_EXPORT void
CFontz633_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'up' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */
	PrivateData *p = drvthis->private_data;

	CFontz633_init_vbar(drvthis);
	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/*
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
CFontz633_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'right' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */
	PrivateData *p = drvthis->private_data;

	CFontz633_init_hbar(drvthis);
	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*
 * Writes a big number.
 * This is not supported on 633 because we only have 2 lines...
 */
MODULE_EXPORT void
CFontz633_num (Driver *drvthis, int x, int num)
{
/*
	PrivateData *p = drvthis->private_data;
	unsigned char out[5];

	if ((x <= 0) || (x > p->width))
		return;

	snprintf(out, sizeof(out), "%c%c%c", 28, x, num);
	write(p->fd, out, 3);
*/
}


/*
 * Sets a custom character from 0 - (NUM_CCs-1)
 *
 * For input, values > 0 mean "on" and values <= 0 are "off".
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
CFontz633_set_char (Driver *drvthis, int n, char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[9];
	int row, col;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	out[0] = n;	/* Custom char to define. xxx */

	for (row = 0; row < p->cellheight; row++) {
		int letter = 0;

		for (col = 0; col < p->cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * p->cellwidth) + col] > 0);
		}
		out[row+1] = letter;
	}
	send_bytes_message(p->fd, CF633_Set_LCD_Special_Character_Data, 9, out);
}


/*
 * Places an icon on screen
 */
MODULE_EXPORT int
CFontz633_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char icons[8][6 * 8] = {
	/* Empty Heart */
		{
		 1, 1, 1, 1, 1, 1,
		 1, 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 0, 0,
		 1, 1, 0, 0, 0, 1,
		 1, 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },
	/* Filled Heart */
		{
		 1, 1, 1, 1, 1, 1,
		 1, 1, 0, 1, 0, 1,
		 1, 0, 1, 0, 1, 0,
		 1, 0, 1, 1, 1, 0,
		 1, 0, 1, 1, 1, 0,
		 1, 1, 0, 1, 0, 1,
		 1, 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },
	/* arrow_up */
		{
		 0, 0, 0, 1, 0, 0,
		 0, 0, 1, 1, 1, 0,
		 0, 1, 0, 1, 0, 1,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 },
	/* arrow_down */
		{
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 1, 0, 1, 0, 1,
		 0, 0, 1, 1, 1, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 },
	/* checkbox_off */
		{
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 1, 1, 1, 1, 1,
		 0, 1, 0, 0, 0, 1,
		 0, 1, 0, 0, 0, 1,
		 0, 1, 0, 0, 0, 1,
		 0, 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0, 0,
		 },
	/* checkbox_on */
		{
		 0, 0, 0, 1, 0, 0,
		 0, 0, 0, 1, 0, 0,
		 0, 1, 1, 1, 0, 1,
		 0, 1, 0, 1, 1, 0,
		 0, 1, 0, 1, 0, 1,
		 0, 1, 0, 0, 0, 1,
		 0, 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0, 0,
		 },
	/* checkbox_gray */
		{
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 1, 1, 1, 1, 1,
		 0, 1, 0, 1, 0, 1,
		 0, 1, 1, 0, 1, 1,
		 0, 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0, 0,
		 },
	 /* Ellipsis */
		{
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1, 0,
		 },
	};

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			CFontz633_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 0, icons[1]);
			CFontz633_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 0, icons[0]);
			CFontz633_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 1, icons[2]);
			CFontz633_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 2, icons[3]);
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
			CFontz633_set_char(drvthis, 3, icons[4]);
			CFontz633_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 4, icons[5]);
			CFontz633_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
		        p->ccmode = custom;
			CFontz633_set_char(drvthis, 5, icons[6]);
			CFontz633_chr(drvthis, x, y, 5);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/*
 * Clears the LCD screen
 */
MODULE_EXPORT void
CFontz633_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
}


/*
 * Hardware clears the LCD screen
 */
static void
CFontz633_hardware_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_zerobyte_message(p->fd, CF633_Clear_LCD_Screen);
}


/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
CFontz633_string (Driver *drvthis, int x, int y, char string[])
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

