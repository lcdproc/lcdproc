/** \file server/drivers/ula200.c
 * LCDd \c ula200 driver for the ULA-200 from ELV (http://www.elv.de).
 */

/*-
 * Copyright (C) 2006 Bernhard Walle <bernhard.walle@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <usb.h>
#include <ftdi.h>

#include "lcd.h"
#include "ula200.h"
#include "report.h"
#include "hd44780-charmap.h"
#include "adv_bignum.h"


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "ula200_";


/* -------------------------- constants ---------------------------------- */
#define DISPLAY_VENDOR_ID    0x0403
#define DISPLAY_PRODUCT_ID   0xf06d

#define CH_STX  0x02
#define CH_ETX  0x03
#define CH_ENQ  0x05
#define CH_ACK  0x06
#define CH_NAK  0x15
#define CH_DC2  0x12
#define CH_DC3  0x13

#define MAX_KEY_MAP 6
static char *default_key_map[MAX_KEY_MAP] = {"Up", "Down", "Left", "Right", "Enter", "Escape"};

#define CELLWIDTH  5
#define CELLHEIGHT 8

/* repeat conts for responses */
#define MAX_REPEATS 20

/* commands for senddata */
#define RS_DATA     0x00
#define RS_INSTR    0x01
#define SETCHAR     0x40

/* boolean data type */
#ifndef bool
# define bool short
# define true 1
# define false 0
#endif

/* ----------- Key ring implementation from CFontz633io ------------------ */

/* KeyRing management */
#define KEYRINGSIZE	16

typedef struct ula200_keyring {
	unsigned char contents[KEYRINGSIZE];
	int head;
	int tail;
} KeyRing;

/*
 * KeyRing handling functions. This separates the producer from the consumer.
 * It is just a small fifo of unsigned char.
 */

/** initialize/empty key ring by resetting its read & write pointers */
void
EmptyKeyRing(KeyRing *kr)
{
	kr->head = kr->tail = 0;
}


/** add byte to key ring; return success (byte added) / failure (key ring is full) */
int
AddKeyToKeyRing(KeyRing *kr, unsigned char key)
{
	if (((kr->head + 1) % KEYRINGSIZE) != (kr->tail % KEYRINGSIZE)) {

		kr->contents[kr->head % KEYRINGSIZE] = key;
		kr->head = (kr->head + 1) % KEYRINGSIZE;
		return 1;
	}

	/* KeyRing overflow: do not accept extra key */
	return 0;
}


/** get byte from key ring (or '\\0' if key ring is empty) */
unsigned char
GetKeyFromKeyRing(KeyRing *kr)
{
	unsigned char retval = '\0';

	kr->tail %= KEYRINGSIZE;

	if ((kr->head % KEYRINGSIZE) != kr->tail) {
		retval = kr->contents[kr->tail];
		kr->tail = (kr->tail + 1) % KEYRINGSIZE;
	}

	return retval;
}


/* ---------------------- ULA-200 specific functions --------------------- */

/** private data for the \c ula200 driver */
typedef struct ula200_private_data {
	struct ftdi_context ftdic;	/**< handle for the USB FTDI library */
	int width;			/**< display width in characters */
	int height;			/**< display height in characters */
	unsigned char *framebuf;	/**< framebuffer LCDd writes to */
	unsigned char *lcd_contents;	/**< content of screen for incr. upd. */
	unsigned char all_dirty;	/**< first time => all is dirty */
	int backlight;			/**< backlight: -1=unset, 0=off, 1=on */
	KeyRing keyring;		/**< input key ring */
	char *key_map[MAX_KEY_MAP];	/**< mapping of input keys */
} PrivateData;


/**
 * Reads a character from USB.
 * \param p  Pointer to drivers private data
 * \return  Character read from USB.
 */
static inline int
ula200_ftdi_usb_read(PrivateData *p)
{
	unsigned char buffer[1];
	int err;

	while ((err = ftdi_read_data(&p->ftdic, buffer, 1)) == 0);

	if (err < 0) {
		return -1;
	}
	else {
		return buffer[0];
	}
}


