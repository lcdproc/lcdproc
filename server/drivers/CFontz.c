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
static void CFontz_linewrap (Driver *drvthis, int on);
static void CFontz_autoscroll (Driver *drvthis, int on);
static void CFontz_hidecursor (Driver *drvthis);
static void CFontz_reboot (Driver *drvthis);
static void CFontz_init_vbar (Driver *drvthis);
static void CFontz_init_hbar (Driver *drvthis);


// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
CFontz_init (Driver *drvthis)
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
CFontz_close (Driver *drvthis)
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
// Returns the display p->width
//
MODULE_EXPORT int
CFontz_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display p->height
//
MODULE_EXPORT int
CFontz_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
CFontz_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
	int i;

	// Custom characters start at 128, not at 0.
	for (i = 0; i < p->width * p->height; i++) {
		if (p->framebuf[i] < 32)
			p->framebuf[i] += 128;
	}

	for (i = 0; i < p->height; i++) {
		snprintf(out, sizeof(out), "%c%c%c", 17, 0, i);
		write(p->fd, out, 3);
		write(p->fd, p->framebuf + (p->width * i), p->width);
	}
	/*
	   snprintf(out, sizeof(out), "%c", 1);
	   write(p->fd, out, 1);
	   write(p->fd, p->framebuf, p->width*p->height);
	 */
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
CFontz_chr (Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x < 0) || (y < 0) || (x >= p->width) || (y >= p->height))
                return;

	if (c < 32)
		c += 128;

	// For V2 of the firmware to get the block to display right
	if (p->newfirmware && (c == 255))
		c = 214;

	p->framebuf[(y * p->width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Returns current p->contrast
// This is only the locally stored p->contrast, the p->contrast value
// cannot be retrieved from the LCD.
// Value 0 to 1000.
//
MODULE_EXPORT int
CFontz_get_contrast (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}

/////////////////////////////////////////////////////////////////
// Changes screen p->contrast (0-1000; 400 seems good)
// Value 0 to 100.
//
MODULE_EXPORT void
CFontz_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	// Check it
	if ((promille < 0) || (promille > 1000))
		return;

	// Store it
	p->contrast = promille;

	// And do it
	snprintf(out, sizeof(out), "%c%c", 15, (unsigned char) (promille / 10)); // converted to be 0 to 100
	write(p->fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate p->brightness...
//
MODULE_EXPORT void
CFontz_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c%c", 14, (on) ? p->brightness : p->offbrightness);
	write(p->fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
CFontz_linewrap (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", (on) ? 23 : 24);
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
CFontz_autoscroll (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", (on) ? 19 : 20);
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking cursor
//
static void
CFontz_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", 4);
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
CFontz_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", 26);
	write(p->fd, out, 1);
	sleep(4);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.
//
static void
CFontz_init_vbar (Driver *drvthis)
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
		CFontz_set_char(drvthis, 1, a);
		CFontz_set_char(drvthis, 2, b);
		CFontz_set_char(drvthis, 3, c);
		CFontz_set_char(drvthis, 4, d);
		CFontz_set_char(drvthis, 5, e);
		CFontz_set_char(drvthis, 6, f);
		CFontz_set_char(drvthis, 7, g);
		p->ccmode = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
CFontz_init_hbar (Driver *drvthis)
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
		CFontz_set_char(drvthis, 1, a);
		CFontz_set_char(drvthis, 2, b);
		CFontz_set_char(drvthis, 3, c);
		CFontz_set_char(drvthis, 4, d);
		CFontz_set_char(drvthis, 5, e);
		CFontz_set_char(drvthis, 6, f);
		p->ccmode = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
CFontz_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */
	PrivateData *p = drvthis->private_data;

	CFontz_init_vbar(drvthis);
	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
CFontz_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */
	PrivateData *p = drvthis->private_data;

	CFontz_init_hbar(drvthis);
	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0 - (NUM_CCs-1)...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
CFontz_set_char (Driver *drvthis, int n, char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4 + p->cellheight];
	int row, col;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	/* define the n'th custom character */
	out[0] = 0x19;
	out[1] = n;

	for (row = 0; row < p->cellheight; row++) {
		int letter = 0;

		for (col = 0; col < p->cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * p->cellheight) + col] > 0);
		}
		out[2+row] = (unsigned char) letter;
	}
	write(p->fd, out, 2 + p->cellheight);
}

/////////////////////////////////////////////////////////////////
// Places an icon on screen
//
MODULE_EXPORT int
CFontz_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char icons[3][6 * 8] = {
	// Empty Heart
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
	// Filled Heart
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
	// Ellipsis
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

	if (p->ccmode == bignum)
		p->ccmode = standard;

	switch (icon) {
		case ICON_BLOCK_FILLED:
			CFontz_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
			CFontz_set_char(drvthis, 0, icons[1]);
			CFontz_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			CFontz_set_char(drvthis, 0, icons[0]);
			CFontz_chr(drvthis, x, y, 0);
			break;
		default:
			return -1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
CFontz_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
CFontz_string (Driver *drvthis, int x, int y, unsigned char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	// Convert 1-based coords to 0-based...
	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		unsigned char c = string[i];

		// For V2 of the firmware to get the block to display right
		if (p->newfirmware && (c == 255))
			c = 214;

		// Check for buffer overflows...
		if (x >= 0)
			p->framebuf[(y * p->width) + x] = c;
	}
}

