/*  This is the LCDproc driver for tyan lcd module (tyan Barebone GS series)
    
    Author: yhlu@tyan.com

    Copyright (C) 2004 Tyan Corp

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
 * 02/04/2004: Working driver
 *
 *
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
#include "tyan_lcdm.h"
#include "report.h"
#include "lcd_lib.h"

#define TYAN_LCDM_KEY_ENTER    0xF2
#define TYAN_LCDM_KEY_ESCAPE   0xF3
#define TYAN_LCDM_KEY_RIGHT    0xF5
#define TYAN_LCDM_KEY_LEFT     0xF6
#define TYAN_LCDM_KEY_UP       0xF7
#define TYAN_LCDM_KEY_DOWN     0xF8

#define TYAN_LCDM_CMD_BEGIN 0xF1
#define TYAN_LCDM_CMD_END 0xF2



typedef enum {
	normal = 0,
	hbar = 1,
	vbar = 2,
	cust = 3,
} custom_type;

typedef struct driver_private_data {
	char device[200];
	int speed;
	int fd;
	unsigned char *framebuf;
	unsigned char *backingstore;
	int width;
	int height;
	int cellwidth;
	int cellheight;
	custom_type custom;
} PrivateData;	

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "tyan_lcdm_";

/* Internal functions */
static void tyan_lcdm_init_vbar(Driver *drvthis);
static void tyan_lcdm_init_hbar(Driver *drvthis);

static void tyan_lcdm_switch_mode(int fdfd);
static void tyan_lcdm_hardware_clear(int fd);

static void tyan_lcdm_set_rampos(int fd, unsigned char pos);
static void tyan_lcdm_write_str(int fd, unsigned char *str, unsigned char start_addr, int length);
#if 0   	 
static void tyan_lcdm_set_cursor(int fd, unsigned char start_addr, int pos);
#endif
static unsigned char tyan_lcdm_read_key(int fd);

/*
 * Opens com port and sets baud correctly...
 */
MODULE_EXPORT int
tyan_lcdm_init (Driver * drvthis, char *args)
{
	PrivateData *p;
	struct termios portset;
	char size[200] = DEFAULT_SIZE;
	int tmp, w, h;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->speed = DEFAULT_SPEED;
	p->custom = normal;
	p->fd = -1;
	p->framebuf = NULL;
	p->backingstore = NULL;
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;

	debug(RPT_INFO, "tyan_lcdm: init(%p,%s)", drvthis, args);

	/* Read config file */
	/* Which serial device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	debug(RPT_INFO,"%s: using Device %s", drvthis->name, p->device);

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size , "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE , "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name , "Speed", 0, DEFAULT_SPEED);
	if (tmp == 4800) p->speed = B4800;
	else if (tmp == 9600) p->speed = B9600;
	else {
		report(RPT_WARNING, "%s: Speed must be 4800 or 9600; using default %d", 
			drvthis->name, DEFAULT_SPEED);
		p->speed = 9600;
	}

	/* Set up io port correctly, and open it... */
	debug(RPT_DEBUG, "tyan_lcdm: Opening serial device: %s", p->device);
    
        p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
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
	portset.c_cflag = CS8 | CREAD | CLOCAL;
	portset.c_iflag = IXON | IXOFF | IGNBRK | IGNCR;
	portset.c_oflag &= ~ONLCR;
	portset.c_lflag = 0;
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 0;
#endif

	/* Set port speed */
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, p->speed);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	p->backingstore = (unsigned char *) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->width * p->height);
	
	/* Set display-specific stuff.. */
	tyan_lcdm_switch_mode(p->fd);
	tyan_lcdm_hardware_clear(p->fd);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}

/*
 * Clean-up
 */
MODULE_EXPORT void
tyan_lcdm_close (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf != NULL)
			free(p->framebuf);

		if (p->backingstore != NULL)
			free(p->backingstore);

	free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/*
 * Returns the display width
 */
MODULE_EXPORT int
tyan_lcdm_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/*
 * Returns the display height
 */
MODULE_EXPORT int
tyan_lcdm_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/*
 * Flushes all output to the lcd...
 */
