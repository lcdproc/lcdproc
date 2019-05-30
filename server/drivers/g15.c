/** \file server/drivers/g15.c
 * LCDd \c g15 driver for the LCD on the Logitech G15 keyboard.
 */

/*
    Copyright (C) 2006 Anthony J. Mirabella.

    2006-07-23 Version 1.0: Most functions should be implemented and working

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301


    ==============================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_G15DAEMON_CLIENT
#include <libg15.h>
#include <g15daemon_client.h>
#else
/* Define stubs for the g15daemon_client (and use hidraw access instead). */
#define G15_G15RBUF 3
static inline const char *g15daemon_version(void) { return NULL; }
static inline int new_g15_screen(int screentype) { return -1; }
static inline int g15_close_screen(int sock) { return -1; }
static inline int g15_send(int sock, char *buf, unsigned int len) { return -1; }
#endif

/*
 * If we have freetype2, assume libg15render is build with TTF support,
 * the TTF_SUPPORT define makes the size of the g15 struct bigger, if we do
 * not set this define while libg15render is build with TTF support we get
 * heap corruption. The other way around does not matter, then we just alloc
 * a little bit too much memory (the TTF related variables live at the end
 * of the struct).
 */
#ifdef HAVE_FT2
#define TTF_SUPPORT
#endif
#include <libg15render.h>

#include "lcd.h"
#include "g15.h"

#include "shared/defines.h"
#include "shared/report.h"

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "g15_";

void g15_close (Driver *drvthis);

static const struct lib_hidraw_id hidraw_ids[] = {
	/* G15 */
	{ { BUS_USB, 0x046d, 0xc222 } },
	/* G15 v2 */
	{ { BUS_USB, 0x046d, 0xc227 } },
	/* G510 without a headset plugged in */
	{ { BUS_USB, 0x046d, 0xc22d },
	  { 0x05, 0x0c, 0x09, 0x01, 0xa1, 0x01, 0x85, 0x02,
	    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x07 } },
	/* G510 with headset plugged in / with extra USB audio interface */
	{ { BUS_USB, 0x046d, 0xc22e },
	  { 0x05, 0x0c, 0x09, 0x01, 0xa1, 0x01, 0x85, 0x02,
	    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x07 } },
	/* Terminator */
	{}
};

// Find the proper usb device and initialize it
//
MODULE_EXPORT int g15_init (Driver *drvthis)
{
	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	drvthis->private_data = p;

	/* Initialize the PrivateData structure */
	p->backlight_state = BACKLIGHT_ON;
	p->g15screen_fd = -1;
	p->g15d_ver = g15daemon_version();

	if((p->g15screen_fd = new_g15_screen(G15_G15RBUF)) < 0)
	{
		/* g15daemon is not running, use hidraw access instead */
		p->hidraw_handle = lib_hidraw_open(hidraw_ids);
		if (!p->hidraw_handle) {
			report(RPT_ERR, "%s: Sorry, cannot find a G15 keyboard", drvthis->name);
			g15_close(drvthis);
			return -1;
		}
	}

	p->font = g15r_requestG15DefaultFont(G15_TEXT_LARGE);
	if (p->font == NULL) {
		report(RPT_ERR, "%s: unable to load default large font", drvthis->name);
		g15_close(drvthis);
		return -1;
	}

	g15r_initCanvas(&p->canvas);
	g15r_initCanvas(&p->backingstore);

	return 0;
}

// Close the connection to the LCD
//
MODULE_EXPORT void g15_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	drvthis->private_data = NULL;

	g15r_deleteG15Font(p->font);
	if (p->g15screen_fd != -1)
		g15_close_screen(p->g15screen_fd);
	if (p->hidraw_handle)
		lib_hidraw_close(p->hidraw_handle);

	free(p);
}

// Returns the display width in characters
//
MODULE_EXPORT int g15_width (Driver *drvthis)
{
	return G15_CHAR_WIDTH;
}

// Returns the display height in characters
//
MODULE_EXPORT int g15_height (Driver *drvthis)
{
	return G15_CHAR_HEIGHT;
}

