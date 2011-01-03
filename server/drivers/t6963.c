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
 *\endverbatim
 *
 * \todo This driver uses writes to port 0x80 to implement delay. Convert it
 * to use \c timing.h.
 */

/*-
 * Base driver module for Toshiba T6963 based LCD displays. ver 2.2
 *
 * Parts of this file are based on the kernel driver by A
 * lexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
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

#include "report.h"
#include "lcd_lib.h"
#include "port.h"


/** private data for the \c t6963 driver */
typedef struct t6963_private_data {
	u16 port;
	u16 display_mode;
	u8 *display_buffer1;
	u8 *display_buffer2;

	int width;
	int height;
	int cellwidth;
	int cellheight;
	short bidirectLPT;
	short graphicON;
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
	int w, h, i, ecp_input;
	char size[200] = DEFAULT_SIZE;

	debug(RPT_INFO, "T6963: init(%p)", drvthis);

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->display_mode = 0;
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;

	debug(RPT_DEBUG, "T6963: reading config file...");

	/* Read config file */

	/* Which size? */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size) - 1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
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

	/* Is ECP mode on? Default: yes */
	p->bidirectLPT = drvthis->config_get_bool(drvthis->name, "ECPlpt", 0, 1);
	/* Use graphic? Default: no */
	p->graphicON = drvthis->config_get_bool(drvthis->name, "graphic", 0, 0);


	/* Get permission to parallel port */
	debug(RPT_DEBUG, "T6963: Getting permission to parallel port %d...", p->port);
	if (port_access_multiple(p->port, 3)) {
		report(RPT_ERR, "%s: no permission to port 0x%03X: (%s)",
		       drvthis->name, p->port, strerror(errno));
		return -1;
	}
	if (port_access(0x80)) {
		report(RPT_ERR, "%s: no permission to port 0x80: (%s)",
		       drvthis->name, strerror(errno));
		return -1;
	}
	debug(RPT_DEBUG, "T6963:   cool, got 'em!");


	/* Allocate memory for double buffering */
	debug(RPT_DEBUG, "T6963: Allocating double buffering...");
	p->display_buffer1 = malloc(p->width * p->height);
	p->display_buffer2 = malloc(p->width * p->height);
	if ((p->display_buffer1 == NULL) || (p->display_buffer2 == NULL)) {
		report(RPT_ERR, "%s: No memory for double buffering", drvthis->name);
		t6963_close(drvthis);
		return -1;
	}

	/* Clear front and back buffer */
	memset(p->display_buffer1, ' ', p->width * p->height);
	memset(p->display_buffer2, ' ', p->width * p->height);
	debug(RPT_DEBUG, "T6963:     done!");

	/* ------------------- I N I T I A L I Z A T I O N --------------- */
	debug(RPT_DEBUG, "T6963: Sending init to display...");

	t6963_low_set_control(drvthis, 1, 1, 1, 1);
	T6963_DATAOUT(p->port);	/* configure port for output */

	/* Test if bidirectional mode is working */
	if (p->bidirectLPT == 1) {
		debug(RPT_WARNING, "T6963: Testing bidirectional mode...");
		i = 0;
		ecp_input = 0;
		T6963_DATAIN(p->port);
		/*
		 * Try to read status from display. STA0 and STA1 must both
		 * be set 1 (= 0x03).
		 */
		do {
			i++;
			t6963_low_set_control(drvthis, 1, 1, 1, 1);
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			ecp_input = port_in(T6963_DATA_PORT(p->port));
			t6963_low_set_control(drvthis, 1, 1, 1, 1);
		} while (i < 100 && (ecp_input & 0x03) != 0x03);
		T6963_DATAOUT(p->port);
		if (i >= 100) {
			debug(RPT_WARNING, "T6963: Bidirectional mode not working!\n -> is now disabled  (STA0: %i, STA1: %i)", ecp_input & 1, ecp_input & 2);
			p->bidirectLPT = 0;
		}
		else
			debug(RPT_WARNING, "T6963: working!");
	}

	debug(RPT_DEBUG, "T6963:  set graphic/text home adress and area");

	/* Set text and graphic addresses */
	t6963_low_command_word(drvthis, SET_GRAPHIC_HOME_ADDRESS, ATTRIB_BASE);
	t6963_low_command_word(drvthis, SET_GRAPHIC_AREA, p->width);
	t6963_low_command_word(drvthis, SET_TEXT_HOME_ADDRESS, TEXT_BASE);
	t6963_low_command_word(drvthis, SET_TEXT_AREA, p->width);

	/* Enable external character generator */
	t6963_low_command(drvthis, SET_MODE | OR_MODE | EXTERNAL_CG);
	/* Set address of CG RAM address start */
	t6963_low_command_2_bytes(drvthis, SET_OFFSET_REGISTER, CHARGEN_BASE >> 11, 0);

	/* Set cursor to 8 lines and place it at position (0,0) */
	t6963_low_command(drvthis, SET_CURSOR_PATTERN | 7);
	t6963_low_command_2_bytes(drvthis, SET_CURSOR_POINTER, 0, 0);

	/* Load font data */
	t6963_set_nchar(drvthis, 0, fontdata_6x8, 256);

	/* Turn on text and optionally graphics, turn off cursor */
	t6963_low_enable_mode(drvthis, TEXT_ON);
	if (p->graphicON == 0)
		t6963_low_disable_mode(drvthis, GRAPHIC_ON);
	else
		t6963_low_enable_mode(drvthis, GRAPHIC_ON);
	t6963_low_disable_mode(drvthis, CURSOR_ON);
	t6963_low_disable_mode(drvthis, BLINK_ON);

	/* Clear display */
	t6963_clear(drvthis);
	t6963_graphic_clear(drvthis, 0, 0, p->width, p->cellheight * p->height);
	t6963_flush(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

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
		t6963_low_disable_mode(drvthis, BLINK_ON);

		port_deny_multiple(p->port, 3);

		if (p->display_buffer1 != NULL)
			free(p->display_buffer1);

		if (p->display_buffer2 != NULL)
			free(p->display_buffer2);

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
 * \param x1       Start column (character!)
 * \param y1       Start line (pixel row)
 * \param x2       End column (character!)
 * \param y2       End line (pixel row)
 */
void
t6963_graphic_clear(Driver * drvthis, int x1, int y1, int x2, int y2)
{
	PrivateData *p = drvthis->private_data;
	int x;

	debug(RPT_DEBUG, "Clearing Graphic %d bytes", (x2 - x1) * (y2 - y1));
	for (; y1 < y2; y1++) {
		t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, ATTRIB_BASE + y1 * p->width + x1);
		for (x = x1; x < x2; x++)
			t6963_low_command_byte(drvthis, DATA_WRITE_INC, 0);
	}
}

