/** \file server/drivers/t6963.c
 * LCDd \c t6963 driver for Toshiba T6963 based LCD displays. The display is
 * driven in text mode with a custom font loaded.
 *
 * Wiring (no pins for the display given. Check with your datasheet!)
 *
 *\verbatim
 *  Parallel:              LCD:
 *  1 (Strobe) ----------- /WR
 *  2-9 (Data) ----------- DB0-DB7
 *  14 (Autofeed) -------- /CE
 *  16 (Init) ------------ C/D
 *  17 (Slct) ------------ /RD
 *                + 5V --- FS (6x8 font)
 *\endverbatim
 */

/*-
 * Base driver module for Toshiba T6963 based LCD displays.
 *
 * Parts of this file are based on the kernel driver by
 * Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * Copyright (c) 2001 Manuel Stahl <mythos@xmythos.de>
 *               2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "t6963.h"
#include "t6963_font.h"

#include "timing.h"
#include "report.h"
#include "lcd_lib.h"
#include "port.h"
#include "lpt-port.h"

/* Define the wiring for display */
#define nWR	nSTRB
#define nRD	nSEL
#define nCE	nLF
#define T_CMD	INIT
#define T_DATA	0x00

/** private data for the \c t6963 driver */
typedef struct t6963_private_data {
	u16 port;
	u8 *display_buffer1;

	int width;
	int height;
	int cellwidth;
	int cellheight;
	short bidirectLPT;
	short delayBus;
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "t6963_";

/**
 * API: Initialize the driver.
 */
MODULE_EXPORT int
t6963_init(Driver * drvthis)
{
	PrivateData *p;
	int w, h;
	char size[200] = DEFAULT_SIZE;

	debug(RPT_INFO, "T6963: init(%p)", drvthis);

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;

	debug(RPT_DEBUG, "T6963: reading config file...");

	/* Read config file */

	/* Which size? */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > T6963_MAX_WIDTH)
	    || (h <= 0) || (h > T6963_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s, Using default %s",
		       drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which port? */
	p->port = drvthis->config_get_int(drvthis->name, "Port", 0, DEFAULT_PORT);
	if ((p->port < 0x200) || (p->port > 0x400)) {
		p->port = DEFAULT_PORT;
		report(RPT_WARNING, "%s: Port value must be between 0x200 and 0x400. Using default 0x%03X",
		       drvthis->name, DEFAULT_PORT);
	}

	/* Use bi-directional mode of LPT port? Default: yes */
	p->bidirectLPT = drvthis->config_get_bool(drvthis->name, "bidirectional", 0, 1);
	/* Additional delay necessary? Default: no */
	p->delayBus = drvthis->config_get_bool(drvthis->name, "delaybus", 0, 0);

	/* Get permission to parallel port */
	debug(RPT_DEBUG, "T6963: Getting permission to parallel port %d...", p->port);
	if (port_access_multiple(p->port, 3)) {
		report(RPT_ERR, "%s: no permission to port 0x%03X: (%s)",
		       drvthis->name, p->port, strerror(errno));
		return -1;
	}

	/* Set up timing */
	if (timing_init() == -1) {
		report(RPT_ERR, "%s: timing_init() failed (%s)", drvthis->name, strerror(errno));
		return -1;
	}

	/* Allocate and clear memory for frame buffer */
	p->display_buffer1 = malloc(p->width * p->height);
	if (p->display_buffer1 == NULL) {
		report(RPT_ERR, "%s: No memory for frame buffer", drvthis->name);
		t6963_close(drvthis);
		return -1;
	}
	memset(p->display_buffer1, ' ', p->width * p->height);

	/* ------------------- I N I T I A L I Z A T I O N --------------- */
	if (p->bidirectLPT == 1) {
		debug(RPT_INFO, "T6963: Testing bidirectional mode...");
		if (t6963_low_dsp_ready(drvthis, 0x03) == -1) {
			report(RPT_WARNING, "T6963: Bidirectional mode not working (now disabled)");
			p->bidirectLPT = 0;
		}
		else {
			debug(RPT_INFO, "T6963: working!");
		}
	}

	debug(RPT_INFO, "T6963: Sending init to display...");
	/*
	 * Note: I assume display has a width that is a multiple of 8 (e.g.
	 * 128, 160, 240, etc). If this is not a multiple of the cellwidth
	 * the display has excess pixels on the right. In this case make the
	 * controller think we have one column more which will be filled with
	 * a space on flush. This avoids incorrect excess pixels on the right.
	 */
	w = p->width;
	if ((p->width * p->cellwidth) % 8)
		w++;

	/* Set text and graphic addresses */
	t6963_low_command_word(drvthis, SET_GRAPHIC_HOME_ADDRESS, GRAPHIC_BASE);
	t6963_low_command_word(drvthis, SET_GRAPHIC_AREA, w);
	t6963_low_command_word(drvthis, SET_TEXT_HOME_ADDRESS, TEXT_BASE);
	t6963_low_command_word(drvthis, SET_TEXT_AREA, w);

	/* Enable external character generator */
	t6963_low_command(drvthis, SET_MODE | OR_MODE | EXTERNAL_CG);
	/* Set address of CG RAM address start */
	t6963_low_command_word(drvthis, SET_OFFSET_REGISTER, CHARGEN_BASE >> 11);

	/* Load font data */
	t6963_set_nchar(drvthis, 0, fontdata_6x8, 256);

	/* Clear display */
	t6963_clear(drvthis);
	if (drvthis->config_get_bool(drvthis->name, "ClearGraphic", 0, 0) == 1)
		t6963_graphic_clear(drvthis);
	t6963_flush(drvthis);

	/* Turn on display, text on, graphics off, cursor off */
	t6963_low_command(drvthis, SET_DISPLAY_MODE | TEXT_ON);

	debug(RPT_INFO, "%s: init() done", drvthis->name);

	return 0;		/* return success */
}

/**
 * API: Close the driver.
 */
MODULE_EXPORT void
t6963_close(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_INFO, "Shutting down!");

	if (p != NULL) {
		port_deny_multiple(p->port, 3);

		if (p->display_buffer1 != NULL)
			free(p->display_buffer1);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/**
 * API: Returns the display width
 */
MODULE_EXPORT int
t6963_width(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/**
 * API: Returns the display height
 */
MODULE_EXPORT int
t6963_height(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/**
 * API: Clears the LCD screen (clear display buffer)
 */
MODULE_EXPORT void
t6963_clear(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Clearing Display of size %d x %d", p->width, p->height);
	memset(p->display_buffer1, ' ', p->width * p->height);
	debug(RPT_DEBUG, "Done");
}

/**
 * Clears the specified area of graphic RAM.
 * \param drvthis  Pointer to driver structure.
 */
static void
t6963_graphic_clear(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i, num;

	/*
	 * If width * cellwidth is not a multiple of 8 we have an additional
	 * column on the right.
	 */
	if ((p->width * p->cellwidth) % 8)
		num = (p->width + 1) * p->height * p->cellheight;
	else
		num = p->width * p->height * p->cellheight;

	debug(RPT_DEBUG, "Clearing Graphic %d bytes", num);

	t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, GRAPHIC_BASE);
	t6963_low_command(drvthis, AUTO_WRITE);
	for (i = 0; i < num; i++)
		t6963_low_auto_write(drvthis, 0);
	t6963_low_command(drvthis, AUTO_RESET);
}

/**
 * API: Flushes all output to the lcd.
 */
MODULE_EXPORT void
t6963_flush(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int r, c, line_address;

	debug(RPT_DEBUG, "Flushing %d x %d", p->width, p->height);

	t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, TEXT_BASE);
	t6963_low_command(drvthis, AUTO_WRITE);

	/*
	 * Note: There used to be a double buffering algorithm here that
	 * transferred only changes to the display. However, even complete
	 * screen updates are so fast (typically 5 ms on my 128x64) that I
	 * rewrote it not to use partial updates but use fast 'auto write'
	 * commands instead.
	 */
	for (r = 0; r < p->height; r++) {
		line_address = r * p->width;
		for (c = 0; c < p->width; c++) {
			t6963_low_auto_write(drvthis, p->display_buffer1[line_address + c]);
		}
		/*
		 * If width * cellwidth is not a multiple of 8 write a space
		 * into an additional column on the right.
		 */
		if ((p->width * p->cellwidth) % 8)
			t6963_low_auto_write(drvthis, ' ');
	}
	t6963_low_command(drvthis, AUTO_RESET);
}

/**
 * API: Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (width,height).
 */
MODULE_EXPORT void
t6963_string(Driver * drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int len;

	debug(RPT_DEBUG, "String out");

	/* Don't accept start coordinates outside the screen at all */
	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	x--;			/* Convert 1-based coords to 0-based */
	y--;

	/* Restrict string length to screen width */
	len = strlen(string);
	if (x + len > p->width)
		len = p->width - x;

	memcpy(&p->display_buffer1[y * p->width + x], string, len);
}

/**
 * API: Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (width,height).
 */
MODULE_EXPORT void
t6963_chr(Driver * drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Char out");

	/* Only copy if within screen bounds */
	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	y--;
	x--;

	p->display_buffer1[(y * p->width) + x] = c;
}

/**
 * Changes the font data of character n.
 * \param drvthis  Pointer to driver structure.
 * \param n        Index of character in CGRAM to update.
 * \param dat      Data (must consist of num*cellwidth*cellheight bytes).
 * \param num      Number of characters stored in data.
 */
static void
t6963_set_nchar(Driver * drvthis, int n, unsigned char *dat, int num)
{
	PrivateData *p = drvthis->private_data;
	int row, col;
	char letter;

	debug(RPT_DEBUG, "Setting char %d", n);

	if ((!dat) || (n + num > 256))
		return;

	t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, CHARGEN_BASE + n * 8);
	t6963_low_command(drvthis, AUTO_WRITE);
	for (row = 0; row < p->cellheight * num; row++) {
		letter = 0;
		/* Accumulate data for one pixel row */
		for (col = 0; col < p->cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * p->cellwidth) + col] > 0);
		}

		t6963_low_auto_write(drvthis, letter);
	}
	t6963_low_command(drvthis, AUTO_RESET);
}

