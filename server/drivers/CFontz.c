/*  This is the LCDproc driver for CrystalFontz devices (http://crystalfontz.com)

    Copyright (C) 2001 ????

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

/*configfile support added by Rene Wagner (c) 2001*/
/*backlight support modified by Rene Wagner (c) 2001*/
/*block patch by Eddie Sheldrake (c) 2001 inserted by Rene Wagner*/

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
#include "CFontz.h"
//#include "drv_base.h"
//#include "shared/debug.h"
//#include "shared/str.h"
#include "report.h"
#include "lcd_lib.h"
//#include "server/configfile.h"


static int custom = 0;
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
	beat = 8
} custom_type;

static int fd;
static char *framebuf = NULL;
static int width = 0;
static int height = 0;
static int cellwidth = DEFAULT_CELL_WIDTH;
static int cellheight = DEFAULT_CELL_HEIGHT;
static int contrast = DEFAULT_CONTRAST;
static int brightness = DEFAULT_BRIGHTNESS;
static int offbrightness = DEFAULT_OFFBRIGHTNESS;
static int newfirmware = 0;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "CFontz_";


// Internal functions
static void CFontz_linewrap (int on);
static void CFontz_autoscroll (int on);
static void CFontz_hidecursor ();
static void CFontz_reboot ();
static void CFontz_init_vbar (Driver * drvthis);
static void CFontz_init_hbar (Driver * drvthis);


// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
CFontz_init (Driver * drvthis, char *args)
{
	struct termios portset;
	int tmp, w, h;
	int reboot = 0;

	int contrast = DEFAULT_CONTRAST;
	char device[200] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[200] = DEFAULT_SIZE;

	debug(RPT_INFO, "CFontz: init(%p,%s)", drvthis, args );

	/*Read config file*/

	/*Which serial device should be used*/
	strncpy(device, drvthis->config_get_string ( drvthis->name , "Device" , 0 , DEFAULT_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	debug (RPT_INFO,"CFontz: Using device: %s", device);

	/*Which size*/
	strncpy(size, drvthis->config_get_string ( drvthis->name , "Size" , 0 , DEFAULT_SIZE),sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size , "%dx%d", &w, &h ) != 2
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report (RPT_WARNING, "CFontz_init: Cannot read size: %s. Using default value.\n", size);
		sscanf( DEFAULT_SIZE , "%dx%d", &w, &h );
	} else {
		width = w;
		height = h;
	}

	/*Which contrast*/
	if (0<=drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) && drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) <= 255) {
		contrast = drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST);
	} else {
		report (RPT_WARNING, "CFontz_init: Contrast must between 0 and 255. Using default value.\n");
	}

	/*Which backlight brightness*/
	if (0<=drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS) && drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS) <= 255) {
		brightness = drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS);
	} else {
		report (RPT_WARNING, "CFontz_init: Brightness must between 0 and 255. Using default value.\n");
	}

	/*Which backlight-off "brightness"*/
	if (0<=drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS) && drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS) <= 255) {
		offbrightness = drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS);
	} else {
		report (RPT_WARNING, "CFontz_init: OffBrightness must between 0 and 255. Using default value.\n");
	}


	/*Which speed*/
	tmp = drvthis->config_get_int ( drvthis->name , "Speed" , 0 , DEFAULT_SPEED);
	if (tmp == 1200) speed = B1200;
	else if (tmp == 2400) speed = B2400;
	else if (tmp == 9600) speed = B9600;
	else { report (RPT_WARNING, "CFontz_init: Speed must be 1200, 2400, or 9600. Using default value.\n", speed);
	}

	/*New firmware version?*/
	if(drvthis->config_get_bool( drvthis->name , "NewFirmware" , 0 , 0)) {
		newfirmware = 1;
	}

	/*Reboot display?*/
	if (drvthis->config_get_bool( drvthis->name , "Reboot" , 0 , 0)) {
		report (RPT_INFO, "LCDd: rebooting CrystalFontz LCD...\n");
		reboot = 1;
	}

	// Set up io port correctly, and open it...
	debug( RPT_DEBUG, "CFontz: Opening serial device: %s", device);
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report (RPT_ERR, "CFontz_init: failed (%s)\n", strerror (errno));
		return -1;
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

	// Make sure the frame buffer is there...
	framebuf = (unsigned char *) malloc (width * height);
	memset (framebuf, ' ', width * height);

	// Set display-specific stuff..
	if (reboot) {
		CFontz_reboot ();
		sleep (4);
		reboot = 0;
	}
	sleep (1);
	CFontz_hidecursor ();
	CFontz_linewrap (1);
	CFontz_autoscroll (0);
	//CFontz_backlight (drvthis, backlight_brightness);  // render.c variables should not be used in drivers !

	CFontz_set_contrast (drvthis, contrast);


	report (RPT_DEBUG, "CFontz_init: done\n");

	return 0;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
