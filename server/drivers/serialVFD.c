/** \file server/drivers/serialVFD.c
 * LCDd \c serialVFD driver for various serial (& parallel) VFD devices.
 *
 * The driver should operate most of NEC, Futaba and Noritake 7x5 dot VFDs
 * with serial(rs232) and/or parallel interface. See
 * /docs/lcdproc-user/serialvfd-howto.html for further information.
 *
 * This driver consists of three parts:
 * \li This file \c serialVFD.c implements most the driver API.
 * \li \c serialVFD_io.c implements low-level I/O functions for serial and
 * parallel ports.
 * \li \c serialVFD_displays.c initializes display-specific character mappings
 * and command sequences.
 */

/*-
 * Copyright (C) 2006 Stefan Herdler
 *
 * This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
 * driver. It may contain parts of other drivers of this package too.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * 2006-05-16 Version 0.3: everything should work (not all hardware tested!)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#include "lcd.h"
#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"
#include "serialVFD.h"
#include "serialVFD_displays.h"
#include "serialVFD_io.h"

/*
 * This file uses __FUNCTION__ in debug() and drvthis->name in report() calls.
 */

#define pos1_cursor	4 //moves cursor to top left character.
#define mv_cursor	5 //moves cursor to position specified by the next byte.
#define reset		6 //reset
#define init_cmds	7 //commands needed to initialize the display.
#define set_user_char	8 //set user character.
#define hor_tab		9 //moves cursor 1 chr right
#define next_line	10 //moves cursor to the first character of the next line (= LF + CR)

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "serialVFD_";

/* Internal functions */
static void serialVFD_init_vbar (Driver *drvthis);
static void serialVFD_init_hbar (Driver *drvthis);
static void serialVFD_put_char (Driver *drvthis, int n);
static void serialVFD_hw_write (Driver *drvthis, int i);


/**
 * Initialize the driver.
 * Open com port and set baud correctly...
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
serialVFD_init (Driver *drvthis)
{
	int tmp, w, h;
	char size[200] = DEFAULT_SIZE;

	PrivateData *p;

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis);

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;
	p->ccmode = standard;
	p->ISO_8859_1 = 1;
	p->refresh_timer = 480;
	p->hw_brightness = 0;
	p->para_wait = DEFAULT_PARA_WAIT;
	p->hw_cmd[next_line][0] = 0;	/* disable line / set normal mode */

	/* Connection type: parallel or serial? */
	p->use_parallel	= drvthis->config_get_bool(drvthis->name, "use_parallel", 0, 0);

	/* Which device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	if (p->use_parallel) {
		p->port	= drvthis->config_get_int(drvthis->name, "port", 0, DEFAULT_LPTPORT);
	}
	else {
		/* Which speed */
		tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
		if ((tmp != 1200) && (tmp != 2400) && (tmp != 9600) && (tmp != 19200) && (tmp != 115200)) {
			report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200. Using default %d",
				drvthis->name, DEFAULT_SPEED);
			tmp = DEFAULT_SPEED;
		}
		if (tmp == 1200) p->speed = B1200;
		else if (tmp == 2400) p->speed = B2400;
		else if (tmp == 9600) p->speed = B9600;
		else if (tmp == 19200) p->speed = B19200;
		else if (tmp == 115200) p->speed = B115200;
	}

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot parse size: %s; using default %s",
			drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_ON_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_ON_BRIGHTNESS);
		tmp = DEFAULT_ON_BRIGHTNESS;
	}
	p->on_brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFF_BRIGHTNESS);
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_OFF_BRIGHTNESS);
		tmp = DEFAULT_OFF_BRIGHTNESS;
	}
	p->off_brightness = tmp;

	/* ISO 8859 1 */
	p->ISO_8859_1 = drvthis->config_get_bool(drvthis->name, "ISO_8859_1", 0, 1);

	/* Which displaytype */
	p->display_type = drvthis->config_get_int(drvthis->name, "Type", 0, DEFAULT_DISPLAYTYPE);

	/* Number of custom characters */
	tmp = drvthis->config_get_int(drvthis->name, "Custom-Characters", 0, CC_UNSET);
	if ((tmp < 0) || (tmp > 99)) {
		report(RPT_WARNING, "%s: The number of Custom-Characters must be between 0 and 99. Using default",
			drvthis->name, 0);
		tmp = CC_UNSET;
	}
	p->customchars = tmp;

	/* Do connection type specific io-port init */
	if (Port_Function[p->use_parallel].init_fkt(drvthis) == -1) {
		report(RPT_ERR, "%s: unable to initialize io-port", drvthis->name);
		return -1;
	}

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
	memset(p->backingstore, 0, p->width * p->height);

	/* setup displayspecific data */
	memset(p->usr_chr_mapping, 0, 31);	/* no character mapping by default */
	memset(p->usr_chr_load_mapping, 0, 31);
	if (serialVFD_load_display_data(drvthis) != 0) {
		report(RPT_WARNING, "%s: Type %d not defined; using default %d",
			drvthis->name, p->display_type, DEFAULT_DISPLAYTYPE);
		p->display_type = DEFAULT_DISPLAYTYPE;
		if (serialVFD_load_display_data(drvthis) != 0) {
			report(RPT_ERR, "%s: unable to load display_data", drvthis->name);
			return -1;
		}
	}

	/* Things to set up after loading display specific data */

	/* parallel port wait, overwrites value set by display specific data */
	tmp = p->para_wait;
	p->para_wait = drvthis->config_get_int(drvthis->name, "PortWait", 0, p->para_wait);

	/* load user defined character mapping table */
	if ((p->usr_chr_load_mapping[0] == 0) && (p->usr_chr_load_mapping[1] == 0)){ //this should not happen if usr_chr_load_mapping had been set
		memcpy(p->usr_chr_load_mapping, p->usr_chr_mapping, 31);
	}

	/* initialise display */
	Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[reset][1],p->hw_cmd[reset][0]);
	Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);
	serialVFD_backlight(drvthis, 1);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialVFD_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p != NULL) {
		Port_Function[p->use_parallel].close_fkt(drvthis);
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
serialVFD_width (Driver *drvthis)
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
serialVFD_height (Driver *drvthis)
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
serialVFD_cellwidth (Driver *drvthis)
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
serialVFD_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialVFD_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);

}


