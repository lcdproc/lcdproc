/*
 * Driver for SED1330/1335 graphical displays
 *
 * EPSON has changed the the chip-labeling.
 * SED1330 now is S1D13300 and SED1335 is S1D13305
 *
 * This driver drives the LCD in text mode.
 * Probably the driver can easily be adapted to work for 1336 too.
 *
 * Moved the delay timing code by Charles Steinkuehler to timing.h.
 * Guillaume Filion <gfk@logidac.com>, December 2001
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001-2003, Joris Robijn <joris@robijn.net>
 * 		 2003, Michael Rohde <Micha.R@online.de>
 *
 *
 * Changelog:
 *
 * November 2001, Joris Robijn:
 * - Created the driver
 * - Parts copied from HD44780 driver
 * December 2001, Joris Robijn:
 * - Adapted to v0.5 API
 * June 2002, Joris Robijn:
 * - Modified init things to support multiple font sizes
 * - More calculations v.s. static init data
 * - Finished keypad stuff
 * May 2003, Micha_R
 * - removing all the cursor visible/invisible stuff, because it produce much flicker
 *   (software rendering is now used (Joris))
 * - added support for displays with 192x192 resulution (type 4 = Seiko G191D, but beware,
 *   this display has no controller. You need an SED1330/1335 based interface circuit).
 * - fixed problem with clear display function. With displays for which the formula
 *   "display-hight / character hight" don't give a integer result, unusable lines at the
 *   bottom of the display wasn't cleared.
 * - verified with SED1335. The only difference between the two chips is the minimum
 *   reset-raise-time (SED1330 = 1 ms, SED1335 = 0.2 ms)
 * - added function sed1330_icon to get correct display of full blocks
 * May 2003, Joris Robijn
 * - Made bars have spaces between them (I hope people like this)
 *
 *
 * IMPORTANT: MODULES OTHER THAN G321D
 * ===================================
 * If you are using a module other than the G321D, beware that the software
 * has not been tested. You may need to adapt the initialization parameters
 * to get it working properly.
 *
 *
 *
 *
 * Connections
 * ===========
 *
 * Connections below are for the G242C, G121C and G321D displays.
 * Always consult documentation about the specific display before asuming
 * the connections given here are also correct for your display !
 *
 *   Ordered by LCD pins
 *	LCD		pin?	<--->	pin	LPT port
 *	^RESET		1		1	^STROBE
 *	^RD		2	+5V
 *	^WR		3		16	^INIT
 *	SEL1		4	GND
 *	SEL2		5	GND
 *	^CS		6	GND
 *	A0		7		17	^SELECT_IN
 *	D0		8		2	D0
 *	D1		9		3	D1
 *	D2		10		4	D2
 *	D3		11		5	D3
 *	D4		12		6	D4
 *	D5		13		7	D5
 *	D6		14		8	D6
 *	D7		15		9	D7
 *	Vdd		16	+5V
 *	Vss		17	GND	18..25	GND
 *	V0		18	potmeter
 *	Vlc		19	-24V
 *	Frame		20	GND
 *					10	^ACK
 *				GND	11	BUSY
 *					12	PAPEREND
 *					13	^SELECT
 *					14	^LF
 *					15	^ERROR
 *
 *   Or ordered by the LPT port pins:
 *	LCD		pin?	<--->	pin	LPT port
 *	^RESET		1		1	^STROBE
 *	D0		8		2	D0
 *	D1		9		3	D1
 *	D2		10		4	D2
 *	D3		11		5	D3
 *	D4		12		6	D4
 *	D5		13		7	D5
 *	D6		14		8	D6
 *	D7		15		9	D7
 *					10	^ACK
 *				GND	11	BUSY
 *					12	PAPEREND
 *					13	^SELECT
 *					14	^LF
 *					15	^ERROR
 *	^WR		3		16	^INIT
 *	A0		7		17	^SELECT_IN
 *	Vss		17	GND	18..25	GND
 *	^RD		2	+5V
 *	SEL1		4	GND
 *	SEL2		5	GND
 *	^CS		6	GND
 *	Vdd		16	+5V
 *	V0		18	potmeter
 *	Vlc		19	-24V (not required for G242C)
 *	Frame		20	GND
 *
 *	The potmeter should be connected like this on these display modules:
 *
 *	=== GND
 *	 |
 *	.-.
 *	| |
 *	| |5k
 *	'-'
 *	 |
 *	 |
 *	.-.10k potmeter
 *	| |
 *	| |<----------------o V0
 *	| |
 *	'-'
 *	 |
 *	 O Vlc (= -24V )
 *
 * The G242C generates -24V internally. It is available on Vlc.
 *
 * To generate -24 from the +5V without an external power source, you can
 * use the following circuit.
 *
 * 5V O------+----------+                                 pinout:
 *           |          |                                  _____
 *           |         --- 100uF                          |  _  |
 *           |         --- 10V                            | (_) | <-3
 *           |          |                                 |_____|
 *           |          +--------+--------+--------+      | max |
 *           |5         |        |        |        |      | 724 |
 *       ---------     === GND   C        -        |      |_____|
 *      |         |              C coil  | |       |       |||||
 *      |         |              C 47uH  | |10k    |       |||||.
 *      |         |4             |        -        |       | | |
 *      | MAX724  |--------------+        |        |       12345
 *      |   or    |              |        |        |
 *      | MAX726  |1             |        |        |+
 *      |         |-----------------------+       --- 47uF
 *      |         |              |        |       --- 50V
 *      |         |              |        |        |
 *       ---------               |        -        |
 *        |2    |3               |       | |       |
 *        |     |              '---,     | |1k     |
 *       ---    |        SB160  / \       -        |
 *  100nF---    |               ^T^       |        |
 *        |     |                |        |        |
 *        +-----+----------------+--------+--------+----O -24V out
 *
 *
 *
 *
 * Config options
 * ==============
 *
 * With the display= option you should specify what display module you have.
 * Accepted values are:
 * display=G121C
 * display=G242C
 * display=G321D
 * display=G191D
 * display=G2446
 * The port= value should be set to the LPT port address that the LCD is
 * connected to. Examples:
 * port=0x378
 * port=0x278
 * port=0x3BC
 *
 * The cellsize= value indicates the size of a character. Default:
 * cellsize=6x10
 *
 */

