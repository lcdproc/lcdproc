/** \file server/drivers/tyan_lcdm.c
 * LCDd \c tyan driver for the CLD module found in Tyan GS series barebones.
 */

/*
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */

/*
 * See ftp://ftp.tyan.com/barebone_support/GS10-GS12%20LCD-Pack.zip
 * for documentation */


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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "tyan_lcdm.h"
#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"


/* Internal functions */
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
tyan_lcdm_init (Driver *drvthis, char *args)
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
	p->ccmode = standard;
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

	return 0;
}


/*
 * Clean-up
 */
MODULE_EXPORT void
tyan_lcdm_close (Driver *drvthis)
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
 * Returns the display's character cell width
 */
MODULE_EXPORT int
tyan_lcmd_cellwidth(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  return p->cellwidth;
}


/*
 * Returns the display's character cell height
 */
MODULE_EXPORT int
tyan_lcmd_cellheight(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  return p->cellheight;
}


/*
 * Flushes all output to the lcd...
 */
MODULE_EXPORT void
tyan_lcdm_flush (Driver *drvthis)
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

	report(RPT_DEBUG, "%s: get_key(): raw key code: 0x%02X", drvthis->name, key);

	switch (key) {
		case TYAN_LCDM_KEY_LEFT:
			return "Left";
			/* NOTREACHED */
		case TYAN_LCDM_KEY_UP:
			return "Up";
			/* NOTREACHED */
		case TYAN_LCDM_KEY_DOWN:
			return "Down";
			/* NOTREACHED */
		case TYAN_LCDM_KEY_RIGHT:
			return "Right";
			/* NOTREACHED */
		case TYAN_LCDM_KEY_ENTER:
			return "Enter";
			/* NOTREACHED */
		case TYAN_LCDM_KEY_ESCAPE:
			return "Escape";
			/* NOTREACHED */
		case 0xF4:
			report(RPT_WARNING, "%s: Error while reading key", drvthis->name);
			return NULL;
			/* NOTREACHED */
		default:
			report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
			return NULL;
			/* NOTREACHED */
	}
}


/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_chr (Driver *drvthis, int x, int y, char c)
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
tyan_lcdm_backlight (Driver *drvthis, int on)
{
}


/*
 * Draws a vertical bar...
 */
MODULE_EXPORT void
tyan_lcdm_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i] = 0xFF;
			tyan_lcdm_set_char(drvthis, i, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/*
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
tyan_lcdm_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}
		p->ccmode = hbar;

		for (i = 1; i <= p->cellwidth; i++) {
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			tyan_lcdm_set_char(drvthis, i, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*
 * Writes a big number.
 */
MODULE_EXPORT void
tyan_lcdm_num (Driver *drvthis, int x, int num)
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


/*
 * Gets number of custom chars (always NUM_CCs)
 */
MODULE_EXPORT int
tyan_lcdm_get_free_chars(Driver *drvthis)
{
//PrivateData *p = drvthis->private_data;

  return NUM_CCs;
}


/*
 * Sets a custom character from 0 - (NUM_CCs-1)...
 *
 * The input is an array of bytes, each representing a pixel row
 * starting from the top to bottom.
 * The bits in each byte represent the pixels where the LSB
 * (least significant bit) is the rightmost pixel in each pixel row
 */
MODULE_EXPORT void
tyan_lcdm_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char mask = (1 << p->cellwidth) - 1;
	unsigned char out[p->cellheight + 1];
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (dat == NULL)
		return;

	for (row = 0; row < p->cellheight; row++) {
		int letter = dat[row] & mask;

		if (p->cc[n].cache[row] != letter)
			p->cc[n].clean = 0;        /* only mark dirty if really different */
		p->cc[n].cache[row] = letter;

		out[row+1] = letter;
	}
	tyan_lcdm_write_str(p->fd, out, (unsigned char) (0x40 + n * 8), 8);
}

/*
 * Places an icon on screen
 */
MODULE_EXPORT int
tyan_lcdm_icon (Driver *drvthis, int x, int y, int icon)
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
			tyan_lcdm_chr(drvthis, x, y, 255);
			//tyan_lcdm_set_char(drvthis, 6, block_filled);
			//tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			tyan_lcdm_set_char(drvthis, 0, heart_open);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_FILLED:
			tyan_lcdm_set_char(drvthis, 0, heart_filled);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
			p->ccmode = custom;
			tyan_lcdm_set_char(drvthis, 1, arrow_up);
			tyan_lcdm_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
			p->ccmode = custom;
			tyan_lcdm_set_char(drvthis, 2, arrow_down);
			tyan_lcdm_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_LEFT:
			tyan_lcdm_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			tyan_lcdm_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
			p->ccmode = custom;
			tyan_lcdm_set_char(drvthis, 3, checkbox_off);
			tyan_lcdm_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			p->ccmode = custom;
			tyan_lcdm_set_char(drvthis, 4, checkbox_on);
			tyan_lcdm_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			p->ccmode = custom;
			tyan_lcdm_set_char(drvthis, 5, checkbox_gray);
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
tyan_lcdm_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
}


/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_string (Driver *drvthis, int x, int y, const char string[])
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


/* Note: this implementation works although it differs from the documentation
 * available at ftp://ftp.tyan.com/barebone_support/GS10-GS12%20LCD-Pack.zip
 * (i.e. with respect to this the docs are wrong) */
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

