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
 *  2. Call your initialisation roUine
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
 * Moved the delay timing code by Charles Steinkuehler to timing.h.
 * Guillaume Filion <gfk@logidac.com>, December 2001
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


// Uncomment one of the lines below to select your desired delay generation
// mechanism.  If both defines are commented, the original I/O read timing
// loop is used.  Using DELAY_NANOSLEEP  seems to provide the best performance.
//#define DELAY_GETTIMEOFDAY
#define DELAY_NANOSLEEP
//#define DELAY_IOCALLS

// Default parallel port address
#define LPTPORT	 0x378

// Autorepeat values
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "hd44780.h"
#include "report.h"

#include "timing.h"
#include "hd44780-low.h"
#include "hd44780-drivers.h"
#include "hd44780-charmap.h"

// Only one alternate delay method at a time, please ;-)
#if defined DELAY_GETTIMEOFDAY
# undef DELAY_NANOSLEEP
#elif defined DELAY_NANOSLEEP
# include <sched.h>
# include <time.h>
#endif


static char *defaultKeyMapDirect[KEYPAD_MAXX] = { "A", "B", "C", "D", "E" };

static char *defaultKeyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
		{ "1", "2", "3", "A", "E" },
		{ "4", "5", "6", "B", "F" },
		{ "7", "8", "9", "C", "G" },
		{ "*", "0", "#", "D", "H" },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL }};

// function declarations
void HD44780_position (Driver *drvthis, int x, int y);
static void uPause (PrivateData *p, int usecs);
unsigned char HD44780_scankeypad(PrivateData *p);
static int parse_span_list (int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], char *spanlist);

// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1; // yes, we have no global variables (except for constants)
MODULE_EXPORT char *symbol_prefix = "HD44780_";