/**
 * Flush data on screen to the VFD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialVFD_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i, j, last_chr = -10;
	char custom_char_changed[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


	for (i = 0; i < p->customchars; i++) {
		for (j = 0; j < p->usr_chr_dot_assignment[0]; j++) {
			if (p->custom_char[i][j] != p->custom_char_store[i][j]) {
				custom_char_changed[i] = 1;
			}
			p->custom_char_store[i][j] = p->custom_char[i][j];
		}
	}

	/* Do a full refresh every 500 refreshs. */
	if (p->refresh_timer > 500) {
		/* With this it is possible to switch the display on and off
		 * while LCDd is running. */
		Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);

		Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[p->hw_brightness][1],\
		p->hw_cmd[p->hw_brightness][0]);	/* restore brightness */

		memset(p->backingstore, 0, p->width * p->height);	/* clear Backing-store */

		for (i = 0; i < p->customchars; i++)	/* refresh all customcharacters */
			custom_char_changed[i] = 1;
		p->refresh_timer = 0;
	}

	p->refresh_timer++;

	if (p->display_type == 1) {	/* KD Rev 2.1 only */
		if (custom_char_changed[p->last_custom])
			p->last_custom = -10;
	}
	else {			/* other Displays */
		for (i = 0; i < p->customchars; i++)	/* set customcharacters */
			if (custom_char_changed[i])
				serialVFD_put_char(drvthis, i);
	}

	/* normal mode Display */
	if (p->hw_cmd[next_line][0] == 0) {
		/* Workaround for Displays that doesn't support mv_cursor
		 * command */
		if (p->hw_cmd[mv_cursor][0] == 0) {
			Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[pos1_cursor][1], p->hw_cmd[pos1_cursor][0]);
			last_chr = -1;
		}

		for (i = 0; i < (p->height * p->width); i++) {
			/*
			 * Backing-store implementation.  If it's already
			 * on the screen, don't put it there again.
			 */
			if ((p->framebuf[i] != p->backingstore[i]) || (p->hw_cmd[hor_tab][0] == 0) ||
			((p->framebuf[i] <= 30) && (custom_char_changed[(int)p->framebuf[i]] != 0))) {
				if (last_chr < i-1) {	/* if not last char written cursor has to be moved. */
					if (((p->hw_cmd[hor_tab][0] * (i-1-last_chr)) > (p->hw_cmd[mv_cursor][0]+1)) && (p->hw_cmd[mv_cursor][0] != 0)) {
						Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[mv_cursor][1],
							p->hw_cmd[mv_cursor][0]);
						Port_Function[p->use_parallel].write_fkt(drvthis, (unsigned char *) &i, 1);
						debug(RPT_DEBUG, "%s: move  %d", __FUNCTION__, i);
					}
					else {
						for (j = last_chr; j < (i-1); j++)
							Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[hor_tab][1], p->hw_cmd[hor_tab][0]);
						debug(RPT_DEBUG, "%s: TAB  %d", __FUNCTION__, j-last_chr);
					}
				}
				serialVFD_hw_write(drvthis, i);
				last_chr = i;
			}
		}
	}
	/* line mode Display (partitially borrowed from serialPOS.c) */
	else {
		for (j = 0; j < p->height; j++) {
			/* set pointers to start of the line in frame buffer
			 * & backing store */
			unsigned char *sp = p->framebuf + (j * p->width);
			unsigned char *sq = p->backingstore + (j * p->width);
			/* Move to start of line */
			if (j == 0) {
				Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[pos1_cursor][1], p->hw_cmd[pos1_cursor][0]);
			}
			else {
				Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[next_line][1], p->hw_cmd[next_line][0]);
			}
			/* skip over identical lines */
			if (memcmp(sp, sq, p->width) == 0) {
				continue;
			}
			/* write the data */
			for (i = 0; i < p->width; i++) {
				serialVFD_hw_write(drvthis, (i + (j * p->width)));
			}
			last_chr = 10;
		}
	}

	/* update backingstore if something changed */
	if (last_chr >= 0) {
		memcpy(p->backingstore, p->framebuf, p->height * p->width);
		debug(RPT_DEBUG, "%s: memcpy", __FUNCTION__);
	}
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
serialVFD_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;
	for (i = 0; string[i] != '\0'; i++) {
		/* Check for buffer overflows... */
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
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
serialVFD_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	if ((x > p->width) || (y > p->height))
		return;
	y--;
	x--;

	p->framebuf[(y * p->width) + x ] = c;
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
serialVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->customchars >= p->cellheight || p->predefined_vbar == 1) {
		serialVFD_init_vbar(drvthis);
		lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, p->vbar_cc_offset);
	}
	else {
		lib_vbar_static(drvthis, x, y, len, promille, options, 2, 0x5E);
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
serialVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->customchars >= p->cellwidth || p->predefined_hbar == 1) {
		serialVFD_init_hbar(drvthis);
		lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, p->hbar_cc_offset);
	}
	else {
		lib_hbar_static(drvthis, x, y, len, promille, options, 2, 0x2C);
	}
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
serialVFD_num(Driver * drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if (p->ccmode != bignum) {
		/* If custom characters are not yet set up Lib_adv_bignum has
		 * to do it. */
		do_init = 1;
		p->ccmode = bignum;
	}
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
serialVFD_icon (Driver *drvthis, int x, int y, int icon)
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

	switch (icon) {
		case ICON_BLOCK_FILLED:
			serialVFD_chr(drvthis, x, y, 127);
			break;
		case ICON_HEART_FILLED:
			if (p->customchars > 0) {
		        	p->ccmode = standard;
				serialVFD_set_char(drvthis, 0, heart_filled);
				serialVFD_chr(drvthis, x, y, 0);
			}
			else
				serialVFD_icon(drvthis, x, y, ICON_BLOCK_FILLED);
			break;
		case ICON_HEART_OPEN:
			if (p->customchars > 0) {
				p->ccmode = standard;
				serialVFD_set_char(drvthis, 0, heart_open);
				serialVFD_chr(drvthis, x, y, 0);
			}
			else
				serialVFD_chr(drvthis, x, y, 0x23);
			break;
		default:
			return -1;	/* Let the core do other icons */
	}
	return 0;
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters.
 */
MODULE_EXPORT int
serialVFD_get_free_chars (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->customchars;
}


/**
 * Define a custom character and write it to the VFD.
 *
 * Converts the 5x7 matrix of bits stored in 7 bytes into a VFD specific
 * byte sequence.
 *
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (p->customchars)].
 * \param dat      Array of 7(=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
serialVFD_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned int byte, bit;

	if (n < 0 || n > p->customchars-1)
		return;
	if (!dat)
		return;

	for (byte = 0; byte < p->usr_chr_dot_assignment[0]; byte++) {
		int letter = 0;

		for (bit = 0; bit < 8; bit++) {
			/* map bit from 5x7 matrix into current byte/bit */
			int pos = (int) p->usr_chr_dot_assignment[bit+8*byte+1];

			if (pos > 0) {
				int posbyte = (pos-1) / 5;
				int posbit = 4 - ((pos-1) % 5);

				letter |= ((dat[posbyte] >> posbit) & 1) << bit;
			}
		}
		p->custom_char[n][byte] = letter;
	}

}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return         Stored brightness in promille.
 */
