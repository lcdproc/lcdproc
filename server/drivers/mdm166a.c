/** \file server/drivers/mdm166a.c
 * LCDd \c mdm166a driver for Futuba MDM166A displays.
 *
 * The 96x16 pixel dot matrix area is used for 16x2 characters with a
 * 5x8 pixel font (in a 6x8 cell). The icons on the display are supported by
 * using the output-function.
 *
 * The display features a self-running clock mode, which can be
 * activated in the config file and will be shown on LCDproc shutdown.
 */

/*-
 * Copyright (C) 2003,2007 Intra2net AG
 *               2010 Christoph Rasim (christoph@rasim.net)
 *
 * This driver is released under the GPL. See file COPYING in this
 * package for further details.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <hid.h>

#include "lcd.h"
#include "mdm166a.h"
#include "glcd_font5x8.h"
#include "report.h"

/*
 * The display itself stores eight pixels in one byte. We waste a little
 * memory as we store one pixel per byte as we want to keep the drawing code
 * simple. Take a look at mdm166a_flush for the conversion.
 */

#define MDM166A_XSIZE 96
#define MDM166A_YSIZE 16
#define MDM166A_SCREENSIZE MDM166A_XSIZE*MDM166A_YSIZE
#define MDM166A_PACKEDSIZE 96*2

#define WIDTH           16
#define HEIGHT          2
#define CELLWIDTH       6
#define CELLHEIGHT      8

#define DEFAULT_CLOCK   "no"

#define CMD_PREFIX      0x1b	/**< Command prefix, indicates a command follows */
#define CMD_SETCLOCK    0x00	/**< Send current time to display */
#define CMD_SETSYMBOL   0x30	/**< Enable or disable a symbol */
#define CMD_SETDIMM     0x40	/**< Set the dimming level of the display */
#define CMD_RESET       0x50	/**< Reset all configuration data to default and clear */
#define CMD_SETRAM      0x60	/**< Set the actual graphics RAM offset for next data write */
#define CMD_SETPIXEL    0x70	/**< Write pixel data to RAM of the display */


/** private data for the \c mdm166a driver */
typedef struct mdm166a_private_data {
	HIDInterface *hid;	/**< Pointer to HID interface */
	int showClock;		/**< What clock to show on close */
	bool dimm;		/**< Brightness level */
	bool offDimm;		/**< Brightness level on close */
	unsigned char *framebuf;	/**< Pointer to internal framebuffer */
	int changed;		/**< Indicator for framebuffer changes */
	int last_output;	/**< Icon states after last update */
	char info[255];		/**< Pointer to driver description */
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "mdm166a_";

/**
 * Helper function for clock (conversion to BCD)
 * \param x  int_value
 * \return   BCD_value
 */
inline byte
toBCD(int x)
{
	return (byte) (((x) / 10 * 16) + ((x) % 10));
}

/**
 * Draws char z from fontmap to the framebuffer at position
 * x,y. These are zero-based textmode positions.
 * The Fontmap is stored in rows while the framebuffer is stored
 * in columns, so we need a little conversion.
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param z        Character that gets written.
 */
void
drawchar2fb(Driver *drvthis, int x, int y, unsigned char z)
{
	int font_x, font_y;
	PrivateData *p = drvthis->private_data;

	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return;

	x++;

	for (font_y = 0; font_y < 8; font_y++) {
		for (font_x = 5; font_x > -1; font_x--) {
			if ((glcd_iso8859_1[z][font_y] & 1 << font_x) == 1 << font_x)
				p->framebuf[x * 6 - 1 - font_x + (y * 8 + font_y) * MDM166A_XSIZE] = 1;
			else
				p->framebuf[x * 6 - 1 - font_x + (y * 8 + font_y) * MDM166A_XSIZE] = 0;
		}
	}

	p->changed = 1;
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
mdm166a_init(Driver *drvthis)
{
	char clock[256] = DEFAULT_CLOCK;
	PrivateData *p;
	int const PATH_OUT[1] = {0xff7f0004};
	HIDInterfaceMatcher matcher = {0x19c2, 0x6a11, NULL, NULL, 0};
	hid_return ret;
	char Cmd[5];

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;
	strcpy(p->info, "Targa USB Graphic Vacuum Fluorescent Display (mdm166a) driver v0.1 : 19c2:6a11");

	/* Get clock setting from config */
	strncpy(clock, drvthis->config_get_string(drvthis->name, "Clock", 0, DEFAULT_CLOCK), sizeof(clock));
	clock[sizeof(clock) - 1] = '\0';
	p->showClock = 0;
	if (strcmp(clock, "small") == 0)
		p->showClock = 1;
	else if (strcmp(clock, "big") == 0)
		p->showClock = 2;

	/* Get dimming setting from config */
	p->dimm = drvthis->config_get_bool(drvthis->name, "Dimming", 0, 0);
	p->offDimm = drvthis->config_get_bool(drvthis->name, "OffDimming", 0, 0);

	/* Silence libhid and libusb */
	hid_set_debug(HID_DEBUG_NONE);
	hid_set_debug_stream(0);
	hid_set_usb_debug(0);

	/*
	 * We use one central error exit point utilizing forward goto below!
	 */

	/* Init HID device */
	ret = hid_init();
	if (ret != HID_RET_SUCCESS) {
		report(RPT_ERR, "%s: hid_init failed: %s", drvthis->name, hid_strerror(ret));
		goto error;
	}

	p->hid = hid_new_HIDInterface();
	if (p->hid == 0) {
		report(RPT_ERR, "%s: hid_new_HIDInterface() failed, out of memory?", drvthis->name);
		goto error;
	}

	ret = hid_force_open(p->hid, 0, &matcher, 3);
	if (ret != HID_RET_SUCCESS) {
		report(RPT_ERR, "%s: hid_force_open failed: %s", drvthis->name, hid_strerror(ret));
		goto error;
	}

	/* Allocate our framebuffer */
	p->framebuf = (unsigned char *)malloc(MDM166A_SCREENSIZE + MDM166A_PACKEDSIZE + 1);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		goto error;
	}

	/* Reset and clear display */
	Cmd[0] = 0x02;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_RESET;
	hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 3);
	p->last_output = 0;

	/* Set dimming */
	Cmd[0] = 0x03;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_SETDIMM;
	if (p->dimm)
		Cmd[3] = 1;
	else
		Cmd[3] = 2;
	hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 4);

	/* Clear internal screen */
	mdm166a_clear(drvthis);

	report(RPT_INFO, "%s: init() done", drvthis->name);
	return 0;

	/* Central error exit point */