#include "lcd.h"
#include "sed1330.h"

#include "port.h"
#include "lpt-port.h"
#include "report.h"
#include "timing.h"
#define uPause timing_uPause

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

// Autorepeat values
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15

// LPT lines
#define A0	SEL
#define nRESET	STRB
#define nWR	INIT

// Command definitions
#define CMD_SYSTEM_SET	0x40
#define CMD_SLEEP_IN	0x53
#define CMD_DISP_DIS	0x58
#define CMD_DISP_EN	0x59
#define CMD_SCROLL	0x44
#define CMD_CSR_FORM	0x5D
#define CMD_CGRAM_ADR	0x5C
#define CMD_CSR_DIR_R	0x4C
#define CMD_CSR_DIR_L	0x4D
#define CMD_CSR_DIR_U	0x4E
#define CMD_CSR_DIR_D	0x4F
#define CMD_HDOT_SCR	0x5A
#define CMD_OVLAY	0x5B
#define CMD_CSRW	0x46
#define CMD_CSRR	0x47
#define CMD_MWRITE	0x42
#define CMD_MREAD	0x43

// Data definitions
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 8

#define TYPE_G321D 1
#define TYPE_G121C 2
#define TYPE_G242C 3
#define TYPE_G191D 4
#define TYPE_G2446 5

#define SCR1_L 0x00 // Memory locations
#define SCR1_H 0x00
#define SCR2_L 0x00
#define SCR2_H 0x06

typedef struct p {
	int type;
	int port;
	char * keymap[KEYPAD_MAXX];
	char * framebuf_text;
	char * lcd_contents_text;
	char * framebuf_graph;
	char * lcd_contents_graph;
	int width, height;
	int cellwidth, cellheight;
	int graph_width, graph_height;
	int bytesperline, textlines_in_memory;

	char have_keypad;

	// keyMapDirect contains an array of the ascii-codes that should be generated
	// when a directly connected key is pressed (not in matrix).
	char *keyMapDirect[KEYPAD_MAXX];

	// keyMapMatrix contrains an array with arrays of the ascii-codes that should be generated
	// when a key in the matrix is pressed.
	char *keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX];

	char *pressed_key;
	int pressed_key_repetitions;
	struct timeval pressed_key_time;

	int stuckinputs;

} PrivateData;

static char *defaultKeyMapDirect[KEYPAD_MAXX] = { "Enter", "Up", "Down", "Escape", "F1" };

