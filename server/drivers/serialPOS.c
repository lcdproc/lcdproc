/**
 * \file server/drivers/serialPOS.c
 *
 * LCDd \c serialPOS driver for Point Of Sale ("POS") devices using
 * various protocols.
 */

/*-
 * This is the LCDproc driver for Point Of Sale ("POS") devices using
 * various protocols.  While it currently only supports AEDEX,
 * it can be extended to provide support for many POS emulation types.
 *
 * Copyright (C) 2006, 2007 Eric Pooch
 * Copyright (C) 2018 Shenghao Yang
 *
 * This driver is based on MtxOrb.c driver and is subject to its copyrights.
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
 *
 *
 * List of driver entry point:
 *
 * init        Implemented.
 * close       Implemented.
 * width       Implemented.
 * height      Implemented.
 * clear       Implemented.
 * flush       Implemented.
 * string      Implemented.
 * chr     Implemented.
 * vbar        Implemented.
 * hbar        Implemented.
 * num     Implemented.
 * heartbeat   Implemented.
 * icon        NOT IMPLEMENTED: not part of any POS protocol
 * cursor      Implemented.
 * set_char    NOT IMPLEMENTED
 * get_free_chars  Implemented: always returns 0
 * cellwidth   Implemented.
 * cellheight  Implemented.
 * get_contrast    NOT IMPLEMENTED: not part of AEDEX protocol
 * set_contrast    NOT IMPLEMENTED: not part of AEDEX protocol
 * get_brightness  IMPLEMENTED
 * set_brightness  IMPLEMENTED
 * backlight   IMPLEMENTED
 * output      NOT IMPLEMENTED: not part of any POS protocol
 * get_key     Implemented for devices using a pass-through serial port
 * 	       connected to an RS232 terminal or keyboard.
 * get_info    Implemented.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lcd.h"
#include "serialPOS.h"
#include "serialPOS_common.h"
#include "adv_bignum.h"

#include "shared/report.h"

/*
 * The serialPOS driver does not use a lot of hardware features,
 * because the available hardware features (automatic blinking,
 * scrolling, multi-segment scrolling) vary between displays,
 * and are also not very configurable.
 *
 * In order to keep the interface between protocol drivers and the
 * main driver simple, we let the server core handle most of the work
 * while we step in to draw some of the widgets like hbar and vbar.
 *
 * The frame buffer holds all the requested changes until the
 * server asks us to flush the changes to the display.
 * This also permits us to do an incremental update and reduce the
 * number of characters to be sent to the display across the serial link.
 *
 * The rationale for sending as little commands as possible through
 * the serial link is to ensure that the display does not get flooded
 * with commands. Some displays cannot keep up with high command rates.
 *
 * At the same time, we do not want to minimize expenditure too much.
 * Updating individual characters by commanding multiple cursor shifts
 * is a good idea, but some displays cannot cope, and will garble
 * the screen, as they misplace newly received characters.
 *
 * Hence, most protocol drivers utilize the atomic single
 * line update operations as much as possible, (see AEDEX, CD5220: this
 * has the added side effect of making updating smoother, as the user
 * does not see intermediate characters), or, shift the cursor to
 * each line and update a whole line's worth of information at one go.
 *
 * In order to display graphic widgets, we define and use our own
 * custom characters. At this time, only custom characters for
 * vertical bars, and horizontal bars, the most commonly used
 * custom character features by LCDproc, are defined. Additions to
 * support icons are welcome, but that has to be done in the protocol
 * driver initialization code itself.
 *
 * Note that for some displays, the custom characters uploaded
 * are immediately written to EEPROM, and, hence, we do not support
 * uploading custom characters after display
 * initialization, as that can cause excessive EEPROM wear.
 */