/**
 * Reads the response.
 *
 * \return true   if ACK was responded
 * \return false  if NAK was responded
 */
static bool
ula200_ftdi_read_response(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	bool result = false;
	bool answer_read = false;
	int ret;
	int ch;

	while (!answer_read) {
		/* wait until STX */
		while (((ret = ula200_ftdi_usb_read(p)) != CH_STX) && (ret > 0));
		if (ret < 0) {
			return false;
		}

		/* read next char */
		ch = ula200_ftdi_usb_read(p);

		switch (ch) {
		    case 't':
			ch = ula200_ftdi_usb_read(p);
			AddKeyToKeyRing(&p->keyring, (unsigned char)(ch - 0x40));
			break;

		    case CH_ACK:
			answer_read = true;
			result = true;
			break;

		    case CH_NAK:
			answer_read = true;
			break;

		    default:
			answer_read = true;
			report(RPT_INFO, "%s: read invalid answer (0x%02X)", drvthis->name, ch);
		}

		/* wait until ETX */
		while (((ret = ula200_ftdi_usb_read(p)) != CH_ETX) && (ret > 0));
		if (ret < 0) {
			return false;
		}
	}

	return result;
}

/**
 * Write a command to the display. Adds the STX and ETX header/trailer.
 *
 * \param drvthis  Pointer to driver structure
 * \param data     the data bytes
 * \param length   the number of bytes in @p data which are valid
 * \param escape   if the data should be escaped (see the User's Guide of the
 *                 ULA-200)
 * \return  0 on success, negative value on error
 */
static int
ula200_ftdi_write_command(Driver *drvthis, unsigned char *data, int length, bool escape)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i, err;
	int repeat_count = 0;
	int pos = 0;
	unsigned char buffer[1024];

	if (length > 512) {
		return -EINVAL;
	}

	/* fill the array */
	buffer[pos++] = CH_STX;
	for (i = 0; i < length; i++) {
		if (escape) {
			if (data[i] == CH_STX) {
				buffer[pos++] = CH_ENQ;
				buffer[pos++] = CH_DC2;
			}
			else if (data[i] == CH_ETX) {
				buffer[pos++] = CH_ENQ;
				buffer[pos++] = CH_DC3;
			}
			else if (data[i] == CH_ENQ) {
				buffer[pos++] = CH_ENQ;
				buffer[pos++] = CH_NAK;
			}
			else {
				buffer[pos++] = data[i];
			}
		}
		else {
			buffer[pos++] = data[i];
		}
	}
	buffer[pos++] = CH_ETX;

	do {
		/* bytes */
		err = ftdi_write_data(&p->ftdic, buffer, pos);
		if (err < 0) {
			report(RPT_WARNING, "%s: ftdi_write_data failed", drvthis->name);
			return -1;
		}
	}
	while (!ula200_ftdi_read_response(drvthis)&&(repeat_count++ < MAX_REPEATS));

	return 0;
}

/**
 * Clear the display using display command.
 * \return  Error code from ula200_ftdi_write_command.
 */
static int
ula200_ftdi_clear(Driver *drvthis)
{
	unsigned char command[1];
	int err;

	command[0] = 'l';
	err = ula200_ftdi_write_command(drvthis, command, 1, true);
	if (err < 0) {
		report(RPT_WARNING, "%s: ula200_ftdi_clear: "
		       "ula200_ftdi_write_command failed", drvthis->name);
	}

	return err;
}


/**
 * Set the position.
 */
static int
ula200_ftdi_position(Driver *drvthis, int x, int y)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char command[5];
	int err;

	if (y >= 2) {
		y -= 2;
		x += p->width;
	}

	command[0] = 'p';
	command[1] = x;
	command[2] = y;
	err = ula200_ftdi_write_command(drvthis, command, 3, true);
	if (err < 0) {
		report(RPT_WARNING, "%s: ula200_ftdi_position(%d,%d): "
		       "ula200_ftdi_write_command failed",
		       drvthis->name, x, y);
	}

	return err;
}