/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
HD44780_init (Driver * drvthis, char *args)
{
	// TODO: remove the two magic numbers below
	// TODO: single point of return
	char buf[40];
	char *s;
	int i;
	int usb = 0;
	PrivateData *p;

	// Alocate and store private data
	p = (PrivateData *) malloc( sizeof( PrivateData) );
	if( ! p )
		return -1;
	if( drvthis->store_private_ptr( drvthis, p ) )
		return -1;

	// Clear data struct
	memset( p, 0, sizeof(*p) );
	p->cellheight = 8; /* Do not change this !!! This is a controller property, not a display property !!! */
	p->cellwidth = 5;
	p->ccmode = CCMODE_STANDARD;


	//// READ THE CONFIG FILE

	p->port			= drvthis->config_get_int( drvthis->name, "port", 0, LPTPORT );
	p->ext_mode		= drvthis->config_get_bool( drvthis->name, "extendedmode", 0, 0 );
	p->have_keypad		= drvthis->config_get_bool( drvthis->name, "keypad", 0, 0 );
	p->have_backlight	= drvthis->config_get_bool( drvthis->name, "backlight", 0, 0 );
	p->have_output		= drvthis->config_get_bool( drvthis->name, "outputport", 0, 0 );
	p->delayMult 		= drvthis->config_get_int( drvthis->name, "delaymult", 0, 1 );
	p->delayBus 		= drvthis->config_get_bool( drvthis->name, "delaybus", 0, 1 );
	p->lastline 		= drvthis->config_get_bool( drvthis->name, "lastline", 0, 1 );

	// Get and search for the connection type
	s = drvthis->config_get_string( drvthis->name, "connectiontype", 0, "4bit" );
	for (i = 0; connectionMapping[i].name != NULL && strcmp (s, connectionMapping[i].name) != 0; i++);
	if (connectionMapping[i].name == NULL) {
		report (RPT_ERR, "HD44780_init: Unknown connection type: %s", s);
		return -1; // fatal error
	} else {
		p->connectiontype_index = i;
		/* check if ConnectionType contains the string "USB" or "usb" */
		if ((strstr(connectionMapping[i].name, "usb") != NULL) ||
		    (strstr(connectionMapping[i].name, "USB") != NULL))
			usb = 1;
	}

	// Get and parse vspan only when specified
	s = drvthis->config_get_string( drvthis->name, "vspan", 0, "" );
	if( s[0] != 0 ) {
		if (parse_span_list (&(p->spanList), &(p->numLines), &(p->dispVOffset), &(p->numDisplays), &(p->dispSizes), s) == -1) {
			report (RPT_ERR, "HD44780_init: invalid vspan value: %s", s );
			return -1;
		}
	}

	// Get and parse size
	s = drvthis->config_get_string( drvthis->name, "size", 0, "20x4" );
	if( sscanf( s, "%dx%d", &(p->width), &(p->height) ) != 2
	|| (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
	|| (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report (RPT_ERR, "HD44780_init: Cannot read size: %s", s );
	}

	// default case for when spans aren't indicated
	// - add a sanity check against p->height ??
	if (p->numLines == 0) {
		if ((p->spanList = (int *) malloc (sizeof (int) * p->height))) {
			int i;
			for (i = 0; i < p->height; ++i) {
				p->spanList[i] = 1;
				p->numLines = p->height;
			}
		} else
			report (RPT_ERR, "Error mallocing");
	}
	if (p->numDisplays == 0) {
		if ((p->dispVOffset = (int *) malloc (sizeof (int))) && (p->dispSizes = (int *) malloc (sizeof (int)))) {
			p->dispVOffset[0] = 0;
			p->dispSizes[0] = p->height;
			p->numDisplays = 1;
		} else
			report (RPT_ERR, "Error mallocing");
	}

	if (timing_init() == -1) {
		report(RPT_ERR, "timing_init: failed (%s)\n", strerror(errno));
		return -1;
	}	

#if defined DELAY_NANOSLEEP
	// Change to Round-Robin scheduling for nanosleep
	{
		// Set priority to 1
		struct sched_param param;
		param.sched_priority=1;
		if (( sched_setscheduler(0, SCHED_RR, &param)) == -1) {
			report (RPT_ERR, "HD44780_init: failed (%s)", strerror (errno));
			return -1;
		}
	}
#endif

	// Allocate framebuffer
	p->framebuf = (unsigned char *) malloc (p->width * p->height);
	if (!p->framebuf) {
		//HD44780_close();
		return -1;
	}

	// Allocate and clear the buffer for incremental updates
	p->lcd_contents = (unsigned char *) malloc (p->width * p->height);
	if (!p->lcd_contents) {
		return -1;
	}
	memset(p->lcd_contents, 0, p->width * p->height);

	// Allocate and clear the buffer for defineable characters
	p->cc_buf = (unsigned char *) malloc (NUM_CCs * p->cellheight);
	p->cc_dirty = (unsigned char *) malloc (NUM_CCs);
	if (!p->cc_buf || !p->cc_dirty) {
		return -1;
	}
	memset(p->cc_buf, 0, NUM_CCs * p->cellheight);
	memset(p->cc_dirty, 1, NUM_CCs); /* all custom chars dirty */

	// Keypad ?
	if ( p->have_keypad ) {
		int x, y;

		// Read keymap
		for( x=0; x<KEYPAD_MAXX; x++ ) {
			char buf[40];

			// First fill with default value
			p->keyMapDirect[x] = defaultKeyMapDirect[x];

			// Read config value
			sprintf( buf, "keydirect_%1d", x+1 );
			s = drvthis->config_get_string( drvthis->name, buf, 0, NULL );

			// Was a key specified in the config file ?
			if( s ) {
				p->keyMapDirect[x] = strdup( s );
				report( RPT_INFO, "HD44780: Direct key %d: \"%s\"", x, s );
			}
		}

		for( x=0; x<KEYPAD_MAXX; x++ ) {
			for( y=0; y<KEYPAD_MAXY; y++ ) {
				char buf[40];

				// First fill with default value
				p->keyMapMatrix[y][x] = defaultKeyMapMatrix[y][x];

				// Read config value
				sprintf( buf, "keymatrix_%1d_%d", x+1, y+1 );
				s = drvthis->config_get_string( drvthis->name, buf, 0, NULL );

				// Was a key specified in the config file ?
				if( s ) {
					p->keyMapMatrix[y][x] = strdup( s );
					report( RPT_INFO, "HD44780: Matrix key %d %d: \"%s\"", x, y, s );
				}
			}
		}
	}

	// Output latch state - init to a non-valid value
	p->output_state = 999999;

	if ((p->hd44780_functions = (HD44780_functions *) malloc (sizeof (HD44780_functions))) == NULL) {
		report (RPT_ERR, "Error mallocing");
		return -1;
	}
	p->hd44780_functions->uPause = uPause;
	p->hd44780_functions->scankeypad = HD44780_scankeypad;
	p->hd44780_functions->output = NULL;

	// Do connection type specific display init
	connectionMapping[p->connectiontype_index].init_fn (drvthis);

	// Display startup parameters on the LCD
	HD44780_clear (drvthis);
	sprintf (buf, "HD44780 %dx%d", p->width, p->height );
	HD44780_string (drvthis, 1, 1, buf);
	if (usb) {
		sprintf (buf, "USB %s%s%s",
				(p->have_backlight?" bl":""),
				(p->have_keypad?" key":""),
				(p->have_output?" out":"")
			);
	}
	else {
	sprintf (buf, "LPT 0x%x%s%s%s", p->port,
			(p->have_backlight?" bl":""),
			(p->have_keypad?" key":""),
			(p->have_output?" out":"")
		);
	}
	HD44780_string (drvthis, 1, 2, buf);
	HD44780_flush (drvthis);
	sleep (2);

	return 1;
}

/////////////////////////////////////////////////////////////////
// Common initialisation sequence - sets cursor off and not blinking,
// clear display and homecursor
// Does not set twoline mode nor small characters (5x8). The init function of
// the connectiontype should do this.
//
void
common_init (PrivateData *p, unsigned char if_bit)
{
	if (p->ext_mode) {
		// Set up extended mode */
		p->hd44780_functions->senddata (p, 0, RS_INSTR, FUNCSET | if_bit | TWOLINE | SMALLCHAR | EXTREG );
		p->hd44780_functions->uPause (p, 40);
		p->hd44780_functions->senddata (p, 0, RS_INSTR, EXTMODESET | FOURLINE );
		p->hd44780_functions->uPause (p, 40);
	}
	p->hd44780_functions->senddata (p, 0, RS_INSTR, FUNCSET | if_bit | TWOLINE | SMALLCHAR );
	p->hd44780_functions->uPause (p, 40);
	p->hd44780_functions->senddata (p, 0, RS_INSTR, ONOFFCTRL | DISPON | CURSOROFF | CURSORNOBLINK);
	p->hd44780_functions->uPause (p, 40);
	p->hd44780_functions->senddata (p, 0, RS_INSTR, CLEAR);
	p->hd44780_functions->uPause (p, 1600);
	p->hd44780_functions->senddata (p, 0, RS_INSTR, HOMECURSOR);
	p->hd44780_functions->uPause (p, 1600);
}

/////////////////////////////////////////////////////////////////
// Delay a number of microseconds
//
void
uPause (PrivateData *p, int usecs)
{
	timing_uPause( usecs * p->delayMult );
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
HD44780_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if(p->framebuf) free( p->framebuf );
	if(p->lcd_contents) free( p->lcd_contents );
	free( p );
	drvthis->store_private_ptr( drvthis, NULL );
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
HD44780_width (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
HD44780_height (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->height;
}

/////////////////////////////////////////////////////////////////
// Set position (not part of API)
//
// x and y here are for the virtual p->height x p->width display
void
HD44780_position (Driver *drvthis, int x, int y)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int dispID = p->spanList[y];
	int relY = y - p->dispVOffset[dispID - 1];
	int DDaddr;

	if (p->ext_mode) {
		// Linear addressing, each line starts 0x20 higher.
		DDaddr = x + relY * 0x20;
	} else {
		// 16x1 is a special case
		if (p->dispSizes[dispID - 1] == 1 && p->width == 16) {
			if (x >= 8) {
				x -= 8;
				relY = 1;
			}
		}
	
		DDaddr = x + (relY % 2) * 0x40;
		if ((relY % 4) >= 2)
			DDaddr += p->width;
	}
	p->hd44780_functions->senddata (p, dispID, RS_INSTR, POSITION | DDaddr);
	p->hd44780_functions->uPause (p, 40);  // Minimum exec time for all commands
}

/////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
MODULE_EXPORT void
HD44780_flush (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int x, y;
	int wid = p->width;
	char ch;
	char drawing;
	int row;
	int i;
	int count;

	// Update LCD incrementally by comparing with last contents
	count = 0;
	for (y=0; y<p->height; y++) {
		drawing = 0;
		for (x=0 ; x<wid; x++) {
			ch = p->framebuf[(y * wid) + x];
			if( ch != p->lcd_contents[(y*wid)+x] ) {
				if( !drawing || x % 8 == 0 ) { // x%8 is for 16x1 displays !
					drawing = 1;
					HD44780_position(drvthis,x,y);
				}
				p->hd44780_functions->senddata (p, p->spanList[y], RS_DATA, HD44780_charmap[(unsigned char)ch]);
				p->hd44780_functions->uPause (p, 40);  // Minimum exec time for all commands
				p->lcd_contents[(y*wid)+x] = ch;
				count ++;
			}
			else {
				drawing = 0;
			}
		}
	}
	debug( RPT_DEBUG, "HD44780: flushed %d chars", count );

	/* Check which defineable chars we need to update */
	count = 0;
	for( i = 0; i < NUM_CCs; i ++ ) {
		if( p->cc_dirty[i] ) {

			/* Tell the HD44780 we will redefine char number i */
			p->hd44780_functions->senddata (p, 0, RS_INSTR, SETCHAR | i * 8);
			p->hd44780_functions->uPause (p, 40);  // Minimum exec time for all commands

			/* Send the subsequent rows */
			for (row = 0; row < p->cellheight; row++) {
				p->hd44780_functions->senddata (p, 0, RS_DATA, p->cc_buf[i*p->cellheight+row]);
				p->hd44780_functions->uPause (p, 40);  /* Minimum exec time for all commands */
			}
			/* Mark as not dirty anymore */
			p->cc_dirty[i] = 0;
			count ++;
		}
	}
	debug( RPT_DEBUG, "HD44780: flushed %d custom chars's", count );
}

/////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
MODULE_EXPORT void
HD44780_clear (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = CCMODE_STANDARD;
}

/////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
MODULE_EXPORT void
HD44780_chr (Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	y--;
	x--;

	p->framebuf[ (y * p->width) + x] = ch;
}

/////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
MODULE_EXPORT void
HD44780_string (Driver *drvthis, int x, int y, char *s)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x --;  // Convert 1-based coords to 0-based
	y --;

	for (i = 0; s[i]; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y*p->width) + x + i] = s[i];
	}
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
MODULE_EXPORT void
HD44780_backlight (Driver *drvthis, int on)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	p->hd44780_functions->backlight (p, on);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before HD44780->vbar()
//
static void
HD44780_init_vbar (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

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

	if( p->ccmode == CCMODE_VBAR ) {
		/* Work already done */
		return;
	}

	if( p->ccmode != CCMODE_STANDARD ) {
		/* Not supported (yet) */
		report( RPT_WARNING, "HD44780_init_vbar: Cannot combine two modes using user defined characters" );
		return;
	}
	p->ccmode = CCMODE_VBAR;

	HD44780_set_char (drvthis, 1, a);
	HD44780_set_char (drvthis, 2, b);
	HD44780_set_char (drvthis, 3, c);
	HD44780_set_char (drvthis, 4, d);
	HD44780_set_char (drvthis, 5, e);
	HD44780_set_char (drvthis, 6, f);
	HD44780_set_char (drvthis, 7, g);
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
HD44780_init_hbar (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

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
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};

	if( p->ccmode == CCMODE_HBAR ) {
		/* Work already done */
		return;
	}

	if( p->ccmode != CCMODE_STANDARD ) {
		/* Not supported (yet) */
		report( RPT_WARNING, "HD44780_init_hbar: Cannot combine two modes using user defined characters" );
		return;
	}
	p->ccmode = CCMODE_HBAR;

	HD44780_set_char (drvthis, 1, a);
	HD44780_set_char (drvthis, 2, b);
	HD44780_set_char (drvthis, 3, c);
	HD44780_set_char (drvthis, 4, d);
	HD44780_set_char (drvthis, 5, e);

}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
HD44780_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	HD44780_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
HD44780_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	HD44780_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
MODULE_EXPORT void
HD44780_init_num (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	char bignum_ccs[8][5*8] = {{
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	}, {
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0
	}, {
		1, 1, 0, 1, 1,
		1, 1, 0, 1, 1,
		1, 1, 0, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	}, {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0
	}, {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 0, 0
	}, {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 0, 1, 1,
		1, 1, 0, 1, 1,
		1, 1, 0, 1, 1,
		0, 0, 0, 0, 0
	}, {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		0, 0, 0, 0, 0
	}, {
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	}};

	if (p->ccmode != CCMODE_BIGNUM) {
		int i;

		if( p->ccmode != CCMODE_STANDARD ) {
			/* Not supported (yet) */
			report( RPT_WARNING, "HD44780_init_num: Cannot combine two modes using user defined characters" );
			return;
		}
		p->ccmode = CCMODE_BIGNUM;

		for (i = 0; i < 8; i++)
			HD44780_set_char (drvthis, i, bignum_ccs[i]);
	}
}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
HD44780_num (Driver *drvthis, int x, int num)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	char bignum_map[11][4][3] = {
	{ /* 0: */
		{  1,  2,  3 }, 
		{  6, 32,  6 }, 
		{  6, 32,  6 }, 
		{  7,  2, 32 } }, 
	{ /* 1: */
		{  7,  6, 32 }, 
		{ 32,  6, 32 }, 
		{ 32,  6, 32 }, 
		{  7,  2, 32 } }, 
	{ /* 2: */
		{  1,  2,  3 }, 
		{ 32,  5,  0 }, 
		{  1, 32, 32 }, 
		{  2,  2,  0 } }, 
	{ /* 3: */
		{  1,  2,  3 }, 
		{ 32,  5,  0 }, 
		{  3, 32,  6 }, 
		{  7,  2, 32 } }, 
	{ /* 4: */
		{ 32,  3, 6 }, 
		{  1, 32, 6 }, 
		{  2,  2, 6 }, 
		{ 32, 32, 0 } }, 
	{ /* 5: */
		{  1,  2,  0 }, 
		{  2,  2,  3 }, 
		{  3, 32,  6 }, 
		{  7,  2, 32 } }, 
	{ /* 6: */
		{  1,  2, 32 }, 
		{  6,  5, 32 }, 
		{  6, 32,  6 }, 
		{  7,  2, 32 } }, 
	{ /* 7: */
		{  2,  2,  6 }, 
		{ 32,  1, 32 }, 
		{ 32,  6, 32 }, 
		{ 32,  0, 32 } }, 
	{ /* 8: */
		{  1,  2,  3 }, 
		{  4,  5,  0 }, 
		{  6, 32,  6 }, 
		{  7,  2, 32 } }, 
	{ /* 9: */
		{  1,  2,  3 }, 
		{  4,  3,  6 }, 
		{ 32,  1, 32 }, 
		{  7, 32, 32 } }, 
	{ /* colon: (only 1st column used) */
		{ 32, 32, 32 },
		{  7, 32, 32 },
		{  7, 32, 32 },
		{ 32, 32, 32 } }
	};

	if ((num < 0) || (num > 10))
		return;

	if ((p->width >= 20) && (p->height >= 4)) {
		int y = (p->height - 2) / 2;
		int x2, y2;

		HD44780_init_num(drvthis);

		for (x2 = 0; x2 < 3; x2++) {
			for (y2 = 0; y2 < 4; y2++) {
				HD44780_chr( drvthis, x+x2, y+y2, bignum_map[num][y2][x2] );
			}
			if (num == 10)
				x2 = 2; /* =break, for colon only */
		}
	}
	else
		HD44780_chr(drvthis, x, 1 + (p->height - 1) / 2,
			    (num == 10) ? ':' : (num + '0'));
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
HD44780_set_char (Driver *drvthis, int n, char *dat)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	for (row = 0; row < p->cellheight; row++) {
		letter = 0;
		if (p->lastline || (row < p->cellheight - 1)) {
			for (col = 0; col < p->cellwidth; col++) {
				letter <<= 1;
				letter |= (dat[(row * p->cellwidth) + col] > 0) ? 1 : 0;
			}
		}	
		if( p->cc_buf[n*p->cellheight+row] != letter ) {
			p->cc_dirty[n] = 1; /* only mark as dirty if really different */
		}
		p->cc_buf[n*p->cellheight+row] = letter;
	}
}

