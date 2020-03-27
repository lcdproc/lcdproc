/** \file server/drivers/icp_a106.c
 * LCDd \c icp_a106 for ICP Peripheral Communication Protocol devices
 */

/* 
 * This is the LCDproc driver for the ICP Peripheral Communication Protocol
 * used by the ICP A106 and A125 LCD boards.  The A125 is used in QNAP devices
 * 
 * Both LCD and alarm functions are accessed via one serial port, using
 * separate commands. Unfortunately, the device runs at slow 1200bps and the
 * LCD does not allow user-defined characters, so the bargraphs do not look
 * very nice.
 * 
 * Copyright (C) 2002 Michael Schwingen <michael@schwingen.org>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * 
 * This driver is mostly based on the HD44780 and the LCDM001 driver.
 * (Hopefully I have NOT forgotten any file I have stolen code from. If so
 * send me an e-mail or add your copyright here!)
 * 
 * - changes to support A125 and buttons made by Sam Bingner <sam@bingner.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lcd.h"
#include "icp_a106.h"
#include "shared/report.h"


/** private data for the \c icp_a106 driver */
typedef struct icp_a106_private_data {
	unsigned char *framebuf;
	unsigned char *last_framebuf;
	unsigned char serial_buf[4];
	int serial_buf_offset;
	struct timeval time_pressed[MAX_BUTTONS];
	bool buttonState[MAX_BUTTONS];
	uint16_t buttonStates;
	int width;
	int height;
	int backlight;
	int fd;
} PrivateData;

enum {
	A106_BL_OFF = 0,
	A106_BL_ON = 1
};


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "icp_a106_";


/**
 * Initialize the driver.  Opens the COM port and sets BAUD rate.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
icp_a106_init(Driver *drvthis)
{
	char device[200];
	int speed = B1200;
	struct termios portset;

	PrivateData *p;

	debug(RPT_INFO, "ICP_A106: init(%p)", drvthis);

	// Alocate and store private data
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	// initialize PrivateData
	p->fd = -1;

	// READ CONFIG FILE:
	// which serial device should be used
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE),
		sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	// which text resolution
	char strSize[7];
	int nCfgTextWidth = 0;
	int nCfgTextHeight = 0;
	strncpy(strSize,
		drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE),
		sizeof(strSize));
	strSize[sizeof(strSize) - 1] = '\0';
	if ((sscanf(strSize, "%dx%d", &nCfgTextWidth, &nCfgTextHeight) != 2)
	    || (nCfgTextWidth <= 0) || (nCfgTextWidth > LCD_MAX_WIDTH)
	    || (nCfgTextHeight <= 0) || (nCfgTextHeight > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING,
		       "%s: cannot read or invalid Size: %s; using default %s",
		       drvthis->name, strSize, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &nCfgTextWidth, &nCfgTextHeight);
	}
	p->width = nCfgTextWidth;
	p->height = nCfgTextHeight;

	p->framebuf = malloc(p->width * p->height);
	p->last_framebuf = malloc(p->width * p->height);
	if ((p->framebuf == NULL) || (p->last_framebuf == NULL)) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);
	memset(p->last_framebuf, ' ', p->width * p->height);

	// Set up io port correctly, and open it...
	debug(RPT_DEBUG, "%s: opening serial device: %s", __FUNCTION__, device);
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: init() failed (%s)", drvthis->name, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: make sure you have rw access to %s!", drvthis->name,
			       device);
		return -1;
	}
	report(RPT_INFO, "%: opened display on %s", drvthis->name, device);

	tcgetattr(p->fd, &portset);
#ifdef HAVE_CFMAKERAW
	/* The easy way: */
	cfmakeraw(&portset);
#else
	/* The hard way: */
	portset.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	portset.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, speed);
	tcsetattr(p->fd, TCSANOW, &portset);
	tcflush(p->fd, TCIOFLUSH);

	// stop auto clock display, clear display
	write(p->fd, "\x4D\x28\x4D\x0D", 4);

	// Turn on the backlight
	icp_a106_backlight(drvthis, true);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
