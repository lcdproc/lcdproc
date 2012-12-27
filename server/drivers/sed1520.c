/** \file server/drivers/sed1520.c
 * LCDd \c sed1520 driver for graphic displays based on the SED1520.
 *
 * This is a driver for 122x32 pixel graphic displays based on the SED1520
 * controller connected to the parallel port. This Controller has no
 * built in character generator. Therefore all fonts and pixels are generated
 * by this driver.
 */

/*-
 * Copyright (C) 2001 Robin Adams <robin@adams-online.de>
 *		 2011 Markus Dolze <bsdfan@nurfuerspam.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lcd.h"
#include "sed1520.h"
#include "glcd_font5x8.h"
#include "sed1520fm.h"
#include "report.h"
#include "port.h"
#include "timing.h"
#include "lpt-port.h"

#define uPause timing_uPause

#ifndef DEFAULT_PORT
# define DEFAULT_PORT	0x378
#endif

#define CELLWIDTH	6
#define CELLHEIGHT	8

#define PIXELWIDTH	122
#define PIXELHEIGHT	32

#define WIDTH		((int) (PIXELWIDTH / CELLWIDTH))	/* 20 */
#define HEIGHT		((int) (PIXELHEIGHT / CELLHEIGHT))	/*  4 */

#define A0	nSEL		/* pin 17 */
#define CS1	nLF		/* pin 14, EN1 for 68-style connection */
#define CS2	INIT		/* pin 16, EN2 for 68-style connection */
#define WR	nSTRB		/* pin 1 */

