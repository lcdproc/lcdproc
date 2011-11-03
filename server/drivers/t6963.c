/** \file server/drivers/t6963.c
 * LCDd \c t6963 driver for Toshiba T6963 based LCD displays. The display is
 * driven in text mode with a custom font loaded.
 *
 * \note  The display must be wired to use 6x8 font. Check with your datasheet.
 */

/*-
 * Base driver module for Toshiba T6963 based LCD displays.
 *
 * Copyright (c) 2001 Manuel Stahl <mythos@xmythos.de>
 *               2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "t6963.h"
#include "glcd_font5x8.h"
#include "report.h"
#include "lcd_lib.h"
#include "t6963_low.h"

/** private data for the \c t6963 driver */
typedef struct t6963_private_data {
	unsigned char *display_buffer1;

	int px_width, px_height;	/* size in pixels */
	int width, height;		/* size in characters */
	u16 bytes_per_line;		/* memory allocated per line */

	T6963_port *port_config;
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "t6963_";

/*
 * Width and height of one character cell including spacing. Thus it is 6x8
 * not 5x7!
 */
#define DEFAULT_CELL_WIDTH  6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_SIZE "128x64"
#define DEFAULT_PORT 0x378


/**
 * API: Initialize the driver.
 */
MODULE_EXPORT int
t6963_init(Driver *drvthis)
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

	debug(RPT_DEBUG, "T6963: reading config file...");

	/* Read display size in pixels */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > T6963_MAX_WIDTH)
	    || (h <= 0) || (h > T6963_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s, Using default %s",
		       drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->px_width = w;
	p->px_height = h;

	/*
	 * Calculate the size in characters and the number of actual bytes
	 * required per display line. The number of bytes may be different
	 * to the number of columns if the the width in pixels is not a
	 * multiple of the cell width.
	 */
	p->width = p->px_width / DEFAULT_CELL_WIDTH;
	p->bytes_per_line = (p->px_width % DEFAULT_CELL_WIDTH) ? p->width + 1 : p->width;
	p->height = p->px_height / DEFAULT_CELL_HEIGHT;

	/* Allocate port configuration structure */
	if ((p->port_config = (T6963_port *) calloc(1, sizeof(T6963_port))) == NULL) {
		report(RPT_ERR, "%s: error mallocing", drvthis->name);
		return -1;
	}

	/* Which port? */
	p->port_config->port = drvthis->config_get_int(drvthis->name, "Port", 0, DEFAULT_PORT);
	if ((p->port_config->port < 0x200) || (p->port_config->port > 0x400)) {
		p->port_config->port = DEFAULT_PORT;
		report(RPT_WARNING, "%s: Port value must be between 0x200 and 0x400. Using default 0x%03X",
		       drvthis->name, DEFAULT_PORT);
	}

	/* Use bi-directional mode of LPT port? Default: yes */
	p->port_config->bidirectLPT = drvthis->config_get_bool(drvthis->name, "bidirectional", 0, 1);
	/* Additional delay necessary? Default: no */
	p->port_config->delayBus = drvthis->config_get_bool(drvthis->name, "delaybus", 0, 0);

	/* Initialize port and timing */
	debug(RPT_DEBUG, "T6963: Initializing parallel port at 0x%03X", p->port_config->port);
	if (t6963_low_init(p->port_config) == -1) {
		report(RPT_ERR, "%s: Error initializing port 0x%03X: %s",
		       drvthis->name, p->port_config->port, strerror(errno));
		return -1;
	}

	/* Allocate and clear memory for frame buffer */
	p->display_buffer1 = malloc(p->bytes_per_line * p->height);
	if (p->display_buffer1 == NULL) {
		report(RPT_ERR, "%s: No memory for frame buffer", drvthis->name);
		t6963_close(drvthis);
		return -1;
	}
	memset(p->display_buffer1, ' ', p->bytes_per_line * p->height);

	/* ------------------- I N I T I A L I Z A T I O N --------------- */
	if (p->port_config->bidirectLPT == 1) {
		debug(RPT_INFO, "T6963: Testing bidirectional mode...");
		if (t6963_low_dsp_ready(p->port_config, 0x03) == -1) {
			report(RPT_WARNING, "T6963: Bidirectional mode not working (now disabled)");
			p->port_config->bidirectLPT = 0;
		}
		else {
			debug(RPT_INFO, "T6963: working!");
		}
	}

	debug(RPT_INFO, "T6963: Sending init to display...");

	/* Set text and graphic addresses */
	t6963_low_command_word(p->port_config, SET_GRAPHIC_HOME_ADDRESS, GRAPHIC_BASE);
	t6963_low_command_word(p->port_config, SET_GRAPHIC_AREA, p->bytes_per_line);
	t6963_low_command_word(p->port_config, SET_TEXT_HOME_ADDRESS, TEXT_BASE);
	t6963_low_command_word(p->port_config, SET_TEXT_AREA, p->bytes_per_line);

	/* Enable external character generator */
	t6963_low_command(p->port_config, SET_MODE | OR_MODE | EXTERNAL_CG);
	/* Set address of CG RAM address start */
	t6963_low_command_word(p->port_config, SET_OFFSET_REGISTER, CHARGEN_BASE >> 11);

	/* Load font data */
	t6963_set_nchar(drvthis, 0, 256);

	/* Clear display */
	t6963_clear(drvthis);
	if (drvthis->config_get_bool(drvthis->name, "ClearGraphic", 0, 0) == 1)
		t6963_graphic_clear(drvthis);
	t6963_flush(drvthis);

	/* Turn on display, text on, graphics off, cursor off */
	t6963_low_command(p->port_config, SET_DISPLAY_MODE | TEXT_ON);

	debug(RPT_INFO, "%s: init() done", drvthis->name);

	return 0;		/* return success */
}

