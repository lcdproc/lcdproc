/** \file hd44780-low.h
 * Interface to low level code to export from hd44780.c to "lower" HW
 * implementation dependent files.
 */

#ifndef HD_LOW_H
#define HD_LOW_H

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


//struct hwDependentFns;

// Maximum sizes of the keypad
// DO NOT CHANGE THESE 2 VALUES, unless you change the functions too
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 11

/* Constants for userdefchar_mode */
#define NUM_CCs 8 /* number of custom characters */

typedef enum {
	standard,	/* only char 0 is used for heartbeat */
	vbar,		/* vertical bars */
	hbar,		/* horizontal bars */
	bignum,		/* big numbers */
	bigchar		/* big characters */
} CGmode;

typedef struct cgram_cache {
	unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
	int clean;
} CGram;

typedef struct ConnectionMapping {
	char *name;
	int (*init_fn)(Driver *drvthis);
	const char *helpMsg;
} ConnectionMapping;

typedef struct driver_private_data {

	unsigned int port;

	/* for serial connection type */
	int fd;
	int serial_type;

	int charmap;

	int width, height;
	int cellwidth, cellheight;

	// The framebuffer
	char *framebuf;

	// For incremental updates store last lcd contents
	char *lcd_contents;

	// The defineable characters
	CGram cc[NUM_CCs];
	CGmode ccmode;

	// Connection type data
	int connectiontype_index;
	struct hwDependentFns *hd44780_functions;

	// spanList[line number] = display line number is in
	int *spanList;
	int numLines;

	// dispVOffset is a cumulative sized array of line numbers for each display.
	// use this to determine the vertical positioning on a given display
	int *dispVOffset;
	int numDisplays;

	// dispSizes is the vertical size of each display. This is the same as the
	// input span list but is kept to save some cpu cycles.
	int *dispSizes;

	// Keypad, backlight extended interface and delay options
	char have_keypad;	// off by default
	char have_backlight;	// off by default
	char have_output;	// have extra output port (off by default)
	char ext_mode;		// use of extended mode required for some weird controllers
	int delayMult;	 // Delay multiplier for slow displays
	char delayBus;	 // Delay if the computer can send data too fast over
				 // its bus to LPT port
        char lastline;   // lastline controls the use of the last line, if pixel addressable (true, default) or
                         // underline effect (false). To avoid the underline effect, last line is always zeroed
                         // for whatever redefined character 

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

	int backlight_bit;

	// force full refresh of display
	time_t nextrefresh;
	int refreshdisplay;     // When >0 make a full display update every <refreshdisplay> seconds
	time_t nextkeepalive;
	int keepalivedisplay;   // When >0 refresh upper left char every <keepalivedisplay> seconds to keep display alive

	int output_state;	// what was most recently output to the output port
} PrivateData;

// Structures holding pointers to HD44780 specific functions
typedef struct hwDependentFns {
	// microsec pauses
	void (*uPause)(PrivateData *p, int usecs);

	// Senddata to the LCD
	// dispID     - display to send data to (0 = all displays)
	// flags      - data or instruction command (RS_DATA | RS_INSTR)
	// ch	      - character to display or instruction value
	void (*senddata)(PrivateData *p, unsigned char dispID, unsigned char flags, unsigned char ch);

	// Switch the backlight on or off
	// state      - to be or not to be on
	void (*backlight)(PrivateData *p, unsigned char state);

	// Read the keypad
	// Ydata      - the up to 11 bits that should be put on the Y side of the matrix
	// return     - the up to 5 bits that are read out on the X side of the matrix
	unsigned char (*readkeypad)(PrivateData *p, unsigned int Ydata);

	// Scan the keypad and return a scancode.
	// The code is the Yvalue in the high nibble and the Xvalue in the low nibble.
	// A subdriver should do only one of two things:
	// - set readkeypad; or
	// - override scankeypad.
	unsigned char (*scankeypad)(PrivateData *p);

	// Output "data" to output latch if there is one
	void (*output)(PrivateData *p, int data);

        // Close the interface on shutdown
        void (*close)(PrivateData *p);

} HD44780_functions;				  /* for want of a better name :-) */


void common_init(PrivateData *p, unsigned char if_bit);


// commands for senddata
#define RS_DATA     0x00
#define RS_INSTR    0x01

#define CLEAR       0x01

#define HOMECURSOR  0x02

#define ENTRYMODE   0x04
#define E_MOVERIGHT 0x02
#define E_MOVELEFT  0x00
#define EDGESCROLL  0x01
#define NOSCROLL    0x00

#define ONOFFCTRL   0x08	/* Only reachable with EXTREG clear */
#define DISPON      0x04
#define DISPOFF     0x00
#define CURSORON    0x02
#define CURSOROFF   0x00
#define CURSORBLINK 0x01
#define CURSORNOBLINK 0x00

#define EXTMODESET  0x08	/* Only reachable with EXTREG set */
#define FONT6WIDE   0x04
#define INVCURSOR   0x02
#define FOURLINE    0x01

#define CURSORSHIFT 0x10	/* Only reachable with EXTREG clear */
#define SCROLLDISP  0x08
#define MOVECURSOR  0x00
#define MOVERIGHT   0x04
#define MOVELEFT    0x00

#define HSCROLLEN   0x10	/* Only reachable with EXTREG set */

#define FUNCSET     0x20
#define IF_8BIT     0x10
#define IF_4BIT     0x00
#define TWOLINE     0x08
#define ONELINE     0x00
#define LARGECHAR   0x04	/* 5x11 characters */
#define SMALLCHAR   0x00	/* 5x8 characters */
#define EXTREG      0x04	/* Select ext. registers (Yes, the same bits)*/
#define SEGBLINK    0x02	/* Only reachable with EXTREG set */
#define POWERDOWN   0x01	/* Only reachable with EXTREG set */

#define SETCHAR     0x40	/* Only reachable with EXTREG clear */

#define SETSEG      0x40	/* Only reachable with EXTREG set */

#define POSITION    0x80	/* Only reachable with EXTREG clear */

#define HSCROLLAMOUNT 0x80	/* Only reachable with EXTREG set */

#endif
