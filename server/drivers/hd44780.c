/*
 * Base driver module for Hitachi HD44780 based LCD displays. This is
 * a modular driver that readily allows support for alternative HD44780
 * designs to be added in a flexible and maintainable manner.
 *
 * This driver also supports the aggregation of multiple displays to form
 * a single virtual display. e.g., Four 16x2 displays can be combined
 * to form a 16x8 display.
 *
 * To add support for additional HD44780 connections:
 *  1. Add a connection type and mapping to hd44780-drivers.h
 *  2. Call your initialisation routine
 *  3. Create the low-level driver (use hd44780-ext8bit.c as a starting point)
 *  4. Modify the makefile
 *
 * Modular driver created and generic support for multiple displays added
 * Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Modified July 2000 by Charles Steinkuehler to use one of 3 methods for delay
 * timing.  I/O reads, gettimeofday, and nanosleep.  Of the three, nanosleep
 * seems to work best, so that's what is set by default.
 *
 * Modified May 2001 by Joris Robijn to add Keypad support.
 *
 * Character mapping for correct display of some special ASCII chars added
 * Sep 2001, Mark Haemmerling <mail@markh.de>.
 *
 * Modified October 2001 to read the configfile.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *		  2001 Mark Haemmerling <mail@markh.de>
 *		  2000 Charles Steinkuehler <cstein@newtek.com>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "port.h"

// Uncomment one of the lines below to select your desired delay generation
// mechanism.  If both defines are commented, the original I/O read timing
// loop is used.  Using DELAY_NANOSLEEP  seems to provide the best performance.
//#define DELAY_GETTIMEOFDAY
#define DELAY_NANOSLEEP

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

// Only one alternate delay method at a time, please ;-)
#if defined DELAY_GETTIMEOFDAY
# undef DELAY_NANOSLEEP
#elif defined DELAY_NANOSLEEP
# include <sched.h>
# include <time.h>
#endif

#include "shared/str.h"
#include "render.h"
#include "lcd.h"
#include "hd44780.h"
// #include "drv_base.h"

#include "hd44780-low.h"
#include "hd44780-drivers.h"
#include "hd44780-charmap.h"

// default parallel port address
#ifndef LPTPORT
#define LPTPORT	 0x378
#endif

unsigned int port = LPTPORT;

lcd_logical_driver *HD44780;
HD44780_functions *hd44780_functions;

// default value
static enum connectionType connection = HD_4bit;
static int connectiontype_index = 0;

// spanList[line number] = display line number is in
static int *spanList = NULL;
static int numLines = 0;

// dispVOffset is a cumulative sized array of line numbers for each display.
// use this to determine the vertical positioning on a given display
static int *dispVOffset = NULL;
static int numDisplays = 0;

// dispSizes is the vertical size of each display. This is the same as the
// input span list but is kept to save some cpu cycles.
static int *dispSizes = NULL;

// Keypad and backlight option
char have_keypad = 0;	 // off by default
char have_backlight = 0; // off by default
char extIF = 0;		 // off by default
			 // These vars is not static, so it's accessable from all sub-drivers
			 // Indeed, this should become cleaner...  later...

// Autorepeat values
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15

// keyMapDirect contains an array of the ascii-codes that should be generated
// when a directly connected key is pressed (not in matrix).
static char keyMapDirect[KEYPAD_MAXX] = { 'A', 'B', 'C', 'D', 'E' };

// keyMapMatrix contrains an array with arrays of the ascii-codes that should be generated
// when a key in the matrix is pressed.
static char keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
	{ '1', '2', '3', 'A', 'E' },
	{ '4', '5', '6', 'B', 'F' },
	{ '7', '8', '9', 'C', 'G' },
	{ '*', '0', '#', 'D', 'H' },
	{ 'I', 'J', 'K', 'L', 'M' },
	{ 'N', 'O', 'P', 'Q', 'R' },
	{ 'S', 'T', 'U', 'V', 'W' },
	{ 'X', 'Y', 'Z', 'a', 'b' },
	{ 'c', 'd', 'e', 'f', 'g' },
	{ 'h', 'i', 'j', 'k', 'l' },
	{ 'm', 'n', 'o', 'p', 'q' }};

static char pressed_key = 0;
static int pressed_key_repetitions = 0;
static struct timeval pressed_key_time;

// function declarations
void HD44780_close ();
void HD44780_flush ();
void HD44780_clear ();
void HD44780_chr (int x, int y, char ch);
void HD44780_string (int x, int y, char *s);
void HD44780_flush_box (int lft, int top, int rgt, int bot);
void HD44780_backlight (int on);
void HD44780_init_vbar ();
void HD44780_init_hbar ();
void HD44780_vbar (int x, int len);
void HD44780_hbar (int x, int y, int len);
void HD44780_init_num ();
void HD44780_num (int x, int num);
void HD44780_heartbeat (int type);
void HD44780_set_char (int n, char *dat);
void HD44780_icon (int which, char dest);
void HD44780_draw_frame (char *dat);
char HD44780_getkey ();

void HD44780_position (int x, int y);
static void uPause (int delayCalls);
unsigned char HD44780_scankeypad();
static int parse_span_list (int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], char *spanlist);

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
HD44780_init (lcd_logical_driver * driver, char *args)
{
	// TODO: remove the two magic numbers below
	// TODO: single point of return
	//char *argv[64];
	//int i;
	char buf[40];
	char *s;

	HD44780 = driver;

	// TODO: replace DriverName with driver->name when that field exists.
	#define DriverName "HD44780"


	//// READ THE CONFIG FILE

	port		= driver->config_get_int( DriverName, "port", 0, 0x278 );

	// Get and search for the connection type
	s = driver->config_get_string( DriverName, "connectiontype", 0, "4bit" );
	for (connectiontype_index = 0; strcmp (s, connectionMapping[connectiontype_index].connectionTypeStr) != 0 && connectionMapping[connectiontype_index].type != HD_unknown; ++connectiontype_index);

	if (connectionMapping[connectiontype_index].type == HD_unknown) {
		fprintf (stderr, "HD44780_init: Unknown connection type: %s\n", s);
		return -1; // fatal error
	} else
		connection = connectionMapping[connectiontype_index].type;

	// Get and parse vspan only when specified
	s = driver->config_get_string( DriverName, "vspan", 0, "" );
	if( s[0] != 0 ) {
		if (parse_span_list (&spanList, &numLines, &dispVOffset, &numDisplays, &dispSizes, s) == -1) {
			fprintf (stderr, "HD44780_init: invalid vspan value: %s\n", s );
			return -1;
		}
	}

	extIF		= driver->config_get_bool( DriverName, "extended", 0, 0 );
	have_keypad	= driver->config_get_bool( DriverName, "keypad", 0, 0 );
	have_backlight	= driver->config_get_bool( DriverName, "backlight", 0, 0 );

	// Get and parse size
	s = driver->config_get_string( DriverName, "size", 0, "20x4" );
	if( sscanf( s, "%dx%d", &(driver->wid), &(driver->hgt) ) != 2 ) {
		fprintf( stderr, "HD44780_init: Cannot read size: %s\n", s );
	}

	// default case for when spans aren't indicated
	// - add a sanity check against HD44780->hgt ??
	if (numLines == 0) {
		if ((spanList = (int *) malloc (sizeof (int) * HD44780->hgt))) {
			int i;
			for (i = 0; i < HD44780->hgt; ++i) {
				spanList[i] = 1;
				numLines = HD44780->hgt;
			}
		} else
			fprintf (stderr, "Error mallocing for span list\n");
	}
	if (numDisplays == 0) {
		if ((dispVOffset = (int *) malloc (sizeof (int))) && (dispSizes = (int *) malloc (sizeof (int)))) {
			dispVOffset[0] = 0;
			dispSizes[0] = HD44780->hgt;
			numDisplays = 1;
		} else
			fprintf (stderr, "Error mallocing for display sizes list\n");
	}


#if defined DELAY_NANOSLEEP
	// Change to Round-Robin scheduling for nanosleep
	{
		// Set priority to 1
		struct sched_param param;
		param.sched_priority=1;
		if (( sched_setscheduler(0, SCHED_RR, &param)) == -1) {
			fprintf (stderr, "HD44780_init: failed (%s)\n", strerror (errno));
			return -1;
		}
	}
#endif

	// Make sure the frame buffer is there...
	if (!HD44780->framebuf)
		HD44780->framebuf = (unsigned char *) malloc (HD44780->wid * HD44780->hgt);

	if (!HD44780->framebuf) {
		//HD44780_close();
		return -1;
	}
	// Set the functions the driver supports...
	// These are the default HD44780 functions
	driver->init = HD44780_init;
	driver->close = HD44780_close;
	driver->flush = HD44780_flush;
	driver->clear = HD44780_clear;
	driver->chr = HD44780_chr;
	driver->string = HD44780_string;
	driver->flush_box = HD44780_flush_box; // TO BE REMOVED
	//driver->contrast = HD44780_contrast; // contrast is set by potmeter we assume

	//driver->output = HD44780_output; // not implemented
	driver->init_vbar = HD44780_init_vbar;
	driver->init_hbar = HD44780_init_hbar;
	driver->vbar = HD44780_vbar;
	driver->hbar = HD44780_hbar;
	driver->init_num = HD44780_init_num;
	driver->num = HD44780_num;
	driver->heartbeat = HD44780_heartbeat;
	driver->set_char = HD44780_set_char;
	driver->icon = HD44780_icon;
	driver->draw_frame = HD44780_draw_frame; // TO BE REMOVED

	if ( have_backlight ) {
		driver->backlight = HD44780_backlight;
	}
	if ( have_keypad ) {
		driver->getkey = HD44780_getkey;
	}

	if ((hd44780_functions = (HD44780_functions *) malloc (sizeof (HD44780_functions))) == NULL) {
		return -1;
	}
	hd44780_functions->uPause = uPause;
	hd44780_functions->scankeypad = HD44780_scankeypad;

	connectionMapping[connectiontype_index].init_fn (hd44780_functions, driver, args, port);

	HD44780_clear ();
	sprintf (buf, "HD44780 %dx%d", HD44780->wid, HD44780->hgt );
	HD44780_string (1, 1, buf);
	sprintf (buf, "LPT 0x%x %s %s", port, (have_backlight?"bl":""), (have_keypad?"key":"") );
	HD44780_string (1, 2, buf);
	HD44780_flush ();
	sleep (1);

	return port;
}

/////////////////////////////////////////////////////////////////
// Common initialisation sequence - sets cursor off and not blinking,
// clear display and homecursor
// Does not set twoline mode nor small characters (5x8). The init function of
// the connectiontype should do this.
//
void
common_init ()
{
	hd44780_functions->senddata (0, RS_INSTR, ONOFFCTRL | DISPON | CURSOROFF | CURSORNOBLINK);
	hd44780_functions->uPause (40);
	hd44780_functions->senddata (0, RS_INSTR, CLEAR);
	hd44780_functions->uPause (1600);
	hd44780_functions->senddata (0, RS_INSTR, HOMECURSOR);
	hd44780_functions->uPause (1600);
}

/////////////////////////////////////////////////////////////////
// IO delay to avoid a task switch
//
void
uPause (int delayCalls)
{
#if defined DELAY_GETTIMEOFDAY
	struct timeval current_time,delay_time,wait_time;

	// Get current time first thing
	gettimeofday(&current_time,NULL);

	// Calculate when delay is over
	delay_time.tv_sec  = 0;
	delay_time.tv_usec = delayCalls;
	timeradd(&current_time,&delay_time,&wait_time);

	do {
		gettimeofday(&current_time,NULL);
	} while (timercmp(&current_time,&wait_time,<));

#elif defined DELAY_NANOSLEEP
	struct timespec delay,remaining;

	delay.tv_sec = 0;
	delay.tv_nsec = delayCalls * 1000;
	while ( nanosleep(&delay,&remaining) == -1 )
	{
		delay.tv_sec  = remaining.tv_sec;
		delay.tv_nsec = remaining.tv_nsec;
	}
#else // using I/O timing
	int i;
	for (i = 0; i < delayCalls; ++i)
		port_in (port);
#endif
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
void HD44780_close()
{
}

/////////////////////////////////////////////////////////////////
// Set position (not part of API)
//
// x and y here are for the virtual HD44780->hgt x HD44780->wid display
void
HD44780_position (int x, int y)
{
	int dispID = spanList[y];
	int relY = y - dispVOffset[dispID - 1];
	int DDaddr;

	// 16x1 is a special case
	if (dispSizes[dispID - 1] == 1 && HD44780->wid == 16) {
		if (x >= 8) {
			x -= 8;
			relY = 1;
		}
	}

	DDaddr = x + (relY % 2) * 0x40;
	if ((relY % 4) >= 2)
		DDaddr += HD44780->wid;

	hd44780_functions->senddata (dispID, RS_INSTR, POSITION | DDaddr);
	hd44780_functions->uPause (40);  // Minimum exec time for all commands
}

/////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
void
HD44780_flush ()
{
	HD44780_draw_frame (HD44780->framebuf);
}

/////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
void HD44780_clear ()
{
	memset(HD44780->framebuf, ' ', HD44780->wid * HD44780->hgt);
}

/////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
void
HD44780_chr (int x, int y, char ch)
{
	y--;
	x--;

	HD44780->framebuf[ (y * HD44780->wid) + x] = ch;
}

/////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
void
HD44780_string (int x, int y, char *s)
{
	int i;

	x --;  // Convert 1-based coords to 0-based
	y --;

	for (i = 0; s[i]; i++) {
		// Check for buffer overflows...
		if ((y * HD44780->wid) + x + i > (HD44780->wid * HD44780->hgt))
			break;
		HD44780->framebuf[(y*HD44780->wid) + x + i] = s[i];
	}
}

/////////////////////////////////////////////////////////////////
// Flush box
//
// TO BE REMOVED
//
void
HD44780_flush_box (int lft, int top, int rgt, int bot)
{
	int x, y;

	//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

	for (y = top; y <= bot; y++) {
		HD44780_position (lft, y);
		//printf("\n%d,%d :",lft,y);
		for (x = lft; x <= rgt; x++) {

			hd44780_functions->senddata (spanList[y], RS_DATA, HD44780->framebuf[(y * HD44780->wid) + x]);
			hd44780_functions->uPause (40);  // Minimum exec time for all commands
		}
		//write(fd, HD44780->framebuf[(y*HD44780->wid)+lft, rgt-lft+1]);
	}

}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
void
HD44780_backlight (int on)
{
	hd44780_functions->backlight (on);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before HD44780->vbar()
//
void
HD44780_init_vbar ()
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

	HD44780_set_char (1, a);
	HD44780_set_char (2, b);
	HD44780_set_char (3, c);
	HD44780_set_char (4, d);
	HD44780_set_char (5, e);
	HD44780_set_char (6, f);
	HD44780_set_char (7, g);
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
HD44780_init_hbar ()
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

	HD44780_set_char (1, a);
	HD44780_set_char (2, b);
	HD44780_set_char (3, c);
	HD44780_set_char (4, d);

}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
void
HD44780_vbar (int x, int len)
{
	char map[9] = { 32, 1, 2, 3, 4, 5, 6, 7, 255 };

	int y;
	for (y = HD44780->hgt; y > 0 && len > 0; y--) {
		if (len >= HD44780->cellhgt)
			HD44780_chr (x, y, 255);
		else
			HD44780_chr (x, y, map[len]);

		len -= HD44780->cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
HD44780_hbar (int x, int y, int len)
{
	char map[6] = { 32, 1, 2, 3, 4, 255 };

	for (; x <= HD44780->wid && len > 0; x++) {
		if (len >= HD44780->cellwid)
			HD44780_chr (x, y, 255);
		else
			HD44780_chr (x, y, map[len]);

		len -= HD44780->cellwid;

	}
}

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void
HD44780_init_num ()
{

}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
HD44780_num (int x, int num)
{
	int offset = HD44780->wid * ( HD44780->hgt / 2 );
	HD44780->framebuf[x+offset] = num + '0';
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
void
HD44780_heartbeat (int type)
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
		HD44780_icon (whichIcon, 0);

		// Put character on screen...
		HD44780_chr (HD44780->wid, 1, 0);

		// change display...
		HD44780_flush ();
	}

	timer++;
	timer &= 0x0f;
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void
HD44780_set_char (int n, char *dat)
{
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	hd44780_functions->senddata (0, RS_INSTR, SETCHAR | n * 8);
	hd44780_functions->uPause (40);  // Minimum exec time for all commands

	for (row = 0; row < HD44780->cellhgt; row++) {
		letter = 0;
		for (col = 0; col < HD44780->cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * HD44780->cellwid) + col] > 0);
		}
		hd44780_functions->senddata (0, RS_DATA, letter);
		hd44780_functions->uPause (40);  // Minimum exec time for all commands
	}
}

/////////////////////////////////////////////////////////////
// Set default icon into a userdef char
//
void
HD44780_icon (int which, char dest)
{
	char icons[3][5 * 8] = {
		{
		 1, 1, 1, 1, 1,		   // Empty Heart
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1,		   // Filled Heart
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0,		   // Ellipsis
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1,
		 },

	};

	HD44780_set_char (dest, &icons[which][0]);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized HD44780->wid*hgt
//
// TO BE REMOVED (move it into flush())
void
HD44780_draw_frame (char *dat)
{
	int x, y;

	if (!dat)
		return;

	for (y = 0; y < HD44780->hgt; y++) {
		//printf("\n%d :",y);
		for (x = 0; x < HD44780->wid; x++) {

			if( x % 8 == 0 )
				HD44780_position (x, y);
			hd44780_functions->senddata (spanList[y], RS_DATA, HD44780_charmap[(unsigned char)dat[(y * HD44780->wid) + x]]);
			hd44780_functions->uPause (40);  // Minimum exec time for all commands
		}
	}

}

/////////////////////////////////////////////////////////////
// Get a key from the keypad (if there is one)
//
char HD44780_getkey()
{
	unsigned char scancode;
	char ch;
	struct timeval curr_time, time_diff;

	gettimeofday(&curr_time,NULL);

	scancode = hd44780_functions->scankeypad();
	if( scancode ) {
		if( scancode & 0xF0 ) {
			ch = keyMapMatrix[((scancode&0xF0)>>4)-1][(scancode&0x0F)-1];
		}
		else {
			ch = keyMapDirect[scancode - 1];
		}
	}
	else {
		ch = 0;
	}

	if (ch) {
		if (ch == pressed_key) {
			timersub (&curr_time, &pressed_key_time, &time_diff);
			if (((time_diff.tv_usec / 1000 + time_diff.tv_sec * 1000) - KEYPAD_AUTOREPEAT_DELAY) < 1000 * pressed_key_repetitions / KEYPAD_AUTOREPEAT_FREQ ) {
				// The key is already pressed quite some time
				// but it's not yet time to return a repeated keypress
				return 0;
			}
			// Otherwise a keypress will be returned
			pressed_key_repetitions ++;
		}
		else {
			// It's a new keypress
			pressed_key_time = curr_time;
			pressed_key_repetitions = 0;
			printf( "Key: %c  (%d,%d)\n", ch, scancode&0x0F, (scancode&0xF0)>>4 );
		}
	}

	// Store the key for the next round
	pressed_key = ch;

	return ch;
}

/////////////////////////////////////////////////////////////
// Scan the keypad
//
unsigned char HD44780_scankeypad()
{
	unsigned int keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned int Ypattern;
	unsigned int Yval;
	char exp;

	unsigned char scancode = 0;

	// First check if a directly connected key is pressed
	// Put all zeros on Y of keypad
	keybits = hd44780_functions->readkeypad (0);

	if (keybits) {
		// A directly connected key was pressed
		// Which key was it ?
		shiftingbit = 1;
		for (shiftcount=0; shiftcount<KEYPAD_MAXX && !scancode; shiftcount++) {
			if ( keybits & shiftingbit) {
				// Found !   Return from function.
				scancode = shiftcount+1;
			}
			shiftingbit <<= 1;
		}
	}
	else {
		// Now check the matrix
		// First check with all 1's
		Ypattern = (1 << KEYPAD_MAXY) - 1;
		if( hd44780_functions->readkeypad (Ypattern)) {
			// Yes, a key on the matrix is pressed

			// OK, now we know a key is pressed.
			// Determine which one it is

			// First determine the row
			// Do a 'binary search' to minimize I/O
			Ypattern = 0;
			Yval = 0;
			for (exp=3; exp>=0; exp--) {
				Ypattern = ((1 << (1 << exp)) - 1) << Yval;
				keybits = hd44780_functions->readkeypad (Ypattern);
				if (!keybits) {
					Yval += (1 << exp);
				}
			}

			// Which key is pressed in that row ?
			keybits = hd44780_functions->readkeypad (1<<Yval);
			shiftingbit=1;
			for (shiftcount=0; shiftcount<KEYPAD_MAXX && !scancode; shiftcount++) {
				if ( keybits & shiftingbit) {
					// Found !
					scancode = (Yval+1) << 4 | (shiftcount+1);
				}
				shiftingbit <<= 1;
			}
		}
	}
	return scancode;
}


/////////////////////////////////////////////////////////////
// Parse the span list
//      spanListArray   - array to store vertical spans
//      spLsize	 - size of spanListArray
//      dispOffsets     - array to store display offsets
//      dOffsize	- size of dispOffsets
//      dispSizeArray   - array of display vertical sizes (= spanlist)
//      spanlist	- null terminated input span list in comma delimited
//			format. All span elements [1-9] e.g. "1,4,2"
//      returns number of span elements, -1 on parse error

int
parse_span_list (int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], char *spanlist)
{
	int j = 0, retVal = 0;

	*spLsize = 0;
	*dOffsize = 0;

	while (j < strlen (spanlist)) {
		if (spanlist[j] >= '1' && spanlist[j] <= '9') {
			int spansize = spanlist[j] - '0';

			// add spansize lines to the span list, note the offset to
			// the previous display and the size of the display
			if ((*spanListArray = (int *) realloc (*spanListArray, sizeof (int) * (*spLsize + spansize)))
				 && (*dispOffsets = (int *) realloc (*dispOffsets, sizeof (int) * (*dOffsize + 1)))
				 && (*dispSizeArray = (int *) realloc (*dispSizeArray, sizeof (int) * (*dOffsize + 1)))) {
				int k;
				for (k = 0; k < spansize; ++k)
					(*spanListArray)[*spLsize + k] = *dOffsize + 1;

				(*dispOffsets)[*dOffsize] = *spLsize;
				(*dispSizeArray)[*dOffsize] = spansize;
				*spLsize += spansize;
				++(*dOffsize);
				retVal = *dOffsize;

				// find the next number (\0 is also outside this range)
				for (++j; spanlist[j] < '1' || spanlist[j] > '9'; ++j);
			} else {
				fprintf (stderr, "Error reallocing for span list\n");
				retVal = -1;
			}
		} else {
			fprintf (stderr, "Error reading spansize\n");
			retVal = -1;
		}
	}
	return retVal;
}