static char *defaultKeyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
		{ "1", "2", "3", "A", "E" },
		{ "4", "5", "6", "B", "F" },
		{ "7", "8", "9", "C", "G" },
		{ "*", "0", "#", "D", "H" },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL }};


// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1; // yes, we have no global variables (except for constants)
MODULE_EXPORT char *symbol_prefix = "sed1330_";


// Local functions
//void uPause (int usecs);
void sed1330_command( PrivateData * p, char command, int datacount, char * data );
void sed1330_rect( PrivateData * p, int x1, int y1, int x2, int y2, char pattern );
void sed1330_line ( PrivateData * p, int x1, int y1, int x2, int y2, char pattern );
inline void sed1330_set_pixel( PrivateData * p, int x, int y, int value );
unsigned char sed1330_scankeypad(PrivateData *p);
unsigned char sed1330_readkeypad (PrivateData *p, unsigned int YData);


/////////////////////////////////////////////////////////////////
// Init the driver and display
//
MODULE_EXPORT int
sed1330_init( Driver * drvthis, char *args )
{
	char * s;
	PrivateData * p;
	char data[8];

	debug( RPT_DEBUG, "%s( %p, args=\"%s\" )", __FUNCTION__, drvthis, args );


	// Alocate and store private p
	p = (PrivateData *) malloc( sizeof(PrivateData) );
	if( ! p )
		return -1;
	if( drvthis->store_private_ptr( drvthis, p ) )
		return -1;

	// Clear keymap
	memset( p->keymap, 0, sizeof(p->keymap) );

	// READ THE CONFIG FILE

	// Port
	p->port = drvthis->config_get_int( drvthis->name, "port", 0, 0x278 );

	// Char size
	s = drvthis->config_get_string( drvthis->name, "cellsize", 0, NULL );
	if( !s ) {
		s = "6x10";
	}
	if( sscanf( s, "%dx%d", &(p->cellwidth), &(p->cellheight)) != 2 ) {
		report( RPT_ERR, "SED1330: cannot interpret cellsize value: %s", s );
		return -1;
	}
	if( p->cellwidth < 6 || p->cellwidth > 8
	|| p->cellheight < 7 || p->cellheight > 16 ) {
		report( RPT_ERR, "SED1330: cellsize exceeds allowed range of 6x7 to 8x16" );
		return -1;
	}

	// Type
	s = drvthis->config_get_string( drvthis->name, "type", 0, NULL );
	if( !s ) {
		report( RPT_ERR, "SED1330: you need to specify the display type" );
		return -1;
	} else if( strcmp( s, "G321D" ) == 0 ) {
		p->type = TYPE_G321D;
		p->graph_width = 320;
		p->graph_height = 200;
	} else if( strcmp( s, "G121C" ) == 0 ) {
		p->type = TYPE_G121C;
		p->graph_width = 128;
		p->graph_height = 128;
	} else if( strcmp( s, "G242C" ) == 0 ) {
		p->type = TYPE_G242C;
		p->graph_width = 240;
		p->graph_height = 128;
	} else if( strcmp( s, "G191D" ) == 0 ) {
		p->type = TYPE_G191D;
		p->graph_width = 192;
		p->graph_height = 192;
	} else if( strcmp( s, "G2446" ) == 0 ) {
		p->type = TYPE_G2446;
		p->graph_width = 256;
		p->graph_height = 64;

	} else {
		report( RPT_ERR, "SED1330: Unknown display type: %s", s );
		return -1;
	}
	report( RPT_INFO, "SED1330: Using LCD type: %s", s );

	// Keypad ?
	p->have_keypad = drvthis->config_get_bool( drvthis->name, "keypad", 0, 0 );

	// Keymap
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
				report( RPT_INFO, "SED1330: Direct key %d: \"%s\"", x, s );
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
					report( RPT_INFO, "SED1330: Matrix key %d,%d: \"%s\"", x, y, s );
				}
			}
		}
	}

	// Calculate some sizes
	p->width = p->graph_width / p->cellwidth;
	p->height = p->graph_height / p->cellheight;
	p->bytesperline = (p->graph_width - 1 ) / p->cellwidth + 1;
	p->textlines_in_memory = (p->graph_height - 1 ) / p->cellheight + 1;

	report( RPT_INFO, "SED1330: Text size: %dx%d", p->width, p->height );
	report( RPT_INFO, "SED1330: Cell size: %dx%d", p->cellwidth, p->cellheight );
	report( RPT_INFO, "SED1330: Graphical size: %dx%d", p->graph_width, p->graph_height );

	// Allocate framebuffer
	p->framebuf_text = (unsigned char *) malloc( p->bytesperline * p->textlines_in_memory );
	if( ! p->framebuf_text  )
		return -1;
	memset( p->framebuf_text, ' ', p->bytesperline * p->textlines_in_memory );

	p->lcd_contents_text = (unsigned char *) malloc( p->bytesperline * p->textlines_in_memory );
	if( ! p->lcd_contents_text  )
		return -1;
	memset( p->lcd_contents_text, 0, p->bytesperline * p->textlines_in_memory );

	p->framebuf_graph = (unsigned char *) malloc( p->bytesperline * p->graph_height );
	if( ! p->framebuf_graph  )
		return -1;
	memset( p->framebuf_graph, 0, p->bytesperline * p->graph_height );

	p->lcd_contents_graph = (unsigned char *) malloc( p->bytesperline * p->graph_height );
	if( ! p->lcd_contents_graph  )
		return -1;
	memset( p->lcd_contents_graph, 0xFF, p->bytesperline * p->graph_height );

	// Arrange for access to port
	debug( RPT_DEBUG, "%s: getting port access", __FUNCTION__ );
	port_access(p->port);
	port_access(p->port+1);
	port_access(p->port+2);

	if (timing_init() == -1)
		return -1;

	// INITIALIZE THE LCD
	// End reset-state
	debug( RPT_DEBUG, "%s: initializing LCD", __FUNCTION__ );
	port_out( p->port+2, (nWR) ^ OUTMASK );	// raise ^RD and ^WR
	port_out( p->port+2, (nRESET|nWR) ^ OUTMASK );	// lower RESET
	uPause( 200 );
	port_out( p->port+2, (nWR) ^ OUTMASK );	// raise RESET
	uPause( 200 );
	port_out( p->port+2, (nRESET|nWR) ^ OUTMASK );	// lower RESET
	uPause( 4000 );

	switch( p->type ) {
	  case TYPE_G321D:
		data[4] = 0x38;
		break;
	  case TYPE_G121C:
		data[4] = 0x7F; // ?     please confirm these numbers
		break;
	  case TYPE_G242C:
		data[4] = 0x7F; // ?
		break;
	  case TYPE_G2446:
		data[4] = 0x7F; // ?
		break;
	  case TYPE_G191D:
		data[4] = 0x5c; // ?
		break;
	  default:
		return -1;
	}

	data[0] = 0x30;
	data[1] = 0x80 + p->cellwidth - 1;
	data[2] = p->cellheight - 1;
	data[3] = p->width - 1;
	// data[4] should be filled already
	// TC/R = ((clock / (refresh * (L/F - 1))) - 1) / 9
	data[5] = p->graph_height - 1;
	data[6] = p->bytesperline;
	data[7] = 0;
	sed1330_command( p, CMD_SYSTEM_SET, 8, data );

	// TODO: The memory locations need to be calculated !
	sed1330_command( p, CMD_SCROLL, 6, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) ); // screen1 and screen2 memory locations

	data[0] = p->cellwidth-1;
	data[1] = 7;
	sed1330_command( p, CMD_CSR_FORM, 2, data );		// set cursor size

	sed1330_command( p, CMD_HDOT_SCR, 1, ((char[1]) {0x00}) );	// horizontal pixel shift=0
	sed1330_command( p, CMD_OVLAY, 1, ((char[1]) {0x01}) );		// XOR mode, screen1 text, screen3 text (screen2 and screen4 are always graph)
	sed1330_command( p, CMD_DISP_DIS, 1, ((char[1]) {0x14}) );	// display off,set cursor off, screen1 on, screen2 on, screen3 off
	sed1330_command( p, CMD_CSR_DIR_R, 0, NULL );			// cursor move right

	sed1330_flush( drvthis ); 	// Clear the contents of the LCD
	sed1330_command( p, CMD_DISP_EN, 0, NULL );	// And display on

	return 0;
}


