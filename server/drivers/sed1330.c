/*
 * Driver for SED1330 graphical displays
 *
 * This driver drives the LCD in text mode.
 * Probably the driver can easily be adapted to work for 1335 and 1336 too.
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

#include "sed1330.h"
#include "port.h"
#include "lpt-port.h"
#include "lcd.h"
#include "shared/str.h"
#include "shared/report.h"
#include "configfile.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

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

typedef struct private_data {
	int type;
	int port;
	char * keymap[MAXKEYS];
	char * framebuf_text;
	char * lcd_contents_text;
	char * framebuf_graph;
	char * lcd_contents_graph;
	int graph_width, graph_height;
	int cursor_x, cursor_y;
	char cursor_state;
	int bytesperline;
} private_data;


// Local functions
void uPause (int usecs);
void sed1330_command( char command, int datacount, char * data );
void sed1330_update_cursor();
void sed1330_rect ( int x1, int y1, int x2, int y2, char pattern );
inline void sed1330_set_pixel( int x, int y );
inline void sed1330_clear_pixel( int x, int y );


// Global vars
static lcd_logical_driver * sed1330;

/////////////////////////////////////////////////////////////////
// Init the driver and display
//
int
sed1330_init( lcd_logical_driver * driver, char *args )
{
	char * s;
	int port, i;
	private_data * data;

	sed1330 = driver;


	debug( RPT_INFO, "SED1330: init(%p,%s)", driver, args );

	// TODO: replace DriverName with driver->name when that field exists.
	#define DriverName "sed1330"


	// Alocate and store private data
	data = (private_data *) malloc( sizeof( private_data) );
	if( ! data )
		return -1;
	if( driver->store_private_ptr( driver, data ) )
		return -1;

	// Clear keymap
	memset( data->keymap, 0, sizeof(data->keymap) );

	// READ THE CONFIG FILE

	// Port
	port = config_get_int( DriverName, "port", 0, 0x278 );
	data->port = port;

	// Type
	s = driver->config_get_string( DriverName, "type", 0, NULL );
	if( !s ) {
		report( RPT_ERR, "SED1330: you need to specify the display type" );
	} else if( strcmp( s, "G321D" ) == 0 ) {
		data->type = TYPE_G321D;
		data->graph_width = 320;
		data->graph_height = 200;
	} else if( strcmp( s, "G121C" ) == 0 ) {
		data->type = TYPE_G121C;
		data->graph_width = 128;
		data->graph_height = 128;
	} else if( strcmp( s, "G242C" ) == 0 ) {
		data->type = TYPE_G242C;
		data->graph_width = 240;
		data->graph_height = 128;
	} else {
		report( RPT_ERR, "SED1330: Unknown display type: %s", s );
		return -1;
	}
	driver->wid = data->graph_width / CHARWIDTH;
	driver->hgt = data->graph_height / CHARHEIGHT;
	data->bytesperline = (data->graph_width - 1 ) / CHARWIDTH + 1;

	report( RPT_INFO, "SED1330: Using LCD type: %s", s );
	report( RPT_INFO, "SED1330: Text size: %dx%d", driver->wid, driver->hgt );


	// Keymap
	for( i=0; i<MAXKEYS; i++ ) {
		char buf[8];
		sprintf( buf, "key_%1d", i );
		s = driver->config_get_string( DriverName, buf, 0, NULL );
		if( s ) {
			data->keymap[i] = (char *) malloc( strlen(s)+1 );
			strcpy( data->keymap[i], s );
			report( RPT_INFO, "SED1330: Key %d: \"%s\"", i, s );

		} else {
			data->keymap[i] = ""; // Pointing to an constant empty string
		}
	}

	// Init cursor data
	data->cursor_x = 1;
	data->cursor_y = 1;
	data->cursor_state = 1;

	// Allocate framebuffer
	data->framebuf_text = (unsigned char *) malloc( data->bytesperline * driver->hgt );
	if( ! data->framebuf_text  )
		return -1;
	memset( data->framebuf_text, ' ', data->bytesperline * driver->hgt);

	data->lcd_contents_text = (unsigned char *) malloc( data->bytesperline * driver->hgt );
	if( ! data->lcd_contents_text  )
		return -1;
	memset( data->lcd_contents_text, 0, data->bytesperline * driver->hgt);

	data->framebuf_graph = (unsigned char *) malloc( data->bytesperline * data->graph_height );
	if( ! data->framebuf_graph  )
		return -1;
	memset( data->framebuf_graph, 0, data->bytesperline * data->graph_height );

	data->lcd_contents_graph = (unsigned char *) malloc( data->bytesperline * data->graph_height );
	if( ! data->lcd_contents_graph  )
		return -1;
	memset( data->lcd_contents_graph, 0xFF, data->bytesperline * data->graph_height );


	// Arrange for access to port
	debug( RPT_DEBUG, "SED1330: getting port access" );
	port_access(data->port);
	port_access(data->port+1);
	port_access(data->port+2);


#if defined DELAY_NANOSLEEP
	// Change to Round-Robin scheduling for nanosleep
	{
		// Set priority to 1
		struct sched_param param;
		param.sched_priority=1;
		if (( sched_setscheduler(0, SCHED_RR, &param)) == -1) {
			report( RPT_ERR, "SED1330: init failed (%s)", strerror (errno));
			return -1;
		}
	}
#endif

	// Set the functions the driver supports...
	driver->init = sed1330_init;
	driver->close = sed1330_close;
	driver->flush = sed1330_flush;
	driver->clear = sed1330_clear;
	driver->chr = sed1330_chr;
	driver->string = sed1330_string;
	//driver->init_vbar = sed1330_init_vbar;
	//driver->init_hbar = sed1330_init_hbar;
	driver->vbar = sed1330_vbar;
	driver->hbar = sed1330_hbar;
	//driver->init_num = sed1330_init_num;
	driver->num = sed1330_num;
	driver->heartbeat = sed1330_heartbeat;
	//driver->set_char = sed1330_set_char;
	//driver->icon = sed1330_icon;

	// driver->contrast = sed1330_contrast; // contrast is set by potmeter we assume
	// driver->output = sed1330_output; // not implemented

	// driver->flush_box = sed1330_flush_box; // NOT SUPPORTED ANYMORE
	// driver->draw_frame = sed1330_draw_frame; // NOT SUPPORTED ANYMORE


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

	switch( data->type ) {
	  case TYPE_G321D:
		sed1330_command( CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x34,0x38,0xC7,0x36,0x00}) );	// Set textmode 53x20
		sed1330_command( CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  case TYPE_G121C:
		sed1330_command( CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x14,0x18,0x7F,0x16,0x00}) );	// Set textmode 21x12
		sed1330_command( CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  case TYPE_G242C:
		sed1330_command( CMD_SYSTEM_SET, 8, ((char[8]) {0x30,0x80+CHARWIDTH-1,CHARHEIGHT-1,0x27,0x2B,0x7F,0x29,0x00}) );	// Set textmode 40x12
		sed1330_command( CMD_SCROLL, 10, ((char[6]) {SCR1_L,SCR1_H,0xC7,SCR2_L,SCR2_H,0xC7}) );		// screen1 and screen2 memory locations
		break;
	  default:
		return -1;
	}
	sed1330_command( CMD_CSR_FORM, 2, ((char[2]) {0x04,0x07}) );	// 5x8 cursor
	sed1330_command( CMD_HDOT_SCR, 1, ((char[1]) {0x00}) );		// horizontal pixel shift=0
	sed1330_command( CMD_OVLAY, 1, ((char[1]) {0x01}) );		// XOR mode, screen1 text, screen3 text (screen2 and screen4 are always graph)
	sed1330_command( CMD_DISP_DIS, 1, ((char[1]) {0x17}) );		// display off, set cursor slow, screen1 on, screen2 on, screen3 off
	sed1330_command( CMD_CSR_DIR_R, 0, NULL );			// cursor move right

	sed1330_flush(); // Clear the contents of the LCD

	sed1330_command( CMD_DISP_EN, 0, NULL );	// And display on

	return 0;
}


/////////////////////////////////////////////////////////////////
// Pause a specified number of milliseconds
// INTERNAL
//
void
uPause (int delay)
{

#if defined DELAY_GETTIMEOFDAY
	struct timeval current_time,delay_time,wait_time;

	// Get current time first thing
	gettimeofday(&current_time,NULL);

	// Calculate when delay is over
	delay_time.tv_sec  = 0;
	delay_time.tv_usec = delay;
	timeradd(&current_time,&delay_time,&wait_time);

	do {
		gettimeofday(&current_time,NULL);
	} while (timercmp(&current_time,&wait_time,<));

#elif defined DELAY_NANOSLEEP
	struct timespec delay_time,remaining;

	delay_time.tv_sec = 0;
	delay_time.tv_nsec = delay * 1000;
	while ( nanosleep(&delay_time,&remaining) == -1 )
	{
		delay_time.tv_sec  = remaining.tv_sec;
		delay_time.tv_nsec = remaining.tv_nsec;
	}
#else // using I/O timing
      // Assuming every call takes 1us (which can be quite incorrect)
	int i;
	for (i = 0; i < delay; ++i)
		port_in (port);
#endif
}


/////////////////////////////////////////////////////////////////
// Send a command and accompanying data
// INTERNAL
//
void
sed1330_command( char command, int datacount, char * data )
{
	private_data * private_data = sed1330->private_data;
	int i;
	int port = private_data->port;

	debug( RPT_INFO, "sed1330_command %x #data=%d", command, datacount );

	port_out( port+2, (nRESET|nWR|A0) ^ OUTMASK );	// set A0 to indicate command
	port_out( port, command );		// set up data
	//uPause( 1 );
	port_out( port+2, (nRESET|A0) ^ OUTMASK );		// activate ^WR
	uPause( 1 );
	port_out( port+2, (nRESET|nWR|A0) ^ OUTMASK );	// deactivate ^WR again


	port_out( port+2, (nRESET|nWR) ^ OUTMASK );		// clear A0 to indicate data

	for( i=0; i<datacount; i ++ ) {

		port_out( port, data[i] );	// set up data
		//uPause( 1 );
		port_out( port+2, (nRESET) ^ OUTMASK );		// activate ^WR
		uPause( 1 );
		port_out( port+2, (nRESET|nWR) ^ OUTMASK );		// deactivate ^WR again
	}
}


/////////////////////////////////////////////////////////////////
// Update cursor showing
// INTERNAL
//
void sed1330_update_cursor()
{
	private_data * data = sed1330->private_data;
	int cursor_pos;
	char csrloc[2];
	char csrform[2];
	char disp_en;
	char fc; // named after CF register in SED1330

	debug( RPT_INFO, "sed1330_update_cursor" );

	cursor_pos = (data->cursor_y-1) * data->bytesperline + (data->cursor_x-1) + 256 * SCR1_H + SCR1_L;
	csrloc[0] = cursor_pos % 256;
	csrloc[1] = cursor_pos / 256;

	switch( data->cursor_state ) {
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

	sed1330_command( CMD_CSRW, 2, csrloc );
	sed1330_command( CMD_DISP_EN, 1, &disp_en ); // cursor on
	sed1330_command( CMD_CSR_FORM, 2, csrform );	// 5x8 cursor
}


/////////////////////////////////////////////////////////////////
// Close the display
//
void
sed1330_close()
{
	debug( RPT_INFO, "sed1330_close" );

	//sed1330_command( CMD_DISP_DIS, 0, NULL ); // display off
	//port_out( port+2, (nWR) ^ OUTMASK ); // give LCD reset signal
					// LCD now will not respond anymore

	// We should take the -24V away before removing the 5V !
}


/////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
void
sed1330_clear()
{
	private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_clear" );

	memset( data->framebuf_text, ' ', data->bytesperline * sed1330->hgt);
	memset( data->framebuf_graph, 0, data->bytesperline * data->graph_height );
}


/////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
void
sed1330_string( int x, int y, char *str )
{
	private_data * data = sed1330->private_data;
	char * start;
	int len;

	debug( RPT_INFO, "sed1330_string x=%d y=%d s=\"%s\"", x, y, str );

	if( y > sed1330->hgt ) {
		return; // outside framebuf_textfer
	}
	// Calculate where to start and length to write
	start = data->framebuf_text + (y-1)*data->bytesperline + (x-1);
	len = strlen(str);
	if( sed1330->wid < len ) {
		len = sed1330->wid;
	}

	memcpy( start, str, len );
}


/////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
void
sed1330_chr( int x, int y, char c )
{
	private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_chr x=%d y=%d c='%c'", x, y, c );

	if( y > sed1330->hgt || x > sed1330->wid ) {
		return; // outside framebuf_textfer
	}
	data->framebuf_text[(y-1)*data->bytesperline + (x-1)] = c;
}


/////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
void
sed1330_flush()
{
	private_data * data = sed1330->private_data;
	unsigned int pos, start_pos, nr_equal, fblen, len, cursor_pos;
	char csrloc[2];

	debug( RPT_INFO, "sed1330_flush" );

	sed1330_command( CMD_DISP_EN, 1, ((char[1]) {0x16}) ); // cursor off

	fblen = data->bytesperline * sed1330->hgt;
	for( pos=0; pos<fblen; ) {
		start_pos = pos;
		for( nr_equal=0; pos<fblen && nr_equal<4; pos++ ) {
			if( data->lcd_contents_text[pos] == data->framebuf_text[pos] ) {
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
			sed1330_command( CMD_CSRW, 2, csrloc );
			sed1330_command( CMD_MWRITE, len, data->framebuf_text + start_pos );
			memcpy( data->lcd_contents_text + start_pos, data->framebuf_text + start_pos, len );
		}
	}

	fblen = data->bytesperline * data->graph_height;
	for( pos=0; pos<fblen; ) {
		start_pos = pos;
		for( nr_equal=0; pos<fblen && nr_equal<4; pos++ ) {
			if( data->lcd_contents_graph[pos] == data->framebuf_graph[pos] ) {
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
			sed1330_command( CMD_CSRW, 2, csrloc );
			sed1330_command( CMD_MWRITE, len, data->framebuf_graph + start_pos );
			memcpy( data->lcd_contents_graph + start_pos, data->framebuf_graph + start_pos, len );
		}
	}


	sed1330_update_cursor();
}


/////////////////////////////////////////////////////////////////
// Let the cursor go to a certain location
//
void sed1330_cursor( int x, int y, char state )
{
	private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_cursor x=%d y=%d state='%c'", x, y, state );

	data->cursor_x = x;
	data->cursor_y = y;
	data->cursor_state = state;

	sed1330_update_cursor();
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
void
sed1330_backlight( int on )
{
	//private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_backlight on='%c'", on );
	// unimplemented
}


/////////////////////////////////////////////////////////////////
// Draws a rectangle
// INTERNAL
//
void
sed1330_rect ( int x1, int y1, int x2, int y2, char pattern )
// pattern: 0=empty 1=filled     later more patterns ?
{
	//private_data * data = sed1330->private_data;
	int x, y;

	debug( RPT_INFO, "sed1330_rect x1=%d y1=%d x2=%d y2=%d pattern=%d", x1, y1, x2, y2, (int) pattern );

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
			sed1330_set_pixel( x, y );
		}
	}
}


/////////////////////////////////////////////////////////////////
// Draws a rectangle
// INTERNAL
//
void
sed1330_line ( int x1, int y1, int x2, int y2, char pattern )
// pattern: 0=empty 1=filled     later more patterns ?
{
	//private_data * data = sed1330->private_data;
	int x, y;

	debug( RPT_INFO, "sed1330_rect x1=%d y1=%d x2=%d y2=%d pattern=%d", x1, y1, x2, y2, (int) pattern );

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
				sed1330_clear_pixel( x, y );
				break;
			  case 1:
				sed1330_set_pixel( x, y );
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
				sed1330_clear_pixel( x, y );
				break;
			  case 1:
				sed1330_set_pixel( x, y );
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
sed1330_set_pixel( int x, int y )
// x, y are graph LCD coordinates, 0-based
{
	private_data * data = sed1330->private_data;
	unsigned int bytepos;
	char bitmask;

	//debug( RPT_INFO, "sed1330_set_pixel x=%d y=%d", x, y );

	bytepos = y*data->bytesperline + x/PIXELSPERBYTE;
	bitmask = 0x80 >> (x % PIXELSPERBYTE);
	data->framebuf_graph[bytepos] |= bitmask;
}


/////////////////////////////////////////////////////////////////
// clears a specified pixel
// INTERNAL
//
inline void
sed1330_clear_pixel( int x, int y )
// x, y are graph LCD coordinates, 0-based
{
	private_data * data = sed1330->private_data;
	int bytepos;
	char bitmask;

	//debug( RPT_INFO, "sed1330_clear_pixel x=%d y=%d", x, y );

	bytepos = y*data->bytesperline + x/PIXELSPERBYTE;
	bitmask = 0x80 >> (x % PIXELSPERBYTE);
	data->framebuf_graph[bytepos] &= ~bitmask;
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar at the bottom
//
void
sed1330_vbar( int x, int len )
{
	private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_hbar x=%d len=%d", x, len );

	sed1330_rect ( (x-1) * CHARWIDTH, data->graph_height-1, x * CHARWIDTH - 1, data->graph_height-1 - ((long) len * CHARHEIGHT / sed1330->cellhgt ), 1 );
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right (len=pos)
// or to the left (len=neg)
//
void
sed1330_hbar( int x, int y, int len )
{
	//private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_hbar x=%d y=%d len=%d", x, y, len );

	sed1330_rect ( (x-1) * CHARWIDTH, (y-1) * CHARHEIGHT, x * CHARWIDTH + len, y * CHARHEIGHT - 1, 1 );
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
sed1330_num( int x, int num )
{
	//private_data * data = sed1330->private_data;

	debug( RPT_INFO, "sed1330_bignum x=%d num=%d", x, num );


}


/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
void
sed1330_heartbeat( int type )
{
	private_data * data = sed1330->private_data;
	static int timer = 0;
	int pos;
	int whichIcon;
	int n;

	char heartdata[2][CHARHEIGHT] = {
		{ 0xFF, 0xFF, 0xAF, 0x07, 0x8F, 0xDF, 0xFF, 0xFF, 0x00, 0x00 },
		{ 0xFF, 0xAF, 0x07, 0x07, 0x07, 0x8F, 0xDF, 0xFF, 0x00, 0x00 }
	};

	report( RPT_INFO, "sed1330_heartbeat type=%d", type );

	data->framebuf_text[sed1330->wid-1] = ' ';
	whichIcon = (! ((timer + 4) & 5));

	pos = sed1330->wid - 1;
	for( n=0; n<CHARHEIGHT; n++ ) {
		data->framebuf_graph[pos] = heartdata[whichIcon][n];
		pos += data->bytesperline;
	}

	timer++;
}