/* Vars for the server core */
MODULE_EXPORT char* api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char* symbol_prefix = "serialPOS_";

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
serialPOS_init(Driver* drvthis)
{
	struct termios portset;

	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	char cell_size[256] = DEFAULT_CELL_SIZE;
	char buf[256] = "";
	int tmp, w, h, cw, ch;

	PrivateData *p;

	/* Alocate and store private data */
	p = (PrivateData*) calloc(1, sizeof(PrivateData));
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialise the PrivateData structure */
	p->fd = -1;

	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	p->buffered_misc_state.brightness = 1000;
	p->buffered_misc_state.backlight_state = BACKLIGHT_ON;
	p->buffered_misc_state.cursor_state = CURSOR_OFF;

	p->custom_chars_supported = DEFAULT_CUSTOM_CHARS;

	p->framebuf = NULL;
	p->backingstore = NULL;
	p->protocol_ops = NULL;

	debug(RPT_INFO, "serialPOS: init(%p)", drvthis);

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device,
		drvthis->config_get_string(drvthis->name, "Device", 0,
					   DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	/* Get emulation type */
	strncpy(buf,
		drvthis->config_get_string(drvthis->name, "Type", 0,
					   DEFAULT_TYPE), sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	if (strncasecmp(buf, "AED", 3) == 0) {
		p->protocol_ops = &serialPOS_aedex_ops;
	}
	else if (strncasecmp(buf, "CD5", 3) == 0) {
		p->protocol_ops = &serialPOS_cd5220_ops;
	}
	else if (strncasecmp(buf, "Eps", 3) == 0) {
		p->protocol_ops = &serialPOS_epson_ops;
	}
	else if (strncasecmp(buf, "Ema", 3) == 0) {
		/* We implement the most basic EMAX dialect,
		 * which is compatible with AEDEX */
		p->protocol_ops = &serialPOS_aedex_ops;
	}
	else if (strncasecmp(buf, "Log", 3) == 0) {
		p->protocol_ops = &serialPOS_logic_controls_ops;
	}
	else if (strncasecmp(buf, "Ult", 3) == 0) {
		/* UTC/S compatible with Logic Controls */
		p->protocol_ops = &serialPOS_logic_controls_ops;
	}
	else {
		report(RPT_ERR,
		       "%s: unknown display Type %s; must be one of "
		       "AEDEX, CD5220, Epson, Emax, Logic Controls or Ultimate",
		       drvthis->name, buf);
		return -1;
	}

	/* Get display size */
	strncpy(size,
		drvthis->config_get_string(drvthis->name, "Size", 0,
					   DEFAULT_SIZE), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2) || (w <= 0)
	    || (w > MAX_WIDTH)
	    || (h <= 0) || (h > MAX_HEIGHT)) {
		report(RPT_WARNING,
		       "%s: cannot read Size / "
		       "Size out-of-range: %s; using default %s",
		       drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Read custom character setting */
	long int config_n_cust_chars =
	    drvthis->config_get_int(drvthis->name, "Custom_chars", 0, INT_MIN);
	if ((config_n_cust_chars < 0)
	    || (config_n_cust_chars > MAX_CUSTOM_CHARS)) {
		report(RPT_WARNING,
		       "%s: cannot read / out-of-range values for "
		       "Custom_chars: %ld; using default: %d",
		       drvthis->name, config_n_cust_chars,
		       DEFAULT_CUSTOM_CHARS);
	} else {
		p->custom_chars_supported = config_n_cust_chars;
	}

	/* Read cell size */
	strncpy(cell_size,
		drvthis->config_get_string(drvthis->name, "Cellsize", 0,
					   DEFAULT_CELL_SIZE),
					   sizeof(cell_size));
	cell_size[sizeof(cell_size) - 1] = '\0';
	if ((sscanf(cell_size, "%dx%d", &cw, &ch) != 2) || (cw <= 0)
	    || (cw > MAX_CELLWID)
	    || (ch <= 0) || (ch > MAX_CELLHGT)) {
		report(RPT_WARNING,
		       "%s: cannot read Cellsize / "
		       "Cellsize out-of-range: %s; using default %s",
		       drvthis->name, cell_size, DEFAULT_CELL_SIZE);
		sscanf(DEFAULT_CELL_SIZE, "%dx%d", &cw, &ch);
	}
	p->cellheight = ch;
	p->cellwidth = cw;


	/* Get speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0,
				      DEFAULT_SPEED);
	switch (tmp) {
	    case 1200:
		speed = B1200;
		break;
	    case 2400:
		speed = B2400;
		break;
	    case 4800:
		speed = B4800;
		break;
	    case 9600:
		speed = B9600;
		break;
	    case 19200:
		speed = B19200;
		break;
	    case 115200:
		speed = B115200;
		break;
	    default:
		speed = B9600;
		report(RPT_WARNING,
		       "%s: Speed must be either "
		       "1200, 2400, 4800, 9600, 19200, or 115200 "
		       "; using default %d", drvthis->name, tmp);
	}

	/* Set up io port correctly, and open it... */
	p->fd = open(device, O_RDWR | O_NOCTTY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name,
		       device, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: %s device could not be opened...",
			       drvthis->name, device);
		return -1;
	}
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

	if (tcgetattr(p->fd, &portset)) {
		report(RPT_ERR,
		       "%s: unable to get terminal attributes for "
		       "device: %s",
		       drvthis->name, device);
		return -1;
	}

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
#else
	/* The hard way */
	portset.c_iflag &=
	    ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	portset.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	/* Set timeouts */
	portset.c_cc[VMIN]  = 1;
	portset.c_cc[VTIME] = 3;

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	if (tcsetattr(p->fd, TCSANOW, &portset)) {
		report(RPT_ERR,
		       "%s: unable to set terminal attributes for"
		       " device: %s",
		       drvthis->name, device);
	}

	/* Make sure the frame buffer is there... */
	p->framebuf = (uint8_t*) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer",
		       drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* make sure the framebuffer backing store is there... */
	p->backingstore = (uint8_t*) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR,
		       "%s: unable to create framebuffer backing store",
		       drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->width * p->height);

	p->buffer_size = p->protocol_ops->command_buffer_sz(p);

	/* set initial LCD configuration */
	uint8_t buffer[p->buffer_size];
	int rtn;
	if ((rtn = p->protocol_ops->init(p, buffer)) < 0) {
		report(RPT_ERR,
		       "%s: unable to initialize protocol sub-driver"
		       " for the %s protocol. Check config.",
		       drvthis->name, buf);
		return -1;
	}
	write(p->fd, buffer, rtn);

	report(RPT_INFO,
	       "%s: initialized with display size of %dx%d, "
	       "cell size of: %dx%d, with %d custom characters supported"
	       " using the %s protocol",
	       drvthis->name, p->width, p->height, p->cellwidth, p->cellheight,
	       p->custom_chars_supported, buf);
	debug(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_close(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->backingstore)
			free(p->backingstore);
		p->backingstore = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
	debug(RPT_DEBUG, "serialPOS: closed");
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
serialPOS_width(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	return p->width;
}

/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
serialPOS_height(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	return p->height;
}

/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always \c 0 ).
 */
MODULE_EXPORT int
serialPOS_get_free_chars(Driver* drvthis)
{
	return 0;
}

/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
serialPOS_cellwidth(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	return p->cellwidth;
}

/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
serialPOS_cellheight(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	return p->cellheight;
}

/**
 * Get backlight for a particular state
 *
 * This implementation returns the brightness in promille for
 * \c BACKLIGHT_ON only, otherwise it returns \c 0.
 *
 * \param drvthis pointer to driver structure
 * \param state backlight state
 */
MODULE_EXPORT int
serialPOS_get_brightness(Driver* drvthis, int state)
{
	PrivateData* p = drvthis->private_data;
	return ((state == BACKLIGHT_ON) ? p->buffered_misc_state.brightness :
					  0);
}

/**
 * Set brightness for a particular state
 *
 * The implementation in serialPOS doesn't care about the state, and it
 * always sets the brightness value to the one given, because there is
 * only an OFF state and an ON state - the rest of the backlight states
 * are handled by the core.
 *
 * \param drvthis Pointer to driver structure
 * \param state backlight state
 * \param promille brightness value in promille
 */
MODULE_EXPORT void
serialPOS_set_brightness(Driver* drvthis, int state, int promille)
{
	PrivateData* p			  = drvthis->private_data;
	p->buffered_misc_state.brightness = promille;
	debug(RPT_DEBUG, "serialPOS: backlight value set to: %d", promille);
}

/**
 * Set backlight state
 *
 * \param drvthis Pointer to driver structure
 * \param state backlight state
 */
MODULE_EXPORT void
serialPOS_backlight(Driver* drvthis, int state)
{
	PrivateData* p			       = drvthis->private_data;
	p->buffered_misc_state.backlight_state = state;
	if (state != BACKLIGHT_OFF)
		p->buffered_misc_state.brightness = 1000;
	else
		p->buffered_misc_state.brightness = 0;
	debug(RPT_DEBUG, "serialPOS: backlight state set to: %d", state);
}

/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is
 * (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal character position (column).
 * \param y Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
serialPOS_string(Driver* drvthis, int x, int y, const char string[])
{
	PrivateData* p = drvthis->private_data;
	int i;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		/* Check for buffer overflows... */
		if (x >= 0)
			p->framebuf[(y * p->width) + x] = string[i];
	}

	debug(RPT_DEBUG, "serialPOS: printed string at (%d,%d)", x, y);
}

/**
 * Clear the framebuffer.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_clear(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;

	/* replace all chars in framebuf with spaces */
	memset(p->framebuf, ' ', (p->width * p->height));

	debug(RPT_DEBUG, "serialPOS: cleared framebuffer");
}

/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_flush(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;
	uint8_t buffer[p->buffer_size];
	int rtn;

	if ((rtn = p->protocol_ops->flush(p, buffer)) > 0) {
		write(p->fd, buffer, rtn);
	}
	debug(RPT_DEBUG, "serialPOS: frame buffer flushed");
}

/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is
 * (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal character position (column).
 * \param y Vertical character position (row).
 * \param c Character that gets written.
 */
MODULE_EXPORT void
serialPOS_chr(Driver* drvthis, int x, int y, char c)
{
	PrivateData* p = drvthis->private_data;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;

	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
}

/**
 * Provide general information about the POS display.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char*
serialPOS_get_info(Driver* drvthis)
{
	PrivateData* p = drvthis->private_data;
	strcpy(p->info, "Driver for Point of Sale Displays.");
	return p->info;
}

/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal character position (column) of the starting point.
 * \param y Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
serialPOS_vbar(Driver* drvthis, int x, int y, int len, int promille,
	       int options)
{
	PrivateData* p = drvthis->private_data;
	// map
	char ascii_map[] = {' ', ' ', '-', '-', '=', '=', '%', '%'};
	char* map	= ascii_map;
	int pixels       = (promille / (1000 / (p->cellheight * len)));
	int pos;

	if ((x <= 0) || (y <= 0) || (x > p->width))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar
	 * should be filled.
	 */

	for (pos = 0; pos < len; pos++) {
		if (y - pos <= 0)
			return;

		if (pixels >= p->cellheight) {
			/* write a "full" block to the screen... */
			if (p->vbar_custom)
				serialPOS_chr(drvthis, x, y - pos,
					      p->protocol_ops->cust_char_code(
						  p, p->cellwidth - 1));
			else
				serialPOS_chr(drvthis, x, y - pos, '%');
		}
		else if (pixels > 0) {
			/* write a partial block... */
			if (p->vbar_custom)
				serialPOS_chr(
				    drvthis, x, y - pos,
				    p->protocol_ops->cust_char_code(
					p, p->cellwidth - 1 + pixels));
			else
				serialPOS_chr(drvthis, x, y - pos, map[pixels]);
			break;
		}
		else {
			;
		}

		pixels -= p->cellheight;
	}
}

/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal character position (column) of the starting point.
 * \param y Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
serialPOS_hbar(Driver* drvthis, int x, int y, int len, int promille,
	       int options)
{
	PrivateData* p = drvthis->private_data;
	int pixels     = (promille / (1000 / (p->cellwidth * len)));
	int pos;

	if ((x <= 0) || (y <= 0) || (y > p->height))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar
	 * should be filled.
	 */

	for (pos = 0; pos < len; pos++) {
		if (!p->hbar_custom) {
			if (x + pos > p->width)
				return;

			if ((pixels >= (p->cellwidth * 2 / 3))
			    && (pixels != 0)) {
				serialPOS_chr(drvthis, x + pos, y, '=');
			} else if (pixels > (p->cellwidth * 1 / 3)) {
				serialPOS_chr(drvthis, x + pos, y, '-');
			} else {
				; /* write nothing (not even a space) */
			}
		}
		else {
			if (pixels > p->cellwidth)
				serialPOS_chr(drvthis, x + pos, y,
					      p->protocol_ops->cust_char_code(
						  p, p->cellwidth - 1));
			if ((pixels <= p->cellwidth) && (pixels > 0))
				serialPOS_chr(drvthis, x + pos, y,
					      p->protocol_ops->cust_char_code(
						  p, pixels - 1));
		}

		pixels -= p->cellwidth;
	}
}

/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
serialPOS_num(Driver* drvthis, int x, int num)
{
	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, 0, 0);
}

