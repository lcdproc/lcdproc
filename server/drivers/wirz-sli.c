/*	wirz-sli.c -- Source file for LCDproc Wirz SLI driver
	Copyright (C) 1999 Horizon Technologies-http://horizon.pair.com/
	Written by Bryan Rittmeyer <bryanr@pair.com> - Released under GPL

        LCD info: http://www.wirz.com/                               */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "wirz-sli.h"
//#include "drv_base.h"
#include "report.h"

//#include "shared/debug.h"
#include "shared/str.h"

#define SLI_DEFAULT_DEVICE	"/dev/lcd"

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
static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "sli_";


/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
sli_init (Driver *drvthis)
{
	struct termios portset;
	char out[2];

	char device[256] = SLI_DEFAULT_DEVICE;
	int speed = B19200;

	/* Read config file */

	/* What device should be used */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   SLI_DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';

	/* What speed to use */
	speed = drvthis->config_get_int(drvthis->name, "Speed", 0, 19200);
  
	if (speed == 1200)        speed = B1200;
	else if (speed == 2400)   speed = B2400;
	else if (speed == 9600)   speed = B9600;
	else if (speed == 19200)  speed = B19200;
	else if (speed == 38400)  speed = B38400;
	else if (speed == 57600)  speed = B57600;
	else if (speed == 115200) speed = B115200;
	else {
		report(RPT_WARNING, "sli: Illegal speed: %d. Must be one of 1200, 2400, 9600, 19200, 38400, 57600, or 115200. Using default.\n", speed);
		speed = B19200;
	}

	/* End of config file parsing */

	// Set up io port correctly, and open it...
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report(RPT_ERR, "sli_init: open(%s) failed (%s)\n", device, strerror (errno));
		return -1;
	}
	report(RPT_DEBUG, "sli_init: opened device %s\n", device);
	
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

	/* Initialize SLI using autobaud detection, and then turn off cursor
	   and clear screen */
	usleep (150000);			  /* 150ms delay to allow SLI to power on */
	out[0] = 13;				  /* CR for SLI autobaud */
	write (fd, out, 1);
	usleep (3000);				  /* 3ms delay.. wait for it to autobaud */
	out[0] = 0x0FE;
	out[1] = 0x00C;				  /* No cursor */
	write (fd, out, 2);
	out[0] = 0x0FE;
	out[1] = 0x001;				  /* Clear LCD, not sure if this belongs here */
	write (fd, out, 2);

	// Set LCD parameters (I use a 16x2 LCD) -- small but still useful
	// Its also much cheaper than the higher quality Matrix Orbital modules
	// Currently, $30 for interface kit and 16x2 non-backlit LCD...
	width = 15;
	height = 2;

	return fd;
}

/////////////////////////////////////////////////////////////////
// Clean up
//
MODULE_EXPORT void
sli_close (Driver *drvthis)
{
	close (fd);

	if (framebuf)
		free (framebuf);

	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
sli_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
sli_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Flush framebuffer to LCD
//
MODULE_EXPORT void
sli_flush (Driver *drvthis)
{
	char out[2];					  /* Again, why does the Matrix driver allocate so much here? */

	/*
	   out[0]=0x0FE;
	   out[1]=0x001;
	   write(fd, out, 2);
	 */

	/* Don't update if we have no new data
	   this keeps me from getting a migraine
	   (just like those copyleft penguin mints... mmmmmm)  */

	//   if (!strncmp(dat,lastframe,32)) /* Nothing has changed */
	//     return;

	/* Do the actual refresh */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write (fd, out, 2);
	write (fd, &framebuf[0], 16);
	usleep (10);
	write (fd, &framebuf[16], 15);

	//   strncpy(lastframe,dat,32); // Update lastframe...
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
sli_clear (Driver *drvthis)
{
	memset (framebuf, ' ', width * height);

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
sli_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

	for (i = 0; string[i]; i++) {
		// Check for buffer overflows...
		if ((y * width) + x + i > (width * height))
			break;
		framebuf[(y * width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
sli_chr (Driver *drvthis, int x, int y, char c)
{
	y--;
	x--;

	framebuf[(y * width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before sli->vbar()
//

/* Because of the way we do this (custom characters in CGRAM)
   we can't have both horizontal and vertical bars at once...
   this also appears to be a limitation of the Matrix Orbital
   modules so I will assume that all client coders know about it

   I think it would be cool to use triangular stuff for the non-full
   characters, so that you can do both bar types at once.. maybe I
   will release a new version of the SLI driver that attempts this */

static void
sli_init_vbar (Driver *drvthis)
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
		sli_set_char (drvthis, 1, a);
		sli_set_char (drvthis, 2, b);
		sli_set_char (drvthis, 3, c);
		sli_set_char (drvthis, 4, d);
		sli_set_char (drvthis, 5, e);
		sli_set_char (drvthis, 6, f);
		sli_set_char (drvthis, 7, g);
		custom = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
sli_init_hbar (Driver *drvthis)
{

	char a[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};
	char e[] = {
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
		sli_set_char (drvthis, 1, a);
		sli_set_char (drvthis, 2, b);
		sli_set_char (drvthis, 3, c);
		sli_set_char (drvthis, 4, d);
		sli_set_char (drvthis, 5, e);
		custom = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//

/* It would be cool if you could start a vertical bar at any y
   like the horizontal bar routine can start at any x... but
   since we only have 2 lines anyway this is rather pointless
   for me to add                                               */

MODULE_EXPORT void
sli_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	sli_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
sli_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	sli_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 0);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
sli_set_char (Driver *drvthis, int n, char *dat)
{
	char out[2];
	int row, col;
	int letter;

	/* SLI also has 8 user definable characters */
	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	/* Move cursor to CGRAM */
	out[0] = 0x0FE;
	out[1] = 0x040 + 8 * n;
	write (fd, out, 2);

	for (row = 0; row < LCD_DEFAULT_CELLHEIGHT; row++) {
		letter = 0;
		for (col = 0; col < LCD_DEFAULT_CELLWIDTH; col++) {
			letter <<= 1;
			letter |= (dat[(row * LCD_DEFAULT_CELLWIDTH) + col] > 0);
		}
		letter |= 0x020;			  /* SLI can't accept CR, LF, etc in this character! */
		write (fd, &letter, 1);
	}

	/* Move cursor back to DDRAM */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write (fd, out, 2);
}

MODULE_EXPORT int
sli_icon (Driver *drvthis, int x, int y, int icon)
{
	char icons[3][5 * 8] = {
		{
		 1, 1, 1, 1, 1,			  // Empty Heart
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1,			  // Filled Heart
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0,			  // Ellipsis
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1,
		 },

	};

	if (custom == bign)
		custom = beat;
	switch( icon ) {
		case ICON_BLOCK_FILLED:
			sli_chr( drvthis, x, y, 255 );
			break;
		case ICON_HEART_FILLED:
			sli_set_char( drvthis, 0, icons[1] );
			sli_chr( drvthis, x, y, 0 );
			break;
		case ICON_HEART_OPEN:
			sli_set_char( drvthis, 0, icons[0] );
			sli_chr( drvthis, x, y, 0 );
			break;
		default:
			return -1;
	}
	return 0;
}