CFontz_close (Driver * drvthis)
{
	close (fd);

	if(framebuf) free (framebuf);
	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
CFontz_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
CFontz_height (Driver *drvthis)
{
	return height;
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
CFontz_flush (Driver * drvthis)
{
	char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
	int i;

	// Custom characters start at 128, not at 0.
	/*
	   for(i=0; i<width*height; i++)
	   {
	   if(framebuf[i] < 32  &&  framebuf[i] >= 0) framebuf[i] += 128;
	   }
	 */

	for (i = 0; i < height; i++) {
		snprintf (out, sizeof(out), "%c%c%c", 17, 0, i);
		write (fd, out, 3);
		write (fd, framebuf + (width * i), width);
	}
	/*
	   snprintf(out, sizeof(out), "%c", 1);
	   write(fd, out, 1);
	   write(fd, framebuf, width*height);
	 */
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
CFontz_chr (Driver * drvthis, int x, int y, char c)
{
	y--;
	x--;

	if (c < 32 && c >= 0)
		c += 128;

	// For V2 of the firmware to get the block to display right
	if (newfirmware && c==-1) {
		c=214;
	}

	framebuf[(y * width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Returns current contrast
// This is only the locally stored contrast, the contrast value
// cannot be retrieved from the LCD.
// Value 0 to 1000.
//
MODULE_EXPORT int
CFontz_get_contrast (Driver * drvthis)
{
	return contrast;
}

/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
// Value 0 to 100.
//
MODULE_EXPORT void
CFontz_set_contrast (Driver * drvthis, int promille)
{
	char out[4];

	// Check it
	if( promille < 0 || promille > 1000 )
		return;

	// Store it
	contrast = promille;

	// And do it
	snprintf (out, sizeof(out), "%c%c", 15, (unsigned char) (promille / 10) ); // converted to be 0 to 100
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
MODULE_EXPORT void
CFontz_backlight (Driver * drvthis, int on)
{
	char out[4];
	if (on) {
		snprintf (out, sizeof(out), "%c%c", 14, brightness);
	} else {
		snprintf (out, sizeof(out), "%c%c", 14, offbrightness);
	}
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
CFontz_linewrap (int on)
{
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 23);
	else
		snprintf (out, sizeof(out), "%c", 24);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
CFontz_autoscroll (int on)
{
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 19);
	else
		snprintf (out, sizeof(out), "%c", 20);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void
CFontz_hidecursor ()
{
	char out[4];
	snprintf (out, sizeof(out), "%c", 4);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
CFontz_reboot ()
{
	char out[4];
	snprintf (out, sizeof(out), "%c", 26);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.
//
static void
CFontz_init_vbar (Driver * drvthis)
{
	char a[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};

	if (custom != vbar) {
		CFontz_set_char (drvthis, 1, a);
		CFontz_set_char (drvthis, 2, b);
		CFontz_set_char (drvthis, 3, c);
		CFontz_set_char (drvthis, 4, d);
		CFontz_set_char (drvthis, 5, e);
		CFontz_set_char (drvthis, 6, f);
		CFontz_set_char (drvthis, 7, g);
		custom = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
CFontz_init_hbar (Driver * drvthis)
{

	char a[] = {
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
	};

	if (custom != hbar) {
		CFontz_set_char (drvthis, 1, a);
		CFontz_set_char (drvthis, 2, b);
		CFontz_set_char (drvthis, 3, c);
		CFontz_set_char (drvthis, 4, d);
		CFontz_set_char (drvthis, 5, e);
		CFontz_set_char (drvthis, 6, f);
		custom = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
CFontz_vbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	CFontz_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
CFontz_hbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	CFontz_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 0);
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
CFontz_num (Driver * drvthis, int x, int num)
{
	char out[5];
	snprintf (out, sizeof(out), "%c%c%c", 28, x, num);
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
CFontz_set_char (Driver * drvthis, int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	snprintf (out, sizeof(out), "%c%c", 25, n);
	write (fd, out, 2);

	for (row = 0; row < cellheight; row++) {
		letter = 0;
		for (col = 0; col < cellheight; col++) {
			letter <<= 1;
			letter |= (dat[(row * cellheight) + col] > 0);
		}
		write (fd, &letter, 1);
	}
}

/////////////////////////////////////////////////////////////////
// Places an icon on screen
//
MODULE_EXPORT void
CFontz_icon (Driver * drvthis, int x, int y, int icon)
{
	char icons[3][6 * 8] = {
		{
		 1, 1, 1, 1, 1, 1,		  // Empty Heart
		 1, 0, 1, 0, 1, 1,
		 0, 0, 0, 0, 0, 1,
		 0, 0, 0, 0, 0, 1,
		 0, 0, 0, 0, 0, 1,
		 1, 0, 0, 0, 1, 1,
		 1, 1, 0, 1, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1, 1,		  // Filled Heart
		 1, 0, 1, 0, 1, 1,
		 0, 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0, 1,
		 0, 1, 1, 1, 0, 1,
		 1, 0, 1, 0, 1, 1,
		 1, 1, 0, 1, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0, 0,		  // Ellipsis
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1, 0,
		 },

	};

	if (custom == bign)
		custom = beat;

	switch( icon ) {
		case ICON_BLOCK_FILLED:
			CFontz_chr( drvthis, x, y, 255 );
			break;
		case ICON_HEART_FILLED:
			CFontz_set_char( drvthis, 0, icons[1] );
			CFontz_chr( drvthis, x, y, 0 );
			break;
		case ICON_HEART_OPEN:
			CFontz_set_char( drvthis, 0, icons[0] );
			CFontz_chr( drvthis, x, y, 0 );
			break;
		default:
			report( RPT_WARNING, "CFontz_icon: unknown or unsupported icon: %d", icon );
	}

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
CFontz_clear (Driver * drvthis)
{
	memset (framebuf, ' ', width * height);

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
CFontz_string (Driver * drvthis, int x, int y, char string[])
{
	int i;

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

	for (i = 0; string[i]; i++) {


		// For V2 of the firmware to get the block to display right
		if (newfirmware && string[i]==-1) {
		string[i]=214;
		}


		// Check for buffer overflows...
		if ((y * width) + x + i > (width * height))
			break;
		framebuf[(y * width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
MODULE_EXPORT void
CFontz_heartbeat (Driver *drvthis, int type)
{
	static int timer = 0;
	int whichIcon;
	static int saved_type = HEARTBEAT_ON;

	if (type)
		saved_type = type;

	if (type == HEARTBEAT_ON) {
		/* Set this to pulsate like a real heart beat... */
		if( ((timer + 4) & 5))
			whichIcon = ICON_HEART_OPEN;
		else
			whichIcon = ICON_HEART_FILLED;

		/* place the icon */
		CFontz_icon (drvthis, width, 1, whichIcon);
	}

	timer++;
	timer &= 0x0f;
}
