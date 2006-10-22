/**
 * Driver for Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD Module
 *
 * In order to be able to use it, you have to get and install one of
 * the following kernel modules:
 *  - standalone iMON VFD driver from http://venky.ws/projects/imon/
 *  - the iMON module included with LIRC ver. 0.7.1 or newer
 *    from http://www.lirc.org/
 *
 * Copyright (c) 2004, Venky Raju <dev@venky.ws>, original author of
 * the LCDproc 0.4.5 iMON driver, the standalone and the LIRC kernel
 * modules for the iMON IR/VFD at http://venky.ws/projects/imon/
 * Inspired by:
 * 	TextMode driver (LCDproc authors?)
 *	Sasem driver	(Oliver Stabel)
 *
 * Copyright (c)  2005 Lucian Muresan <lucianm AT users.sourceforge.net>,
 * porting the LCDproc 0.4.5 code to LCDproc 0.5
 *
 * This source code is being released under the GPL.
 * Please see the file COPYING in this package for details.
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
#include "lcd_lib.h"
#include "shared/debug.h"
//#define DEBUG
#include "report.h"


#include "imon.h"

#define PAD		'#'
#define DEFAULT_DEVICE	"/dev/usb/lcd"
#define DEFAULT_SIZE	"16x2"
/* The two value below are only used internally, we don't support custom char. */
#define VFD_DEFAULT_CELL_WIDTH	5
#define VFD_DEFAULT_CELL_HEIGHT	8


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "imon_";

// our private data
typedef struct {
	char info[255];
	int imon_fd;
	unsigned char *framebuf;
	int height;
	int width;
	int cellwidth;
	int cellheight;
} PrivateData;


/** 
 * driver initialization
 */
MODULE_EXPORT int imon_init (Driver *drvthis)
{
	PrivateData *p = NULL;

	// Alocate, initialize and store private p
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL) {
		debug(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}

	if (drvthis->store_private_ptr(drvthis, p)) {
		debug(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		return -1;
	}

	char buf[256];
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
		report(RPT_ERR, "%s: ERROR opening %s (%s).", drvthis->name, buf, strerror(errno));
		report(RPT_ERR, "%s: Did you load the iMON VFD kernel module?", drvthis->name);
		report(RPT_ERR, "%s: More info in lcdproc/docs/README.imon", drvthis->name);
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

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;		 
}

/** 
 * provides some info about this driver
 */
MODULE_EXPORT const char * imon_get_info (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	strcpy(p->info, "Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD driver");
	return p->info;
}

/** 
 * closes driver
 */
MODULE_EXPORT void imon_close (Driver *drvthis)
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
 * Clears the VFD screen 
 */
MODULE_EXPORT void imon_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/**
 * Flushes all output to the VFD...  
 */
MODULE_EXPORT void imon_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	write(p->imon_fd, p->framebuf, p->width * p->height);
}

/**
 * Prints a string on the VFD display, at position (x,y).
 * The upper-left is (1,1) and the lower right is (16, 2).  
 */
MODULE_EXPORT void imon_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	for (i = 0; string[i] != '\0'; i++)
		imon_chr(drvthis, x+i, y, string[i]);
}

/**
 * Prints a character on the VFD display, at position (x,y).
 * The upper-left is (1,1) and the lower right is (16,2).
 */
MODULE_EXPORT void imon_chr (Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = drvthis->private_data;
	y--; x--;

	if ((x < 0) || (y < 0) || (x >= p->width) || (y >= p->height))
		return;

	switch (ch) {
		case '\0':
		case -1:	/* ugly: this is 255 unsigned */
			ch = PAD;
			break;
		default:
			;
	}
	p->framebuf[(y * p->width) + x] = ch;
}

/**
 * Draws a vertical bar (adapted from the curses driver, because
 * this device does not support custom characters, as Venky
 * states in the original LCDproc-0.4.5 implementation)
 */
MODULE_EXPORT void imon_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos++) {
		if (pixels >= p->cellheight) {
			/* write a "full" block to the screen... */
			//drvthis->icon (drvthis, x, y-pos, ICON_BLOCK_FILLED);
			imon_chr(drvthis, x, y-pos, '#');
		}
		else if (pixels > 0) {
			/* write a "partial" block to the screen... */
			imon_chr(drvthis, x, y-pos, '|');
			break;
		}
		else {
			; // write nothing (not even a space) 
		}

		pixels -= p->cellheight;
	}
}

/**
 * Draws a horizontal bar (adapted from the curses driver, because
 * this device does not support custom characters, as Venky
 * states in the original LCDproc-0.4.5 implementation)
 */
MODULE_EXPORT void imon_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos++) {
		if (pixels >= p->cellwidth) {
			/* write a "full" block to the screen... */
			//drvthis->icon (drvthis, x+pos, y, ICON_BLOCK_FILLED);
			imon_chr (drvthis, x+pos, y, '#');
		}
		else if (pixels > 0) {
			/* write a "partial" block to the screen... */
			imon_chr (drvthis, x+pos, y, '-');
			break;
		}
		else {
			; // write nothing (not even a space) 
		}

		pixels -= p->cellwidth;
	}
}

/** 
 * returns the display's width
 */
MODULE_EXPORT int imon_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/** 
 * returns the display's height
 */
MODULE_EXPORT int  imon_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/** 
 * returns the display's cell width
 */
MODULE_EXPORT int imon_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}

/** 
 * returns the display's cell height
 */
MODULE_EXPORT int  imon_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

// EOF