// Returns the width of a character in pixels
//
MODULE_EXPORT int g15_cellwidth (Driver *drvthis)
{
	return G15_CELL_WIDTH;
}

// Returns the height of a character in pixels
//
MODULE_EXPORT int g15_cellheight (Driver *drvthis)
{
	return G15_CELL_HEIGHT;
}

// Clears the LCD screen
//
MODULE_EXPORT void g15_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	g15r_clearScreen(&p->canvas, 0);
	g15r_clearScreen(&p->backingstore, 0);
}

// Convert libg15render canvas format to raw data for the USB output endpoint.
// Based on libg15.c code, which is licensed GPL v2 or later.
//
static void g15_pixmap_to_lcd(unsigned char *lcd_buffer, unsigned char const *data)
{
/* For a set of bytes (A, B, C, etc.) the bits representing pixels will appear
   on the LCD like this:

	A0 B0 C0
	A1 B1 C1
	A2 B2 C2
	A3 B3 C3 ... and across for G15_LCD_WIDTH bytes
	A4 B4 C4
	A5 B5 C5
	A6 B6 C6
	A7 B7 C7

	A0
	A1  <- second 8-pixel-high row starts straight after the last byte on
	A2     the previous row
	A3
	A4
	A5
	A6
	A7
	A8

	A0
	...
	A0
	...
	A0
	...
	A0
	A1 <- only the first three bits are shown on the bottom row (the last three
	A2    pixels of the 43-pixel high display.)
*/

	const unsigned int stride = G15_LCD_WIDTH / 8;
	unsigned int row, col;

	lcd_buffer[0] = 0x03; /* Set output report 3 */
	memset(lcd_buffer + 1, 0, G15_LCD_OFFSET - 1);
	lcd_buffer += G15_LCD_OFFSET;

	/* 43 pixels height, requires 6 bytes for each column */
	for (row = 0; row < 6; row++) {
		for (col = 0; col < G15_LCD_WIDTH; col++) {
			unsigned int bit = col % 8;

			/* Copy a 1x8 column of pixels across from the source
			 * image to the LCD buffer. */

			*lcd_buffer++ =
				(((data[stride * 0] << bit) & 0x80) >> 7) |
				(((data[stride * 1] << bit) & 0x80) >> 6) |
				(((data[stride * 2] << bit) & 0x80) >> 5) |
				(((data[stride * 3] << bit) & 0x80) >> 4) |
				(((data[stride * 4] << bit) & 0x80) >> 3) |
				(((data[stride * 5] << bit) & 0x80) >> 2) |
				(((data[stride * 6] << bit) & 0x80) >> 1) |
				(((data[stride * 7] << bit) & 0x80) >> 0);

			if (bit == 7)
				data++;
		}
		/* Jump down seven pixel-rows in the source image,
		 * since we've just done a row of eight pixels in one pass
		 * (and we counted one pixel-row while we were going). */
		data += 7 * stride;
	}
}

// Blasts a single frame onscreen, to the lcd...
//
MODULE_EXPORT void g15_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	/* 43 pixels height, requires 6 bytes for each column */
	unsigned char lcd_buf[G15_LCD_OFFSET + 6 * G15_LCD_WIDTH];

	if (memcmp(p->backingstore.buffer, p->canvas.buffer, G15_BUFFER_LEN * sizeof(unsigned char)) == 0)
		return;

	memcpy(p->backingstore.buffer, p->canvas.buffer, G15_BUFFER_LEN * sizeof(unsigned char));

	if (p->g15screen_fd != -1) {
		g15_send(p->g15screen_fd, (char*)p->canvas.buffer, 1048);
	} else {
		g15_pixmap_to_lcd(lcd_buf, p->canvas.buffer);
		lib_hidraw_send_output_report(p->hidraw_handle, lcd_buf, sizeof(lcd_buf));
	}
}