/////////////////////////////////////////////////////////////
// Set default icon into a userdef char
//
MODULE_EXPORT int
HD44780_icon (Driver *drvthis, int x, int y, int icon)
{
	char heart_open[] = {
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1 };
	char heart_filled[] = {
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1 };
	char arrow_up[] = {
		 0, 0, 1, 0, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0 };
	char arrow_down[] = {
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0 };
	char checkbox_off[] = {
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 };
	char checkbox_on[] = {
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 1, 1, 0, 1,
		 1, 0, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 };
	char checkbox_gray[] = {
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 };
	char block_filled[] = {
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1 };

	/* Yes I know, this is a VERY BAD implementation */
	switch( icon ) {
		case ICON_BLOCK_FILLED:
			HD44780_set_char( drvthis, 6, block_filled );
			HD44780_chr( drvthis, x, y, 6);
			break;
		case ICON_HEART_FILLED:
			HD44780_set_char( drvthis, 0, heart_filled );
			HD44780_chr( drvthis, x, y, 0 );
			break;
		case ICON_HEART_OPEN:
			HD44780_set_char( drvthis, 0, heart_open );
			HD44780_chr( drvthis, x, y, 0 );
			break;
		case ICON_ARROW_UP:
			HD44780_set_char( drvthis, 1, arrow_up );
			HD44780_chr( drvthis, x, y, 1 );
			break;
		case ICON_ARROW_DOWN:
			HD44780_set_char( drvthis, 2, arrow_down );
			HD44780_chr( drvthis, x, y, 2 );
			break;
		case ICON_ARROW_LEFT:
			HD44780_chr( drvthis, x, y, 0x7F );
			break;
		case ICON_ARROW_RIGHT:
			HD44780_chr( drvthis, x, y, 0x7E );
			break;
		case ICON_CHECKBOX_OFF:
			HD44780_set_char( drvthis, 3, checkbox_off );
			HD44780_chr( drvthis, x, y, 3 );
			break;
		case ICON_CHECKBOX_ON:
			HD44780_set_char( drvthis, 4, checkbox_on );
			HD44780_chr( drvthis, x, y, 4 );
			break;
		case ICON_CHECKBOX_GRAY:
			HD44780_set_char( drvthis, 5, checkbox_gray );
			HD44780_chr( drvthis, x, y, 5 );
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}

/////////////////////////////////////////////////////////////
// Get a key from the keypad (if there is one)
//
MODULE_EXPORT char *
HD44780_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char scancode;
	char * keystr = NULL;
	struct timeval curr_time, time_diff;

	if( ! p->have_keypad ) return NULL;

	gettimeofday(&curr_time,NULL);

	scancode = p->hd44780_functions->scankeypad(p);
	if( scancode ) {
		if( scancode & 0xF0 ) {
			keystr = p->keyMapMatrix[((scancode&0xF0)>>4)-1][(scancode&0x0F)-1];
		}
		else {
			keystr = p->keyMapDirect[scancode - 1];
		}
	}

	if( keystr != NULL ) {
		if (keystr == p->pressed_key) {
			timersub (&curr_time, &(p->pressed_key_time), &time_diff);
			if (((time_diff.tv_usec / 1000 + time_diff.tv_sec * 1000) - KEYPAD_AUTOREPEAT_DELAY) < 1000 * p->pressed_key_repetitions / KEYPAD_AUTOREPEAT_FREQ ) {
				// The key is already pressed quite some time
				// but it's not yet time to return a repeated keypress
				return NULL;
			}
			// Otherwise a keypress will be returned
			p->pressed_key_repetitions ++;
		}
		else {
			// It's a new keypress
			p->pressed_key_time = curr_time;
			p->pressed_key_repetitions = 0;
			report( RPT_INFO, "HD44780_get_key: Key pressed: %s  (%d,%d)\n", keystr, scancode&0x0F, (scancode&0xF0)>>4 );
		}
	}

	// Store the key for the next round
	p->pressed_key = keystr;

	return keystr;
}

