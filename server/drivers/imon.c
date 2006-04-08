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
} imonPD;


/** 
 * driver initialization
 */
MODULE_EXPORT int imon_init (Driver *drvthis)
{
	imonPD * pPD = NULL;

	// Alocate, initialize and store private p
	pPD = (imonPD *) calloc(1, sizeof(imonPD));
	if (pPD == NULL) {
		debug(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}

	if (drvthis->store_private_ptr(drvthis, pPD)) {
		debug(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		return -1;
	}

	char buf[256];
	pPD->imon_fd = -1;
	pPD->width = 0;
	pPD->height = 0;

	/* Get settings from config file*/

	/* Get device */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, buf);

	/* Open device for writing */
	if ((pPD->imon_fd = open(buf, O_WRONLY)) < 0) {
		report(RPT_ERR, "%s: ERROR opening %s (%s).", drvthis->name, buf, strerror(errno));
		report(RPT_ERR, "%s: Did you load the iMON VFD kernel module?", drvthis->name);
		report(RPT_ERR, "%s: More info in lcdproc/docs/README.imon", drvthis->name);
		return -1;
	}

	/* Get size settings*/
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	if ((sscanf(buf , "%dx%d", &pPD->width, &pPD->height) != 2)
	    || (pPD->width <= 0) || (pPD->width > LCD_MAX_WIDTH)
	    || (pPD->height <= 0) || (pPD->height > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s", 
				drvthis->name, buf, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE , "%dx%d", &pPD->width, &pPD->height);
	}

	/* Make sure the frame buffer is there... */
	pPD->framebuf = (unsigned char *) malloc(pPD->width * pPD->height);
	if (pPD->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}
	memset(pPD->framebuf, ' ', pPD->width * pPD->height);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	
	return 1;		 
}

/** 
 * provides some info about this driver
 */
MODULE_EXPORT char * imon_get_info (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;
	strcpy(pPD->info, "Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD driver");
	return pPD->info;
}

/** 
 * closes driver
 */
MODULE_EXPORT void imon_close (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;

	if (pPD != NULL) {
		if (pPD->imon_fd >= 0)
			close(pPD->imon_fd);

		if (pPD->framebuf != NULL)
			free(pPD->framebuf);
		pPD->framebuf = NULL;

		free(pPD);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/** 
 * Clears the VFD screen 
 */
MODULE_EXPORT void imon_clear (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;
	memset(pPD->framebuf, ' ', pPD->width * pPD->height);
}

/**
 * Flushes all output to the VFD...  
 */
MODULE_EXPORT void imon_flush (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;
	write(pPD->imon_fd, pPD->framebuf, pPD->width * pPD->height);
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
	imonPD * pPD = drvthis->private_data;
	y--; x--;

	if ((x < 0) || (y < 0) || (x >= pPD->width) || (y >= pPD->height))
		return;

	switch (ch) {
		case '\0':
		case -1:	/* ugly: this is 255 unsigned */
			ch = PAD;
			break;
		default:
			;
	}
	pPD->framebuf[(y * pPD->width) + x] = ch;
}

/**
 * Draws a vertical bar (adapted from the curses driver, because
 * this device does not support custom characters, as Venky
 * states in the original LCDproc-0.4.5 implementation)
 */
MODULE_EXPORT void imon_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;
	int pixels;
	int total_pixels = ((long) 2 * len * VFD_DEFAULT_CELL_HEIGHT + 1) * promille / 2000;

	for (pos = 0; pos < len; pos++) {
		pixels = total_pixels - VFD_DEFAULT_CELL_HEIGHT * pos;
		if (pixels >= VFD_DEFAULT_CELL_HEIGHT) {
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
	}
}

/**
 * Draws a horizontal bar (adapted from the curses driver, because
 * this device does not support custom characters, as Venky
 * states in the original LCDproc-0.4.5 implementation)
 */
MODULE_EXPORT void imon_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;
	int pixels;
	int total_pixels = ((long) 2 * len * VFD_DEFAULT_CELL_WIDTH + 1) * promille / 2000;

	for (pos = 0; pos < len; pos++) {
		pixels = total_pixels - VFD_DEFAULT_CELL_WIDTH * pos;
		if (pixels >= VFD_DEFAULT_CELL_WIDTH) {
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
	}
}

/** 
 * returns the display's width
 */
MODULE_EXPORT int imon_width (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;
	return pPD->width;
}

/** 
 * returns the display's height
 */
MODULE_EXPORT int  imon_height (Driver *drvthis)
{
	imonPD * pPD = drvthis->private_data;
	return pPD->height;
}

// EOF
