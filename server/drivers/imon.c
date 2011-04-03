/** \file server/drivers/imon.c
 * LCDd \c imon driver for the Soundgraph iMON IR/VFD module.
 *
 * In order to be able to use it, you have to get and install one of
 * the following kernel modules:
 *  - standalone iMON VFD driver from http://venky.ws/projects/imon/
 *  - the iMON module included with LIRC ver. 0.7.1 or newer
 *    from http://www.lirc.org/
 */

/*-
 * Driver for Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD Module
 *
 * Copyright (c) 2004, Venky Raju <dev@venky.ws>, original author of
 * the LCDproc 0.4.5 iMON driver, the standalone and the LIRC kernel
 * modules for the iMON IR/VFD at http://venky.ws/projects/imon/
 * Inspired by:
 * 	TextMode driver (LCDproc authors?)
 *	Sasem driver	(Oliver Stabel)
 *
 * Copyright (c)  2005 Lucian Muresan <lucianm AT users.sourceforge.net>,
 *                     porting the LCDproc 0.4.5 code to LCDproc 0.5
 * Copyright (c)  2006 John Saunders, use graphics characters
 *
 * This source code is being released under the GPL.
 * Please see the file COPYING in this package for details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "report.h"
#include "imon.h"
#include "hd44780-charmap.h"
#include "adv_bignum.h"

// iMon reserves the first 8 locations for the
// special bargraph characters
#define IMON_CHAR_1_BAR		0x00
#define IMON_CHAR_2_BARS	0x01
#define IMON_CHAR_3_BARS	0x02
#define IMON_CHAR_4_BARS	0x03
#define IMON_CHAR_5_BARS	0x04
#define IMON_CHAR_6_BARS	0x05
#define IMON_CHAR_7_BARS	0x06
#define IMON_CHAR_8_BARS	0x07

// Standard music control characters
#define IMON_CHAR_PLAY		0x10	// >  Play
#define IMON_CHAR_RPLAY		0x11	// <  Reverse Play
#define IMON_CHAR_PAUSE		0xA0	// || Pause
#define IMON_CHAR_RECORD	0x16	// O  Record

#define IMON_CHAR_TRI_UP	0x1E	// ^
#define IMON_CHAR_TRI_DOWN	0x1F	// V
#define IMON_CHAR_DLB_TRI_UP	0x14
#define IMON_CHAR_DBL_TRI_DOWN	0x15

#define IMON_CHAR_ARROW_UP	0x18
#define IMON_CHAR_ARROW_DOWN	0x19
#define IMON_CHAR_ARROW_RIGHT	0x1A
#define IMON_CHAR_ARROW_LEFT	0x1B
#define IMON_CHAR_ENTER		0x17

#define IMON_CHAR_HOUSE		0x7F

#define IMON_CHAR_HEART		0x9D

#define IMON_CHAR_BLOCK_FILLED	IMON_CHAR_8_BARS
#define IMON_CHAR_BLOCK_EMPTY	' '

#define DEFAULT_DEVICE	"/dev/usb/lcd"
#define DEFAULT_SIZE	"16x2"

#define VFD_DEFAULT_CELL_WIDTH	5
#define VFD_DEFAULT_CELL_HEIGHT	8


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "imon_";

/* Constants for userdefchar_mode */
#define NUM_CCs		0 /* imon can't define a custom character */

/** private data for the \c imon driver */
typedef struct imon_private_data {
	char info[255];			/**< info string contents */
	int imon_fd;			/**< file descriptor to the display */
	unsigned char *framebuf;	/**< fram buffer */
	int height;			/**< display height in characters */
	int width;			/**< display width in characters */
	int cellwidth;			/**< character cell width */
	int cellheight;			/**< character cell height */
	const unsigned char *charmap;	/**< character mapping table */
} PrivateData;

/**
 * NULL-terminated list of charmaps that can be used by this driver. This
 * list is sorted by relevance. Most likely used entries first. */
static char * imon_charmaps[] = {
	"none",
	"hd44780_euro",
#ifdef EXTRA_CHARMAPS
	"hd44780_koi8_r",
	"hd44780_cp1251",
	"hd44780_8859_5",
	"upd16314",
#endif
	NULL
};

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
imon_init(Driver *drvthis)
{
	PrivateData *p = NULL;
	char buf[256];
	int i;

	/* Allocate, initialize and store private p */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL) {
		report(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}

	if (drvthis->store_private_ptr(drvthis, p)) {
		report(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		return -1;
	}

	p->imon_fd = -1;
	p->width = 0;
	p->height = 0;
	p->cellwidth = VFD_DEFAULT_CELL_WIDTH;
	p->cellheight = VFD_DEFAULT_CELL_HEIGHT;


	/* Get settings from config file*/

	/* Get device */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, buf);

	/* Open device for writing */
	if ((p->imon_fd = open(buf, O_WRONLY)) < 0) {
		report(RPT_ERR, "%s: ERROR opening %s (%s)", drvthis->name, buf, strerror(errno));
		report(RPT_ERR, "%s: Did you load the iMON VFD kernel module?", drvthis->name);
		return -1;
	}

	/* Get size settings*/
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	if ((sscanf(buf , "%dx%d", &p->width, &p->height) != 2)
	    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
	    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, buf, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE , "%dx%d", &p->width, &p->height);
	}

	/* Make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* Load character mapping table */
	p->charmap = NULL;
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Charmap", 0, "none"), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	for (i = 0; imon_charmaps[i] != NULL; i++) {
		if (strcasecmp(imon_charmaps[i], buf) == 0) {
			int idx = charmap_get_index(buf);
			if (idx != -1) {
				p->charmap = available_charmaps[idx].charmap;
				report(RPT_INFO, "%s: using %s charmap",
				       drvthis->name, available_charmaps[idx].name);
			}
		}
	}
	if (p->charmap == NULL) {
		report(RPT_ERR, "%s: unable to load charmap: %s", drvthis->name, buf);
		return -1;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imon_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->imon_fd >= 0)
			close(p->imon_fd);

		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
