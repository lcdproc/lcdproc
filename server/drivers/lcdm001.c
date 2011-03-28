/** \file server/drivers/lcdm001.c
 * LCDd \c lcdm001 driver for the LCDM001 device from kernelconcepts.de.
 */

/*-
 *  This driver is mostly based on the MtxOrb driver. See the file MtxOrb.c
 *  for copyright details. The function calls needed for reporting and getting
 *  settings from the configfile have been written taking the calls in
 *  sed1330.c.
 *
 *  Copyright (C) 2001  Rene Wagner <reenoo@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * LCDM001 does NOT support custom chars. You can modify the characters that
 * are displayed instead of the normal icons for the heartbeat in lcdm001.h.
 */

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
#include "lcdm001.h"
#include "report.h"


/** private data for the \c lcdm001 driver */
typedef struct lcdm001_private_data {
	char device[200];
	int fd;
	int speed;

	char pause_key;
	char back_key;
	char forward_key;
	char main_menu_key;

	char *framebuf;

	int width, height;
	int cellwidth, cellheight;
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "lcdm001_";

/* Local function prototypes */
static void lcdm001_cursorblink(Driver *drvthis, int on);
static char lcdm001_parse_keypad_setting(Driver *drvthis, char * keyname, char * default_value);


/**
 * Parse one key from the configfile.
 * \param drvthis        Pointer to driver
 * \param keyname        Name of the key in config file
 * \param default_value  Default key value to assign if keyname does not exist.
 * \return  The key character that will be assigned if the key on the device is
 *          pressed.
 */
static char
lcdm001_parse_keypad_setting(Driver *drvthis, char * keyname, char * default_value)
{
	char return_val = 0;

	if (strcmp(drvthis->config_get_string(drvthis->name, keyname, 0, default_value), "LeftKey") == 0) {
		return_val = LEFT_KEY;
	} else if (strcmp(drvthis->config_get_string(drvthis->name, keyname, 0, default_value), "RightKey") == 0) {
		return_val = RIGHT_KEY;
	} else if (strcmp(drvthis->config_get_string(drvthis->name, keyname, 0, default_value), "UpKey") == 0) {
		return_val = UP_KEY;
	} else if (strcmp(drvthis->config_get_string(drvthis->name, keyname, 0, default_value), "DownKey") == 0) {
		return_val = DOWN_KEY;
	} else {
		report(RPT_WARNING, "%s: invalid config setting for %s; using default %s",
				drvthis->name, keyname, default_value);
		if (strcmp(default_value, "LeftKey") == 0) {
			return_val = LEFT_KEY;
		} else if (strcmp(default_value, "RightKey") == 0) {
			return_val = RIGHT_KEY;
		} else if (strcmp(default_value, "UpKey") == 0) {
			return_val = UP_KEY;
		} else if (strcmp(default_value, "DownKey") == 0) {
			return_val = DOWN_KEY;
		}
	}
	return return_val;
}

/**
 * Turn cursor blinking on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       Integer: 0 to turn off blinking, 1 to turn on blinking.
 */
static void
lcdm001_cursorblink(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (on) {
		write(p->fd, "~K1", 3);
		debug(RPT_INFO, "%s: cursorblink turned on", drvthis->name);
	} else {
		write(p->fd, "~K0", 3);
		debug(RPT_INFO, "%s: cursorblink turned off", drvthis->name);
	}
}


/**
 * API: Set up any device-specific stuff.
 */
MODULE_EXPORT int
lcdm001_init (Driver *drvthis)
{
	PrivateData *p;
	struct termios portset;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
       		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->speed = B38400;
	p->pause_key = DOWN_KEY;
	p->back_key = LEFT_KEY;
	p->forward_key = RIGHT_KEY;
	p->main_menu_key = UP_KEY;
	p->framebuf = NULL;
	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	debug(RPT_INFO, "LCDM001: init(%p)", drvthis);

	p->framebuf = calloc(1, p->width * p->height);
	if (p->framebuf == NULL) {
                report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* READ CONFIG FILE: */

	/* which serial device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, "/dev/lcd"), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* keypad settings */
	p->pause_key =      lcdm001_parse_keypad_setting(drvthis, "PauseKey", "DownKey");
	p->back_key =       lcdm001_parse_keypad_setting(drvthis, "BackKey", "LeftKey");
	p->forward_key =    lcdm001_parse_keypad_setting(drvthis, "ForwardKey", "RightKey");
	p->main_menu_key =  lcdm001_parse_keypad_setting(drvthis, "MainMenuKey", "UpKey");

	/* Set up io port correctly, and open it... */
	debug(RPT_DEBUG, "%s: opening serial device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%d) failed (%s)",
				drvthis->name, p->device, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: make sure you have rw access to %s!",
					drvthis->name, p->device);
  		return -1;
	}
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, p->device);

	tcgetattr(p->fd, &portset);
#ifdef HAVE_CFMAKERAW
	/* The easy way: */
	cfmakeraw(&portset);
#else
	/* The hard way: */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, p->speed);
	tcsetattr(p->fd, TCSANOW, &portset);
	tcflush(p->fd, TCIOFLUSH);

	/* Reset and clear the LCDM001 */
	write(p->fd, "~C", 2);
	/* Set cursorblink default */
	lcdm001_cursorblink(drvthis, DEFAULT_CURSORBLINK);
	/* Turn all LEDs off */
	lcdm001_output(drvthis, 0);

	report(RPT_INFO, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * API: Cleanup all data structures and close device.
 */
MODULE_EXPORT void
lcdm001_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->fd >= 0) {
			/* switch off all LEDs */
			lcdm001_output(drvthis, 0);
			close(p->fd);
		}
		p->fd = -1;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

        report(RPT_INFO, "%s: closed", drvthis->name);
}