MODULE_EXPORT void
tyan_lcdm_flush (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;
	unsigned char *xp = p->framebuf;
	unsigned char *xq = p->backingstore;
/*
 * We don't use delta update yet.
 * It is possible but not easy, we can only update a line, full or begining.
 */

	for (i = 0; i < p->width; i++) {
		if (*xp != *xq) {
			tyan_lcdm_write_str(p->fd, p->framebuf, 0x80, 16);
			memcpy(p->backingstore, p->framebuf, p->width);
			break;
		}
		xp++; xq++;
	}

	xp = p->framebuf + p->width;
	xq = p->backingstore + p->width;

	for (i = 0; i < p->width; i++) {
		if (*xp != *xq) {
			tyan_lcdm_write_str(p->fd, p->framebuf + p->width, 0xc0, 16);
			memcpy(p->backingstore + p->width, p->framebuf + p->width, p->width);
			break;
		}
		xp++; xq++;
	}
}

/*
 * Return one char from the KeyRing
 */
MODULE_EXPORT const char *
tyan_lcdm_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

        unsigned char key = tyan_lcdm_read_key(p->fd);

        switch (key) {
                case TYAN_LCDM_KEY_LEFT:
                        return "Left";
                        break;
                case TYAN_LCDM_KEY_UP:
                        return "Up";
                        break;
                case TYAN_LCDM_KEY_DOWN:
                        return "Down";
                        break;
                case TYAN_LCDM_KEY_RIGHT:
                        return "Right";
                        break;
                case TYAN_LCDM_KEY_ENTER:
                        return "Enter"; 
                        break;
                case TYAN_LCDM_KEY_ESCAPE:
                        return "Escape";
                        break;
                default:
                        report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
                        return NULL;
                        break;
        }
}


/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_chr (Driver * drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}

/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 * Need to find out if we have support for intermediate value.
 */
MODULE_EXPORT void
tyan_lcdm_backlight (Driver * drvthis, int on)
{
}


/*
 * Sets up for vertical bars.
 */
static void
tyan_lcdm_init_vbar (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->custom != vbar) {
		tyan_lcdm_set_char(drvthis, 1, a);
		tyan_lcdm_set_char(drvthis, 2, b);
		tyan_lcdm_set_char(drvthis, 3, c);
		tyan_lcdm_set_char(drvthis, 4, d);
		tyan_lcdm_set_char(drvthis, 5, e);
		tyan_lcdm_set_char(drvthis, 6, f);
		tyan_lcdm_set_char(drvthis, 7, g);
		p->custom = vbar;
	}
}

/*
 * Inits horizontal bars...
 */
static void
tyan_lcdm_init_hbar (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->custom != hbar) {
		tyan_lcdm_set_char(drvthis, 1, a);
		tyan_lcdm_set_char(drvthis, 2, b);
		tyan_lcdm_set_char(drvthis, 3, c);
		tyan_lcdm_set_char(drvthis, 4, d);
		tyan_lcdm_set_char(drvthis, 5, e);
		tyan_lcdm_set_char(drvthis, 6, f);
		p->custom = hbar;
	}
}


/*
 * Draws a vertical bar...
 */
MODULE_EXPORT void
tyan_lcdm_vbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'up' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */
	PrivateData *p = drvthis->private_data;

	tyan_lcdm_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/*
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
tyan_lcdm_hbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'right' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */
	PrivateData *p = drvthis->private_data;

	tyan_lcdm_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*
 * Writes a big number.
 * This is not supported on 633 because we only have 2 lines...
 */
MODULE_EXPORT void
tyan_lcdm_num (Driver * drvthis, int x, int num)
{
	//PrivateData *p = drvthis->private_data;
/*
	char out[5];

	snprintf(out, sizeof(out), "%c%c%c", 28, x, num);
	write(fd, out, 3);
*/
}

/*
 * Sets a custom character from 0-7...
 *
 * For input, values > 0 mean "on" and values <= 0 are "off".
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
tyan_lcdm_set_char (Driver * drvthis, int n, char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[8];
	int row, col;

	if ((n < 0) || (n > 7))
		return;
	if (!dat)
		return;

	for (row = 0; row < p->cellheight; row++) {
		int letter = 0;

		for (col = 0; col < p->cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * p->cellwidth) + col] > 0);
			/* I should remove that debug code. */
			//if (dat[(row * cellheight) + col] == 0) printf(".");
			//if (dat[(row * cellheight) + col] == 1) printf("+");
			//if (dat[(row * cellheight) + col] == 2) printf("x");
			//if (dat[(row * cellheight) + col] == 3) printf("*");
			//printf("'%1d'", dat[(row * cellwidth) + col]);
			//printf("%3d ", letter);
		}
		out[row+1] = letter;
		//printf(": %d\n", letter);
	}
	tyan_lcdm_write_str(p->fd, out, (unsigned char) (0x40 + n * 8), 8);
}

