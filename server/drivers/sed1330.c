/*
 * Driver for SED1330 graphical displays
 *
 * This driver drives the LCD in text mode.
 * Probably the driver can easily be adapted to work for 1335 and 1336 too.
 *
 * Moved the delay timing code by Charles Steinkuehler to timing.h.
 * Guillaume Filion <gfk@logidac.com>, December 2001
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn <joris@robijn.net
 *
 *
 * Changelog:
 *
 * November 2001, Joris Robijn:
 * - Created the driver
 * - Parts copied from HD44780 driver
 * December 2001, Joris Robijn:
 * - Adapted to v0.5 API
 *
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
 *					14	^LF
 *	D0		8		2	D0
 *					15	^ERROR
 *	D1		9		3	D1
 *	^WR		3		16	^INIT
 *	D2		10		4	D2
 *	A0		7		17	^SELECT_IN
 *	D3		11		5	D3
 *	Vss		17	GND	18..25	GND
 *	D4		12		6	D4
 *	D5		13		7	D5
 *	D6		14		8	D6
 *	D7		15		9	D7
 *					10	^ACK
 *				GND	11	BUSY
 *					12	PAPEREND
 *					13	^SELECT
 *	Vdd		16	+5V
 *	^RD		2	+5V
 *	Frame		20	GND
 *	SEL1		4	GND
 *	SEL2		5	GND
 *	^CS		6	GND
 *	Vlc		19	-24V (not required for G242C)
 *	V0		18	potmeter
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
 *
 * The port= value should be set to the LPT port address that the LCD is
 * connected to. Examples:
 * port=0x378
 * port=0x278
 * port=0x3BC
 *
 */

#include "lcd.h"
#include "sed1330.h"

#include "port.h"
#include "lpt-port.h"
#include "shared/str.h"
#include "shared/report.h"
#include "configfile.h"
#include "timing.h"
#define uPause timing_uPause

#include <string.h>
#include <errno.h>
#include <stdlib.h>

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
#define MAXKEYS 8
#define TYPE_G321D 1
#define TYPE_G121C 2
#define TYPE_G242C 3

#define SCR1_L 0x00 // Memory locations
#define SCR1_H 0x00
#define SCR2_L 0x00
#define SCR2_H 0x06

#define CHARWIDTH 6
#define CHARHEIGHT 10
#define PIXELSPERBYTE CHARWIDTH
// The above should be (CHARWIDTH/2) if CHARWIDTH > 8