/**
 * Set cursor position and state.
 * \param drvthis  Pointer to driver structure.
 * \param x Horizontal cursor position (column).
 * \param y Vertical cursor position (row).
 * \param state    New cursor state.
 */
MODULE_EXPORT void
serialPOS_cursor(Driver* drvthis, int x, int y, int state)
{
	PrivateData* p = drvthis->private_data;
	if ((x > p->width) || (x < 0) || (y > p->height) || (y < 0))
		return;

	p->buffered_misc_state.cursor_state = state;
	p->buffered_misc_state.cx	   = x;
	p->buffered_misc_state.cy	   = y;
}

/**
 * Get key from a pass-through port of the POS display.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char*
serialPOS_get_key(Driver* drvthis)
{
	PrivateData *p = drvthis->private_data;
	int ret;
	char buf;
	const char *key = NULL;
	static struct timeval selectTimeout = { 0, 0 };
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(p->fd, &fdset);

	if ((ret = select(FD_SETSIZE, &fdset, NULL, NULL, &selectTimeout))
	    < 0) {
		debug(RPT_DEBUG, "%s: get_key: select() failed (%s)",
		      drvthis->name, strerror(errno));
		return NULL;
	}
	if (!ret) {
		FD_SET(p->fd, &fdset);
		return NULL;
	}

	if (!FD_ISSET(p->fd, &fdset))
		return NULL;

	if ((ret = read(p->fd, &buf, 1)) < 0) {
		debug(RPT_DEBUG, "%s: get_key: read() failed (%s)",
		      drvthis->name, strerror(errno));
		return NULL;
	}

	if (ret == 1) {
		switch (buf) {
		    case 13:
			key = "Enter";
			break;
		    case 65:
			key = "Up";
			break;
		    case 66:
			key = "Down";
			break;
		    case 67:
			key = "Right";
			break;
		    case 68:
			key = "Left";
			break;
		    case 8:
			key = "Escape";
			break;
		    default:
			debug(RPT_DEBUG,
				"%s get_key: illegal key 0x%02X",
				drvthis->name, buf);
			return NULL;
		}

		debug(RPT_DEBUG, "%s: get_key: returns %s", drvthis->name,
		      key);
		return key;
	}

	return NULL;
}

/* EOF */