/*
 * Places an icon on screen
 */
MODULE_EXPORT int
tyan_lcdm_icon (Driver * drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char icons[8][5 * 8] = {
	/* Empty Heart */
		{
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },
	/* Filled Heart */
		{
		 1, 1, 1, 1, 1,		  
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },
	/* arrow_up */
		{
		 0, 0, 1, 0, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0,
		 },
	/* arrow_down */
		{
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_off */
		{
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_on */
		{
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 1, 1, 0, 1,
		 1, 0, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_gray */
		{
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	 /* Ellipsis */
		{
		 0, 0, 0, 0, 0,		 
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 1, 0, 1, 0,
		 },
	};

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			tyan_lcdm_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 0, icons[1]);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 0, icons[0]);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 1, icons[2]);
			tyan_lcdm_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 2, icons[3]);
			tyan_lcdm_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_LEFT:
			tyan_lcdm_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			tyan_lcdm_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 3, icons[4]);
			tyan_lcdm_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 4, icons[5]);
			tyan_lcdm_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
		        p->custom = cust;
			tyan_lcdm_set_char(drvthis, 5, icons[6]);
			tyan_lcdm_chr(drvthis, x, y, 5);
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
tyan_lcdm_clear (Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_string (Driver * drvthis, int x, int y, char string[])
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


/*
 * switch mode()
 */
static void
tyan_lcdm_switch_mode(int fd)
{
	char lcdcmd1[4] = { TYAN_LCDM_CMD_BEGIN, 0x73, 0x01, TYAN_LCDM_CMD_END };
	char lcdcmd2[3] = { TYAN_LCDM_CMD_BEGIN, 0x6c, TYAN_LCDM_CMD_END };

	//set os selection
	write(fd, lcdcmd1, 4);
	sleep(1);

	//send "LCD Ready" cmd
	write(fd, lcdcmd2, 3);
	sleep(1);
}


/*
 * Hardware clears the LCD screen
 */
static void
tyan_lcdm_hardware_clear(int fd)
{
	char lcdcmd[5] = { TYAN_LCDM_CMD_BEGIN, 0x70, 0x00, 0x01, TYAN_LCDM_CMD_END};

	//set os selection
	write(fd, lcdcmd, 5);
}


static 
void tyan_lcdm_set_rampos(int fd, unsigned char pos)
{
	char cmd_str[5] = { TYAN_LCDM_CMD_BEGIN, 0x70, 0x00, 0x00, TYAN_LCDM_CMD_END };

	cmd_str[3] = pos;
	write(fd, cmd_str, 5);
}


static 
void tyan_lcdm_write_str(int fd, unsigned char *str,unsigned char start_addr, int length)
{
//CGRAM 0x40, 0x48,....	
//if Line 1: start_addr = 0x80
//if Line 2: start_addr = 0xc0
// 1<= length <=16
	unsigned char cmd_str[20] = { TYAN_LCDM_CMD_BEGIN, 0x70, 0x02 };
	
	tyan_lcdm_set_rampos(fd, start_addr);
	memset(cmd_str, ' ', 20);
	cmd_str[0] = TYAN_LCDM_CMD_BEGIN;
	cmd_str[1] = 0x70;
	cmd_str[2] = 0x02;
	cmd_str[19] = TYAN_LCDM_CMD_END;
	memcpy(cmd_str+3, str, (length < 16) ? length : 16);
        write(fd, cmd_str, 20);
}      

#if 0   	 
static 
void tyan_lcdm_set_cursor(int fd, unsigned char start_addr, int pos)
{
	char cmd_str[5] = { TYAN_LCDM_CMD_BEGIN, 0x70, 0x00, 0x0e, TYAN_LCDM_CMD_END };	

	tyan_lcdm_set_rampos(pos+start_addr); 
	write(fd,cmd_str,5);       
}	     
#endif

static 
unsigned char tyan_lcdm_read_key(int fd)
{
	int count = 0;	
	char key_str[4];

	memset(key_str, '\0', 4);
	count = read(fd, key_str, 4);
	if ((count == 4)
	    && (key_str[0] == (char) TYAN_LCDM_CMD_BEGIN)
	    && (key_str[1] == (char) 0x72)
	    && (key_str[3] == (char) TYAN_LCDM_CMD_END)) {
		return key_str[2];
	}
	return 0xF4;  //error
}