// LCDd 1-dimension char coordinates to g15r 0-(dimension-1) pixel coords */
int g15_convert_coords(int x, int y, int *px, int *py)
{
	*px = (x - 1) * G15_CELL_WIDTH;
	*py = (y - 1) * G15_CELL_HEIGHT;

	/* We have 5 lines of 8 pixels heigh, so 40 pixels, but the LCD is
	 * 43 pixels high. This allows us to add an empty line between 4 of
	 * the 5 lines. This is desirable to avoid the descenders from the
	 * non caps 'g' and 'y' glyphs touching the top of the chars of the
	 * next line.
	 * This also makes us better use the whole height of the LCD.
	 */
	*py += min(y - 1, 3);

	if ((*px + G15_CELL_WIDTH)  > G15_LCD_WIDTH ||
	    (*py + G15_CELL_HEIGHT) > G15_LCD_HEIGHT)
		return 0; /* Failure */

	return 1; /* Success */
}

// Character function for the lcdproc driver API
//
MODULE_EXPORT void g15_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;
	int px, py;

	if (!g15_convert_coords(x, y, &px, &py))
		return;

	/* Clear background */
	g15r_pixelReverseFill(&p->canvas, px, py,
			      px + G15_CELL_WIDTH - 1,
			      py + G15_CELL_HEIGHT - 1,
			      G15_PIXEL_FILL, G15_COLOR_WHITE);
	/* Render character, coords - 1 because of g15r peculiarities  */
	g15r_renderG15Glyph(&p->canvas, p->font, c, px - 1, py - 1, G15_COLOR_BLACK, 0);
}

// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,5).
//
MODULE_EXPORT void g15_string (Driver *drvthis, int x, int y, const char string[])
{
	int i;

	for (i = 0; string[i] != '\0'; i++)
		g15_chr(drvthis, x + i, y, string[i]);
}

// Draws an icon on the screen
MODULE_EXPORT int g15_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	unsigned char character;
	int px1, py1, px2, py2;

	switch (icon) {
	/* Special cases */
	case ICON_BLOCK_FILLED:
		if (!g15_convert_coords(x, y, &px1, &py1))
			return -1;

		px2 = px1 + G15_CELL_WIDTH - 2;
		py2 = py1 + G15_CELL_HEIGHT - 2;
		g15r_pixelBox(&p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);
		return 0;

	case ICON_HEART_OPEN:
		p->canvas.mode_reverse = 1;
		g15_chr(drvthis, x, y, G15_ICON_HEART_OPEN);
		p->canvas.mode_reverse = 0;
		return 0;

	/* Simple 1:1 mapping cases */
	case ICON_HEART_FILLED:	 character = G15_ICON_HEART_FILLED;	break;
	case ICON_ARROW_UP:	 character = G15_ICON_ARROW_UP;		break;
	case ICON_ARROW_DOWN:	 character = G15_ICON_ARROW_DOWN;	break;
	case ICON_ARROW_LEFT:	 character = G15_ICON_ARROW_LEFT;	break;
	case ICON_ARROW_RIGHT:	 character = G15_ICON_ARROW_RIGHT;	break;
	case ICON_CHECKBOX_OFF:	 character = G15_ICON_CHECKBOX_OFF;	break;
	case ICON_CHECKBOX_ON:	 character = G15_ICON_CHECKBOX_ON;	break;
	case ICON_CHECKBOX_GRAY: character = G15_ICON_CHECKBOX_GRAY;	break;
	case ICON_STOP:		 character = G15_ICON_STOP;		break;
	case ICON_PAUSE:	 character = G15_ICON_PAUSE;		break;
	case ICON_PLAY:		 character = G15_ICON_PLAY;		break;
	case ICON_PLAYR:	 character = G15_ICON_PLAYR;		break;
	case ICON_FF:		 character = G15_ICON_FF;		break;
	case ICON_FR:		 character = G15_ICON_FR;		break;
	case ICON_NEXT:		 character = G15_ICON_NEXT;		break;
	case ICON_PREV:		 character = G15_ICON_PREV;		break;
	case ICON_REC:		 character = G15_ICON_REC;		break;
	/* Let the core do other icons */
	default:
		return -1;
	}

	g15_chr(drvthis, x, y, character);
	return 0;
}