icp_a106_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p != NULL) {
		if (p->framebuf != NULL)
			free(p->framebuf);
		if (p->last_framebuf != NULL)
			free(p->last_framebuf);

		// clear display, start auto display
		if (p->fd >= 0) {
			write(p->fd, "\x4D\x0D\x4D\x29", 4);
			close(p->fd);
		}
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

	report(RPT_INFO, "%s: closed", drvthis->name);
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
icp_a106_width(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->width;
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
icp_a106_height(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->height;
}

/**
 * Clear the LCD screen
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
icp_a106_clear(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}

/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
icp_a106_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	/* 
	 * The ICP protocol is a bit difficult to handle - 1200bps, ho handshake,
	 * and the controller is easily overrun and displays garbage when it
	 * has too much of work to do. It seems we can handle two full updates 
	 * per second, so if the last update was less than 0.5 seconds ago, we 
	 * simply skip this one and update the display the next time. */
	struct timeval tv, tv2;
	static struct timeval tv_old;	/* time of last update */
	int line;
	static char cmd[] = "\x4D\x0c\x00\x00";
	cmd[3] = p->width;

	gettimeofday(&tv, NULL);
	timersub(&tv, &tv_old, &tv2);

	// Don't allow updates faster than 2fps
	if ((tv2.tv_sec == 0) && (tv2.tv_usec < 500000))
		return;

	tv_old = tv;

	for (line = 0; line < p->height; line++) {
		// If line changed
		if (memcmp(p->framebuf + line * p->width,
			   p->last_framebuf + line * p->width, p->width) != 0) {
			cmd[2] = line;
			write(p->fd, cmd, 4);
			write(p->fd, p->framebuf + line * p->width, p->width);
		}
	}
	memcpy(p->last_framebuf, p->framebuf, p->width * p->height);
}

/**
 * Print a character on the LCD display at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
icp_a106_chr(Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	y--;
	x--;
	// debug(RPT_DEBUG, "icp_a106_chr: x=%d, y=%d, chr=%x", x,y,ch);
	if ((x >= 0) && (x < p->width) && (y >= 0) && (y < p->height))
		p->framebuf[y * p->width + x] = ch;
}

/**
 * Print a string on the LCD display at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
icp_a106_string(Driver *drvthis, int x, int y, char *s)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	x--;			// Convert 1-based coords to 0-based
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (; (*s != '\0') && (x < p->width); s++, x++)
		if (x >= 0)
			p->framebuf[y * p->width + x] = *s;
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
icp_a106_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int total_pixels = ((long) 2 * len * LCD_DEFAULT_CELLHEIGHT + 1) * promille / 2000;
	int pos;
	static char map[] = " __---=#";

	for (pos = 0; pos < len; pos++) {
		int pixels = total_pixels - LCD_DEFAULT_CELLHEIGHT * pos;

		if (pixels >= LCD_DEFAULT_CELLHEIGHT) {
			/* write a "full" block to the screen... */
			icp_a106_icon(drvthis, x, y - pos, ICON_BLOCK_FILLED);
		}
		else {
			/* write a partial block... */
			icp_a106_chr(drvthis, x, y - pos, map[pixels]);
			break;
		}
	}
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
icp_a106_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int total_pixels = ((long) 2 * len * LCD_DEFAULT_CELLWIDTH + 1) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos++) {
		int pixels = total_pixels - LCD_DEFAULT_CELLWIDTH * pos;

		if (pixels >= LCD_DEFAULT_CELLWIDTH) {
			/* write a "full" block to the screen... */
			icp_a106_icon(drvthis, x + pos, y, ICON_BLOCK_FILLED);
		}
		else if (pixels > 0) {
			/* write a partial block... */
			icp_a106_chr(drvthis, x + pos, y, '|');
			break;
		}
		else {
			;	/* write nothing (not even a space) */
		}
	}
}


/**
 * Write a big number to the LCD display.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
icp_a106_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if ((num < 0) || (num > 10))
		return;

	icp_a106_chr(drvthis, x, 1 + (p->height - 1) / 2, (num == 10) ? ':' : (num + '0'));
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
icp_a106_icon(Driver *drvthis, int x, int y, int icon)
{
	switch (icon) {
	case ICON_BLOCK_FILLED:
		icp_a106_chr(drvthis, x, y, '\xff');
		break;
	case ICON_HEART_FILLED:
		break;
	case ICON_HEART_OPEN:
		break;
	default:
		return -1;
	}
	return 0;
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
icp_a106_backlight(Driver *drvthis, int on)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	static char cmd[] = "\x4D\x5E\x00";
	if (on != p->backlight) {
		p->backlight = cmd[2] = on ? A106_BL_ON : A106_BL_OFF;
		write(p->fd, cmd, 3);
	}
}

/**
 * Get key from the device.  There are probably only two buttons.
 *     Enter:  short press = ENTER  long press = ESC
 *     Select: short press = DOWN   long press = UP
 *
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL for nothing available / unmapped key.
 */
MODULE_EXPORT const char *
icp_a106_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	char *button = NULL;
	unsigned char byte;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	int nbuf = read(p->fd, &byte, 1);

	if (nbuf != 1)
		return NULL;

	// Don't allow buffer overflows and resync if 0x53 is received
	if (byte == 0x53 || p->serial_buf_offset >= 4)
		p->serial_buf_offset = 0;

	// Write current byte to buffer
	p->serial_buf[p->serial_buf_offset++] = byte;

	// Only check full buffers
	if (p->serial_buf_offset != 4)
		return NULL;

	// Test that the signature matches that of a key press
	int mschange;

	// Make sure this is a "Button Status" packet
	if (memcmp(p->serial_buf, "\x53\x05", 2) != 0)
		return NULL;

	// Read a bitmap of buttonStates for 0xXXYY => buttons 1-15
	uint16_t buttonStates = (p->serial_buf[2] << 8) | p->serial_buf[3];
	debug(RPT_DEBUG, "%s: Button state: 0x%04x", __FUNCTION__, buttonStates);

	// Don't do anything unless it's different from last time
	if (buttonStates != p->buttonStates) {
		// Check each button
		int i;
		for (i = 0; i < MAX_BUTTONS; i++) {
			// Read the state of this button from the bitmap
			bool buttonState = (buttonStates >> i) & 0x1;

			// Skip to the next button if this one didn't change
			if (p->buttonState[i] == buttonState)
				continue;

			p->buttonState[i] = buttonState;
			debug(RPT_DEBUG, "%s: Button %d state updated to %s", __FUNCTION__, i,
			      p->buttonState[i] ? "on" : "off");

			if (buttonState) {
				debug(RPT_INFO, "ICP_A106: Button %d pressed", i);
				memcpy(&p->time_pressed[i], &tv, sizeof(struct timeval));
			}
			else {
				debug(RPT_INFO, "ICP_A106: Button %d released", i);
				mschange = (tv.tv_sec - p->time_pressed[i].tv_sec) * 1000 +
					(tv.tv_usec - p->time_pressed[i].tv_usec) / 1000;
				switch (i) {
				case 0:	// Button 1
					if (mschange < 300)
						button = "Enter";
					else
						button = "Escape";
					break;
				case 1:	// Button 2
					if (mschange < 300)
						button = "Down";
					else
						button = "Up";
					break;
				default:
					// Don't support other buttons yet.
					// Does anything have them?
					break;
				}
			}
		}
		p->buttonStates = buttonStates;
	}

	return button;
}