/**
 * API: Flushes all output to the lcd.
 */
MODULE_EXPORT void
t6963_flush(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	debug(RPT_DEBUG, "Flushing %d x %d", p->width, p->height);

	for (i = 0; i < (p->width * p->height); i++) {
		//debug(RPT_DEBUG, "%i%|i", p->display_buffer1[i], p->display_buffer2[i]);
		if (p->display_buffer1[i] != p->display_buffer2[i]) {
			t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, TEXT_BASE + i);
			t6963_low_command_byte(drvthis, DATA_WRITE, p->display_buffer1[i]);
		}
	}
	debug(RPT_DEBUG, "Done");
	/* Reverse the two buffers and clear buffer1 */
	t6963_swap_buffers(drvthis);
	t6963_clear(drvthis);
}

/**
 * API: Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,6).
 */
MODULE_EXPORT void
t6963_string(Driver * drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "String out");

	x--;			/* Convert 1-based coords to 0-based */
	y--;

	if ((y * p->width + x + strlen(string)) <= (p->width * p->height))
		memcpy(&p->display_buffer1[y * p->width + x], string, strlen(string));
}

/**
 * API: Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,6).
 */
MODULE_EXPORT void
t6963_chr(Driver * drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "Char out");

	y--;
	x--;
	if ((y * p->width) + x <= (p->width * p->height))
		p->display_buffer1[(y * p->width) + x] = c;
}

