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
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *                2001 Mark Haemmerling <mail@markh.de>
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
static int connIdx = 0;

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

// Keypad option
char keypad = 0;	 // off by default
			 // This var is not static, so it's accessable from all sub-drivers
			 // Indeed, this should become cleaner...  later...

// Maximum sizes of the keypad
// DO NOT CHANGE THESE 2 VALUES, unless you change the functions too
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 11
// Autorepeat values
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15

// keyMapDirect contains an array of the ascii-codes that should be generated
// when a directly connected key is pressed (not in matrix).
static char keyMapDirect[KEYPAD_MAXX] = { 'A', 'B', 'C', 'D', 'E' };

// keyMapMatrix contrains an array with arrays of the ascii-codes that should be generated
// when a key in the matrix is pressed.
static char keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
	{ '1', '2', '3', 'A', 0 },
	{ '4', '5', '6', 'B', 0 },
	{ '7', '8', '9', 'C', 0 },
	{ '*', '0', '#', 'D', 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 }};

static char pressed_key = 0;
static int pressed_key_repetitions = 0;
static struct timeval pressed_key_time;


// function declarations
void HD44780_flush ();
void HD44780_flush_box (int lft, int top, int rgt, int bot);
void HD44780_backlight (int on);
void HD44780_init_vbar ();
void HD44780_init_hbar ();
void HD44780_vbar (int x, int len);
void HD44780_hbar (int x, int y, int len);
void HD44780_init_num ();
void HD44780_num (int x, int num);
void HD44780_set_char (int n, char *dat);
void HD44780_icon (int which, char dest);
void HD44780_draw_frame (char *dat);
void HD44780_close ();

static void HD44780_linewrap (int on);
static void HD44780_autoscroll (int on);
static void HD44780_position (int x, int y);
static void HD44780_chr (int x, int y, char ch);

static void common_position (int display, int DDaddr);
static void common_autoscroll (int on);
static void uPause (int delayCalls);
static void HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);
static char HD44780_getkey ();