// Draws a horizontal bar growing to the right
//
MODULE_EXPORT void g15_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int total_pixels = ((long) 2 * len * G15_CELL_WIDTH + 1) * promille / 2000;
	int px1, py1, px2, py2;

	if (!g15_convert_coords(x, y, &px1, &py1))
		return;

	px2 = px1 + total_pixels;
	py2 = py1 + G15_CELL_HEIGHT - 2;

	g15r_pixelBox(&p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);
}

// Draws a vertical bar growing up
//
MODULE_EXPORT void g15_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int total_pixels = ((long) 2 * len * G15_CELL_WIDTH + 1) * promille / 2000;
	int px1, py1, px2, py2;

	if (!g15_convert_coords(x, y, &px1, &py1))
		return;

	/* vbar grow from the bottom upwards, flip the Y-coordinates */
	py1 = py1 + G15_CELL_HEIGHT - total_pixels;
	py2 = py1 + total_pixels - 1;
	px2 = px1 + G15_CELL_WIDTH - 2;

	g15r_pixelBox(&p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);
}

#ifdef HAVE_G15DAEMON_CLIENT
//  Return one char from the Keyboard
//
MODULE_EXPORT const char * g15_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int toread = 0;
	unsigned int key_state = 0;

	if (p->g15screen_fd == -1)
		return NULL;

	if ((strncmp("1.2", p->g15d_ver, 3)))
	  {	/* other than g15daemon-1.2 (should be >=1.9) */
		fd_set fds;
		struct timeval tv;
		memset (&tv, 0, sizeof(struct timeval));

		FD_ZERO(&fds);
		FD_SET(p->g15screen_fd, &fds);

		toread = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
	  }
	else
	  {	/* g15daemon-1.2 */
		if(send(p->g15screen_fd, "k", 1, MSG_OOB)<1) /* request key status */
		  {
	 	  	report(RPT_INFO, "%s: Error in send to g15daemon", drvthis->name);
			return NULL;
		  }
		toread = 1;
	  }

	if (toread >= 1)
	  read(p->g15screen_fd, &key_state, sizeof(key_state));
	else
	  return NULL;

	if (key_state & G15_KEY_G1)
		return "Escape";
	else if (key_state & G15_KEY_L1)
	    return "Enter";
	else if (key_state & G15_KEY_L2)
	    return "Left";
	else if (key_state & G15_KEY_L3)
	    return "Up";
	else if (key_state & G15_KEY_L4)
	    return "Down";
	else if (key_state & G15_KEY_L5)
	    return "Right";
	else
	    return NULL;
}

// Set the backlight
//
MODULE_EXPORT void g15_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (p->g15screen_fd == -1)
		return;

	if (p->backlight_state == on)
		return;

	p->backlight_state = on;

	char msgbuf[256];

	switch (on) {
		case BACKLIGHT_ON:
			{
			msgbuf[0]=G15_BRIGHTNESS_BRIGHT|G15DAEMON_BACKLIGHT;
			send(p->g15screen_fd,msgbuf,1,MSG_OOB);
			break;
			}
		case BACKLIGHT_OFF:
			{
			msgbuf[0]=G15_BRIGHTNESS_DARK|G15DAEMON_BACKLIGHT;
			send(p->g15screen_fd,msgbuf,1,MSG_OOB);
			break;
			}
		default:
			{
			break;
			}
		}
}
#endif

MODULE_EXPORT void g15_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;

	x--;
	int ox = x * G15_CELL_WIDTH;

	if ((num < 0) || (num > 10))
		return;

	int width = 0;
	int height = 43;

	if ((num >= 0) && (num <=9))
		width = 24;
	else
		width = 9;

	int i=0;

   	for (i=0;i<(width*height);++i)
   	{
		int color = (g15_bignum_data[num][i] ? G15_COLOR_WHITE : G15_COLOR_BLACK);
		int px = ox + i % width;
		int py = i / width;
		g15r_setPixel(&p->canvas, px, py, color);
   	}
}