MODULE_EXPORT int
serialVFD_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->on_brightness : p->off_brightness;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
 */
MODULE_EXPORT void
serialVFD_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	if (state == BACKLIGHT_ON) {
		p->on_brightness = promille;
	}
	else {
		p->off_brightness = promille;
	}
}


/**
 * Turn display backlight on or off.
 * This does not really toggle the backlight, but  tries to handle it more intelligently:
 * it sets the brightness to the value defined for the related backlight state.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
serialVFD_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->on_brightness
			     : p->off_brightness;

	/*
	 * map range [0, 1000] -> [0, 4] that the hardware understands
	 * (4 steps 0-250, 251-500, 501-750, 751-1000)
	 */
	hardware_value /= 251;
	if (hardware_value != p->hw_brightness) {
		p->hw_brightness = hardware_value;
		Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[p->hw_brightness][1],\
		p->hw_cmd[p->hw_brightness][0]);
	}
}


/**
 * Provide general information about the LCD/VFD display/driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
serialVFD_get_info (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	strcpy(p->info, "Driver for many serialVFDs from NEC(all FIPC based), Noritake, Futaba and the \"KD Rev2.1\"VFD");
	return p->info;
}


/**
 * Set up vertical bars.
 * \param drvthis  Pointer to driver structure.
 *
 * \todo
 * Called only by vbar(). Can possibly be included there.
 * This should also make CCMODE independent of the implementation of the bars.
 */