/////////////////////////////////////////////////////////////////
// Send a command and accompanying p
// INTERNAL
//
void
sed1330_command( PrivateData * p, char command, int datacount, char * data )
{
	int i;
	int port = p->port;

	port_out( port+2, (nRESET|nWR|A0) ^ OUTMASK );		// set A0 to indicate command
	port_out( port, command );				// set up p
	port_out( port+2, (nRESET|A0) ^ OUTMASK );		// activate ^WR
	port_out( port+2, (nRESET|nWR|A0) ^ OUTMASK );		// deactivate ^WR again
	port_out( port+2, (nRESET|nWR) ^ OUTMASK );		// clear A0 to indicate p

	for( i=0; i<datacount; i ++ ) {
		port_out( port, data[i] );			// set up data
		port_out( port+2, (nRESET) ^ OUTMASK );		// activate ^WR
		port_out( port+2, (nRESET|nWR) ^ OUTMASK );	// deactivate ^WR again
	}
}


/////////////////////////////////////////////////////////////////
// Close the display
//
MODULE_EXPORT void
sed1330_close( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	free( p );
}


/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
sed1330_width( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "%s()", __FUNCTION__ );

	return p->width;
}


/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
sed1330_height( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "%s()", __FUNCTION__ );

	return p->height;
}