typedef struct p {
	int type;
	int port;
	char * keymap[MAXKEYS];
	char * framebuf_text;
	char * lcd_contents_text;
	char * framebuf_graph;
	char * lcd_contents_graph;
	int width, height;
	//int cellwidth, cellheight;
	int graph_width, graph_height;
	int cursor_x, cursor_y;
	char cursor_state;
	int bytesperline;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 1; // yes, we have no global variables (except for constants)
MODULE_EXPORT char *symbol_prefix = "sed1330_";


// Local functions
void uPause (int usecs);
void sed1330_command( PrivateData * p, char command, int datacount, char * data );
void sed1330_update_cursor( PrivateData * p );
void sed1330_rect( PrivateData * p, int x1, int y1, int x2, int y2, char pattern );
inline void sed1330_set_pixel( PrivateData * p, int x, int y );
inline void sed1330_clear_pixel( PrivateData * p, int x, int y );


/////////////////////////////////////////////////////////////////
// Init the driver and display
//
int
sed1330_init( Driver * drvthis, char *args )
{
	char * s;
	int port, i;
	PrivateData * p;

	debug( RPT_INFO, "SED1330: init(%p,%s)", drvthis, args );

	// TODO: replace DriverName with driver->name when that field exists.


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
	port = config_get_int( drvthis->name, "port", 0, 0x278 );
	p->port = port;

	// Type
	s = drvthis->config_get_string( drvthis->name, "type", 0, NULL );
	if( !s ) {
		report( RPT_ERR, "SED1330: you need to specify the display type" );
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
	} else {
		report( RPT_ERR, "SED1330: Unknown display type: %s", s );
		return -1;
	}
	p->width = p->graph_width / CHARWIDTH;
	p->height = p->graph_height / CHARHEIGHT;
	p->bytesperline = (p->graph_width - 1 ) / CHARWIDTH + 1;

	report( RPT_INFO, "SED1330: Using LCD type: %s", s );
	report( RPT_INFO, "SED1330: Text size: %dx%d", p->width, p->height );


	// Keymap
	for( i=0; i<MAXKEYS; i++ ) {
		char buf[8];
		sprintf( buf, "key_%1d", i );
		s = drvthis->config_get_string( drvthis->name, buf, 0, NULL );
		if( s ) {
			p->keymap[i] = (char *) malloc( strlen(s)+1 );
			strcpy( p->keymap[i], s );
			report( RPT_INFO, "SED1330: Key %d: \"%s\"", i, s );

		} else {
			p->keymap[i] = ""; // Pointing to an constant empty string
		}
	}

	// Init cursor p
	p->cursor_x = 1;
	p->cursor_y = 1;
	p->cursor_state = 1;

	// Allocate framebuffer
	p->framebuf_text = (unsigned char *) malloc( p->bytesperline * p->height );
	if( ! p->framebuf_text  )
		return -1;
	memset( p->framebuf_text, ' ', p->bytesperline * p->height);

	p->lcd_contents_text = (unsigned char *) malloc( p->bytesperline * p->height );
	if( ! p->lcd_contents_text  )
		return -1;
	memset( p->lcd_contents_text, 0, p->bytesperline * p->height);

	p->framebuf_graph = (unsigned char *) malloc( p->bytesperline * p->graph_height );
	if( ! p->framebuf_graph  )
		return -1;
	memset( p->framebuf_graph, 0, p->bytesperline * p->graph_height );

	p->lcd_contents_graph = (unsigned char *) malloc( p->bytesperline * p->graph_height );
	if( ! p->lcd_contents_graph  )
		return -1;
	memset( p->lcd_contents_graph, 0xFF, p->bytesperline * p->graph_height );


	// Arrange for access to port
	debug( RPT_DEBUG, "SED1330: getting port access" );
	port_access(p->port);
	port_access(p->port+1);
	port_access(p->port+2);

	if (timing_init() == -1)
		return -1;

	// Set variables for server
	drvthis->stay_in_foreground = &stay_in_foreground;
	drvthis->api_version = api_version;
	drvthis->supports_multiple = &supports_multiple;

	// Set the functions the driver supports...
	drvthis->init = sed1330_init;
	drvthis->close = sed1330_close;

	drvthis->width = sed1330_width;
	drvthis->height = sed1330_height;

	drvthis->flush = sed1330_flush;
	drvthis->clear = sed1330_clear;
	drvthis->chr = sed1330_chr;
	drvthis->string = sed1330_string;
	//drvthis->init_vbar = sed1330_init_vbar;
	//drvthis->init_hbar = sed1330_init_hbar;
	drvthis->vbar = sed1330_vbar;
	drvthis->hbar = sed1330_hbar;
	//drvthis->init_num = sed1330_init_num;
	drvthis->num = sed1330_num;
	drvthis->heartbeat = sed1330_heartbeat;
	//drvthis->set_char = sed1330_set_char;
	//drvthis->icon = sed1330_icon;

	// drvthis->contrast = sed1330_contrast; // contrast is set by potmeter we assume
	// drvthis->output = sed1330_output; // not implemented

	// drvthis->flush_box = sed1330_flush_box; // NOT SUPPORTED ANYMORE
	// drvthis->draw_frame = sed1330_draw_frame; // NOT SUPPORTED ANYMORE


	// INITIALIZE THE LCD
	// End reset-state
	debug( RPT_DEBUG, "SED1330: initializing LCD" );
	port_out( port+2, (nWR) ^ OUTMASK );	// raise ^RD and ^WR
	port_out( port+2, (nRESET|nWR) ^ OUTMASK );	// lower RESET
	uPause( 200 );
	port_out( port+2, (nWR) ^ OUTMASK );	// raise RESET
	uPause( 200 );
	port_out( port+2, (nRESET|nWR) ^ OUTMASK );	// lower RESET
	uPause( 3000 );

	switch( p->type ) {
	  case TYPE_G321D:
		sed1330_command( p, CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x34,0x38,0xC7,0x36,0x00}) );	// Set textmode 53x20
		sed1330_command( p, CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  case TYPE_G121C:
		sed1330_command( p, CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x14,0x18,0x7F,0x16,0x00}) );	// Set textmode 21x12
		sed1330_command( p, CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  case TYPE_G242C:
		sed1330_command( p, CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x27,0x2B,0x7F,0x29,0x00}) );	// Set textmode 40x12
		sed1330_command( p, CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  default:
		return -1;
	}
	sed1330_command( p, CMD_CSR_FORM, 2, ((char[2]) {0x04,0x07}) );	// 5x8 cursor
	sed1330_command( p, CMD_HDOT_SCR, 1, ((char[1]) {0x00}) );	// horizontal pixel shift=0
	sed1330_command( p, CMD_OVLAY, 1, ((char[1]) {0x01}) );		// XOR mode, screen1 text, screen3 text (screen2 and screen4 are always graph)
	sed1330_command( p, CMD_DISP_DIS, 1, ((char[1]) {0x17}) );	// display off, set cursor slow, screen1 on, screen2 on, screen3 off
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
	//uPause( 1 );
	port_out( port+2, (nRESET|A0) ^ OUTMASK );		// activate ^WR
	uPause( 1 );
	port_out( port+2, (nRESET|nWR|A0) ^ OUTMASK );		// deactivate ^WR again


	port_out( port+2, (nRESET|nWR) ^ OUTMASK );		// clear A0 to indicate p

	for( i=0; i<datacount; i ++ ) {

		port_out( port, data[i] );			// set up data
		//uPause( 1 );
		port_out( port+2, (nRESET) ^ OUTMASK );		// activate ^WR
		uPause( 1 );
		port_out( port+2, (nRESET|nWR) ^ OUTMASK );	// deactivate ^WR again
	}
}