/**
 * API: Define a custom character and write it to the LCD.
 */
MODULE_EXPORT void
t6963_set_char(Driver * drvthis, int n, char *dat)
{
	t6963_set_nchar(drvthis, n, (unsigned char *)dat, 1);
}

/**
 * API: Draws a vertical bar, from the bottom of the screen up.
 */
MODULE_EXPORT void
t6963_vbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 211);
}

/**
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
t6963_hbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 219);
}

/**
 * API: Sets an icon...
 */
MODULE_EXPORT int
t6963_icon(Driver * drvthis, int x, int y, int icon)
{
	debug(RPT_DEBUG, "T6963: set icon %d", icon);
	switch (icon) {
	    case ICON_BLOCK_FILLED:
		t6963_chr(drvthis, x, y, 219);
		break;
	    case ICON_HEART_FILLED:
		t6963_chr(drvthis, x, y, 3);
		break;
	    case ICON_HEART_OPEN:
		t6963_chr(drvthis, x, y, 4);
		break;
	    default:
		return -1;
	}
	return 0;
}


/**
 * Check display status.
 * \param drvthis  Pointer to driver structure.
 * \param sta      Bitmap of expected STA flags
 * \return  0 on success, -1 if ready could not be read
 */
static inline int
t6963_low_dsp_ready(Driver * drvthis, u8 sta)
{
	PrivateData *p = drvthis->private_data;
	int portcontrol = 0;

	if (p->bidirectLPT == 1) {
		int val;
		int loop = 0;

		do {
			portcontrol = T_CMD | nWR | nRD | nCE;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			/* lower nRD, nCE, set bi-directional mode */
			portcontrol = T_CMD | nWR | ENBI;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			/* possible wait required here: tACC = 150 ns max */
			if (p->delayBus)
				timing_uPause(1);
			val = port_in(T6963_DATA_PORT(p->port));
			portcontrol = T_CMD | nWR | nRD | nCE;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			loop++;
			if (loop == 100) {
				report(RPT_WARNING, "Ready check failed, STA=0x%02x", val);
				return -1;
			}
		} while ((val & sta) != sta);
	}
	else {
		portcontrol = T_CMD | nWR | nRD | nCE;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
		portcontrol = T_CMD | nWR;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
		timing_uPause(150);
		portcontrol = T_CMD | nWR | nRD | nCE;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	}

	return 0;
}