/**
 * API: Draws a big (4-row) number. (NOT IMPLEMENTED)
 */
MODULE_EXPORT void
t6963_num(Driver * drvthis, int x, int num)
{
	//printf("BigNum(%i, %i)\n", x, num);
}

/**
 * Changes the font data of character n.
 * \param drvthis  Pointer to driver structure.
 * \param n        Index of character in CGRAM to update.
 * \param dat      Data (must consist of num*cellwidth*cellheight bytes).
 * \param num      Number of characters stored in data.
 */
void
t6963_set_nchar(Driver * drvthis, int n, unsigned char *dat, int num)
{
	PrivateData *p = drvthis->private_data;
	int row, col;
	char letter;

	debug(RPT_DEBUG, "Setting char %d", n);

	if ((!dat) || (n + num > 256))
		return;

	t6963_low_command_word(drvthis, SET_ADDRESS_POINTER, CHARGEN_BASE + n * 8);
	for (row = 0; row < p->cellheight * num; row++) {
		letter = 0;
		/* Accumulate data for one pixel row */
		for (col = 0; col < p->cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * p->cellwidth) + col] > 0);
		}

		t6963_low_command_byte(drvthis, DATA_WRITE_INC, letter);
	}
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

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 212);
}

/**
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
t6963_hbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 220);
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


/*---------------------- internal functions -------------------------------*/

/**
 * Set control lines for the display. State values are \c 0 or \c 1 and
 * represent the real level as it arrives at the display. This function
 * takes into account the hardware inversion of control lines.
 *
 * \param drvthis  Pointer to driver structure.
 * \param wr       State of the /RW pin (0 or 1).
 * \param ce       State of the /CE pin (0 or 1).
 * \param cd       State of the C/D pin (0 or 1).
 * \param rd       State of the /RD pin (0 or 1).
 */
void
t6963_low_set_control(Driver * drvthis, char wr, char ce, char cd, char rd)
{
	PrivateData *p = drvthis->private_data;

	unsigned char status = port_in(T6963_CONTROL_PORT(p->port));
	if (wr == 1)		/* WR = HI */
		status &= 0xfe;
	else if (wr == 0)
		status |= 0x01;
	if (ce == 1)		/* CE = HI */
		status &= 0xfd;
	else if (ce == 0)
		status |= 0x02;
	if (cd == 0)		/* CD = HI */
		status &= 0xfb;
	else if (cd == 1)
		status |= 0x04;
	if (rd == 1)		/* RD = HI */
		status &= 0xf7;
	else if (rd == 0)
		status |= 0x08;
	port_out(T6963_CONTROL_PORT(p->port), status);
}

/**
 * Check display status.
 * \param drvthis  Pointer to driver structure.
 */
void
t6963_low_dsp_ready(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i = 0;
	int input;

	T6963_DATAIN(p->port);
	if (p->bidirectLPT == 1) {
		do {
			i++;
			t6963_low_set_control(drvthis, 1, 1, 1, 1);
			/*
			 * Output control lines 3 times (hold value for tacc
			 * max 150ns
			 */
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			t6963_low_set_control(drvthis, 1, 0, 1, 0);
			input = port_in(T6963_DATA_PORT(p->port));
			t6963_low_set_control(drvthis, 1, 1, 1, 1);

		} while (i < 100 && (input & 3) != 3);
	}
	else {
		t6963_low_set_control(drvthis, 1, 1, 1, 1);
		t6963_low_set_control(drvthis, 1, 0, 1, 0);
		t6963_low_set_control(drvthis, 1, 1, 1, 1);
		port_out(0x80, 0x00);	/* wait 1ms */
	}
	T6963_DATAOUT(p->port);
}


/**
 * Send one data byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param byte     Data byte.
 */
