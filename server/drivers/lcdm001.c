/*lcdm001.c*/
/*  This is the LCDproc driver for the "LCDM001" device from kernelconcepts.de

    Copyright (C) 2001  Rene Wagner <reenoo@gmx.de>

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

/* This driver is mostly based on the MtxOrb driver.
   See the file MtxOrb.c for copyright details */
/* The heartbeat workaround has been taken from the curses driver
   See the file curses_drv.c for copyright details */
/* The function calls needed for reporting and getting settings from the
   configfile have been written taking the calls in
   sed1330.c ((C) by Joris Robijn) as examples*/
/* (Hopefully I have NOT forgotten any file I have stolen code from.
   If so send me an e-mail or add your copyright here!) */

/* LCDM001 does NOT support custom chars
   Most of the displaying problems have been fixed
   using ASCII workarounds*/

/* You can modify the characters that are displayed instead of
   the normal icons for the heartbeat in lcdm001.h*/

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

# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif

#include "lcd.h"
#include "lcdm001.h"
#include "report.h"
//#include "configfile.h"


int fd;
static int clear = 1;
static char icon_char = '@';
static char pause_key = DOWN_KEY, back_key = LEFT_KEY, forward_key = RIGHT_KEY, main_menu_key = UP_KEY;
static char *framebuf = NULL;
static int width = LCD_DEFAULT_WIDTH;
static int height = LCD_DEFAULT_HEIGHT;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "lcdm001_";

static void lcdm001_cursorblink (Driver *drvthis, int on);
static char lcdm001_parse_keypad_setting ( Driver *drvthis, char * keyname, char * default_value );

#define ValidX(x) if ((x) > width) { (x) = width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > height) { (y) = height; } else (y) = (y) < 1 ? 1 : (y);

// Parse one key from the configfile
static char lcdm001_parse_keypad_setting (Driver *drvthis, char * keyname, char * default_value)
{
	char return_val = 0;

	if (strcmp( drvthis->config_get_string ( drvthis->name, keyname, 0, default_value), "LeftKey")==0) {
		return_val=LEFT_KEY;
	} else if (strcmp( drvthis->config_get_string ( drvthis->name, keyname, 0, default_value), "RightKey")==0) {
		return_val=RIGHT_KEY;
	} else if (strcmp( drvthis->config_get_string ( drvthis->name, keyname, 0, default_value), "UpKey")==0) {
		return_val=UP_KEY;
	} else if (strcmp( drvthis->config_get_string ( drvthis->name, keyname, 0, default_value), "DownKey")==0) {
		return_val=DOWN_KEY;
	} else {
		report (RPT_WARNING, "LCDM001: Invalid  config file setting for %s. Using default value %s.\n", keyname, default_value);
		if (strcmp (default_value, "LeftKey")==0) {
			return_val=LEFT_KEY;
		} else if (strcmp (default_value, "RightKey")==0) {
			return_val=RIGHT_KEY;
		} else if (strcmp (default_value, "UpKey")==0) {
			return_val=UP_KEY;
		} else if (strcmp (default_value, "DownKey")==0) {
			return_val=DOWN_KEY;
		}
	}
	return return_val;
}

/* Set cursorblink on/off */
static void
lcdm001_cursorblink (Driver *drvthis, int on)
{
	if (on) {
		write (fd, "~K1", 3);
		debug(RPT_INFO, "LCDM001: cursorblink turned on");
	} else {
		write (fd, "~K0", 3);
		debug(RPT_INFO, "LCDM001: cursorblink turned off");
	}
}


/* TODO: Get lcd.framebuf to properly work as whatever driver is running...*/

/*********************************************************************
 * init() should set up any device-specific stuff, and
 * point all the function pointers.
 */
MODULE_EXPORT int
lcdm001_init (Driver *drvthis)
{
        char device[200];
	int speed=B38400;
        struct termios portset;

	char out[5]="";

	debug( RPT_INFO, "LCDM001: init(%p)", drvthis );

	framebuf = malloc (width * height);

	if (!framebuf) {
                report(RPT_ERR, "\nError: unable to create LCDM001 framebuffer.\n");
		return -1;
	}
	memset (framebuf, ' ', width * height);

	// READ CONFIG FILE:

	// which serial device should be used
	strncpy(device, drvthis->config_get_string ( drvthis->name , "Device" , 0 , "/dev/lcd"),sizeof(device));
	device[sizeof(device)-1]=0;
	report (RPT_INFO,"LCDM001: Using device: %s", device);

	// keypad settings
	pause_key =      lcdm001_parse_keypad_setting (drvthis, "PauseKey", "DownKey");
	back_key =       lcdm001_parse_keypad_setting (drvthis, "BackKey", "LeftKey");
	forward_key =    lcdm001_parse_keypad_setting (drvthis, "ForwardKey", "RightKey");
	main_menu_key =  lcdm001_parse_keypad_setting (drvthis, "MainMenuKey", "UpKey");

	// Set up io port correctly, and open it...
	debug( RPT_DEBUG, "LCDM001: Opening serial device: %s", device);
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		switch (errno) {
			case ENOENT: report( RPT_ERR, "LCDM001: lcdm001_init() failed: Device file missing: %s\n", device);
				break;
			case EACCES: report( RPT_ERR, "LCDM001: lcdm001_init() failed: Could not open device: %s\n", device);
				report( RPT_ERR, "LCDM001: lcdm001_init() failed: Make sure you have rw access to %s!\n", device);
				break;
			default: report( RPT_ERR, "LCDM001: lcdm001_init() failed (%s)\n", strerror (errno));
				break;
		}
  		return -1;
	} else {
		report (RPT_INFO, "opened LCDM001 display on %s", device);
	}
	tcgetattr(fd, &portset);
