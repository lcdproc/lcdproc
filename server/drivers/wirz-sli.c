/** \file server/drivers/wirz-sli.c
 * LCDd \c sli driver for the SLI devices by Wirz, http://www.wirz.com/.
 */

/*	wirz-sli.c -- Source file for LCDproc Wirz SLI driver
	Copyright (C) 1999 Horizon Technologies-http://horizon.pair.com/
	Written by Bryan Rittmeyer <bryanr@pair.com> - Released under GPL

        LCD info: http://www.wirz.com/                               */

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
#include "wirz-sli.h"
#include "report.h"
#include "lcd_lib.h"

#define SLI_DEFAULT_DEVICE	"/dev/lcd"

/** private data for the \c sli driver */
typedef struct sli_private_data {
	char device[256];
	int speed;
	int fd;
	unsigned char *framebuf;
	int width;
	int height;
	int cellwidth;
	int cellheight;
	CGmode custom;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "sli_";


/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
sli_init (Driver *drvthis)
{
	PrivateData *p;
	struct termios portset;
	char out[2];

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->custom = standard;
	p->fd = -1;
	p->framebuf = NULL;
	p->width = 16;
	p->height = 2;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	/* Read config file */

	/* What device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   SLI_DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* What speed to use */
	p->speed = drvthis->config_get_int(drvthis->name, "Speed", 0, 19200);

	if (p->speed == 1200)        p->speed = B1200;
	else if (p->speed == 2400)   p->speed = B2400;
	else if (p->speed == 9600)   p->speed = B9600;
	else if (p->speed == 19200)  p->speed = B19200;
	else if (p->speed == 38400)  p->speed = B38400;
	else if (p->speed == 57600)  p->speed = B57600;
	else if (p->speed == 115200) p->speed = B115200;
	else {
		report(RPT_WARNING, "%s: illegal Speed: %d; must be one of 1200, 2400, 9600, 19200, 38400, 57600, or 115200; using default %d",
				drvthis->name, p->speed, 19200);
		p->speed = B19200;
	}

	/* End of config file parsing */

	// Set up io port correctly, and open it...
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}
	report(RPT_DEBUG, "%s: opened device %s", drvthis->name, p->device);

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
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	// Set port speed
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, B0);

	// Do it...
	tcsetattr(p->fd, TCSANOW, &portset);

	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* Initialize SLI using autobaud detection, and then turn off cursor
	   and clear screen */
	usleep(150000);			  /* 150ms delay to allow SLI to power on */
	out[0] = 13;			  /* CR for SLI autobaud */
	write(p->fd, out, 1);
	usleep(3000);			  /* 3ms delay.. wait for it to autobaud */
	out[0] = 0x0FE;
	out[1] = 0x00C;			  /* No cursor */
	write(p->fd, out, 2);
	out[0] = 0x0FE;
	out[1] = 0x001;			  /* Clear LCD, not sure if this belongs here */
	write(p->fd, out, 2);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

/////////////////////////////////////////////////////////////////
// Clean up
//
MODULE_EXPORT void
sli_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if ( p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf != NULL)
			free(p->framebuf);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
sli_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
sli_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell width
//
MODULE_EXPORT int
sli_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell height
//
MODULE_EXPORT int
sli_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

/////////////////////////////////////////////////////////////////
// Flush framebuffer to LCD
//
MODULE_EXPORT void
sli_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[2];

	/* Do the actual refresh */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write(p->fd, out, 2);
	write(p->fd, &p->framebuf[0], p->width);
	usleep(10);
	write(p->fd, p->framebuf + p->width -1, p->width);
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
sli_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
sli_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;				  // Convert 1-based coords to 0-based...
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)
			p->framebuf[(y * p->width) + x] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
sli_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before sli->vbar()
//

/* Because of the way we do this (custom characters in CGRAM)
   we can't have both horizontal and vertical bars at once...
   this also appears to be a limitation of the Matrix Orbital
   modules so I will assume that all client coders know about it

   I think it would be cool to use triangular stuff for the non-full
   characters, so that you can do both bar types at once.. maybe I
   will release a new version of the SLI driver that attempts this */

static void
sli_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	static unsigned char vbar_char[7][8] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
		{0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F},
		{0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
		{0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
		{0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
	};

	if (p->custom != vbar) {
		int i;
		for (i = 0; i < 7; i++)
			sli_set_char(drvthis, i + 1, vbar_char[i]);
		p->custom = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
sli_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	static unsigned char hbar_char[4][8] = {
		{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
		{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
		{0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
		{0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
	};

	if (p->custom != hbar) {
		int i;
		for (i = 0; i < 4; i++)
			sli_set_char(drvthis, i + 1, hbar_char[i]);
		p->custom = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//

/* It would be cool if you could start a vertical bar at any y
   like the horizontal bar routine can start at any x... but
   since we only have 2 lines anyway this is rather pointless
   for me to add                                               */

MODULE_EXPORT void
sli_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	sli_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
sli_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	sli_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
sli_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	char out[2];
	int row;
	unsigned char mask = (1 << p->cellwidth) - 1;

	/* SLI also has 8 user definable characters */
	if ((n < 0) || (n > 7) || (!dat))
		return;

	/* Move cursor to CGRAM */
	out[0] = 0x0FE;
	out[1] = 0x040 + 8 * n;
	write(p->fd, out, 2);

	for (row = 0; row < p->cellheight; row++) {
		int letter = dat[row] & mask;

		letter |= 0x020;	  /* SLI can't accept CR, LF, etc in this character! */
		write(p->fd, &letter, 1);
	}

	/* Move cursor back to DDRAM */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write(p->fd, out, 2);
}

MODULE_EXPORT int
sli_icon (Driver *drvthis, int x, int y, int icon)
{
	static unsigned char icons[2][8] = {
		{0x1F, 0x15, 0x00, 0x00, 0x00, 0x11, 0x1B, 0x1F},	/* Empty Heart */
		{0x1F, 0x15, 0x0A, 0x0E, 0x0E, 0x15, 0x1B, 0x1F}	/* Filled Heart */
	};

	switch (icon) {
		case ICON_BLOCK_FILLED:
			sli_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
			sli_set_char( drvthis, 0, icons[1]);
			sli_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			sli_set_char( drvthis, 0, icons[0]);
			sli_chr(drvthis, x, y, 0);
			break;
		default:
			return -1;
	}
	return 0;
}