imon_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	strcpy(p->info, "Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD driver");
	return p->info;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imon_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imon_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	write(p->imon_fd, p->framebuf, p->width * p->height);
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
imon_string(Driver *drvthis, int x, int y, const char string[])
{
	int i;

	for (i = 0; string[i] != '\0'; i++)
		imon_chr(drvthis, x+i, y, string[i]);
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
imon_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--; x--;

	if ((x < 0) || (y < 0) || (x >= p->width) || (y >= p->height))
		return;

	p->framebuf[(y * p->width) + x] = p->charmap[(unsigned char) c];
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
imon_icon(Driver *drvthis, int x, int y, int icon)
{
	switch (icon) {
		case ICON_BLOCK_FILLED:
			imon_chr(drvthis, x, y, IMON_CHAR_BLOCK_FILLED);
			break;
		case ICON_HEART_OPEN:
			imon_chr(drvthis, x, y, IMON_CHAR_BLOCK_EMPTY);
			break;
		case ICON_HEART_FILLED:
			imon_chr(drvthis, x, y, IMON_CHAR_HEART);
			break;
		case ICON_ARROW_UP:
			imon_chr(drvthis, x, y, IMON_CHAR_ARROW_UP);
			break;
		case ICON_ARROW_DOWN:
			imon_chr(drvthis, x, y, IMON_CHAR_ARROW_DOWN);
			break;
		case ICON_ARROW_LEFT:
			imon_chr(drvthis, x, y, IMON_CHAR_ARROW_LEFT);
			break;
		case ICON_ARROW_RIGHT:
			imon_chr(drvthis, x, y, IMON_CHAR_ARROW_RIGHT);
			break;
		case ICON_STOP:
			imon_chr(drvthis, x, y, IMON_CHAR_BLOCK_FILLED);
			imon_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_PAUSE:
			imon_chr(drvthis, x, y, IMON_CHAR_PAUSE);
			imon_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_PLAY:
			imon_chr(drvthis, x, y, IMON_CHAR_PLAY);
			imon_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_PLAYR:
			imon_chr(drvthis, x, y, IMON_CHAR_RPLAY);
			imon_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_FF:
			imon_chr(drvthis, x, y, IMON_CHAR_PLAY);
			imon_chr(drvthis, x+1, y, IMON_CHAR_PLAY);
			break;
		case ICON_FR:
			imon_chr(drvthis, x, y, IMON_CHAR_RPLAY);
			imon_chr(drvthis, x+1, y, IMON_CHAR_RPLAY);
			break;
		case ICON_NEXT:
			imon_chr(drvthis, x, y, IMON_CHAR_PLAY);
			imon_chr(drvthis, x+1, y, '|');
			break;
		case ICON_PREV:
			imon_chr(drvthis, x, y, '|');
			imon_chr(drvthis, x+1, y, IMON_CHAR_RPLAY);
			break;
		case ICON_REC:
			imon_chr(drvthis, x, y, IMON_CHAR_RECORD);
			imon_chr(drvthis, x+1, y, ' ');
			break;
		default:
		      /* let the server core do the rest */
		      return -1;
	}
	return 0;
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
imon_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	// Special characters start at 0 not 1, so pass -1 as first char.
	// This can be safely done as heartbeat icon is not 0
	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, -1);
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
imon_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;
	int pos;

	if ((x <= 0) || (y <= 0) || (y > p->height))
		return;

	for (pos = 0; pos < len; pos++) {

		if (x + pos > p->width)
			return;

#ifndef IMON_HBARS_OLD
		if (pixels >= p->cellwidth ) {
			/* write a "full" block to the screen... */
			imon_chr(drvthis, x+pos, y, IMON_CHAR_BLOCK_FILLED);
		}
		else if (pixels >= 1) {
			/* write a partial block, albeit vertically... */
			imon_chr(drvthis, x+pos, y, pixels * p->cellheight / p->cellwidth);
		}
#else
		if (pixels >= p->cellwidth * 3/4) {
			/* write a "full" block to the screen... */
			imon_chr(drvthis, x+pos, y, IMON_CHAR_BLOCK_FILLED);
		}
		else if (pixels >= p->cellwidth * 2/4) {
			/* write a partial block... */
			imon_chr(drvthis, x+pos, y, IMON_CHAR_PLAY);
			break;
		}
		else if (pixels >= p->cellwidth * 1/4) {
			/* write a partial block... */
			imon_chr(drvthis, x+pos, y, '>');
			break;
		}
#endif
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
imon_num(Driver *drvthis, int x, int num)
{
	int do_init = 0;
	/* do_init = 0 because imon can't define a custom character */
	/* ccmode does not need because imon can't define a custom character */

	if ((num < 0) || (num > 10))
		return;

	/* Lib_adv_bignum does everything needed to show the bignumbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always NUM_CCs).
 */

MODULE_EXPORT int
imon_get_free_chars(Driver *drvthis)
{
	return NUM_CCs;
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
imon_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
imon_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
imon_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
imon_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

// EOF
