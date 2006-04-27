/*  This is the LCDproc driver for Noritake VFD Device CU20045SCPB-T28A

    Copyright (C) 2005 Simon Funke
    This source Code is based on CFontz Driver of this package.

    2005-08-01 Version 0.1: mostly everything should work (vbar, hbar never tested)

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
#include "NoritakeVFD.h"

#include "report.h"
#include "lcd_lib.h"

/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */

typedef enum {
	standard,	/* only char 0 is used for heartbeat */
	vbar,		/* vertical bars */
	hbar,		/* horizontal bars */
	custom,		/* custom settings */
} CGmode;

typedef struct driver_private_data {
	char device[200];
	int fd;
	int speed;
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;
	/* defineable characters */
	CGmode ccmode;
	int brightness;
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
static void NoritakeVFD_init_vbar (Driver *drvthis);
static void NoritakeVFD_init_hbar (Driver *drvthis);
static void NoritakeVFD_draw_frame (Driver *drvthis, unsigned char *dat);
//static void NoritakeVFD_heartbeat (Driver *drvthis, int type);


// Opens com port and sets baud correctly...
//
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


	/* Reboot display? */
	reboot = drvthis->config_get_bool(drvthis->name, "Reboot", 0, 0);

	/* Set up io port correctly, and open it...*/
	debug(RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open (p->device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (p->fd == -1) {
		report(RPT_ERR, "%s: open() of %s failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw(&portset);
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
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
	NoritakeVFD_set_brightness(drvthis, 1, p->brightness);
	NoritakeVFD_autoscroll(drvthis, 0);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;

}

/////////////////////////////////////////////////////////////////
// Clean-up
//
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

/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
NoritakeVFD_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
NoritakeVFD_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
NoritakeVFD_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
NoritakeVFD_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


MODULE_EXPORT void
NoritakeVFD_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	NoritakeVFD_draw_frame(drvthis, p->framebuf);
}


/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
NoritakeVFD_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;
	/*if (c < 32 && (int)c >= 0)
		c += 128;*/
	p->framebuf[(y * p->width) + x ] = c;
	//NoritakeVFD->framebuf[ 19 ] = c;
}

/////////////////////////////////////////////////////////////////
/*
 * Retrieves brightness
 */
MODULE_EXPORT int
NoritakeVFD_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return p->brightness;
}

/////////////////////////////////////////////////////////////////
// Changes screen brightness (0-255; 140 seems good)
//
MODULE_EXPORT void
NoritakeVFD_set_brightness (Driver *drvthis, int state, int brightness)
{
	PrivateData *p = drvthis->private_data;
	int realbrightness = -1;
	char out[5];

	if (brightness > 0) {
		realbrightness = (int) (140 * 100 / 255);
		snprintf(out, sizeof(out), "%c%c%c", 0x1B, 'L', brightness);
		write(p->fd, out, 3);
	}
	p->brightness = brightness;
}


/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
NoritakeVFD_autoscroll (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", (on) ? 0x12 : 0x11);
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void
NoritakeVFD_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c", 0x14);
	write(p->fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
NoritakeVFD_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[4];

	snprintf(out, sizeof(out), "%c%c", 0x1B, 'I');
	write(p->fd, out, 2);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before NoritakeVFD->vbar()
//
static void
NoritakeVFD_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->ccmode != hbar) {
		p->ccmode = hbar;
		NoritakeVFD_set_char(drvthis, 2, a);
		NoritakeVFD_set_char(drvthis, 3, b);
		NoritakeVFD_set_char(drvthis, 4, c);
		NoritakeVFD_set_char(drvthis, 5, d);
		NoritakeVFD_set_char(drvthis, 6, e);
		NoritakeVFD_set_char(drvthis, 7, f);
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
NoritakeVFD_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 
		1, 0, 0, 0, 0, 
		1, 0, 0, 0, 0, 
	};
	char b[] = {
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
		1, 1, 0, 0, 0, 
	};
	char c[] = {
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
		1, 1, 1, 0, 0, 
	};
	char d[] = {
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
		1, 1, 1, 1, 0, 
	};

	if (p->ccmode != hbar) {
		p->ccmode = hbar;
		NoritakeVFD_set_char(drvthis, 2, a);
		NoritakeVFD_set_char(drvthis, 3, b);
		NoritakeVFD_set_char(drvthis, 4, c);
		NoritakeVFD_set_char(drvthis, 5, d);
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
NoritakeVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
/*	PrivateData *p = drvthis->private_data;
	char map[8] = { 32, 1, 2, 3, 4, 5, 6, 255 };

	int y;
	for (y = p->height; y > 0 && len > 0; y--) {
		if (len >= p->cellheight)
			NoritakeVFD_chr (drvthis, x, y, 255);
		else
			NoritakeVFD_chr (drvthis, x, y, map[len]);

		len -= p->cellheight;
	}*/

	NoritakeVFD_init_vbar(drvthis);
	//lib_vbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
NoritakeVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
/*	PrivateData *p = drvthis->private_data;
	char map[5] = { 32, 1, 2, 3, 4 };

	for (; x <= p->width && len > 0; x++) {
		if (len >= p->cellwidth)
			NoritakeVFD_chr (drvthis, x, y, map[4]);
		else
			NoritakeVFD_chr (drvthis, x, y, map[len]);

		len -= p->cellwidth;

	}*/

	NoritakeVFD_init_hbar(drvthis);
	//lib_hbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, value 1 mean "on" and 0 is "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
NoritakeVFD_set_char (Driver *drvthis, int n, char *dat)
{
	PrivateData *p = drvthis->private_data;
	char out[4];
	int byte, bit;
	char letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	snprintf(out, sizeof(out), "%c%c%c", 0x1B, 'C', n);
	write(p->fd, out, 3);

	for (byte = 0; byte < 5; byte++) {
		letter = dat[(byte+1) * 8 - 1];
		for (bit = 7; bit > 0; bit--) {
			letter <<= 1;
			if ((byte * 8) + bit < 36)
				letter |= dat[(byte * 8) - 1 + bit];
		}
		write(p->fd, &letter, 1);
	}
}

/*
 * Places an icon on screen
 */
MODULE_EXPORT int
NoritakeVFD_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char icons[2][5 * 7] = {
		{
		 0, 0, 0, 0, 0, 		  // Empty Heart
		 0, 1, 0, 1, 0, 
		 1, 1, 1, 1, 1, 
		 1, 1, 1, 1, 1, 
		 1, 1, 1, 1, 1, 
		 0, 1, 1, 1, 0, 
		 0, 0, 1, 0, 0, 
		 },

		{
		 0, 0, 0, 0, 0, 		  // Filled Heart
		 0, 1, 0, 1, 0, 
		 1, 0, 1, 0, 1, 
		 1, 0, 0, 0, 1, 
		 1, 0, 0, 0, 1, 
		 0, 1, 0, 1, 0, 
		 0, 0, 1, 0, 0, 
		},
	};

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_HEART_FILLED:
		        p->ccmode = custom;
			NoritakeVFD_set_char(drvthis, 0, icons[1]);
			NoritakeVFD_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        p->ccmode = custom;
			NoritakeVFD_set_char(drvthis, 0, icons[0]);
			NoritakeVFD_chr(drvthis, x, y, 0);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized NoritakeVFD->width*NoritakeVFD->height
//
static void
NoritakeVFD_draw_frame (Driver *drvthis, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	char out[p->width * p->height];
	unsigned char *row, *b_row;
	int i;

	if (!dat)
		return;

	for (i = 0; i < p->height; i++) {
		row = dat + (p->width * i);
		b_row = p->backingstore + (p->width * i);

		/* Backing-store implementation.  If it's already
		 * on the screen, don't put it there again
		 */
		if (memcmp(b_row, row, p->width) == 0)
			continue;

        	/* else, write out the entire row */
		memcpy(b_row, row, p->width);
		int pos = i * p->width;
		snprintf(out, sizeof(out), "%c%c%c", 0x1B, 'H', pos);
		write(p->fd, out, 3);
		write(p->fd, row, p->width);
	}

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
NoritakeVFD_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
NoritakeVFD_string (Driver *drvthis, int x, int y, char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;
	for (i = 0; string[i] != '\0'; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
}
/*
/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
static void
NoritakeVFD_heartbeat (Driver *drvthis, int type)
{
	PrivateData *p = drvthis->private_data;
	static int timer = 0;
	int whichIcon;
	static int saved_type = HEARTBEAT_ON;

	NoritakeVFD_icon(drvthis, 0, 0);
	NoritakeVFD_icon(drvthis, 1, 1);

	if (type)
		saved_type = type;

	if (type == HEARTBEAT_ON) {
		// Set this to pulsate like a real heart beat...
		whichIcon = (! ((timer + 4) & 5));

		// Put character on screen...
		NoritakeVFD_chr(p->width, 1, whichIcon);

		// change display...
		NoritakeVFD_flush ();
	}

	timer++;
	timer &= 0x0f;
}
*/