/////////////////////////////////////////////////////////////////
// Update cursor showing
// INTERNAL
//
void sed1330_update_cursor( PrivateData * p )
{
	int cursor_pos;
	char csrloc[2];
	char csrform[2];
	char disp_en;
	char fc = 0; // named after FC register in SED1330

	cursor_pos = (p->cursor_y-1) * p->bytesperline + (p->cursor_x-1) + 256 * SCR1_H + SCR1_L;
	csrloc[0] = cursor_pos % 256;
	csrloc[1] = cursor_pos / 256;

	switch( p->cursor_state ) {
	  case 0: // Off
		fc = 0;
		csrform[0] = 0x04;
		csrform[1] = 0x07;
		break;
	  case 1: // Underlined
		fc = 2;
		csrform[0] = 0x04;
		csrform[1] = 0x07;
		break;
	  case 2: // Block
		fc = 2;
		break;
		csrform[0] = 0x84;
		csrform[1] = 0x07;
	}
	disp_en = 0x14 + fc;

	sed1330_command( p, CMD_CSRW, 2, csrloc );
	sed1330_command( p, CMD_DISP_EN, 1, &disp_en ); // cursor on
	sed1330_command( p, CMD_CSR_FORM, 2, csrform );	// 5x8 cursor
}


/////////////////////////////////////////////////////////////////
// Close the display
//
MODULE_EXPORT void
sed1330_close( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_close" );

	free( p );
}


/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
sed1330_width( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	return p->width;
}


/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
sed1330_height( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	return p->height;
}


/////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
MODULE_EXPORT void
sed1330_clear( Driver * drvthis )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_clear" );

	memset( p->framebuf_text, ' ', p->bytesperline * p->height);
	memset( p->framebuf_graph, 0, p->bytesperline * p->graph_height );
}


/////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
MODULE_EXPORT void
sed1330_string( Driver * drvthis, int x, int y, char *str )
{
	PrivateData * p = drvthis->private_data;
	char * start;
	int len;

	debug( RPT_INFO, "sed1330_string x=%d y=%d s=\"%s\"", x, y, str );

	if( y > p->height ) {
		return; // outside framebuf_textfer
	}
	// Calculate where to start and length to write
	start = p->framebuf_text + (y-1)*p->bytesperline + (x-1);
	len = strlen(str);
	if( p->width < len ) {
		len = p->width;
	}

	memcpy( start, str, len );
}


/////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
MODULE_EXPORT void
sed1330_chr( Driver * drvthis, int x, int y, char c )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_chr x=%d y=%d c='%c'", x, y, c );

	if( y > p->height || x > p->width ) {
		return; // outside framebuf_textfer
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

	debug( RPT_INFO, "sed1330_flush" );

	sed1330_command( p, CMD_DISP_EN, 1, ((char[1]) {0x16}) ); // cursor off

	fblen = p->bytesperline * p->height;
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


	sed1330_update_cursor( p );
}