/** private data for the \c sed1520 driver */
typedef struct sed1520_private_data {
    unsigned short port;
    int interface;
    int delayMult;
    int haveInverter;
    unsigned char colStartAdd;

    unsigned char *framebuf;
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "sed1520_";

/**
 * Writes command value to one or both SED1520 selected by chip.
 * \param p      Pointer to private data structure
 * \param value  Command byte to write
 * \param chip   Bitmap of controllers to send value to
 *
 * \note It is a little code duplication having writecommand and writedata
 *       both, but I leave it that way.
 */
static void
writecommand(PrivateData *p, int value, int chip)
{
    if (p->interface == 68) {
	port_out(p->port + 2, 0 ^ OUTMASK);
	port_out(p->port, value);
	/* cycle E */
	port_out(p->port + 2, ((chip & CS1) + (chip & CS2)) ^ OUTMASK);
	if (p->delayMult)
	    uPause(p->delayMult);
	port_out(p->port + 2, 0 ^ OUTMASK);
    }
    else {
	port_out(p->port, value);
	if (p->haveInverter) {
	    /*
	     * lower WR, rise A0 and CS1 and/or CS2 taking bit inversion of
	     * parallel port into account. External inverter required!
	     */
	    port_out(p->port + 2, WR + CS1 - (chip & CS1) + (chip & CS2));
	    /* rise WR */
	    port_out(p->port + 2, CS1 - (chip & CS1) + (chip & CS2));
	    if (p->delayMult)
		uPause(p->delayMult);
	    /* lower WR again */
	    port_out(p->port + 2, WR + CS1 - (chip & CS1) + (chip & CS2));
	    if (p->delayMult)
		uPause(p->delayMult);
	}
	else {			/* No inverter connected */
	    /* Note: CS?-(chip&CS?) drive the pin low if controller is set */
	    port_out(p->port + 2, (WR + CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    port_out(p->port + 2, (CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    if (p->delayMult)
		uPause(p->delayMult);
	    port_out(p->port + 2, (WR + CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    if (p->delayMult)
		uPause(p->delayMult);
	}
    }
}

/**
 * Writes data value to one or both SED1520 selected by chip.
 * \param p      Pointer to private data structure
 * \param value  Data byte to write
 * \param chip   Bitmap of controllers to send value to
 */
static void
writedata(PrivateData *p, int value, int chip)
{
    if (p->interface == 68) {
	port_out(p->port + 2, (A0) ^ OUTMASK);
	port_out(p->port, value);
	/* cycle E */
	port_out(p->port + 2, (A0 + (chip & CS1) + (chip & CS2)) ^ OUTMASK);
	if (p->delayMult)
	    uPause(p->delayMult);
	port_out(p->port + 2, (A0) ^ OUTMASK);
    }
    else {
	port_out(p->port, value);
	if (p->haveInverter) {
	    /* lower WR and A0, rise CS1 and/or CS2. See also writecommand. */
	    port_out(p->port + 2, A0 + WR + CS1 - (chip & CS1) + (chip & CS2));
	    port_out(p->port + 2, A0 + CS1 - (chip & CS1) + (chip & CS2));
	    if (p->delayMult)
		uPause(p->delayMult);
	    port_out(p->port + 2, A0 + WR + CS1 - (chip & CS1) + (chip & CS2));
	    if (p->delayMult)
		uPause(p->delayMult);
	}
	else {
	    port_out(p->port + 2, (A0 + WR + CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    port_out(p->port + 2, (A0 + CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    if (p->delayMult)
		uPause(p->delayMult);
	    port_out(p->port + 2, (A0 + WR + CS1 - (chip & CS1) + CS2 - (chip & CS2)) ^ OUTMASK);
	    if (p->delayMult)
		uPause(p->delayMult);
	}
    }
}

/**
 * Selects a page (=row) on both SED1520.
 * \param p     Pointer to private data structure
 * \param page  Page (=row) number (0-3)
 */
static void
selectpage(PrivateData *p, int page)
{
    writecommand(p, PAGE_ADR + (page & 0x03), CS1 + CS2);
}

/**
 * Selects a column on the SED1520 specified by chip.
 * \param p       Pointer to private data structure
 * \param column  Select column (segment) in controller (0-60)
 * \param chip    Bitmap of controllers to send value to
 */
static void
selectcolumn(PrivateData *p, int column, int chip)
{
    writecommand(p, COLUMN_ADR + (column & 0x7F), chip);
}

/*-
 * Display memory layout information:
 *
 * The SED1520 does not use a linear display data memory but a paged memory
 * with four pages (0-3) corresponding to rows 0-7, 8-15, 16-23, and 24-31.
 * Within each page, one bytes represents 8 pixels of one column (from 0-60)
 * with bit 0 (LSB) being the top and bit 7 (MSB) the bottom pixel within
 * that page.
 *
 * To write data to display memory one selects the page and column and then
 * writes bytes of pixel data to the display. The column counter is increased
 * by each write so the display fills from left to right (normal mode) or
 * right to left (inverted mapping).
 */

/**
 * Draws character z from fontmap to the framebuffer at position x,y.
 * The fontmap is stored in rows while the framebuffer is stored in columns,
 * so we need a little conversion.
 *
 * \param framebuf  Pointer to framebuffer
 * \param x         Character column (zero-based)
 * \param y         Line (zero-based)
 * \param z         Character index in fontmap
 */
static void
drawchar2fb(unsigned char *framebuf, int x, int y, unsigned char z)
{
    int i, j;

    if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT))
	return;

    /* for each raster column */
    for (i = CELLWIDTH; i > 0; i--) {
	int k = 0;

	/* gather the bits from the fontmap for each raster row */
	for (j = 0; j < CELLHEIGHT; j++)
	    k |= ((glcd_iso8859_1[(int)z][j] >> (i - 1)) & 0x01) << j;

	/* And store it in framebuffer pixel column */
	framebuf[(y * PIXELWIDTH) + (x * CELLWIDTH) + (CELLWIDTH - i)] = k;
    }
}

/**
 * API: Initialize the driver.
 */
MODULE_EXPORT int
sed1520_init(Driver * drvthis)
{
    PrivateData *p;
    char inverted;

    /* Allocate and store private data */
    p = (PrivateData *) calloc(1, sizeof(PrivateData));
    if (p == NULL)
	return -1;
    if (drvthis->store_private_ptr(drvthis, p))
	return -1;

    /* What port to use */
    p->port = drvthis->config_get_int(drvthis->name, "Port", 0, DEFAULT_PORT);

    /* Initialize timing */
    if (timing_init() == -1) {
	report(RPT_ERR, "%s: timing_init() failed (%s)", drvthis->name, strerror(errno));
	return -1;
    }
    p->delayMult = drvthis->config_get_int(drvthis->name, "delaymult", 0, 1);
    if (p->delayMult < 0 || p->delayMult > 1000) {
	report(RPT_WARNING, "%s: DelayMult value invalid, using default (1)", drvthis->name);
	p->delayMult = 1;
    }
    if (p->delayMult == 0)
	report(RPT_INFO, "%s: Delay is disabled", drvthis->name);

    /* Allocate our framebuffer */
    p->framebuf = (unsigned char *) calloc(PIXELWIDTH * HEIGHT, sizeof(unsigned char));
    if (p->framebuf == NULL) {
	report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
	return -1;
    }

    /* Clear screen */
    memset(p->framebuf, '\0', PIXELWIDTH * HEIGHT);

    /* Open port */
    if (port_access_multiple(p->port, 3)) {
	report(RPT_ERR, "%s: unable to access port 0x%03X", drvthis->name, p->port);
	return -1;
    }

    /* Get interface style (68-family or 80-family) */
    p->interface = drvthis->config_get_int(drvthis->name, "InterfaceType", 0, 80);
    if (p->interface != 68 && p->interface != 80) {
	report(RPT_WARNING, "%s: Invalid interface configured, using type 80", drvthis->name);
	p->interface = 80;
    }

    /*
     * The original wiring used an inverter to drive the control lines. As
     * someone may still be using this, the following setting in ON by default.
     */
    p->haveInverter = drvthis->config_get_bool(drvthis->name, "HaveInverter", 0, 1);

    /*
     * Inverted Mapping: Segments are addressed from right to left and start
     * at column 19 (address 0x13) up to column 79 (address 0x4F).
     */
    inverted = drvthis->config_get_bool(drvthis->name, "InvertedMapping", 0, 0);
    if (inverted)
	p->colStartAdd = 0x13;
    else
	p->colStartAdd = 0;

    /* Initialize display */
    if (drvthis->config_get_bool(drvthis->name, "UseHardReset", 0, 0) == 1) {
	writedata(p, 0xFF, CS1 + CS2);
	writedata(p, 0xFF, CS1 + CS2);
	writedata(p, 0xFF, CS1 + CS2);
    }
    writecommand(p, SOFT_RESET, CS1 + CS2);
    writecommand(p, (inverted) ? ADC_INV : ADC_NORM, CS1 + CS2);
    writecommand(p, DISP_ON, CS1 + CS2);
    writecommand(p, DISP_START_LINE, CS1 + CS2);
    selectpage(p, 3);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);
    return 0;
}

/**
 * API: Frees the framebuffer and exits the driver.
 */
MODULE_EXPORT void
sed1520_close(Driver * drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (p != NULL) {
	if (p->framebuf != NULL)
	    free(p->framebuf);

	free(p);
    }
    drvthis->store_private_ptr(drvthis, NULL);
}

/**
 * API: Returns the display width
 */
MODULE_EXPORT int
sed1520_width(Driver * drvthis)
{
    return WIDTH;
}

/**
 * API: Returns the display height
 */
MODULE_EXPORT int
sed1520_height(Driver * drvthis)
{
    return HEIGHT;
}

/**
 * API: Return the width of a character in pixels.
 */
MODULE_EXPORT int
sed1520_cellwidth(Driver * drvthis)
{
    return CELLWIDTH;
}

/**
 * API: Return the height of a character in pixels.
 */
MODULE_EXPORT int
sed1520_cellheight(Driver * drvthis)
{
    return CELLHEIGHT;
}

/**
 * API: Clears the LCD screen
 */
MODULE_EXPORT void
sed1520_clear(Driver * drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->framebuf, '\0', PIXELWIDTH * HEIGHT);
}

/**
 * API: Flushes all output to the lcd. No conversion needed here as
 * framebuffer is prepared by \c drawchar2fb.
 */
MODULE_EXPORT void
sed1520_flush(Driver * drvthis)
{
    PrivateData *p = drvthis->private_data;
    int i, j;

    for (i = 0; i < HEIGHT; i++) {
	/* select line/page */
	selectpage(p, i);

	/* update left half of display */
	selectcolumn(p, p->colStartAdd, CS1);
	for (j = 0; j < PIXELWIDTH / 2; j++)
	    writedata(p, p->framebuf[j + (i * PIXELWIDTH)], CS1);

	/* update right half of display */
	selectcolumn(p, p->colStartAdd, CS2);
	for (j = PIXELWIDTH / 2; j < PIXELWIDTH; j++)
	    writedata(p, p->framebuf[j + (i * PIXELWIDTH)], CS2);
    }
}

/**
 * API: Prints a string on the lc display, at position (x,y). The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
sed1520_string(Driver * drvthis, int x, int y, const char string[])
{
    PrivateData *p = drvthis->private_data;
    int i;

    x--;			/* Convert 1-based coords to 0-based */
    y--;

    for (i = 0; string[i] != '\0'; i++)
	drawchar2fb(p->framebuf, x + i, y, string[i]);
}

/**
 * API: Writes  char c at position x,y into the framebuffer.
 * x and y are 1-based textmode coordinates.
 */
MODULE_EXPORT void
sed1520_chr(Driver * drvthis, int x, int y, char c)
{
    PrivateData *p = drvthis->private_data;

    y--;
    x--;
    drawchar2fb(p->framebuf, x, y, c);
}

/**
 * API: This function draws a number num into the last 3 rows of the
 * framebuffer at 1-based position x. It should draw a 4-row font, but me
 * thinks this would look a little stretched. When num=10 a colon is drawn.
 */
MODULE_EXPORT void
sed1520_num(Driver * drvthis, int x, int num)
{
    PrivateData *p = drvthis->private_data;
    int z, c;

    x--;

    /* return on illegal char or illegal position */
    if ((x >= WIDTH) || (num < 0) || (num > 10))
	return;

    /*
     * For each page (= row of 8 dots) starting in page 1 (page 0 is left
     * empty - thus the numbers are aligned at the bottom of the display),
     * copy <width> number of bytes into the framebuffer.
     */
    for (z = 0; z < 3; z++) {
	for (c = 0; c < widtbl_NUM[num]; c++) {
	    if ((x * CELLWIDTH + c >= 0) && (x * CELLWIDTH + c < PIXELWIDTH)) {
		p->framebuf[((z + 1) * PIXELWIDTH) + (x * CELLWIDTH) + c] =
		    chrtbl_NUM[num][c * 3 + z];
	    }
	}
    }
}

/**
 * API: Changes the font of character n to a pattern given by *dat.
 * HD44780 controllers only posses 8 programmable chars. But we store the
 * fontmap completely in RAM, so every character can be altered.
 *
 * Important: Characters have to be redrawn by drawchar2fb() to show their
 * new shape. Because we use a non-standard 6x8 font a *dat not calculated
 * from width and height will fail.
 */
MODULE_EXPORT void
sed1520_set_char(Driver * drvthis, int n, char *dat)
{
    int row;
    unsigned char mask = (1 << CELLWIDTH) - 1;

    if (n < 0 || n > 255)
	return;
    if (!dat)
	return;

    for (row = 0; row < CELLHEIGHT; row++)
	glcd_iso8859_1[n][row] = dat[row] & mask;
}

/**
 * API: Draws a vertical from the bottom up at 1-based position x.
 */
MODULE_EXPORT void
sed1520_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = drvthis->private_data;
    int i, j, k;
    int pixels;

    x--;

    if (x < 0 || y < 1 || x >= WIDTH || y > HEIGHT || len > HEIGHT)
	return;

    pixels = len * CELLHEIGHT * promille / 1000;

    for (j = 0; j < 3; j++) {
	k = 0;
	/* Set height of block. Bottom is leftmost bit */
	for (i = 0; i < CELLHEIGHT; i++) {
	    if (pixels > i)
		k |= (1 << (CELLHEIGHT - 1 - i));
	}

	/* Draw directly into framebuffer starting from bottom */
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 0] = 0;
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 1] = k;
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 2] = k;
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 3] = k;
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 4] = k;
	p->framebuf[((3 - j) * PIXELWIDTH) + (x * CELLWIDTH) + 5] = 0;

	pixels -= CELLHEIGHT;
    }
}

/**
 * API: Draws a horizontal bar from left to right at 1-based position x,y into
 * the framebuffer.
 */
MODULE_EXPORT void
sed1520_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = drvthis->private_data;
    int i;
    int pixels;

    x--;
    y--;

    if ((y < 0) || (y >= HEIGHT) || (x < 0) || (len < 0) || (x + len > WIDTH))
	return;

    pixels = len * CELLWIDTH * promille / 1000;

    /*
     * Draw directly into framebuffer. Drawing is easy, as one byte is one
     * column in a page. Use 5 dots in the middle (0x3E).
     */
    for (i = 0; i < pixels; i++)
	p->framebuf[(y * PIXELWIDTH) + (x * CELLWIDTH) + i] = 0x7C;
}

/**
 * API: Place an icon on the screen.
 */
MODULE_EXPORT int
sed1520_icon(Driver * drvthis, int x, int y, int icon)
{
    int icon_char;

    if ((icon_char = glcd_icon5x8(icon)) != -1) {
        sed1520_chr(drvthis, x, y, icon_char);
        return 0;
    }
    return -1;
}