#ifdef HAVE_CFMAKERAW
	/* The easy way: */
	cfmakeraw( &portset );
#else
	/* The hard way: */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, speed);
	tcsetattr(fd, TCSANOW, &portset);
	tcflush(fd, TCIOFLUSH);

	// Reset and clear the LCDM001
	write (fd, "~C", 2);
	//Set cursorblink default
	lcdm001_cursorblink (drvthis, DEFAULT_CURSORBLINK);
	// Turn all LEDs off
	snprintf (out, sizeof(out), "\%cL%c%c", 126, 0, 0);
	write (fd, out, 4);

	return 0;
}

/* Below here, you may use either lcd.framebuf or driver->framebuf..
 * lcd.framebuf will be set to the appropriate buffer before calling
 * your driver.
 */

MODULE_EXPORT void
lcdm001_close (Driver *drvthis)
{
	char out[5];
	if(framebuf) free (framebuf);
	framebuf = NULL;

	//switch off all LEDs
	snprintf (out, sizeof(out), "\%cL%c%c", 126, 0, 0);
	write (fd, out, 4);
	close (fd);

        report (RPT_INFO, "LCDM001: closed");
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
lcdm001_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
lcdm001_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
lcdm001_clear (Driver *drvthis)
{
        if (framebuf != NULL)
                memset (framebuf, ' ', (width * height));
	/* An instant clear is NOT neccessary, it only makes the display
	 *  flicker
	 */
	/*write (fd, "~C", 2);*/ /* instant clear...*/
        clear = 1;

	debug (RPT_DEBUG, "LCDM001: cleared screen");
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
lcdm001_flush (Driver *drvthis)
{
	// Next 4 lines are moved from draw_frame (Joris)

        //TODO: Check whether this is still correct

	write(fd,framebuf,80);

        debug (RPT_DEBUG, "LCDM001: frame buffer flushed");
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
lcdm001_chr (Driver *drvthis, int x, int y, char c)
{
	int offset;

	ValidX(x);
	ValidY(y);

        if (c==0) {
                c = icon_char;   //heartbeat workaround
        }

	// write to frame buffer
	y--; x--; // translate to 0-coords

	offset = (y * width) + x;
	framebuf[offset] = c;

	debug (RPT_DEBUG, "LCDM001: writing character %02X to position (%d,%d)", c, x, y);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
lcdm001_string (Driver *drvthis, int x, int y, char *string)
{
	int offset, siz;

	ValidX(x);
	ValidY(y);

	x--; y--; // Convert 1-based coords to 0-based...
	offset = (y * width) + x;
	siz = (width * height) - offset - 1;
	siz = siz > strlen(string) ? strlen(string) : siz;

	memcpy(framebuf + offset, string, siz);

	debug (RPT_DEBUG, "LCDM001: printed string at (%d,%d)", x, y);
}

/////////////////////////////////////////////////////////////////
// Controls LEDs
MODULE_EXPORT void
lcdm001_output (Driver *drvthis, int state)
{
	char out[5];
	int one = 0, two = 0;

	if (state<=255)
	{
		one=state;
		two=0;
	}
	else
	{
		one = state & 0xff;
		two = (state >> 8) & 0xff;
	}
        snprintf (out, sizeof(out), "~L%c%c",one,two);
        write(fd,out,4);

        debug (RPT_DEBUG, "LCDM001: current LED state: %d", state);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
lcdm001_old_vbar(Driver *drvthis, int x, int len)
{
   int y = 4;

   debug (RPT_DEBUG , "LCDM001: vertical bar at %d set to %d", x, len);

   while (len >= LCD_DEFAULT_CELLHEIGHT)
     {
       lcdm001_chr(drvthis, x, y, 0xFF);
       len -= LCD_DEFAULT_CELLHEIGHT;
       y--;
     }

   if(!len)
     return;

  //TODO: Distinguish between len>=4 and len<4

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
lcdm001_old_hbar(Driver *drvthis, int x, int y, int len)
{

  ValidX(x);
  ValidY(y);

  debug (RPT_DEBUG, "LCDM001: horizontal bar at %d set to %d", x, len);

  //TODO: Improve this function

  while((x <= width) && (len > 0))
  {
    if(len < LCD_DEFAULT_CELLWIDTH)
      {
	//lcdm001_chr(x, y, 0x98 + len);
	break;
      }

    lcdm001_chr(drvthis, x, y, 0xFF);
    len -= LCD_DEFAULT_CELLWIDTH;
    x++;
  }

  return;
}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT void
lcdm001_old_icon (Driver *drvthis, int which, char dest)
{

/*Heartbeat workaround:
   As custom chars are not supported OPEN_HEART
   and FILLED_HEART are displayed instead.
   Change them in lcdm001.h*/

	if (dest == 0)
		switch (which) {
			case 0:
				icon_char = OPEN_HEART;
				break;
			case 1:
				icon_char = FILLED_HEART;
				break;
			default:
				icon_char = PAD;
				break;
		}
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return NULL for "nothing available".
//

MODULE_EXPORT const char *
lcdm001_get_key (Driver *drvthis)
{
        char in = '\0';
	const char *key = NULL;

        read (fd, &in, 1);
	if (in == pause_key) {
		key = "Escape";
	} else if (in == back_key) {
		key = "Left";
	} else if (in == forward_key) {
		key = "Right";
	} else if (in == main_menu_key) {
		key = "Enter";
	}
	debug(RPT_DEBUG, "%s, get_key: %s",
			drvthis->name, (key != NULL) ? key : "<null>");
        return key;
}