/////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
MODULE_EXPORT void
sed1330_clear( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	memset( p->framebuf_text, ' ', p->bytesperline * p->textlines_in_memory);
	memset( p->framebuf_graph, 0, p->bytesperline * p->graph_height );
}


/////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
MODULE_EXPORT void
sed1330_string( Driver * drvthis, int x, int y, char *str )
{
	PrivateData * p = drvthis->private_data;
	char * dest;
	int offset, len;

	debug( RPT_DEBUG, "%s( x=%d, y=%d, str=\"%s\" )", __FUNCTION__, x, y, str );

	if( y < 1 || y > p->height ) {
		return; // outside framebuf
	}
	// Calculate offset and length to write
	if( x < 1 ) {
		offset = (1 - x);
		x = 1;
	} else {
		offset = 0;
	}
	len = strlen(str) - offset;
	if( len > p->width - x + 1 ) {
		len = p->width - x + 1;
	}
	// Calculate destination address
	dest = p->framebuf_text + (y-1)*p->bytesperline + (x-1);

	// And write
	memcpy( dest, str, len );
}


/////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
MODULE_EXPORT void
sed1330_chr( Driver * drvthis, int x, int y, char c )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s( x=%d, y=%d, c='%c' )", __FUNCTION__, x, y, c );

	if( y < 1 || y > p->height || x < 1 || x > p->width ) {
		return; // outside framebuf
	}
	p->framebuf_text[(y-1)*p->bytesperline + (x-1)] = c;
}


/////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
MODULE_EXPORT void
sed1330_flush( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;
	unsigned int pos, start_pos, nr_equal, fblen, len, cursor_pos;
	char csrloc[2];

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	/* sed1330_command( p, CMD_DISP_EN, 1, ((char[1]) {0x14}) ); // cursor off */
	// TODO: Flickering here needs to be prevented

	fblen = p->bytesperline * p->textlines_in_memory;
	for( pos=0; pos<fblen; ) {
		start_pos = pos;
		for( nr_equal=0; pos<fblen && nr_equal<4; pos++ ) {
			if( p->lcd_contents_text[pos] == p->framebuf_text[pos] ) {
				nr_equal ++;
			} else {
				nr_equal = 0;
			}
		}
		len = pos - start_pos - nr_equal;
		if( len > 0 ) {
			cursor_pos = start_pos + 256 * SCR1_H + SCR1_L;
			csrloc[0] = cursor_pos % 256;
			csrloc[1] = cursor_pos / 256;
			sed1330_command( p, CMD_CSRW, 2, csrloc );
			sed1330_command( p, CMD_MWRITE, len, p->framebuf_text + start_pos );
			memcpy( p->lcd_contents_text + start_pos, p->framebuf_text + start_pos, len );
		}
	}

	fblen = p->bytesperline * p->graph_height;
	for( pos=0; pos<fblen; ) {
		start_pos = pos;
		for( nr_equal=0; pos<fblen && nr_equal<4; pos++ ) {
			if( p->lcd_contents_graph[pos] == p->framebuf_graph[pos] ) {
				nr_equal ++;
			} else {
				nr_equal = 0;
			}
		}
		len = pos - start_pos - nr_equal;
		if( len > 0 ) {
			cursor_pos = start_pos + 256 * SCR2_H + SCR2_L;
			csrloc[0] = cursor_pos % 256;
			csrloc[1] = cursor_pos / 256;
			sed1330_command( p, CMD_CSRW, 2, csrloc );
			sed1330_command( p, CMD_MWRITE, len, p->framebuf_graph + start_pos );
			memcpy( p->lcd_contents_graph + start_pos, p->framebuf_graph + start_pos, len );
		}
	}

}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
MODULE_EXPORT void
sed1330_backlight( Driver * drvthis, int on )
{
	//PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s( on=%d )", __FUNCTION__, on );
	// unimplemented
}


