/*  This is the LCDproc driver for tyan lcd module (tyan Barebone GS series)
    
    Author: yhlu@tyan.com

    Copyright (C) 2004 Tyan Corp

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */


/*
 * Driver status
 * 02/04/2004: Working driver
 *
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "tyan_lcdm.h"
#include "report.h"
#include "lcd_lib.h"

#define TYAN_LCDM_KEY_ENTER    0xf2
#define TYAN_LCDM_KEY_ESCAPE   0xf3
#define TYAN_LCDM_KEY_RIGHT    0xf5
#define TYAN_LCDM_KEY_LEFT     0xf6
#define TYAN_LCDM_KEY_UP       0xf7
#define TYAN_LCDM_KEY_DOWN     0xf8

#define TYAN_LCDM_CMD_BEGIN 0xf1
#define TYAN_LCDM_CMD_END 0xf2



static int custom = 0;
typedef enum {
	hbar = 1,
	vbar = 2,
	cust = 3,
} custom_type;

static int fd;
static unsigned char *framebuf = NULL;
static unsigned char *old = NULL;
static int width = 0;
static int height = 0;
static int cellwidth = DEFAULT_CELL_WIDTH;
static int cellheight = DEFAULT_CELL_HEIGHT;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "tyan_lcdm_";

/* Internal functions */
static void tyan_lcdm_init_vbar (Driver * drvthis);
static void tyan_lcdm_init_hbar (Driver * drvthis);
static void tyan_lcdm_switch_mode ();
static void tyan_lcdm_hardware_clear (Driver * drvthis);

static void tyan_lcdm_set_rampos(unsigned char pos);
static void tyan_lcdm_write_str(unsigned char *str, unsigned char start_addr, int length);
#if 0   	 
static void tyan_lcdm_set_cursor(unsigned char start_addr, int pos);
#endif
static unsigned char tyan_lcdm_read_key();

/*
 * Opens com port and sets baud correctly...
 */
MODULE_EXPORT int
tyan_lcdm_init (Driver * drvthis, char *args)
{
	struct termios portset;
	int tmp, w, h;

	char device[200] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[200] = DEFAULT_SIZE;

	debug(RPT_INFO, "tyan_lcdm: init(%p,%s)", drvthis, args);

	/* Read config file */
	/* Which serial device should be used */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	debug(RPT_INFO,"%s: using Device %s", drvthis->name, device);

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size , "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE , "%dx%d", &w, &h);
	}
	width = w;
	height = h;

	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name , "Speed", 0, DEFAULT_SPEED);
	if (tmp == 4800) speed = B4800;
	else if (tmp == 9600) speed = B9600;
	else {
		report(RPT_WARNING, "%s: Speed must be 4800 or 9600; using default %d", 
			drvthis->name, DEFAULT_SPEED);
		speed = 9600;
	}

	/* Set up io port correctly, and open it... */
	debug(RPT_DEBUG, "tyan_lcdm: Opening serial device: %s", device);
    
        fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)",
				drvthis->name, device, strerror(errno));
		return -1;
	}

	tcgetattr(fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
#else
	/* The hard way */
	portset.c_cflag = CS8 | CREAD | CLOCAL;
	portset.c_iflag = IXON | IXOFF | IGNBRK | IGNCR;
	portset.c_oflag &= ~ONLCR;
	portset.c_lflag = 0;
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 0;

#endif

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, speed);

	/* Do it... */
	tcsetattr(fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	framebuf = (unsigned char *) malloc(width * height);
	if (framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(framebuf, ' ', width * height);

	/* Set display-specific stuff.. */
	tyan_lcdm_switch_mode();

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}

/*
 * Clean-up
 */
MODULE_EXPORT void
tyan_lcdm_close (Driver * drvthis)
{
	close(fd);

	if (framebuf)
		free(framebuf);
	framebuf = NULL;

	if (old)
		free(old);
	old = NULL;
}

/*
 * Returns the display width
 */
MODULE_EXPORT int
tyan_lcdm_width (Driver *drvthis)
{
	return width;
}

/*
 * Returns the display height
 */
MODULE_EXPORT int
tyan_lcdm_height (Driver *drvthis)
{
	return height;
}

/*
 * Flushes all output to the lcd...
 */
MODULE_EXPORT void
tyan_lcdm_flush (Driver * drvthis)
{
	int i;
	unsigned char *xp, *xq;
/*
 * We don't use delta update yet.
 * It is possible but not easy, we can only update a line, full or begining.
 */
	if (old == NULL) {
		old = (unsigned char *) malloc(width * height);
		memset(old, ' ', width * height);
		tyan_lcdm_hardware_clear(drvthis);
	}

	xp = framebuf;
	xq = old;

	for (i = 0; i < width; i++) {
		if (*xp != *xq) {
			tyan_lcdm_write_str(framebuf, 0x80, 16);
			memcpy(old, framebuf, width);
			break;
		}
		xp++; xq++;
	}

	xp = &framebuf[width];
	xq = &old[width];

	for (i = 0; i < width; i++) {
		if (*xp != *xq) {
			tyan_lcdm_write_str(&framebuf[width], 0xc0, 16);
			memcpy(&old[width], &framebuf[width], width);
			break;
		}
		xp++; xq++;
	}
}

/*
 * Return one char from the KeyRing
 */
MODULE_EXPORT const char *
tyan_lcdm_get_key (Driver *drvthis)
{
        unsigned char key = tyan_lcdm_read_key();

        switch (key) {
                case TYAN_LCDM_KEY_LEFT:
                        return "Left";
                        break;
                case TYAN_LCDM_KEY_UP:
                        return "Up";
                        break;
                case TYAN_LCDM_KEY_DOWN:
                        return "Down";
                        break;
                case TYAN_LCDM_KEY_RIGHT:
                        return "Right";
                        break;
                case TYAN_LCDM_KEY_ENTER:
                        return "Enter"; 
                        break;
                case TYAN_LCDM_KEY_ESCAPE:
                        return "Escape";
                        break;
                default:
                        report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
                        return NULL;
                        break;
        }
}


/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_chr (Driver * drvthis, int x, int y, char c)
{
	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < width) && (y < height))
		framebuf[(y * width) + x] = c;
}