/**
 * Send raw data.
 *
 * \param drvthis  Pointer to driver
 * \param flags    Flag indicating byte is command or data
 * \param ch       Data byte to send
 * \return  Error code from ula200_ftdi_write_command.
 */
static int
ula200_ftdi_rawdata(Driver *drvthis, unsigned char flags, unsigned char ch)
{
	unsigned char command[3];
	int err;

	command[0] = 'R';
	command[1] = flags == (RS_DATA) ? '2' : '0';
	command[2] = ch;
	err = ula200_ftdi_write_command(drvthis, command, 3, false);
	if (err < 0) {
		report(RPT_ERR, "%s: ftdi_write_command() failed", drvthis->name);
	}

	return err;
}


/**
 * Writes a string to the display.
 *
 * \param drvthis  Pointer to driver
 * \param string   Pointer to string
 * \param len      Length of the string
 * \return  -EINVAL  If the string is to long (max 80 characters)
 * \return  err      Error code from ula200_ftdi_write_command
 */
static int
ula200_ftdi_string(Driver *drvthis, const unsigned char *string, int len)
{
	unsigned char buffer[128];
	int err;
	int i;

	if (len > 80) {
		return -EINVAL;
	}

	buffer[0] = 's';
	buffer[1] = len;
	for (i = 0; i < len; i++) {
		buffer[i + 2] = HD44780_charmap[(unsigned char)string[i]];
	}

	err = ula200_ftdi_write_command(drvthis, buffer, len + 2, true);
	if (err < 0) {
		report(RPT_WARNING, "%s: ula200_ftdi_string: "
		       "ula200_ftdi_write_command() failed", drvthis->name);
	}

	return err;
}


/**
 * Enables the raw register access mode.
 * \param drvthis  Pointer to driver
 * \return  Error code from ula200_ftdi_write_command.
 */
static int
ula200_ftdi_enable_raw_mode(Driver *drvthis)
{
	unsigned char command[3];

	report(RPT_DEBUG, "%s: enable raw mode", drvthis->name);

	command[0] = 'R';
	command[1] = 'E';
	command[2] = '1';
	return ula200_ftdi_write_command(drvthis, command, 3, false);
}

/**
 * Loads custom characters in the display.
 */
static int
ula200_load_custom_chars(Driver *drvthis)
{
	int i, row;
	int err = 0;
	unsigned char mask = (1 << CELLWIDTH) - 1;
	char custom_chars[8][CELLHEIGHT] = {
		{
			b__XXXXX,
			b__XXXXX,
			b__XXXXX,
			b__XXXXX,
			b__XXXXX,
			b__XXXXX,
			b__XXXXX,
			b__XXXXX
		},
		{
			b__XXXXX,
			b__X_X_X,
			b_______,
			b_______,
			b_______,
			b__X___X,
			b__XX_XX,
			b__XXXXX
		},
		{
			b__XXXXX,
			b__X_X_X,
			b___X_X_,
			b___XXX_,
			b___XXX_,
			b__X_X_X,
			b__XX_XX,
			b__XXXXX
		},
		{
			b____X__,
			b___XXX_,
			b__X_X_X,
			b____X__,
			b____X__,
			b____X__,
			b____X__,
			b_______
		},
		{
			b____X__,
			b____X__,
			b____X__,
			b____X__,
			b__X_X_X,
			b___XXX_,
			b____X__,
			b_______
		},
		{
			b_______,
			b_______,
			b__XXXXX,
			b__X___X,
			b__X___X,
			b__X___X,
			b__XXXXX,
			b_______
		},
		{
			b____X__,
			b____X__,
			b__XXX_X,
			b__X_XX_,
			b__X_X_X,
			b__X___X,
			b__XXXXX,
			b_______
		},
		{
			b_______,
			b_______,
			b__XXXXX,
			b__X_X_X,
			b__XX_XX,
			b__X_X_X,
			b__XXXXX,
			b_______
		}
	};

	for (i = 0; i < 8 && err == 0; i++) {
		/* Tell the HD44780 we will redefine char number i */
		ula200_ftdi_rawdata(drvthis, RS_INSTR, SETCHAR | i * 8);
		if (err < 0) {
			report(RPT_WARNING, "%s: ula200_ftdi_rawdata failed", drvthis->name);
			break;
		}

		/* Send the subsequent rows */
		for (row = 0; row < CELLHEIGHT; row++) {
			int value = custom_chars[i][row] & mask;
			err = ula200_ftdi_rawdata(drvthis, RS_DATA, value);
			if (err < 0) {
				report(RPT_WARNING, "%s: ula200_ftdi_rawdata failed", drvthis->name);
				break;
			}
		}
	}

	return err;
}