/**
 * API: Close the driver.
 */
MODULE_EXPORT void
t6963_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_INFO, "Shutting down!");

	if (p != NULL) {
		if (p->port_config != NULL) {
			t6963_low_close(p->port_config);
			free(p->port_config);
		}

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
t6963_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/**
 * API: Returns the display height
 */
MODULE_EXPORT int
t6963_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/**
 * API: Return the width of a character in pixels.
 * \note  This function actually returns the width of one character (as defined
 *        in the font, not the actual cell width. Otherwise bar graph
 *        calculations would take character spacing into account.
 */
MODULE_EXPORT int
t6963_cellwidth(Driver *drvthis)
{
	return GLCD_FONT_WIDTH;
}

/**
 * API: Return the height of a character in pixels.
 * \note  As with t6963_cellwidth this function returns the height of one
 *        character as defined by the font instead of the cell size (which is
 *        the same anyway).
 */
MODULE_EXPORT int
t6963_cellheight(Driver *drvthis)
{
	return GLCD_FONT_HEIGHT;
}

/**
 * API: Clears the LCD screen (clear display buffer)
 */
MODULE_EXPORT void
t6963_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Clearing Display of size %d x %d", p->bytes_per_line, p->height);
	memset(p->display_buffer1, ' ', p->bytes_per_line * p->height);
	debug(RPT_DEBUG, "Done");
}

/**
 * Clears the specified area of graphic RAM.
 * \param drvthis  Pointer to driver structure.
 */
static void
t6963_graphic_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int num = p->bytes_per_line * p->px_height;
	int i;

	debug(RPT_DEBUG, "Clearing Graphic %d bytes", num);

	t6963_low_command_word(p->port_config, SET_ADDRESS_POINTER, GRAPHIC_BASE);
	t6963_low_command(p->port_config, AUTO_WRITE);
	for (i = 0; i < num; i++)
		t6963_low_auto_write(p->port_config, 0);
	t6963_low_command(p->port_config, AUTO_RESET);
}

/**
 * API: Flushes all output to the lcd.
 */
MODULE_EXPORT void
t6963_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int r, c, line_address;

	debug(RPT_DEBUG, "Flushing %d x %d", p->width, p->height);

	t6963_low_command_word(p->port_config, SET_ADDRESS_POINTER, TEXT_BASE);
	t6963_low_command(p->port_config, AUTO_WRITE);

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
			t6963_low_auto_write(p->port_config, p->display_buffer1[line_address + c]);
		}
		/*
		 * If width is not identical with bytes_per_line there must be
		 * one additional empty column on the right.
		 */
		if (p->width != p->bytes_per_line)
			t6963_low_auto_write(p->port_config, ' ');
	}
	t6963_low_command(p->port_config, AUTO_RESET);
}

/**
 * API: Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (width,height).
 */
MODULE_EXPORT void
t6963_string(Driver *drvthis, int x, int y, const char string[])
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
t6963_chr(Driver *drvthis, int x, int y, char c)
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
 * Load the custom font into LCD. Font data may be modified by the \c set_char
 * function on the fly.
 *
 * \param drvthis  Pointer to driver structure.
 * \param n        Index of starting character in CGRAM
 * \param num      Number of characters to update
 */
static void
t6963_set_nchar(Driver *drvthis, int n, int num)
{
	int chr, row;
	char letter;
	unsigned char mask = (1 << GLCD_FONT_WIDTH) - 1;
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Loading font");

	t6963_low_command_word(p->port_config, SET_ADDRESS_POINTER, CHARGEN_BASE + n * DEFAULT_CELL_HEIGHT);
	t6963_low_command(p->port_config, AUTO_WRITE);
	for (chr = 0; chr < num; chr++) {
		for (row = 0; row < DEFAULT_CELL_HEIGHT; row++) {
			letter = glcd_iso8859_1[chr][row] & mask;
			t6963_low_auto_write(p->port_config, letter);
		}
	}
	t6963_low_command(p->port_config, AUTO_RESET);
}

/**
 * API: Define a custom character and write it into the font data, possibly
 * overwriting an existing character.
 */
MODULE_EXPORT void
t6963_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	if (!dat || n < 0 || n > 255)
		return;

	memcpy(glcd_iso8859_1[n], dat, 8);
	t6963_set_nchar(drvthis, n, 1);
}

/**
 * API: Draws a vertical bar, from the bottom of the screen up.
 */
MODULE_EXPORT void
t6963_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	lib_vbar_static(drvthis, x, y, len, promille, options, GLCD_FONT_HEIGHT, 0x90);
}

/**
 * API: Draws a horizontal bar to the right.
 *
 * \note  For hBars the actual width of the font (5) has to be used instead
 *        of the cell width (6)!
 */
MODULE_EXPORT void
t6963_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pixels = ((long) 2 * len * GLCD_FONT_WIDTH) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos++) {
		if (pixels >= GLCD_FONT_WIDTH )
			t6963_chr(drvthis, x + pos, y, 0x9e);
		else if (pixels >= 1)
			t6963_chr(drvthis, x + pos, y, 0x99 + pixels);
		else
			;	/* do nothing */
		pixels -= GLCD_FONT_WIDTH;
	}
}

/**
 * API: Sets an icon...
 */
MODULE_EXPORT int
t6963_icon(Driver *drvthis, int x, int y, int icon)
{
	int icon_char;
	debug(RPT_DEBUG, "T6963: set icon %d", icon);

	if ((icon_char = glcd_icon5x8(icon)) != -1) {
		t6963_chr(drvthis, x, y, icon_char);
		return 0;
	}
	return -1;
}

/* EOF */
