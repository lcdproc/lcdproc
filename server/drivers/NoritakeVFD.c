/** \file server/drivers/NoritakeVFD.c
 * LCDd \c NoritakeVFD driver for the Noritake VFD Device CU20045SCPB-T28A.
 */

/*
    Copyright (C) 2005 Simon Funke
    Copyright (C) 2007 Richard Muratti ricacho@gmail.com
    Copyright (C) 2007 Peter Marschall

    This source Code is based on CFontz Driver of this package.

    2005-08-01 Version 0.1: mostly everything should work (vbar, hbar never tested)

    2007-09-16 Version 0.2:
     - Fixed vbar & hbar - Allowed for displays with 2 custom characters
     - Fixed cursor off  - 16H - Data sheet for CU20045SCPB has error -> Cursor Off 14,16,17H
     - Tested with CU40026SCPB-T20A - 40x2 - Hardware Defaults to 19200 8E1
     - Added new config parameter [Parity] to set serial data parity 0(none),1(odd),2(even)
     - Added config parameter OffBrightness
     - added support for big numbers

       CU40026SCPB-T20A  SOFTWARE COMMANDS
       Back Space              08H     International Font              18H
       Horizontal Tab          09H     Katakana Font                   19H
       Line Feed               0AH     Escape                          1BH
       Form Feed               0CH             Send User Font          +43H
       Carriage Return         0DH             Position cursor         +48H
       Clear Display           0EH             Software Reset          +49H
       Increment Write Mode    11H             Luminance               +4CH
       Vertical Scroll Mode    12H             Flickerless Write       +53H
       Underline Cursor On     14H             Cursor Blink Speed      +54H
       5x7 Block Cursor On     15H     Character Data                  20H+
       Cursor Off              16H     User Character Data             00H+

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
#include "NoritakeVFD.h"
#include "report.h"
#include "adv_bignum.h"


/* Constants for userdefchar_mode */
#define NUM_CCs		2 /* max. number of custom characters */

/** private data for the \c NoritakeVFD driver */
typedef struct NoritakeVFD_private_data {
	char device[200];
	int fd;
	int speed;
	int parity;
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;
	/* definable characters */
	CGmode ccmode;
	int brightness;
	int offbrightness;
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "NoritakeVFD_";


/* Internal functions */
static void NoritakeVFD_autoscroll (Driver *drvthis, int on);
static void NoritakeVFD_hidecursor (Driver *drvthis);
static void NoritakeVFD_reboot (Driver *drvthis);
static void NoritakeVFD_cursor_goto (Driver *drvthis, int x, int y);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
NoritakeVFD_init (Driver *drvthis)
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
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
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

	/* Which parity */
	tmp = drvthis->config_get_int(drvthis->name, "Parity", 0, DEFAULT_PARITY);
	if ((tmp != 0) && (tmp != 1) && (tmp != 2) ) {
		report(RPT_WARNING, "%s: Parity must be 0(=none), 1(=odd), 2(=even); using default %d",
			drvthis->name, DEFAULT_PARITY);
		tmp = DEFAULT_PARITY;
	}
	if (tmp != 0)
		p->parity = (tmp & 1) ? (PARENB | PARODD) : PARENB;


	/* Reboot display? */
	reboot = drvthis->config_get_bool(drvthis->name, "Reboot", 0, 0);

	/* Set up io port correctly, and open it...*/
	debug(RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (p->fd == -1) {
		report(RPT_ERR, "%s: open() of %s failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	// We use RAW mode (with varying parity)
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw(&portset);
	// set parity the way we want it
	portset.c_cflag &= ~(PARENB | PARODD);
	portset.c_cflag |= p->parity;
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL | p->parity;
#endif

	// Set port speed
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, B0);

	// Do it...
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


	/* Set display-specific stuff..*/
	if (reboot) {
		NoritakeVFD_reboot(drvthis);
		sleep(4);
	}
	NoritakeVFD_hidecursor(drvthis);
	NoritakeVFD_autoscroll(drvthis, 0);
	NoritakeVFD_set_brightness(drvthis, 1, p->brightness);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;

}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
NoritakeVFD_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);

		if (p->backingstore)
			free(p->backingstore);
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
NoritakeVFD_width (Driver *drvthis)
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
NoritakeVFD_height (Driver *drvthis)
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
NoritakeVFD_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
NoritakeVFD_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
NoritakeVFD_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	for (i = 0; i < p->height; i++) {
		int offset = i * p->width;

		/* Backing-store based implementation:
		 * Only put it on the screen if it's not already there */
		if (memcmp(p->backingstore+offset, p->framebuf+offset, p->width) != 0) {
			memcpy(p->backingstore+offset, p->framebuf+offset, p->width);

			NoritakeVFD_cursor_goto(drvthis, 1, i+1);
			write(p->fd, p->framebuf+offset, p->width);
		}
	}
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
NoritakeVFD_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
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
NoritakeVFD_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
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
NoritakeVFD_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)     // no write left of left border
			p->framebuf[(y * p->width) + x] = string[i];
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
NoritakeVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;
        int pos;
	static unsigned char half[] =
		{ b_______,
		  b_______,
		  b_______,
		  b_______,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };

	if (p->ccmode != vbar) {
		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}

		p->ccmode = vbar;

		/* define half full block at position 1 */
		NoritakeVFD_set_char(drvthis, 1, half);
	}

	for (pos = 0; pos < len; pos++) {
		/* if pixels > 2/3 cellheight [in integer arithmetics] ... */
		if (3 * pixels >= p->cellheight * 2) {
	       		/* ... write a full block to the screen */
			NoritakeVFD_chr(drvthis, x+pos, y, 0xBE);
		}
		/* if pixels > 1/3 cellheight [in integer arithmetics] ... */
		else if (3 * pixels > p->cellheight * 1) {
			/* ... write a partial block */
			NoritakeVFD_chr(drvthis, x+pos, y, 1);
			break;
		}
		else {
			; // write nothing (not even a space)
		}
		pixels -= p->cellheight;
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
NoritakeVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;
        int pos;
	static unsigned char half[] =
		{ b__XX___,
		  b__XX___,
		  b__XX___,
		  b__XX___,
		  b__XX___,
		  b__XX___,
		  b__XX___ };

	if (p->ccmode != hbar) {
		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}

		p->ccmode = hbar;

		/* define half full block at position 1 */
		NoritakeVFD_set_char(drvthis, 1, half);
	}

        for (pos = 0; pos < len; pos++) {
		/* if pixels > 2/3 cellwidth [in integer arithmetics] ... */
                if (3 * pixels >= p->cellwidth * 2) {
                        /* ... write a full block to the screen */
                        NoritakeVFD_chr(drvthis, x+pos, y, 0xBE);
                }
		/* if pixels > 1/3 cellwidth [in integer arithmetics] ... */
                else if (3 * pixels > p->cellwidth * 1) {
                        /* ... write a partial block */
                        NoritakeVFD_chr(drvthis, x+pos, y, 1);
                        break;
                }
		else {
			; // write nothing (not even a space)
		}
		pixels -= p->cellwidth;
	}
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
NoritakeVFD_num(Driver *drvthis, int x, int num)
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
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
NoritakeVFD_icon (Driver *drvthis, int x, int y, int icon)
{
	//PrivateData *p = drvthis->private_data;
	static unsigned char heart_open[] =
		{ b_______,
		  b___X_X_,
		  b__X_X_X,
		  b__X___X,
		  b__X___X,
		  b___X_X_,
		  b____X__ };
	static unsigned char heart_filled[] =
		{ b_______,
		  b___X_X_,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b___XXX_,
		  b____X__ };

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			NoritakeVFD_chr(drvthis, x, y, 0xBE);
				      break;
		case ICON_HEART_FILLED:
			NoritakeVFD_set_char(drvthis, 0, heart_filled);
			NoritakeVFD_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			NoritakeVFD_set_char(drvthis, 0, heart_open);
			NoritakeVFD_chr(drvthis, x, y, 0);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/*
 * Set cursor position and state.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal cursor position (column).
 * \param y        Vertical cursor position (row).
 * \param state    New cursor state.
 */
/*
MODULE_EXPORT void
CFontzPacket_cursor (Driver *drvthis, int x, int y, int state)
{
	PrivateData *p = drvthis->private_data;
	char out[2] = { 0x15 };

	// set cursor state
	switch (state) {
		case CURSOR_OFF:	// no cursor
			out[0] = 0x16;
			break;
		case CURSOR_UNDER:	// underline cursor
			out[0] = 0x14;
			break;
		case CURSOR_BLOCK:	// inverting blinking block
		case CURSOR_DEFAULT_ON:	// blinking block
		default:
			out[0] = 0x15;
			break;
	}
	write(p->fd, out, 1);

	NoritakeVFD_cursor_goto(x, y);
}
*/


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always \c NUM_CCs).
 */