/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 * Need to find out if we have support for intermediate value.
 */
MODULE_EXPORT void
tyan_lcdm_backlight (Driver * drvthis, int on)
{
}

/*
 * switch mode()
 */
static void
tyan_lcdm_switch_mode (Driver * drvthis)
{
	char lcdcmd[4];

	//set os selection
	lcdcmd[0] = TYAN_LCDM_CMD_BEGIN;
	lcdcmd[1] = 0x73;
	lcdcmd[2] = 0x01;
	lcdcmd[3] = TYAN_LCDM_CMD_END;
	write(fd, lcdcmd, 4);
	sleep(1);

	//send "LCD Ready" cmd
	lcdcmd[1] = 0x6c;
	lcdcmd[2] = TYAN_LCDM_CMD_END;
	write(fd, lcdcmd, 3);
	sleep(1);

}



/*
 * Sets up for vertical bars.
 */
static void
tyan_lcdm_init_vbar (Driver * drvthis)
{
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (custom != vbar) {
		tyan_lcdm_set_char(drvthis, 1, a);
		tyan_lcdm_set_char(drvthis, 2, b);
		tyan_lcdm_set_char(drvthis, 3, c);
		tyan_lcdm_set_char(drvthis, 4, d);
		tyan_lcdm_set_char(drvthis, 5, e);
		tyan_lcdm_set_char(drvthis, 6, f);
		tyan_lcdm_set_char(drvthis, 7, g);
		custom = vbar;
	}
}

/*
 * Inits horizontal bars...
 */
static void
tyan_lcdm_init_hbar (Driver * drvthis)
{

	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (custom != hbar) {
		tyan_lcdm_set_char(drvthis, 1, a);
		tyan_lcdm_set_char(drvthis, 2, b);
		tyan_lcdm_set_char(drvthis, 3, c);
		tyan_lcdm_set_char(drvthis, 4, d);
		tyan_lcdm_set_char(drvthis, 5, e);
		tyan_lcdm_set_char(drvthis, 6, f);
		custom = hbar;
	}
}


/*
 * Draws a vertical bar...
 */
MODULE_EXPORT void
tyan_lcdm_vbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'up' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */

	tyan_lcdm_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 0);
}


/*
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
tyan_lcdm_hbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'right' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */

	tyan_lcdm_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 0);
}


/*
 * Writes a big number.
 * This is not supported on 633 because we only have 2 lines...
 */
MODULE_EXPORT void
tyan_lcdm_num (Driver * drvthis, int x, int num)
{
/*
	char out[5];

	snprintf(out, sizeof(out), "%c%c%c", 28, x, num);
	write(fd, out, 3);
*/
}

/*
 * Sets a custom character from 0-7...
 *
 * For input, values > 0 mean "on" and values <= 0 are "off".
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
tyan_lcdm_set_char (Driver * drvthis, int n, char *dat)
{
	unsigned char out[8];
	int row, col;

	if ((n < 0) || (n > 7))
		return;
	if (!dat)
		return;

	for (row = 0; row < cellheight; row++) {
		int letter = 0;

		for (col = 0; col < cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * cellwidth) + col] > 0);
			/* I should remove that debug code. */
			//if (dat[(row * cellheight) + col] == 0) printf(".");
			//if (dat[(row * cellheight) + col] == 1) printf("+");
			//if (dat[(row * cellheight) + col] == 2) printf("x");
			//if (dat[(row * cellheight) + col] == 3) printf("*");
			//printf("'%1d'", dat[(row * cellwidth) + col]);
			//printf("%3d ", letter);
		}
		out[row+1] = letter;
		//printf(": %d\n", letter);
	}
	tyan_lcdm_write_str(out, (unsigned char) (0x40 + n * 8), 8);
}