/////////////////////////////////////////////////////////////////
// Draws a rectangle
// INTERNAL
//
void
sed1330_rect ( PrivateData * p, int x1, int y1, int x2, int y2, char pattern )
/* pattern: 0=clear 1=set     later more patterns ? */
{
	int x, y;

	// Swap coordinates if needed
	if( x1>x2 ) {
		int swap;
		swap=x1;
		x1=x2;
		x2=swap;
	}
	if( y1>y2 ) {
		int swap;
		swap=y1;
		y1=y2;
		y2=swap;
	}
	for( x=x1; x<=x2; x++ ) {
		for( y=y1; y<=y2; y++ ) {
			sed1330_set_pixel( p, x, y, pattern );
		}
	}
}


/////////////////////////////////////////////////////////////////
// Draws a line
// INTERNAL
//
void
sed1330_line ( PrivateData * p, int x1, int y1, int x2, int y2, char pattern )
/* pattern: 0=clear 1=set     later more patterns ? */
{
	int x, y;
	int more_x;

	/* Swap coordinates if needed. We want to draw the line from left
	 * to right.
	 */
	if( x1>x2 ) {
		int swap;
		swap=x1; x1=x2; x2=swap;
		swap=y1; y1=y2; y2=swap;
	}

	/* Draw from left to right... */
	more_x = 1;
	for( x=x1, y=y1; x<=x2; x++ ) {

		int more_y = 1; /* always draw one pixel */
		while( more_y ) {
			/* set the pixel */
			sed1330_set_pixel( p, x, y, pattern );

			/* Check what we need to do next */
			if( y1 < y2 ) {
				more_y = (y<=y2);
				if( x1 != x2 ) {
					more_y &= ((float)y+0.5-y1) < ((float) x+0.5-x1) * (y2-y1) / ((float) x2-x1)  ;
				}
			} else {
				more_y = (y>=y2);
				if( x1 != x2 ) {
					more_y &= ((float)y+0.5-y1) > ((float) x+0.5-x1) * (y2-y1) / ((float) x2-x1)  ;
				}
			}
			/* Increment y if we should draw a other pixel for this x value */
			if( more_y ) {
				if( y1 < y2 ) {
					y ++;
				} else {
					y --;
				}
			}
		}
	}
}