/**
 * Write a single command / or byte to the parallel port.
 * \param drvthis  Pointer to driver structure.
 * \param type     Command or Data
 * \param byte     Data byte.
 */
static inline void
t6963_low_send(Driver * drvthis, u8 type, u8 byte)
{
	PrivateData *p = drvthis->private_data;
	int portcontrol = 0;

	portcontrol = type | nWR | nRD | nCE;
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	port_out(T6963_DATA_PORT(p->port), byte);
	portcontrol = type | nRD;	/* lower nWR, nCE */
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	/* possible wait required here: tWR */
	if (p->delayBus)
		timing_uPause(1);
	portcontrol = type | nWR | nRD | nCE;
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
}


/**
 * Send one data byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param byte     Data byte.
 */
static void
t6963_low_data(Driver * drvthis, u8 byte)
{
	t6963_low_dsp_ready(drvthis, STA0|STA1);
	t6963_low_send(drvthis, T_DATA, byte);
}

/**
 * Send one command byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param byte     Command byte.
 */
static void
t6963_low_command(Driver * drvthis, u8 byte)
{
	t6963_low_dsp_ready(drvthis, STA0|STA1);
	t6963_low_send(drvthis, T_CMD, byte);
}

/**
 * Write one byte of data to display in AUTO mode (needs a different ready
 * check).
 * \param drvthis  Pointer to driver structure.
 * \param byte     Data byte.
 */
static void
t6963_low_auto_write(Driver *drvthis, u8 byte)
{
	t6963_low_dsp_ready(drvthis, STA3);
	t6963_low_send(drvthis, T_DATA, byte);
}

/**
 * Send one byte of data followed by one command byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param cmd      Command byte.
 * \param byte     Data value.
 */
static void
t6963_low_command_byte(Driver * drvthis, u8 cmd, u8 byte)
{
	t6963_low_data(drvthis, byte);
	t6963_low_command(drvthis, cmd);
}

/**
 * Send one word (two bytes) of data followed by one command byte to the
 * display. Low byte is output first.
 * \param drvthis  Pointer to driver structure.
 * \param cmd      Command byte.
 * \param word     Data value (2 bytes)
 */
static void
t6963_low_command_word(Driver * drvthis, u8 cmd, u16 word)
{
	t6963_low_data(drvthis, word & 0xFF);
	t6963_low_data(drvthis, (word >> 8) & 0xFF);
	t6963_low_command(drvthis, cmd);
}

/* EOF */
