/*  This is the LCDproc driver for CrystalFontz devices (http://crystalfontz.com)

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

/*configfile support added by Rene Wagner (c) 2001*/
/*backlight support modified by Rene Wagner (c) 2001*/
/*block patch by Eddie Sheldrake (c) 2001 inserted by Rene Wagner*/
/*big num patch by Luis Llorente (c) 2002*/
/*adaptions to other CFontz* dirvers Peter Marschall (c) 2005*/

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
#include "CFontz.h"
#include "report.h"
#include "lcd_lib.h"
#include "CFontz-charmap.h"
#include "adv_bignum.h"


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


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
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

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
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
	if ((tmp < 0) || (tmp > 255)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 255; using default %d",
				drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	if ((tmp < 0) || (tmp > 255)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 255; using default %d",
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
	if (usb) {
		/* The USB way */
		portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
					| INLCR | IGNCR | ICRNL | IXON );
		portset.c_oflag &= ~OPOST;
		portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
		portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
		portset.c_cflag |= CS8 | CREAD | CLOCAL ;
		portset.c_cc[VMIN] = 1;
		portset.c_cc[VTIME] = 3;
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
		portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
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
	sleep (1);
	CFontz_hidecursor(drvthis);
	CFontz_linewrap(drvthis, 1);
	CFontz_autoscroll(drvthis, 0);
	//CFontz_backlight(drvthis, backlight_brightness);  // render.c variables should not be used in drivers !

	CFontz_set_contrast(drvthis, p->contrast);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
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

/////////////////////////////////////////////////////////////////
// Returns the display's character width
//
MODULE_EXPORT int
CFontz_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display's character height
//
MODULE_EXPORT int
CFontz_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell width
//
MODULE_EXPORT int
CFontz_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell height
//
MODULE_EXPORT int
CFontz_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
CFontz_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[3 * LCD_MAX_WIDTH];
	int i;

	if (p->newfirmware) {
		unsigned char *ptr = out;

		for (i = 0; i < p->height; i++) {
			int j;

			/* move cursor to start of (i+1)'th line */
			CFontz_cursor_goto(drvthis, 1, i+1);
			
			for (j = 0; j < p->width; j++) {
				unsigned char c = p->framebuf[(i * p->width) + j];

				/* characters that need to be treated special */
				if ((c < 0x20) || ((c >= 0xF0) && (c < 0xF8))) {
					if (c < 0x08) {
						// custom chars are at position 0xF0 - 0xF7
						c += 0xF0;
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
		// Custom characters start at 0xF0, not at 0.
		for (i = 0; i < p->width * p->height; i++) {
			if (p->framebuf[i] < 32)
				p->framebuf[i] += 128;
		}

		for (i = 0; i < p->height; i++) {
			/* move cursor to start of (i+1)'th line */
			CFontz_cursor_goto(drvthis, 1, i+1);

			write(p->fd, p->framebuf + (p->width * i), p->width);
		}
	}	
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
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

/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (20,4).
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


/////////////////////////////////////////////////////////////////
// Returns current p->contrast
// This is only the locally stored p->contrast, the contrast value
// cannot be retrieved from the LCD.
// Value 0 to 1000.
//
MODULE_EXPORT int
CFontz_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}

/////////////////////////////////////////////////////////////////
// Changes screen p->contrast (0-1000; 400 seems good)
// Value 0 to 100.
//
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

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate p->brightness...
//
MODULE_EXPORT void
CFontz_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Backlight_Control, 0 };

	out[1] = (on) ? p->brightness : p->offbrightness;
	write(p->fd, out, 2);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
CFontz_linewrap(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	out[0] = (on) ? CFONTZ_Wrap_On : CFONTZ_Wrap_Off;
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
CFontz_autoscroll(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	out[0] = (on) ? CFONTZ_Scroll_On : CFONTZ_Scroll_Off;
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking cursor
//
static void
CFontz_hidecursor(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Hide_Cursor };

	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
CFontz_reboot(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4] = { CFONTZ_Reboot, CFONTZ_Reboot };

	write(p->fd, out, 2);
	sleep(4);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
CFontz_cursor_goto(Driver *drvthis, int x, int y)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { CFONTZ_Set_Cursor_Position, 0, 0 };

	/* set cursor position */
	if ((x > 0) && (x <= p->width))
		out[1] = x - 1;
	if ((y > 0) && (y <= p->height))
		out[2] = y - 1;
	write(p->fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
CFontz_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user defined characters",
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

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
CFontz_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user defined characters",
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


/*
 * Writes a big number.
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
			report(RPT_WARNING, "%s: num: cannot combine two modes using user defined characters",
					drvthis->name);
			return;
		}

		p->ccmode = bignum;

		do_init = 1;
	}

	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, do_init, NUM_CCs);
}


/*
 * Gets number of custom chars (always NUM_CCs)
 */
MODULE_EXPORT int
CFontz_get_free_chars(Driver *drvthis)
{
//PrivateData *p = drvthis->private_data;

  return NUM_CCs;
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0 - (NUM_CCs-1)...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
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

/////////////////////////////////////////////////////////////////
// Places an icon on screen
//
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
	/*
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
			CFontz_raw_chr(drvthis, x, y, (p->newfirmware) ? 214 : 255);
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
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/*
 * Sets cursor position and state
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


/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
CFontz_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
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