/**
 * API: Returns the display width
 */
MODULE_EXPORT int
lcdm001_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/**
 * API: Returns the display height
 */
MODULE_EXPORT int
lcdm001_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/**
 * API: Clears the internal frame buffer. Does not clear display screen
 * immediately as it only makes the display flicker.
 */
MODULE_EXPORT void
lcdm001_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);

	debug(RPT_DEBUG, "LCDM001: cleared screen");
}

/**
 * API: Flushes all output to the lcd...
 */
MODULE_EXPORT void
lcdm001_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	/* TODO: Check whether this is still correct */
	write(p->fd, p->framebuf, p->width * p->height);

	debug(RPT_DEBUG, "LCDM001: frame buffer flushed");
}

/**
 * API: Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
lcdm001_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--; x--;		/* translate to 0-coords */

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;

	debug(RPT_DEBUG, "LCDM001: writing character %02X to position (%d,%d)", c, x, y);
}

/**
 * API: Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
lcdm001_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--; y--;		/* Convert 1-based coords to 0-based... */

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)	/* no write left of left border */
			p->framebuf[(y * p->width) + x] = string[i];
	}

	debug(RPT_DEBUG, "LCDM001: printed string at (%d,%d)", x, y);
}

/**
 * API: Controls LEDs.
 * \param drvthis  Pointer to driver
 * \param state    LED state
 *
 * \todo Document LED state.
 */
MODULE_EXPORT void
lcdm001_output (Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	char out[5];
	int one = (state & 0xFF);
	int two = (state > 255) ? ((state >> 8) & 0xFF) : 0;

        snprintf(out, sizeof(out), "~L%c%c", one, two);
        write(p->fd, out, 4);

        debug(RPT_DEBUG, "LCDM001: current LED state %d", state);
}

/**
 * API: Draws a vertical bar, from the bottom of the screen up.
 */
MODULE_EXPORT void
lcdm001_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;

	if ((x <= 0) || (x > p->width) || (y <= 0) || (y > p->height))
		return;

	debug(RPT_DEBUG , "LCDM001: vertical bar at %d set to %d", x, pixels);

	while (y > 0 && pixels >= p->cellheight) {
		lcdm001_chr(drvthis, x, y, 0xFF);
		pixels -= p->cellheight;
		y--;
	}
}

/**
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
lcdm001_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;

	if ((x <= 0) || (y <= 0) || (y > p->height))
		return;

	debug(RPT_DEBUG, "LCDM001: horizontal bar at %d set to %d", x, pixels);

	while (x <= p->width && pixels >= p->cellwidth) {
		lcdm001_chr(drvthis, x, y, 0xFF);
		pixels -= p->cellwidth;
		x++;
	}

	return;
}

/**
 * API: Place an icon on the screen. As this driver does not support custom
 * characters only generic icons from core are supported. The heartbeat
 * icons however are replaced with some different characters.
 */
MODULE_EXPORT int
lcdm001_icon (Driver *drvthis, int x, int y, int icon)
{
	switch (icon) {
	    case ICON_HEART_OPEN:
		lcdm001_chr(drvthis, x, y, LCDM001_OPEN_HEART);
		break;
	    case ICON_HEART_FILLED:
		lcdm001_chr(drvthis, x, y, LCDM001_FILLED_HEART);
		break;
	    default:
		return -1;	/* Let the core do the other */
	}

	return 0;
}


/**
 * API: Tries to read a character from an input device...
 *
 * \return  NULL for "nothing available" or one of the fixed strings "Enter",
 *          "Left", "Right", or "Escape".
 */
MODULE_EXPORT const char *
lcdm001_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
        char in = '\0';
	const char *key = NULL;

	read(p->fd, &in, 1);
	if (in == p->pause_key) {
		key = "Enter";
	} else if (in == p->back_key) {
		key = "Left";
	} else if (in == p->forward_key) {
		key = "Right";
	} else if (in == p->main_menu_key) {
		key = "Escape";
	}
	debug(RPT_DEBUG, "%s, get_key: %s",
			drvthis->name, (key != NULL) ? key : "<null>");
        return key;
}

