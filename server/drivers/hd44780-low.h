// This header contains low level code to export from hd44780.c to "lower" HW
// implementation dependent files.

#ifndef HD_LOW_H
#define HD_LOW_H

enum ifWidth { IF_4bit, IF_8bit };

//void common_init (enum ifWidth ifwidth);
void common_init ();

// Structures holding pointers to HD44780 specific functions
typedef struct hwDependentFns {
	// microsec pauses
	void (*uPause) (int microSecondsTenths);

	// Senddata to the LCD
	// dispID     - display to send data to (0 = all displays)
	// flags      - data or instruction command (RS_DATA | RS_INSTR)
	// ch	      - character to display or instruction value
	void (*senddata) (unsigned char dispID, unsigned char flags, unsigned char ch);

	// Switch the backlight on or off
	// state      - to be or not to be on
	void (*backlight) (unsigned char state);

	// Read the keypad
	// Ydata      - the up to 11 bits that should be put on the Y side of the matrix
	// return     - the up to 5 bits that are read out on the X side of the matrix
	unsigned char (*readkeypad) (unsigned int Ydata);

	// Scan the keypad and return a scancode.
	// The code is the Yvalue in the high nibble and the Xvalue in the low nibble.
	// A subdriver should do only one of two things:
	// - set readkeypad; or
	// - override scankeypad.
	unsigned char (*scankeypad) ();


} HD44780_functions;				  /* for want of a better name :-) */

extern HD44780_functions *hd44780_functions;

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

#define ONOFFCTRL   0x08
#define DISPON      0x04
#define DISPOFF     0x00
#define CURSORON    0x02
#define CURSOROFF   0x00
#define CURSORBLINK 0x01
#define CURSORNOBLINK 0x00

#define CURSORSHIFT 0x10
#define SCROLLDISP  0x08
#define MOVECURSOR  0x00
#define MOVERIGHT   0x04
#define MOVELEFT    0x00

#define FUNCSET     0x20
#define IF_8BIT     0x10
#define IF_4BIT     0x00
#define TWOLINE     0x08
#define ONELINE     0x00
#define LARGECHAR   0x04		  /* 5x11 characters */
#define SMALLCHAR   0x00		  /* 5x8 characters */

#define SETCHAR     0x40

#define POSITION    0x80

#endif
