/*  This is the LCDproc driver for Seetron devices (http://www.seetron.com)

    Copyright (C) 1999, William Ferrell and Scott Scriven
		  2001, Philip Pokorny
		  2001, David Douthitt
		  2001, David Glaude
		  2001, Joris Robijn
		  2001, Eddie Sheldrake
		  2001, Rene Wagner
		  2002, Mike Patnode
		  2002, Luis Llorente
		  2002, Harald Milz

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

/* derived from the CrystalFontz driver */


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
#include "SGX120.h"
#include "render.h"
#include "shared/str.h"
#include "shared/report.h"
#include "server/configfile.h"

static int fd;
static char* backingstore = NULL;
static char* blankrow = NULL;

static void SGX120_linewrap (int on);
static void SGX120_autoscroll (int on);
static void SGX120_hidecursor ();
static void SGX120_reboot ();
static void SGX120_heartbeat (int type);

static char icon_char = ' ';

// Character used for title bars...
#define PAD ' '

// TODO:  Get rid of this variable?
lcd_logical_driver *SGX120;
// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
SGX120_init (lcd_logical_driver * driver, char *args)
{
	struct termios portset;
	int tmp, w, h;
	int reboot = 0;

	int contrast = SGX120_DEF_CONTRAST;
	char device[200] = SGX120_DEF_DEVICE;
	int speed = SGX120_DEF_SPEED;
	char size[200] = SGX120_DEF_SIZE;

	char buf[256] = "";

	SGX120 = driver;

	debug(RPT_INFO, "SGX120: init(%p,%s)", driver, args );

	/* TODO: replace DriverName with driver->name when that field exists. */
	#define DriverName "SGX120"


	/*Read config file*/

	/*Which serial device should be used*/
	strncpy(device, config_get_string ( DriverName , "Device" , 0 , SGX120_DEF_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	report (RPT_INFO,"SGX120: Using device: %s", device);

	/*Which size*/
	strncpy(size, config_get_string ( DriverName , "Size" , 0 , SGX120_DEF_SIZE),sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size , "%dx%d", &w, &h ) != 2
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report (RPT_WARNING, "SGX120: Cannot read size: %s. Using default value.\n", size);
		sscanf( SGX120_DEF_SIZE , "%dx%d", &w, &h );
	}
	driver->wid = w;
	driver->hgt = h;

	/*contrast doesn't exist for the seetron*/

	/*Which speed*/
	tmp = config_get_int ( DriverName , "Speed" , 0 , SGX120_DEF_SPEED);

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
		default:
			speed = SGX120_DEF_SPEED;
			switch (speed) {
				case B1200:
					strncpy(buf,"1200", sizeof(buf));
					break;
				case B2400:
					strncpy(buf,"2400", sizeof(buf));
					break;
				case B4800:
					strncpy(buf,"4800", sizeof(buf));
					break;
				case B9600:
					strncpy(buf,"9600", sizeof(buf));
					break;
				case B19200:
					strncpy(buf,"19200", sizeof(buf));
					break;
			}
			report (RPT_WARNING , "SGX120: Speed must be 1200, 2400, 4800, 9600, or 19200. Using default value of %s baud!", buf);
			strncpy(buf,"", sizeof(buf));
	}


	/*Reboot display doesn't exist for the Seetron*/

	/* End of config file parsing*/


	/* Allocate framebuffer memory*/
	/* You must use driver->framebuf here, but may use lcd.framebuf later.*/
	if (!driver->framebuf) {
		driver->framebuf = malloc (driver->wid * driver->hgt);
		backingstore = calloc (driver->wid * driver->hgt, 1);
		blankrow = malloc (driver->wid);
		memset(blankrow, ' ', driver->wid);
	}

	if (!driver->framebuf) {
                report(RPT_ERR, "SGX120: Error: unable to create framebuffer.\n");
		return -1;
	}

	/* Set up io port correctly, and open it...*/
	debug( RPT_DEBUG, "SGX120: Opening serial device: %s", device);
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report (RPT_ERR, "SGX120: init() failed (%s)\n", strerror (errno));
		return -1;
	} else {
		report (RPT_INFO, "SGX120: Opened display on %s", device);
	}

	tcgetattr (fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw( &portset );
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	// Set port speed
	cfsetospeed (&portset, speed);
	cfsetispeed (&portset, B0);

	// Do it...
	tcsetattr (fd, TCSANOW, &portset);

	/* Set display-specific stuff..*/
	if (reboot) {
		SGX120_reboot ();
	}
	SGX120_hidecursor ();
	SGX120_linewrap (1);
	SGX120_autoscroll (0);
	SGX120_backlight (0);

	/* Set the functions the driver supports...*/

	driver->daemonize = 1; /* make the server daemonize after initialization*/

	driver->clear = SGX120_clear;
	driver->string = SGX120_string;
	driver->chr = SGX120_chr;
	driver->vbar = SGX120_vbar;
	driver->init_vbar = SGX120_init_vbar;
	driver->hbar = SGX120_hbar;
	driver->init_hbar = SGX120_init_hbar;
	driver->num = SGX120_num;

	driver->init = SGX120_init;
	driver->close = SGX120_close;
	driver->flush = SGX120_flush;
	driver->flush_box = SGX120_flush_box;
	driver->contrast = NULL;
	driver->backlight = SGX120_backlight;
	driver->set_char = NULL;
	driver->icon = SGX120_icon;
	driver->draw_frame = SGX120_draw_frame;

	SGX120_contrast (contrast);
	SGX120_clear ();

	driver->cellwid = SGX120_DEF_CELL_WIDTH;
	driver->cellhgt = SGX120_DEF_CELL_HEIGHT;

	driver->heartbeat = SGX120_heartbeat;

	report (RPT_DEBUG, "SGX120_init: done\n");

	return fd;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
void
SGX120_close ()
{
	close (fd);

	if (SGX120->framebuf)
		free (SGX120->framebuf);

	if (backingstore)
		free (backingstore);

	if (blankrow)
		free (blankrow);

	SGX120->framebuf = NULL;
	backingstore = NULL;
	blankrow = NULL;
}

void
SGX120_flush ()
{
	SGX120_draw_frame (SGX120->framebuf);
}

void
SGX120_flush_box (int lft, int top, int rgt, int bot)
{

/* FIXME ??? */

	int y;
	char out[LCD_MAX_WIDTH];

	debug (RPT_DEBUG, "SGX120: flush_box (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

	for (y = top; y <= bot; y++) {
		snprintf (out, sizeof(out), "%c%c", 
			16, 64 + lft + y * SGX120->wid);
		write (fd, out, 2);
		write (fd, SGX120->framebuf + (y * SGX120->wid) + lft, rgt - lft + 1);

	}

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
SGX120_chr (int x, int y, char c)
{
	y--;
	x--;

	if (c < 32 && c >= 0)
		c = ' ';
//		c += 128;

	SGX120->framebuf[(y * SGX120->wid) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
//
int
SGX120_contrast (int contrast)
{
	/* seetron has no contrast */
	return contrast;
}

/////////////////////////////////////////////////////////////////
// Sets the backlight brightness
//
void
SGX120_backlight (int on)
{
/* Seetron can only do on / off */

	char out[4];
	if (on) {
		snprintf (out, sizeof(out), "%c", 14);
	} else {
		snprintf (out, sizeof(out), "%c", 15);
	}
	
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
SGX120_linewrap (int on)
{
/* Seetron does always linewrap */
/*
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 23);
	else
		snprintf (out, sizeof(out), "%c", 24);
	write (fd, out, 1);
*/
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
SGX120_autoscroll (int on)
{
/* as above. autoscroll is always on for the seetron */
/*
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 19);
	else
		snprintf (out, sizeof(out), "%c", 20);
	write (fd, out, 1);
*/
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void
SGX120_hidecursor ()
{
/* can't hide cursor, and don't need to.  */
/*
	char out[4];
	snprintf (out, sizeof(out), "%c", 4);
	write (fd, out, 1);
*/
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
SGX120_reboot ()
{
/* actually, for the Seetron, it's a FF and clear */
	char out[4];
	snprintf (out, sizeof(out), "%c", 12);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before SGX120->vbar()
//
void
SGX120_init_vbar ()
{
/* have to set in the EEPROM! */
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
SGX120_init_hbar ()
{
/* have to set in the EEPROM */
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
// Must be used with the custom character set in CHARS1.BMP
// 152 = single pixel height
// 159 = full height (8 pixel)
//
void
SGX120_vbar (int x, int len)
{
	char map[9] = { 32, 152, 153, 154, 155, 156, 157, 158, 159 };

	int y;
	for (y = SGX120->hgt; y > 0 && len > 0; y--) {
		if (len >= SGX120->cellhgt)
			SGX120_chr (x, y, map[8]);
		else
			SGX120_chr (x, y, map[len]);

		len -= SGX120->cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
// Must be used with the custom character set in CHARS1.BMP
//
void
SGX120_hbar (int x, int y, int len)
{
	char map[7] = { 32, 172, 173, 174, 175, 176, 177 };

	for (; x <= SGX120->wid && len > 0; x++) {
		if (len >= SGX120->cellwid)
			SGX120_chr (x, y, map[6]);
		else
			SGX120_chr (x, y, map[len]);

		len -= SGX120->cellwid;

	}

}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
SGX120_num (int x, int num)
{
// The Seetron does handle double wide / height chars in hardware
// but we don't get full height. TODO: Emulate in software :-( 
// THIS DOESN'T WORK YET. 

	char out[4];
	// Esc-F3      - double height / double width
	// Ctrl-P 64+x - go to position x
	// Esc-F0      - normal char width / height
	snprintf (out, sizeof(out), "%c%c%c", 27, 'F', '3');
	write (fd, out, 3);
	snprintf (out, sizeof(out), "%c%c", 16, 64 + x);
	write (fd, out, 2);
	snprintf (out, sizeof(out), "%c", num + 0x30);
	write (fd, out, 1);
	snprintf (out, sizeof(out), "%c%c%c", 27, 'F', '0');
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
void
SGX120_set_char (int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

// Seetron: we use higher numbers! 
	if (n < 141 || n > 143)
		return;
	if (!dat)
		return;

	write (fd, dat, 1);

	return;

// rest of CFontz: 

	snprintf (out, sizeof(out), "%c%c", 25, n);
	write (fd, out, 2);

	for (row = 0; row < SGX120->cellhgt; row++) {
		letter = 0;
		for (col = 0; col < SGX120->cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * SGX120->cellwid) + col] > 0);
		}
		write (fd, &letter, 1);
	}
}

void
SGX120_icon (int which, char dest)
{
/* to be used with the custom character set in CHARS1.BMP
   Empty Heart is 141
   Filled Heart is 142
   Ellipsis is 143
*/

	if (dest == 0) {
		switch (which) {
			case 0:
				icon_char = 141;
				break;
			case 1:
				icon_char = 142;
				break;
			case 2:
				icon_char = 143;
				break;
			default:
				icon_char = ' ';
				break;
		}
	}

}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized SGX120->wid*SGX120->hgt
//
void
SGX120_draw_frame (char *dat)
{
	char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
	char *row, *b_row;
	int i;

	if (!dat)
		return;

	for (i = 0; i < SGX120->hgt; i++) {

		row = dat + (SGX120->wid * i);
		b_row = backingstore + (SGX120->wid * i);

		/* Backing-store implementation.  If it's already
		 * on the screen, don't put it there again
		 */
		if (memcmp(b_row, row, SGX120->wid) == 0)
		    continue;

        /* else, write out the entire row */
		memcpy(b_row, row, SGX120->wid);
		snprintf (out, sizeof(out), "%c%c", 16, 64 + SGX120->wid * i);
		write (fd, out, 2);
		write (fd, row, SGX120->wid);
	}

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
SGX120_clear ()
{
	memset (SGX120->framebuf, ' ', SGX120->wid * SGX120->hgt);

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
SGX120_string (int x, int y, char string[])
{
	int i, offset;

	x--;	  // Convert 1-based coords to 0-based...
	y--;

	for (i = 0; string[i]; i++) {
		// we only want to calculate this once. 
		offset = (y * SGX120->wid) + x + i;

		// Check for buffer overflows...
		if (offset > (SGX120->wid * SGX120->hgt))
			break;

		// we need to convert char #255 to PAD
		// the Seetron display doesn't understand #255. 
		switch (string[i]) {
			case -1:   /* This magically translates to 255 */
			string[i] = PAD;
			break;
		}

		SGX120->framebuf[offset] = string[i];
	}
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
static void
SGX120_heartbeat (int type)
{
	static int timer = 0;
	int whichIcon;
	static int saved_type = HEARTBEAT_ON;

	if (type)
		saved_type = type;

	if (type == HEARTBEAT_ON) {
		// Set this to pulsate like a real heart beat...
		whichIcon = (! ((timer + 4) & 5));

		// This defines a custom character EVERY time...
		// not efficient... is this necessary?
		// answer hm: no :-)) 
		SGX120_icon (whichIcon, 0);

		// Put character on screen...
		SGX120_chr (SGX120->wid, 1, icon_char);

		// change display...
		SGX120_flush ();
	}

	timer++;
	timer &= 0x0f;
}