static int parse_span_list (int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], char *spanlist);

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
HD44780_init (lcd_logical_driver * driver, char *args)
{
	// TODO: remove the two magic numbers below
	// TODO: single point of return
	char *argv[64], *str;
	int argc, i;

	HD44780 = driver;

	// get_args inserts \0 into args - nasty !
	if (args)
		if ((str = (char *) malloc (strlen (args) + 1)))
			strcpy (str, args);
		else {
			fprintf (stderr, "Error mallocing\n");
			return -1;
	} else
		str = NULL;

	argc = get_args (argv, args, 64);

	// arguments are passed in with -d HD44780 "-p 0x278"
	// getopt will work well here
	for (i = 0; i < argc; i++) {
		if (0 == strcmp (argv[i], "-p") || 0 == strcmp (argv[i], "--port")) {
			if (i + 1 >= argc) {
				fprintf (stderr, "HD44780_init: %s requires an argument\n", argv[i]);
				return -1;
			} else {
				int myport;
				if (sscanf (argv[i + 1], "%i", &myport) != 1) {
					fprintf (stderr, "HD44780_init: Couldn't read port address -" " using default value 0x%x\n", port);
					return -1;
				} else {
					port = myport;
					++i;
				}
			}
		} else if (0 == strcmp (argv[i], "-c") || 0 == strcmp (argv[i], "--connect")) {
			int j = 0;
			if (i + 1 >= argc) {
				fprintf (stderr, "HD44780_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			// search the next argument for the connection type
			for (j = 0; strcmp (argv[i + 1], connectionMapping[j].connectionTypeStr) != 0 && connectionMapping[j].type != HD_unknown; ++j);

			if (connectionMapping[j].type == HD_unknown) {
				fprintf (stderr, "HD44780_init: Unknown connection type - using default %s driver\n", connectionMapping[0].connectionTypeStr);
				connection = connectionMapping[0].type;
				j = 0;
			} else
				connection = connectionMapping[j].type;

			connIdx = j;
			++i;
		} else if (0 == strcmp (argv[i], "-v") || 0 == strcmp (argv[i], "--vspan")) {
			if (i + 1 >= argc) {
				fprintf (stderr, "HD44780_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			if (parse_span_list (&spanList, &numLines, &dispVOffset, &numDisplays, &dispSizes, argv[i + 1]) == -1) {
				fprintf (stderr, "HD44780_init: invalid vspan argument\n");
				return -1;
			}
			++i;
		} else if (strcmp (argv[i], "-k") == 0 || strcmp (argv[i], "--keypad") == 0) {
			keypad = 1;
			printf( "HD44780 keypad extension enabled\n" );

		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			int i;
			printf ("LCDproc HD44780 driver\n" "\t-p n\t--port n\tSelect the output device to use port n\n" "\t-c type\t--connect type\tSet the connection type (default 4 bit)\n" "\t\t\t\ttype = %s", connectionMapping[0].connectionTypeStr);
			for (i = 1; connectionMapping[i].type != HD_unknown; ++i)
				printf (" | %s", connectionMapping[i].connectionTypeStr);
			printf ("\n\t-v spanlist\t--vspan spanlist\tvertical span n{,m}\n" "\t-h\t--help\t\tShow this help information\n" "\n\tNote that the '-t' option should precede the '-d' option\n");

			if (connectionMapping[connIdx].type != HD_unknown) {
				printf ("\n    Parameters for %s driver\n", connectionMapping[connIdx].connectionTypeStr);
				printf ("%s", connectionMapping[connIdx].helpMsg);
			}
			return -1;
		}
		/* ignore invalid arguments as they might be valid to the lower-level init
		   else
		   {
		   printf("Invalid parameter: %s\n", argv[i]);
		   }
		 */
	}

	// default case for when spans aren't indicated
	// - add a sanity check against HD44780->hgt ??
	// - this only works if the -t option is specified before -d
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
	driver->clear = (void *) -1;
	driver->string = (void *) -1;
	driver->chr = (void *) -1;
	driver->vbar = HD44780_vbar;
	driver->init_vbar = HD44780_init_vbar;
	driver->hbar = HD44780_hbar;
	driver->init_hbar = HD44780_init_hbar;
	driver->num = HD44780_num;
	driver->init_num = HD44780_init_num;

	driver->init = HD44780_init;
	driver->close = (void *) -1;
	driver->flush = HD44780_flush;
	driver->flush_box = HD44780_flush_box;
	//driver->contrast = HD44780_contrast; // contrast by potmeter
	driver->backlight = HD44780_backlight;
	driver->set_char = HD44780_set_char;
	driver->icon = HD44780_icon;
	driver->draw_frame = HD44780_draw_frame;

	if( keypad ) {
		driver->getkey = HD44780_getkey;
	}

	if ((hd44780_functions = (HD44780_functions *) malloc (sizeof (HD44780_functions))) == NULL) {
		return -1;
	}
	hd44780_functions->uPause = uPause;
	hd44780_functions->position = common_position;
	hd44780_functions->autoscroll = common_autoscroll;

	connectionMapping[connIdx].init_fn (hd44780_functions, driver, str, port);

	HD44780_autoscroll (0);

	return port;
}

// Common initialisation sequence - sets cursor off and not blinking,
// clear display and homecursor
// Does not set twoline mode nor small characters (5x8). The init function of
// the connectiontype should do this.

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

// IO delay to avoid a task switch
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

// displayID     - ID of display to use (0 = all displays)
static void
HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch)
{
	hd44780_functions->senddata (displayID, flags, ch);
	hd44780_functions->uPause (40);  // Minimum exec time for all commands
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
/*
  void HD44780_close()
  {
  drv_base_close();
  }
*/

// x and y here are for the virtual HD44780->hgt x HD44780->wid display
static void
HD44780_position (int x, int y)
{
	int dispID = spanList[y];
	int relY = y - dispVOffset[dispID - 1];
	int DDaddr;

	// 16x1 is a special case
	if (dispSizes[dispID - 1] == 1 && HD44780->wid == 16)
		if (x >= 8) {
			x -= 8;
			relY = 1;
		}

	DDaddr = x + (relY % 2) * 0x40;
	if ((relY % 4) >= 2)
		DDaddr += HD44780->wid;

	hd44780_functions->position (dispID, DDaddr);
}

void
common_position (int display, int DDaddr)
{
	hd44780_functions->senddata (display, RS_INSTR, POSITION | DDaddr);
	hd44780_functions->uPause (40);  // Minimum exec time for all commands
}

void HD44780_chr (int x, int y, char ch)
{
	HD44780->framebuf[ (y * HD44780->wid) + x] = ch;
}

void
HD44780_flush ()
{
	HD44780_draw_frame (HD44780->framebuf);
}

void
HD44780_flush_box (int lft, int top, int rgt, int bot)
{
	int x, y;

	//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

	for (y = top; y <= bot; y++) {
		HD44780_position (lft, y);
		//printf("\n%d,%d :",lft,y);
		for (x = lft; x <= rgt; x++) {
			HD44780_senddata (spanList[y], RS_DATA, HD44780->framebuf[(y * HD44780->wid) + x]);
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
	//hd44780_functions->backlight (on);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
HD44780_linewrap (int on)
{
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
HD44780_autoscroll (int on)
{
	hd44780_functions->autoscroll (on);
}

void
common_autoscroll (int on)
{
	hd44780_functions->senddata (0, RS_INSTR, ENTRYMODE | E_MOVERIGHT | ((on) ? EDGESCROLL : NOSCROLL));
	hd44780_functions->uPause (40);
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

	HD44780_senddata (0, RS_INSTR, SETCHAR | n * 8);

	for (row = 0; row < HD44780->cellhgt; row++) {
		letter = 0;
		for (col = 0; col < HD44780->cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * HD44780->cellwid) + col] > 0);
		}
		HD44780_senddata (0, RS_DATA, letter);
	}
}

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
void
HD44780_draw_frame (char *dat)
{
	int x, y;

	if (!dat)
		return;

	for (y = 0; y < HD44780->hgt; y++) {
		HD44780_position (0, y);
		//printf("\n%d :",y);
		for (x = 0; x < HD44780->wid; x++) {
			HD44780_senddata (spanList[y], RS_DATA, HD44780_charmap[(unsigned char)dat[(y * HD44780->wid) + x]]);
		}
	}

}

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

char HD44780_getkey()
{
	unsigned int keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned int Ypattern;
	unsigned int Yval;
	char exp;
	char ch = 0;
	char ch_set = 0;
	struct timeval curr_time, time_diff;

	gettimeofday(&curr_time,NULL);

	// First check if a directly connected key is pressed
	// Put all zeros on Y of keypad
	keybits = hd44780_functions->readkeypad (0);

	if (keybits) {
		// A directly connected key was pressed
		// Which key was it ?
		shiftingbit = 1;
		for (shiftcount=0; shiftcount<KEYPAD_MAXX && !ch; shiftcount++) {
			if ( keybits & shiftingbit) {
				// Found !   Return from function.
				ch = keyMapDirect[shiftcount];
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
			// Do a 'binary search' to determine in what row a key is pressed
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
			for (shiftcount=0; shiftcount<KEYPAD_MAXX && !ch; shiftcount++) {
				if ( keybits & shiftingbit) {
					// Found !
					ch = keyMapMatrix[Yval][shiftcount];
				}
				shiftingbit <<= 1;
			}
		}
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
			printf( "Key: %c\n", ch );
		}
	}

	// Store the key for the next round
	pressed_key = ch;

	return ch;
}