void
t6963_low_data(Driver * drvthis, u8 byte)
{
	PrivateData *p = drvthis->private_data;

	t6963_low_dsp_ready(drvthis);	/* Check if display ready */
	t6963_low_set_control(drvthis, 1, 1, 0, 1);	/* CD down (data) */
	t6963_low_set_control(drvthis, 0, 0, 0, 1);	/* CE & WR down */
	port_out(T6963_DATA_PORT(p->port), byte);	/* present data */
	port_out(0x80, 0x00);	/* short wait */
	t6963_low_set_control(drvthis, 1, 1, 1, 1);	/* all up again */
}

/**
 * Send one command byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param byte     Command byte.
 */
void
t6963_low_command(Driver * drvthis, u8 byte)
{
	PrivateData *p = drvthis->private_data;

	t6963_low_dsp_ready(drvthis);	/* Check if display ready */
	t6963_low_set_control(drvthis, 1, 1, 1, 1);	/* CD up (command) */
	t6963_low_set_control(drvthis, 0, 0, 1, 1);	/* CE & WR down */
	port_out(T6963_DATA_PORT(p->port), byte);	/* present data */
	port_out(0x80, 0x00);	/* short wait */
	t6963_low_set_control(drvthis, 1, 1, 0, 1);	/* CE & WR up, CD down */
}

/**
 * Send one byte of data followed by one command byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param cmd      Command byte.
 * \param byte     Data value.
 */
void
t6963_low_command_byte(Driver * drvthis, u8 cmd, u8 byte)
{
	t6963_low_data(drvthis, byte);
	t6963_low_command(drvthis, cmd);
}

/**
 * Send two bytes of data followed by one command byte to the display.
 * \param drvthis  Pointer to driver structure.
 * \param cmd      Command byte.
 * \param byte1    Data value (byte 1)
 * \param byte2    Data value (byte 2)
 */
void
t6963_low_command_2_bytes(Driver * drvthis, u8 cmd, u8 byte1, u8 byte2)
{
	t6963_low_data(drvthis, byte1);
	t6963_low_data(drvthis, byte2);
	t6963_low_command(drvthis, cmd);
}

/**
 * Send one word (two bytes) of data followed by one command byte to the
 * display. Low byte is output first.
 * \param drvthis  Pointer to driver structure.
 * \param cmd      Command byte.
 * \param word     Data value (2 bytes)
 */
void
t6963_low_command_word(Driver * drvthis, u8 cmd, u16 word)
{
	t6963_low_data(drvthis, word % 256);
	t6963_low_data(drvthis, word >> 8);
	t6963_low_command(drvthis, cmd);
}

/**
 * Send 'SET DISPLAY MODE' command to LCD setting the desired mode and store
 * it in private data.
 * \param drvthis  Pointer to driver structure.
 * \param mode     Display mode to set.
 */
void
t6963_low_enable_mode(Driver * drvthis, u8 mode)
{
	PrivateData *p = drvthis->private_data;

	p->display_mode |= mode;
	t6963_low_command(drvthis, SET_DISPLAY_MODE | p->display_mode);
}

/**
 * Send 'SET DISPLAY MODE' command to LCD with the desired mode cleared and
 * store the new value in private data.
 * \param drvthis  Pointer to driver structure.
 * \param mode     Display mode to clear.
 */
void
t6963_low_disable_mode(Driver * drvthis, u8 mode)
{
	PrivateData *p = drvthis->private_data;

	p->display_mode &= ~mode;
	t6963_low_command(drvthis, SET_DISPLAY_MODE | p->display_mode);
}

/**
 * Swap the two display buffers. This function does not swap the actual
 * data but the pointers to the buffers.
 * \param drvthis  Pointer to driver structure.
 */
void
t6963_swap_buffers(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	u8 *tmp_buffer;
	tmp_buffer = p->display_buffer1;
	p->display_buffer1 = p->display_buffer2;
	p->display_buffer2 = tmp_buffer;
	tmp_buffer = NULL;
}