MODULE_EXPORT int
NoritakeVFD_get_free_chars (Driver *drvthis)
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
NoritakeVFD_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	char out[9] = { 0x1B, 0x43, 0, 0, 0, 0, 0, 0, 0 };
	int i;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (dat == NULL)
		return;

	out[2] = n;

	/* mangle the character data bits so that the VFD can chew them ;-) */
	for (i = 0; i < 35; i++) {
		out[3 + i/8] |= ((dat[i/5] >> (4 - i%5)) & 1) << i%8;
	}

	write(p->fd, out, 8);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return         Stored brightness in promille.
 */
MODULE_EXPORT int
NoritakeVFD_get_brightness(Driver *drvthis, int state)
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
NoritakeVFD_set_brightness(Driver *drvthis, int state, int promille)
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
	//Noritake_backlight(drvthis, state);
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
NoritakeVFD_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { 0x1B, 0x4C, 0 };
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->brightness
			     : p->offbrightness;

	// Changes screen brightness (0-255; 140 seems good)
	/* not sure if the formula is correct:
	 * What is the allowed range for the brightness value ? */
	out[2] = (int) (hardware_value * 255 / 1000);
	write(p->fd, out, 3);
}


/**
 * Toggle the built-in automatic scrolling feature.
 * \param drvthis  Pointer to driver structure.
 * \param on       New scrolling status.
 */
static void
NoritakeVFD_autoscroll (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[2];

	out[0] = (on) ? 0x12 : 0x11;
	write(p->fd, out, 1);
}


/**
 * Get rid of the blinking cursor.
 * \param drvthis  Pointer to driver structure.
 */
static void
NoritakeVFD_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[2] = { 0x16 };

	write(p->fd, out, 1);
}


/**
 * Reset the display bios.
 * \param drvthis  Pointer to driver structure.
 */
static void
NoritakeVFD_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char reset_out[3] = { 0x1B, 0x49 };
	char flickerless_out[3] = { 0x1B, 0x53 };

	/* reset display */
	write(p->fd, reset_out, 2);

	/* switch on flickerless write */
	write(p->fd, flickerless_out, 2);
}


/**
 * Move cursor to position (x,y).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 */
static void
NoritakeVFD_cursor_goto(Driver *drvthis, int x, int y)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { 0x1B, 0x48, 0 };

	/* set cursor position */
	if ((x > 0) && (x <= p->width) && (y > 0) && (y <= p->height))
		out[2] = (x-1) * p->width + (y-1);
	write(p->fd, out, 3);
}


/* EOF */