/////////////////////////////////////////////////////////////////
// Sets a specified pixel
// INTERNAL
//
inline void
sed1330_set_pixel( PrivateData * p, int x, int y, int value )
/* x, y are graph LCD coordinates, 0-based */
/* value: 0=clear 1=set */
{
	unsigned int bytepos;
	char bitmask;

	bytepos = y*p->bytesperline + x/p->cellwidth;
	bitmask = 0x80 >> (x % p->cellwidth);
	if( value ) {
		p->framebuf_graph[bytepos] |= bitmask; /* set it */
	} else {
		p->framebuf_graph[bytepos] &= ~bitmask; /* clear it */
	}
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar at the bottom
//
MODULE_EXPORT void
sed1330_vbar( Driver * drvthis, int x, int y, int len, int promille, int pattern )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", __FUNCTION__, x, y, len, promille, pattern );

	sed1330_rect ( p, (x-1) * p->cellwidth, y * p->cellheight, x * p->cellwidth - 2, y * p->cellheight - (long) len * p->cellheight * promille / 1000 - 1, 1 );
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right (len=pos)
// or to the left (len=neg)
//
MODULE_EXPORT void
sed1330_hbar( Driver * drvthis, int x, int y, int len, int promille, int pattern )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", __FUNCTION__, x, y, len, promille, pattern );

	sed1330_rect ( p, (x-1) * p->cellwidth, (y-1) * p->cellheight, (x-1) * p->cellwidth + (long) len * p->cellwidth * promille / 1000 - 1, y * p->cellheight - 3, 1 );
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
sed1330_num( Driver * drvthis, int x, int y, int num )
{
	//PrivateData * p = drvthis->private_data;

	debug( RPT_DEBUG, "%s( x=%d, y=%d, num=%d )", __FUNCTION__, x, y, num );

	// TODO: add code here :)
}


/////////////////////////////////////////////////////////////
// Does the heartbeat...
// Or in fact a bouncing ball :)
//
MODULE_EXPORT void
sed1330_heartbeat( Driver * drvthis, int type )
{
	PrivateData * p = drvthis->private_data;
	static int timer = 0;
	int pos;
	//int whichIcon;
	int n;

	//char heartdata[2][CHARHEIGHT] = {
	//	{ 0xFF, 0xFF, 0xAF, 0x07, 0x8F, 0xDF, 0xFF, 0xFF, 0x00, 0x00 },
	//	{ 0xFF, 0xAF, 0x07, 0x07, 0x07, 0x8F, 0xDF, 0xFF, 0x00, 0x00 }
	//};
	char bouncing_ball[8][8] = {
		{ 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF },
		{ 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF },
		{ 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF },
		{ 0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF },
		{ 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF },
		{ 0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF },
		{ 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF },
		{ 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF },
	};

	debug( RPT_DEBUG, "%s( type=%d )", __FUNCTION__, type );

	if( type == HEARTBEAT_OFF ) return;

	p->framebuf_text[p->width-1] = ' ';
	//whichIcon = (! ((timer + 4) & 5));

	pos = p->width - 1;
	for( n=0; n < p->cellheight; n++ ) {
		//p->framebuf_graph[pos] = heartdata[whichIcon][n];
		if( n < 8 ) {
			p->framebuf_graph[pos] = bouncing_ball[timer][n];
		} else {
			p->framebuf_graph[pos] = 0;
		}
		pos += p->bytesperline;
	}

	timer++;
	timer %= 8;
}


/////////////////////////////////////////////////////////////
// Place an icon
//
MODULE_EXPORT int
sed1330_icon( Driver * drvthis, int x, int y, int icon )
{
	switch( icon ) {
	  case ICON_BLOCK_FILLED:
		sed1330_chr( drvthis, x, y, 255 );
		break;
	  default:
		return -1;
	}
	return 0;
}


/////////////////////////////////////////////////////////////
// Get a key from the keypad (if there is one)
//
MODULE_EXPORT char *
sed1330_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char scancode;
	char * keystr = NULL;
	struct timeval curr_time, time_diff;

	if( ! p->have_keypad ) return NULL;

	gettimeofday(&curr_time,NULL);

	scancode = sed1330_scankeypad(p);
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
			report( RPT_INFO, "sed1330_get_key: Key pressed: %s  (%d,%d)\n", keystr, scancode&0x0F, (scancode&0xF0)>>4 );
		}
	}

	// Store the key for the next round
	p->pressed_key = keystr;

	return keystr;
}


/////////////////////////////////////////////////////////////
// Scan the keypad
//
// Called by get_key
//
unsigned char sed1330_scankeypad(PrivateData *p)
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
	keybits = sed1330_readkeypad (p, 0);

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
		if( sed1330_readkeypad (p, Ypattern)) {
			// Yes, a key on the matrix is pressed

			// OK, now we know a key is pressed.
			// Determine which one it is

			// First determine the row
			// Do a 'binary search' to minimize I/O
			Ypattern = 0;
			Yval = 0;
			for (exp=3; exp>=0; exp--) {
				Ypattern = ((1 << (1 << exp)) - 1) << Yval;
				keybits = sed1330_readkeypad (p, Ypattern);
				if (!keybits) {
					Yval += (1 << exp);
				}
			}

			// Which key is pressed in that row ?
			keybits = sed1330_readkeypad (p, 1<<Yval);
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
// Read a keypad byte
//
// Called by scankeypad
//
unsigned char sed1330_readkeypad (PrivateData *p, unsigned int YData)
{
	unsigned char readval;

	// 8 bits output
	// Convert the positive logic to the negative logic on the LPT port
	port_out (p->port, ~YData & 0x00FF );

	// Read inputs
	readval = ~ port_in (p->port + 1) ^ INMASK;

	// And convert value back (MSB first).
	return (((readval & FAULT) / FAULT <<4) |		/* pin 15 */
		((readval & SELIN) / SELIN <<3) |		/* pin 13 */
		((readval & PAPEREND) / PAPEREND <<2) |		/* pin 12 */
		((readval & BUSY) / BUSY <<1) |			/* pin 11 */
		((readval & ACK) / ACK )) & ~p->stuckinputs;	/* pin 10 */
}