error:
	mdm166a_close(drvthis);
	return -1;
}


/**
 * Set the clock mode and close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
mdm166a_close(Driver *drvthis)
{
	int const PATH_OUT[1] = {0xff7f0004};
	char Cmd[5];
	time_t tt;		/* clock */
	struct tm l;		/* time */

	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->hid != 0) {
			/*
			 * XXX: I don't know if it is a good idea to talk to
			 * the display here as this function will be called
			 * on errors during init. p->hid may not be NULL even
			 * if the device could not be open!
			 */

			/* Set clock */
			if (p->showClock > 0) {
				tt = time(NULL);
				localtime_r(&tt, &l);
				/* Set time */
				Cmd[0] = 0x04;
				Cmd[1] = CMD_PREFIX;
				Cmd[2] = CMD_SETCLOCK;
				Cmd[3] = toBCD(l.tm_min);
				Cmd[4] = toBCD(l.tm_hour);
				hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 5);
				/* Show it */
				Cmd[0] = 0x03;
				Cmd[1] = CMD_PREFIX;
				Cmd[2] = p->showClock;
				Cmd[3] = 0x01;	/* 24 hour mode */
				hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 4);
			}
			/* Set dimming */
			Cmd[0] = 0x03;
			Cmd[1] = CMD_PREFIX;
			Cmd[2] = CMD_SETDIMM;
			if (p->offDimm)
				Cmd[3] = 1;
			else
				Cmd[3] = 2;
			hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 4);

			hid_close(p->hid);
			hid_delete_HIDInterface(&p->hid);
			p->hid = 0;
		}
		hid_cleanup();

		if (p->framebuf)
			free(p->framebuf);

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
mdm166a_width(Driver *drvthis)
{
	return WIDTH;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
mdm166a_height(Driver *drvthis)
{
	return HEIGHT;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
mdm166a_cellwidth(Driver *drvthis)
{
	return CELLWIDTH;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
mdm166a_cellheight(Driver *drvthis)
{
	return CELLHEIGHT;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
mdm166a_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, 0, MDM166A_SCREENSIZE);
	p->changed = 1;
}


/**
 * Flush data on screen to the VFD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
mdm166a_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int const PATH_OUT[1] = {0xff7f0004};
	char Cmd[64];
	int packed_begin = MDM166A_SCREENSIZE;
	int xpos = 0, ypos = 0, i, j;

	if (!p->changed)
		return;

	/* First clear the packed framebuffer area */
	memset(p->framebuf + packed_begin, 0, MDM166A_PACKEDSIZE);

	/*
	 * Convert framebuffer by packing pixel values from each column into
	 * 2 adjacent bytes (= 16 bit) of the packed part of the framebuffer.
	 */
	/* FIXME: Using the same memory area for framebuffer and packed
	 * framebuffer is hard to read. Better if two memory areas would be
	 * used.
	 */
	for (xpos = 0; xpos < MDM166A_XSIZE; xpos++)
		for (ypos = 0; ypos < MDM166A_YSIZE; ypos++)
			if (p->framebuf[ypos * MDM166A_XSIZE + xpos])
				p->framebuf[packed_begin + (2 * xpos) + (ypos / 8)] |= (1 << (7 - (ypos % 8)));

	/* Write data to display */
	/* Set position (0,0) */
	Cmd[0] = 0x03;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_SETRAM;
	Cmd[3] = 0;
	hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 4);

	/* Display accepts max. 64 bytes at a time -> Send 4 x 24 columns */
	Cmd[0] = 51;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_SETPIXEL;
	Cmd[3] = 48;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 24; j++) {
			/* Copy two bytes in each step (= 48 bytes) */
			Cmd[4 + 2 * j] = p->framebuf[packed_begin + i * 48 + 2 * j];
			Cmd[5 + 2 * j] = p->framebuf[packed_begin + i * 48 + 2 * j + 1];
		}
		hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 52);
	}

	p->changed = 0;
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
mdm166a_string(Driver *drvthis, int x, int y, const char string[])
{
	int i;
	x--;			/* Convert 1-based coords to 0-based */
	y--;

	for (i = 0; string[i]; i++)
		drawchar2fb(drvthis, x + i, y, string[i]);
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
mdm166a_chr(Driver *drvthis, int x, int y, char c)
{
	y--;			/* Convert 1-based coords to 0-based */
	x--;
	drawchar2fb(drvthis, x, y, c);
}


/**
 * Define a custom character and write it to the VFD. As the font is completely
 * held in memory every character may be overridden.
 *
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - 255].
 * \param dat      Array of 8 (=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom. \n
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
mdm166a_set_char(Driver *drvthis, int n, char *dat)
{
	int row;
	unsigned char mask = (1 << CELLWIDTH) - 1;

	if (n < 0 || n > 255)
		return;
	if (!dat)
		return;

	for (row = 0; row < CELLHEIGHT; row++) {
		glcd_iso8859_1[n][row] = dat[row] & mask;
	}
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param pattern  Options (currently unused).
 */
MODULE_EXPORT void
mdm166a_vbar(Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	PrivateData *p = drvthis->private_data;
	unsigned int offset;
	int i, j, pixels;

	debug(RPT_DEBUG, "%s: x=%i, y=%i, len=%i, promille=%i, options=%i",
	      __FUNCTION__, x, y, len, promille, pattern);

	x--;
	/* don't do y-- as we draw bottom up */

	if (x < 0 || y < 1 || x >= WIDTH || y > HEIGHT || len > HEIGHT) {
		return;
	}

	offset = x * CELLWIDTH + y * MDM166A_XSIZE * CELLHEIGHT;
	pixels = len * CELLHEIGHT * promille / 1000;

	for (i = 0; i < pixels; i++) {
		for (j = 0; j < CELLWIDTH; j++) {
			p->framebuf[offset + j] = 1;
		}
		/* go to next y-line */
		offset -= MDM166A_XSIZE;
	}

	p->changed = 1;
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param pattern  Options (currently unused).
 */
MODULE_EXPORT void
mdm166a_hbar(Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	PrivateData *p = drvthis->private_data;
	unsigned int offset;
	int i, j, pixels;

	debug(RPT_DEBUG, "%s: x=%i, y=%i, len=%i, promille=%i, options=%i",
	      __FUNCTION__, x, y, len, promille, pattern);

	x--;
	y--;

	if (y < 0 || y >= HEIGHT || x < 0 || len < 0 || x + len > WIDTH) {
		return;
	}

	/*
	 * Calculate starting point in framebuffer. Leave the leftmost column
	 * empty (+1). Shorten the length by 1 then.
	 */
	offset = x * CELLWIDTH + (y * CELLHEIGHT + 1) * MDM166A_XSIZE + 1;

	/* calculate length of bar */
	pixels = len * CELLWIDTH * promille / 1000 - 1;

	for (i = 0; i < CELLHEIGHT - 1; i++) {
		for (j = 0; j < pixels; j++) {
			p->framebuf[offset + j] = 1;
		}
		/* go to next y-line */
		offset += MDM166A_XSIZE;
	}

	p->changed = 1;
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     symbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
mdm166a_icon(Driver *drvthis, int x, int y, int icon)
{
	int icon_char;

	if ((icon_char = glcd_icon5x8(icon)) != -1) {
		mdm166a_chr(drvthis, x, y, icon_char);
		return 0;
	}
	return -1;
}


/**
 * Turn display backlight on or off.
 * This does not really toggle the backlight, but  tries to handle it more
 * intelligently: It sets the brightness to the value defined for the related
 * backlight state.
 *
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
mdm166a_backlight(Driver *drvthis, int on)
{
	int const PATH_OUT[1] = {0xff7f0004};
	char Cmd[5];
	PrivateData *p = drvthis->private_data;
	/* Set dimming */
	Cmd[0] = 0x03;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_SETDIMM;
	if ((p->dimm && (on == BACKLIGHT_ON)) || (p->offDimm && (on == BACKLIGHT_OFF)))
		Cmd[3] = 1;
	else
		Cmd[3] = 2;
	hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 4);
}


/**
 * Controls the custom icons.
 *
 * The bits of the \c on value map to icons as this (a bit set '1' means the
 * icon is switched on, a bit set '0' means the icon is turned off):
 * \verbatim
 * Bit    Funktion
 * 0      Play
 * 1      Pause
 * 2      Record
 * 3      Message
 * 4      @
 * 5      Mute
 * 6      WLAN tower
 * 7      Volume (the word)
 * 8..12  Volume level bar (decimal 0..28)
 * 13..14 WLAN strength (decimal 0..3)
 * \endverbatim
 *
 * \param drvthis  Pointer to driver structure.
 * \param on       Integer with bits representing LED states.
 */
MODULE_EXPORT void
mdm166a_output(Driver *drvthis, int on)
{
	char const Icon[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0a};
	PrivateData *p = drvthis->private_data;
	int const PATH_OUT[1] = {0xff7f0004};
	char Cmd[64];
	int icons_changed = on ^ p->last_output;
	int i, volume, wlan;

	/*
	 * Icons, volume bar or WLAN indicator are only changed if different
	 * from previous call.
	 */

	/* Set Icons. */
	Cmd[0] = 0x04;
	Cmd[1] = CMD_PREFIX;
	Cmd[2] = CMD_SETSYMBOL;
	for (i = 0; i < 8; i++) {
		if (icons_changed & (1 << i)) {
			Cmd[3] = Icon[i];
			Cmd[4] = ((on & (1 << i)) < 1) ? 0 : 1;
			hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 5);
		}
	}

	/*
	 * Volume (0..28). Note: Each bar element can individually be set to
	 * one of three states: off, dimmed or fully on.
	 */
	volume = (on >> 8) & 0x1F;
	if (volume != ((p->last_output >> 8) & 0x1F)) {
		Cmd[0] = 56;
		for (i = 0; i < 14; i++) {
			Cmd[i * 4 + 1] = CMD_PREFIX;
			Cmd[i * 4 + 2] = CMD_SETSYMBOL;
			Cmd[i * 4 + 3] = 0x0B + i;
			if (i < (volume / 2))
				Cmd[i * 4 + 4] = 2;
			else if (i == (volume / 2))
				Cmd[i * 4 + 4] = volume % 2;
			else
				Cmd[i * 4 + 4] = 0;
		}
		hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 57);
	}

	/* Wlan Strength (0..3) */
	wlan = (on >> 13) & 0x03;
	if (wlan != ((p->last_output >> 13) & 0x03)) {
		Cmd[0] = 12;
		for (i = 0; i < 3; i++) {
			Cmd[i * 4 + 1] = CMD_PREFIX;
			Cmd[i * 4 + 2] = CMD_SETSYMBOL;
			Cmd[i * 4 + 3] = 0x07 + i;
			Cmd[i * 4 + 4] = (i < wlan) ? 1 : 0;
		}
		hid_set_output_report(p->hid, PATH_OUT, sizeof(PATH_OUT), Cmd, 13);
	}

	p->last_output = on;
}


/**
 * Provide general information about the LCD/VFD display/driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
mdm166a_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->info;
}