/////////////////////////////////////////////////////////////////
// Let the cursor go to a certain location
//
MODULE_EXPORT void
sed1330_cursor( Driver * drvthis, int x, int y, char state )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_cursor x=%d y=%d state='%c'", x, y, state );

	p->cursor_x = x;
	p->cursor_y = y;
	p->cursor_state = state;

	sed1330_update_cursor( p );
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
MODULE_EXPORT void
sed1330_backlight( Driver * drvthis, int on )
{
	//PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_backlight on='%c'", on );
	// unimplemented
}


/////////////////////////////////////////////////////////////////
// Draws a rectangle
// INTERNAL
//
void
sed1330_rect ( PrivateData * p, int x1, int y1, int x2, int y2, char pattern )
// pattern: 0=empty 1=filled     later more patterns ?
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
			sed1330_set_pixel( p, x, y );
		}
	}
}


/////////////////////////////////////////////////////////////////
// Draws a line
// INTERNAL
//
void
sed1330_line ( PrivateData * p, int x1, int y1, int x2, int y2, char pattern )
// pattern: 0=empty 1=filled     later more patterns ?
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
	// Do we have an angle of more or less than 45 degrees ?
	if( x2-x1 >= y2-y1 ) {
		// Mostly horizontal
		for( x=x1; x<=x2; x++ ) {
			y = x * (y2-y1) / (x2-x1);
			switch( pattern ) {
			  case 0:
				sed1330_clear_pixel( p, x, y );
				break;
			  case 1:
				sed1330_set_pixel( p, x, y );
				break;
			}
		}
	}
	else {
		// Mostly vertical
		for( y=y1; y<=y2; y++ ) {
			x = y * (x2-x1) / (y2-y1) ;
			switch( pattern ) {
			  case 0:
				sed1330_clear_pixel( p, x, y );
				break;
			  case 1:
				sed1330_set_pixel( p, x, y );
				break;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////
// Sets a specified pixel
// INTERNAL
//
inline void
sed1330_set_pixel( PrivateData * p, int x, int y )
// x, y are graph LCD coordinates, 0-based
{
	unsigned int bytepos;
	char bitmask;

	bytepos = y*p->bytesperline + x/PIXELSPERBYTE;
	bitmask = 0x80 >> (x % PIXELSPERBYTE);
	p->framebuf_graph[bytepos] |= bitmask;
}


/////////////////////////////////////////////////////////////////
// clears a specified pixel
// INTERNAL
//
inline void
sed1330_clear_pixel( PrivateData * p, int x, int y )
// x, y are graph LCD coordinates, 0-based
{
	int bytepos;
	char bitmask;

	bytepos = y*p->bytesperline + x/PIXELSPERBYTE;
	bitmask = 0x80 >> (x % PIXELSPERBYTE);
	p->framebuf_graph[bytepos] &= ~bitmask;
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar at the bottom
//
MODULE_EXPORT void
sed1330_vbar( Driver * drvthis, int x, int y, int len, int promille, int pattern )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_hbar x=%d len=%d", x, len );

	sed1330_rect ( p, (x-1) * CHARWIDTH, y * CHARHEIGHT - CHARHEIGHT/2, x * CHARWIDTH - 1, y * CHARHEIGHT - CHARHEIGHT/2 + (long) len * CHARHEIGHT * promille / 1000 - 1, 1 );
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right (len=pos)
// or to the left (len=neg)
//
MODULE_EXPORT void
sed1330_hbar( Driver * drvthis, int x, int y, int len, int promille, int pattern )
{
	PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_hbar x=%d y=%d len=%d", x, y, len );

	sed1330_rect ( p, x * CHARWIDTH, (y-1) * CHARHEIGHT, x * CHARWIDTH  - CHARWIDTH/2 + (long) len * CHARWIDTH * promille / 1000 - 1, y * CHARHEIGHT - 1, 1 );
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
sed1330_num( Driver * drvthis, int x, int num )
{
	//PrivateData * p = drvthis->private_data;

	debug( RPT_INFO, "sed1330_bignum x=%d num=%d", x, num );

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
	char bouncing_ball[8][CHARHEIGHT] = {
		{ 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0x00, 0x00 },
		{ 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0x00, 0x00 },
	};

	report( RPT_INFO, "sed1330_heartbeat type=%d", type );

	p->framebuf_text[p->width-1] = ' ';
	//whichIcon = (! ((timer + 4) & 5));

	pos = p->width - 1;
	for( n=0; n<CHARHEIGHT; n++ ) {
		//p->framebuf_graph[pos] = heartdata[whichIcon][n];
		p->framebuf_graph[pos] = bouncing_ball[timer][n];
		pos += p->bytesperline;
	}

	timer++;
	timer %= 8;
}