static void
serialVFD_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			/* add pixel line per pixel line ... */
			vBar[p->cellheight - i] = 0xFF;
			serialVFD_set_char(drvthis, i, vBar);
		}
	}
}


/**
 * Set up horizontal bars.
 * \param drvthis  Pointer to driver structure.
 *
 * \todo
 * Called only by hbar(). Can possibly be included there.
 * This should also make CCMODE independent of the implementation of the bars.
 */
static void
serialVFD_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		p->ccmode = hbar;

		for (i = 1; i < p->cellwidth; i++) {
			/* fill pixel columns from left to right. */
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			serialVFD_set_char(drvthis, i, hBar);
		}
	}
}


/**
 * Put a custom character in the display's RAM. The character definition is
 * read from custom_char cache.
 * \param  drvthis  Pointer to driver
 * \param  n        Index of the custom character in cache
 */
static void
serialVFD_put_char (Driver *drvthis, int n)
{
	PrivateData *p = drvthis->private_data;

	Port_Function[p->use_parallel].write_fkt(drvthis, &p->hw_cmd[set_user_char][1],\
		p->hw_cmd[set_user_char][0]);
	Port_Function[p->use_parallel].write_fkt(drvthis, (unsigned char *) &p->usr_chr_load_mapping[n], 1);
	Port_Function[p->use_parallel].write_fkt(drvthis, &p->custom_char[n][0], p->usr_chr_dot_assignment[0]);// overwrite selected Character
}


/**
 * Finally write character/usercharacter on the display.
 * Written characters in area 128...255 are mapped according to charmap if
 * ISO_8859_1 is enabled.
 * \param  drvthis  Pointer to driver
 * \param  i        Index of character in framebuffer to write
 *
 */
static void
serialVFD_hw_write (Driver *drvthis, int i)
{
	PrivateData *p = drvthis->private_data;

	if (p->framebuf[i] <= 30) {	/* custom character */
		if (p->display_type == 1) {	/* KD Rev 2.1 only */
			if (p->last_custom != p->framebuf[i]) {
				/* substitute and select character to overwrite (237) */
				Port_Function[p->use_parallel].write_fkt(drvthis, (unsigned char *)"\x1A\xDB", 2);
				/* overwrite selected character */
				Port_Function[p->use_parallel].write_fkt(drvthis, &p->custom_char[(int)p->framebuf[i]][0], 7);
			}
			/* write character */
			Port_Function[p->use_parallel].write_fkt(drvthis, (unsigned char *)"\xDB", 1);
			p->last_custom = p->framebuf[i];
		}
		else {		/* all other displays */
			Port_Function[p->use_parallel].write_fkt(drvthis, (unsigned char *) &p->usr_chr_mapping[(int)p->framebuf[i]], 1);
		}
	}
	else if ((p->framebuf[i] == 127) || ((p->framebuf[i] > 127) && (p->ISO_8859_1 != 0))) {
		/* ISO_8859_1 translation for 129 ... 255 */
		Port_Function[p->use_parallel].write_fkt(drvthis, &p->charmap[p->framebuf[i] - 127], 1);
	}
	else {
		Port_Function[p->use_parallel].write_fkt(drvthis, &p->framebuf[i], 1);
	}
}

