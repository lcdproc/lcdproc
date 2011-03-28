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
#include <libg15.h>
#include <g15daemon_client.h>
#include <libg15render.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "g15.h"

#include "report.h"

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "g15_";

// Find the proper usb device and initialize it
//
MODULE_EXPORT int g15_init (Driver *drvthis)
{
   PrivateData *p;

   /* Allocate and store private data */
   p = (PrivateData *) calloc(1, sizeof(PrivateData));
   if (p == NULL)
		return -1;
   if (drvthis->store_private_ptr(drvthis, p))
		return -1;

   /* Initialize the PrivateData structure */
   p->width = G15_CHAR_WIDTH;
   p->height = G15_CHAR_HEIGHT;
   p->cellwidth = G15_CELL_WIDTH;
   p->cellheight = G15_CELL_HEIGHT;
   p->backlight_state = BACKLIGHT_ON;
   p->g15screen_fd = 0;
   p->g15d_ver = g15daemon_version();

   if((p->g15screen_fd = new_g15_screen(G15_G15RBUF)) < 0)
   {
        report(RPT_ERR, "%s: Sorry, cant connect to the G15daemon", drvthis->name);
        return -1;
   }

	/* make sure the canvas is there... */
	p->canvas = (g15canvas *) malloc(sizeof(g15canvas));
	if (p->canvas == NULL) {
		report(RPT_ERR, "%s: unable to create canvas", drvthis->name);
		return -1;
	}

	/* make sure the backingstore is there... */
	p->backingstore = (g15canvas *) malloc(sizeof(g15canvas));
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}

	g15r_initCanvas(p->canvas);
	g15r_initCanvas(p->backingstore);
	p->canvas->buffer[0] = G15_LCD_WRITE_CMD;
	p->backingstore->buffer[0] = G15_LCD_WRITE_CMD;

//	ret = setLCDBrightness(G15_BRIGHTNESS_BRIGHT);

   return 0;
}

// Close the connection to the LCD
//
MODULE_EXPORT void g15_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	g15_close_screen(p->g15screen_fd);

	if (p != NULL) {
		if (p->canvas)
			free(p->canvas);

		if (p->backingstore)
			free(p->backingstore);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

// Returns the display width in characters
//
MODULE_EXPORT int g15_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

// Returns the display height in characters
//
MODULE_EXPORT int g15_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

// Returns the width of a character in pixels
//
MODULE_EXPORT int g15_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}

// Returns the height of a character in pixels
//
MODULE_EXPORT int g15_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

// Clears the LCD screen
//
MODULE_EXPORT void g15_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	g15r_clearScreen(p->canvas, 0);
	g15r_clearScreen(p->backingstore, 0);
}

// Blasts a single frame onscreen, to the lcd...
//
MODULE_EXPORT void g15_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (memcmp(p->backingstore->buffer, p->canvas->buffer, G15_BUFFER_LEN * sizeof(unsigned char)) == 0)
		return;

	memcpy(p->backingstore->buffer, p->canvas->buffer, G15_BUFFER_LEN * sizeof(unsigned char));

	g15_send(p->g15screen_fd,(char*)p->canvas->buffer,1048);
}

// Character function for the lcdproc driver API
//
MODULE_EXPORT void g15_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;
	if ((x > p->width) || (y > p->height))
		return;

	g15r_renderCharacterLarge(p->canvas, x, y, c, 0, 0);
}

// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,5).
//
MODULE_EXPORT void g15_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;

	for (i = 0; string[i] != '\0'; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		g15r_renderCharacterLarge(p->canvas, x + i, y, string[i], 0, 0);
	}
}

// Draws an icon on the screen
MODULE_EXPORT int g15_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;

	x--;
	y--;

	switch (icon) {
		case ICON_BLOCK_FILLED:
			{
			int px1 = x * p->cellwidth;
			int py1 = y * p->cellheight;
			int px2 = px1 + (p->cellwidth - 2);
			int py2 = py1 + (p->cellheight - 2);
			g15r_pixelBox(p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);
			break;
			}
		case ICON_HEART_FILLED:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_HEART_FILLED, 0, 0);
			break;
			}
		case ICON_HEART_OPEN:
			{
			p->canvas->mode_reverse = 1;
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_HEART_OPEN, 0, 0);
			p->canvas->mode_reverse = 0;
			break;
			}
		case ICON_ARROW_UP:
			{
			g15r_renderCharacterLarge(p->canvas, x , y, G15_ICON_ARROW_UP, 0, 0);
			break;
			}
		case ICON_ARROW_DOWN:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_ARROW_DOWN, 0, 0);
			break;
			}
		case ICON_ARROW_LEFT:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_ARROW_LEFT, 0, 0);
			break;
			}
		case ICON_ARROW_RIGHT:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_ARROW_RIGHT, 0, 0);
			break;
			}
		case ICON_CHECKBOX_OFF:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_CHECKBOX_OFF, 0, 0);
			break;
			}
		case ICON_CHECKBOX_ON:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_CHECKBOX_ON, 0, 0);
			break;
			}
		case ICON_CHECKBOX_GRAY:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_CHECKBOX_GRAY, 0, 0);
			break;
			}
		case ICON_STOP:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_STOP, 0, 0);
			break;
			}
		case ICON_PAUSE:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_PAUSE, 0, 0);
			break;
			}
		case ICON_PLAY:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_PLAY, 0, 0);
			break;
			}
		case ICON_PLAYR:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_PLAYR, 0, 0);
			break;
			}
		case ICON_FF:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_FF, 0, 0);
			break;
			}
		case ICON_FR:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_FR, 0, 0);
			break;
			}
		case ICON_NEXT:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_NEXT, 0, 0);
			break;
			}
		case ICON_PREV:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_PREV, 0, 0);
			break;
			}
		case ICON_REC:
			{
			g15r_renderCharacterLarge(p->canvas, x, y, G15_ICON_REC, 0, 0);
			break;
			}
		default:
			return -1; /* Let the core do other icons */
	}

	return 0;
}

// Draws a horizontal bar growing to the right
//
MODULE_EXPORT void g15_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	x--;
	y--;

	int total_pixels = ((long) 2 * len * p->cellwidth + 1) * promille / 2000;
	int px1 = x * p->cellwidth;
	int py1 = y * p->cellheight;
	int px2 = px1 + total_pixels;
	int py2 = py1 + (p->cellheight - 2);

	g15r_pixelBox(p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);
}

// Draws a vertical bar growing up
//
MODULE_EXPORT void g15_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	x--;

	int total_pixels = ((long) 2 * len * p->cellwidth + 1) * promille / 2000;
	int px1 = x * p->cellwidth;
	int py1 = y * p->cellheight - total_pixels;
	int px2 = px1 + (p->cellwidth - 2);
	int py2 = py1 + total_pixels;

	g15r_pixelBox(p->canvas, px1, py1, px2, py2, G15_COLOR_BLACK, 1, G15_PIXEL_FILL);

}

//  Return one char from the Keyboard
//
MODULE_EXPORT const char * g15_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int toread = 0;
	unsigned int key_state = 0;

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

MODULE_EXPORT void g15_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;

	x--;
	int ox = x * p->cellwidth;

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
      	g15r_setPixel(p->canvas, px, py, color);
   	}
}
