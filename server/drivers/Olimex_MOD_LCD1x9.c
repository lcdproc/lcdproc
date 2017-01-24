/** \file server/drivers/Olimex_MOD_LCD1x9.c
 * LCDd \c Olimex_MOD_LCD1x9 driver for MOD-LCD1x9 from Olimex.
 * This display uses a PCF857DU style controller.
 */

/* Copyright (C) 2015 Harald Geyer <harald@ccbib.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lcd.h"
#include "shared/report.h"

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "i2c.h"
#include "Olimex_MOD_LCD1x9_font.h"

#define BUS_ADDRESS 0x38
#define LEN_INIT_SEQ 24

/** private data for the \c Olimex_MOD_LCD1x9 driver */
typedef struct olimex1x9_private_data {
	I2CHandle *dev;		/**< handle of the i2c device */
	uint16_t buf[11];	/**< header + frame buffer */
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "olimex1x9_";

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
olimex1x9_init (Driver *drvthis)
{
	PrivateData *p;
	const char *configvalue;
	char buffer[LEN_INIT_SEQ];
	uint8_t *preamble;
	int i;

	/* Allocate and store private data */
	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	configvalue = drvthis->config_get_string(drvthis->name, "Device", 0, I2C_DEFAULT_DEVICE);
	p->dev = i2c_open(configvalue, BUS_ADDRESS);
	if (!p->dev) {
		report(RPT_ERR, "open i2c device '%s' failed: %s", configvalue, strerror(errno));
		return(-1);
	}

	buffer[0] = 0xC8; /* Mode register */
	buffer[1] = 0xF0; /* Blink register */
	buffer[2] = 0xE0; /* Device select register */
	buffer[3] = 0x00; /* Point register */
	for (i = 4; i < LEN_INIT_SEQ; ++i)
		buffer[i] = 0xFF;

	if (i2c_write(p->dev, buffer, LEN_INIT_SEQ) < 0) {
		report(RPT_ERR, "I2C: %s: sending of initialization data failed: %s", configvalue, strerror(errno));
		return (-1);
	}

	preamble = (uint8_t *) p->buf;
	preamble[0] = 0xE0; /* Device select register */
	preamble[1] = 0x00; /* Point register */

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
olimex1x9_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		i2c_close(p->dev);
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
olimex1x9_width (Driver *drvthis)
{
	return 9;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
olimex1x9_height (Driver *drvthis)
{
	return 1;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
olimex1x9_clear (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	/* Don't overwrite the preamble part of the buffer */
	memset(&p->buf[1], 0, sizeof(p->buf) - sizeof(p->buf[0]));
}


/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
olimex1x9_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	i2c_write(p->dev, p->buf, sizeof(p->buf));
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (9, 1).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
olimex1x9_string (Driver *drvthis, int x, int y, const unsigned char string[])
{
	PrivateData *p = drvthis->private_data;
	int pos = 10 - x; /* The left side is on the upper end of the buffer */
	int count = 0;

	while (string[count]) {
		switch (string[count]) {
		case '\b': /* Allow composition of characters */
			++pos;
			break;
		case ':':  /* We can't display a proper ':' */
		case '.':  /* Use the dot between characters if possible */
			if (pos < 9 &&
				!(p->buf[pos+1] & htons(olimex1x9_font['.'])))
					++pos;
			/* fall through */
		default:
			if (pos >= 1 && pos <= 9)  /* no buffer overflow */
				p->buf[pos] |=
					htons(olimex1x9_font[string[count]]);
			--pos;
		}
		++count;
	}
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (9, 1).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
olimex1x9_chr (Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	if (x < 1 || x > 9)
		return;

	p->buf[10-x] |= olimex1x9_font[c];
}