/////////////////////////////////////////////////////////////
// Scan the keypad
//
unsigned char HD44780_scankeypad(PrivateData *p)
{
	unsigned int keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned int Ypattern;
	unsigned int Yval;
	signed char exp;

	unsigned char scancode = 0;

	// First check if a directly connected key is pressed
	// Put all zeros on Y of keypad
	keybits = p->hd44780_functions->readkeypad (p, 0);

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
		if( p->hd44780_functions->readkeypad (p, Ypattern)) {
			// Yes, a key on the matrix is pressed

			// OK, now we know a key is pressed.
			// Determine which one it is

			// First determine the row
			// Do a 'binary search' to minimize I/O
			Ypattern = 0;
			Yval = 0;
			for (exp=3; exp>=0; exp--) {
				Ypattern = ((1 << (1 << exp)) - 1) << Yval;
				keybits = p->hd44780_functions->readkeypad (p, Ypattern);
				if (!keybits) {
					Yval += (1 << exp);
				}
			}

			// Which key is pressed in that row ?
			keybits = p->hd44780_functions->readkeypad (p, 1<<Yval);
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
// Output to the optional output latch(es)
//
MODULE_EXPORT void
HD44780_output (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (!p->have_output) return; /* output disabled */
	if (!p->hd44780_functions->output) return; /* unsupported for selected connectiontype */

	// perhaps it is better just to do this every time in case of a glitch
	// but leaving this in does make sure that any latch-enable line glitches
	// are more easily seen
	if (p->output_state == on) return;

	p->output_state = on;
	p->hd44780_functions->output(p, on);
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
				retVal = -1;
			}
		} else {
			retVal = -1;
		}
	}
	return retVal;
}