/* --------------------------- API functions ----------------------------- */

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
ula200_init(Driver *drvthis)
{
	PrivateData *p;
	int err, i;
	const char *s;

	/* Allocate and store private data */
	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL) {
		return -1;
	}
	if (drvthis->store_private_ptr(drvthis, p)){
		return -1;
	}

	p->backlight = -1;
	p->all_dirty = 1;
	EmptyKeyRing(&p->keyring);

	/* Get and parse size */
	s = drvthis->config_get_string(drvthis->name, "size", 0, "20x4");
	if ((sscanf(s, "%dx%d", &(p->width), &(p->height)) != 2)
	    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
	    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s: cannot read Size %s", drvthis->name, s);
		return -1;
	}

	/* read the keymap */
	for (i = 0; i < MAX_KEY_MAP; i++) {
		char buf[40];

		/* First fill with default value */
		p->key_map[i] = default_key_map[i];

		/* Read config value */
		sprintf(buf, "KeyMap_%c", i + 'A');
		s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

		/* Was a key specified in the config file ? */
		if (s != NULL) {
			p->key_map[i] = strdup(s);
			report(RPT_INFO, "%s: Key '%c' mapped to \"%s\"",
			       drvthis->name, i + 'A', s);
		}
	}

	/* End of config file parsing */


	/* Allocate framebuffer */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		goto err_begin;
	}

	/* Allocate and clear the buffer for incremental updates */
	p->lcd_contents = (unsigned char *) malloc(p->width * p->height);
	if (p->lcd_contents == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer backing store", drvthis->name);
		goto err_framebuf;
	}
	memset(p->lcd_contents, 0, p->width * p->height);

	/* open the FTDI library */
	ftdi_init(&p->ftdic);
	(&p->ftdic)->usb_write_timeout = 20;
	(&p->ftdic)->usb_read_timeout = 20;

	/* open the device */
	err = ftdi_usb_open(&p->ftdic, DISPLAY_VENDOR_ID, DISPLAY_PRODUCT_ID);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot open USB device", drvthis->name);
		goto err_lcd;
	}

	/* set the baudrate */
	err = ftdi_set_baudrate(&p->ftdic, 19200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	/* set communication parameters */
	err = ftdi_set_line_property(&p->ftdic, BITS_8, STOP_BIT_1, EVEN);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	/* user is able to write commands */
	err = ula200_ftdi_enable_raw_mode(drvthis);
	if (err < 0) {
		report(RPT_ERR, "%s: unable to enable the raw mode", drvthis->name);
		goto err_ftdi;
	}

	/* load the chars */
	err = ula200_load_custom_chars(drvthis);
	if (err < 0) {
		report(RPT_ERR, "%s: unable to write the custom characters", drvthis->name);
		goto err_ftdi;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;

err_ftdi:
	ftdi_usb_close(&p->ftdic);
	ftdi_deinit(&p->ftdic);
err_lcd:
	free(p->lcd_contents);
err_framebuf:
	free(p->framebuf);
err_begin:
	return -1;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ula200_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p != NULL) {
		ftdi_usb_purge_buffers(&p->ftdic);
		ftdi_usb_close(&p->ftdic);
		ftdi_deinit(&p->ftdic);

		if (p->framebuf != NULL)
			free(p->framebuf);

		if (p->lcd_contents != NULL)
			free(p->lcd_contents);

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
ula200_width(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
ula200_height(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ula200_clear(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);
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
ula200_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
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
ula200_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x--;			/* Convert 1-based coords to 0-based */
	y--;

	for (i = 0; string[i] != '\0'; i++) {
		/* Check for buffer overflows... */
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
ula200_backlight(Driver *drvthis, int on)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char command[2];
	int err;

	if (p->backlight != on) {
		p->backlight = on;

		command[0] = 'h';
		command[1] = (on) ? '1' : '0';
		err = ula200_ftdi_write_command(drvthis, command, 2, false);
		if (err < 0)
			report(RPT_WARNING, "%s: error in ula200_ftdi_write_command",
			       drvthis->name);
		else
			report(RPT_INFO, "%s: turn backlight %s",
			       drvthis->name, (on) ? "on" : "off");
	}
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ula200_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int x, y;
	int wid = p->width;
	char ch;
	char drawing;
	int count;
	int firstdiff;
	int lastdiff;

	if (p->all_dirty) {
		ula200_ftdi_clear(drvthis);
		p->all_dirty = 0;
	}

	/* Update LCD incrementally by comparing with last contents */
	count = 0;
	for (y = 0; y < p->height; y++) {
		drawing = 0;
		firstdiff = -1;
		lastdiff = 0;
		for (x = 0; x < wid; x++) {
			ch = p->framebuf[(y * wid) + x];
			if (ch != p->lcd_contents[(y * wid) + x]) {
				p->lcd_contents[(y * wid) + x] = ch;
				if (firstdiff == -1) {
					firstdiff = x;
				}
				lastdiff = x;
			}
		}

		if (firstdiff >= 0) {
			ula200_ftdi_position(drvthis, firstdiff, y);
			ula200_ftdi_string(drvthis, p->framebuf + (y * wid) + firstdiff,
					   lastdiff - firstdiff + 1);
		}
	}
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
ula200_icon(Driver *drvthis, int x, int y, int icon)
{
	switch (icon) {
		case ICON_BLOCK_FILLED:
		ula200_chr(drvthis, x, y, 0xff);
		break;
	    case ICON_HEART_FILLED:
		ula200_chr(drvthis, x, y, 2);
		break;
	    case ICON_HEART_OPEN:
		ula200_chr(drvthis, x, y, 1);
		break;
	    case ICON_ARROW_UP:
		ula200_chr(drvthis, x, y, 3);
		break;
	    case ICON_ARROW_DOWN:
		ula200_chr(drvthis, x, y, 4);
		break;
	    case ICON_ARROW_LEFT:
		ula200_chr(drvthis, x, y, 0x1B);
		break;
	    case ICON_ARROW_RIGHT:
		ula200_chr(drvthis, x, y, 0x1A);
		break;
	    case ICON_CHECKBOX_OFF:
		ula200_chr(drvthis, x, y, 5);
		break;
	    case ICON_CHECKBOX_ON:
		ula200_chr(drvthis, x, y, 6);
		break;
	    case ICON_CHECKBOX_GRAY:
		ula200_chr(drvthis, x, y, 7);
		break;
	    default:
		return -1;	/* Let the core do other icons */
	}
	return 0;
}


/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL for nothing available / unmapped key.
 */
MODULE_EXPORT const char *
ula200_get_key(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char key;
	int i;

	/*
	 * The libftdi has no non-blocking read (`select' system call), so we
	 * force a read that could not block by updating one character on the
	 * display. As long as lcdproc is single-threaded, we can write to
	 * the display because we're not inside a read here.
	 */
	ula200_ftdi_position(drvthis, 0, 0);
	ula200_ftdi_string(drvthis, p->lcd_contents, 1);

	key = GetKeyFromKeyRing(&p->keyring);

	/* search the bit that was set by the hardware */
	for (i = 0; i < MAX_KEY_MAP; i++) {
		if (key & (1 << i))
			return p->key_map[i];
	}

	if (key != '\0') {
		report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
	}
	return NULL;
}