/*
 * Places an icon on screen
 */
MODULE_EXPORT int
tyan_lcdm_icon (Driver * drvthis, int x, int y, int icon)
{
	char icons[8][5 * 8] = {
	/* Empty Heart */
		{
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },
	/* Filled Heart */
		{
		 1, 1, 1, 1, 1,		  
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },
	/* arrow_up */
		{
		 0, 0, 1, 0, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0,
		 },
	/* arrow_down */
		{
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_off */
		{
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_on */
		{
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 1, 1, 0, 1,
		 1, 0, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	/* checkbox_gray */
		{
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0,
		 },
	 /* Ellipsis */
		{
		 0, 0, 0, 0, 0,		 
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 1, 0, 1, 0,
		 },
	};

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			tyan_lcdm_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 0, icons[1]);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 0, icons[0]);
			tyan_lcdm_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 1, icons[2]);
			tyan_lcdm_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 2, icons[3]);
			tyan_lcdm_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_LEFT:
			tyan_lcdm_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			tyan_lcdm_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 3, icons[4]);
			tyan_lcdm_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 4, icons[5]);
			tyan_lcdm_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
		        custom = cust;
			tyan_lcdm_set_char(drvthis, 5, icons[6]);
			tyan_lcdm_chr(drvthis, x, y, 5);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;

}

/*
 * Clears the LCD screen
 */
MODULE_EXPORT void
tyan_lcdm_clear (Driver * drvthis)
{
	memset(framebuf, ' ', width * height);
}

/*
 * Hardware clears the LCD screen
 */
static void
tyan_lcdm_hardware_clear (Driver * drvthis)
{
	char lcdcmd[5];

	//set os selection
	lcdcmd[0] = TYAN_LCDM_CMD_BEGIN;
	lcdcmd[1] = 0x70;
	lcdcmd[2] = 0x00;
	lcdcmd[3] = 0x01;	
	lcdcmd[4] = TYAN_LCDM_CMD_END;
	write(fd, lcdcmd, 5);

}

/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
tyan_lcdm_string (Driver * drvthis, int x, int y, char string[])
{
	int i;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((y < 0) || (y >= height))
		return;

	for (i = 0; (string[i] != '\0') && (x < width); i++, x++) {
		/* Check for buffer overflows... */
		if (x >= 0)
			framebuf[(y * width) + x] = string[i];
	}
}

static 
void tyan_lcdm_set_rampos(unsigned char pos)
{
	char cmd_str[5];

	cmd_str[0] = TYAN_LCDM_CMD_BEGIN;
	cmd_str[1] = 0x70;
	cmd_str[2] = 0x00;
	cmd_str[3] = pos;
	cmd_str[4] = TYAN_LCDM_CMD_END;
	write(fd, cmd_str, 5);
}

static 
void tyan_lcdm_write_str(unsigned char *str,unsigned char start_addr, int length)
{
//CGRAM 0x40, 0x48,....	
//if Line 1: start_addr = 0x80
//if Line 2: start_addr = 0xc0
// 1<= length <=16
	unsigned char cmd_str[20];
	
	tyan_lcdm_set_rampos(start_addr);
	memset(cmd_str, ' ', 20);
	cmd_str[0] = TYAN_LCDM_CMD_BEGIN;
	cmd_str[1] = 0x70;
	cmd_str[2] = 0x02;
	cmd_str[19] = TYAN_LCDM_CMD_END;
	memcpy(cmd_str+3, str, length);
        write(fd, cmd_str, 20);
}      
#if 0   	 
static 
void tyan_lcdm_set_cursor(unsigned char start_addr, int pos){
	char cmd_str[5];	

	tyan_lcdm_set_rampos(pos+start_addr); 
	cmd_str[0] = TYAN_LCDM_CMD_BEGIN;
	cmd_str[1] = 0x70;
	cmd_str[2] = 0x00;
	cmd_str[3] = 0x0e;
	cmd_str[4] = TYAN_LCDM_CMD_END;
	write(fd,cmd_str,5);       
}	     

#endif

static 
unsigned char tyan_lcdm_read_key()
{
	int count = 0;	
	char key_str[4];

	memset(key_str, 0, 4);
	count = read(fd, key_str, 4);
	if ((key_str[0] == (char) TYAN_LCDM_CMD_BEGIN)
	    && (key_str[1] == (char) 0x72)
	    && (key_str[3] == (char) TYAN_LCDM_CMD_END)) {
		return key_str[2];
	}
	return 0xF4;  //error
}
