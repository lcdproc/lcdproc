/*
 * Serial LPT driver module for Hitachi HD44780 based LCD displays by
 * Andrew McMeikan. The LCD is operated in it's 4 bit-mode through a
 * 4094 shift register and supports a keypad.
 *
 * Copyright (c)  1999 Andrew McMeikan <andrewm@engineer.com>
 *		modular driver 1999 Benjamin Tse <blt@Comports.com>
 *
 *              2001 Joris Robijn <joris@robijn.net>
 *                - Keypad support
 *                - Changed for 2 line wire control
 *
 * Full connection details at http://members.xoom.com/andrewmuck/LCD.htm
 *
 * printer port   4094/LCD
 * D2 (4)	  EN  (6 - LCD)
 * D3 (5)	  D   (2 - 4094)
 * D4 (6)	  CLK (3 - 4094)
 * +Vcc	   	  OE, STR (15, 1 - 4094)
 * D7 (9)	  EN2 (6 - LCD2) (optional)
 *
 * 4094	   	  LCD
 * Q1 (4)	  D4 (11)
 * Q2 (5)	  D5 (12)
 * Q3 (6)	  D6 (13)
 * Q4 (7)	  D7 (14)
 * Q6 (13)	  RS (4)
 * Gnd	          nRW (5)
 *
 * Keypad connection (optional):
 * This is connected on the 4094 parallel to the LCD.
 * Some diodes and resistors are needed, see further documentation.
 * Q1 (4)	  Y0
 * Q2 (5)	  Y1
 * Q3 (6)	  Y2
 * Q4 (7)	  Y3
 * Q5 (14)	  Y4
 * Q6 (13)	  Y5
 * The 'output' of the keys should be connected to the following LPT pins:
 * nACK   (10)    X0
 * BUSY   (11)    X1
 * PAPEREND (12)  X2
 * SELIN  (13)    X3
 * nFAULT (15)    X4
 * If you want to use as few LPT lines as possible, only use X0.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-serialLpt.h"
#include "hd44780.h"

#include "port.h"
#include "shared/str.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Hardware specific functions
void lcdserLpt_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);
unsigned char lcdserLpt_HD44780_readkeypad (unsigned int YData);

static char lcdserLpt_HD44780_getkey (void);  // old keypad code
void rawshift (unsigned char r);
void shiftreg (unsigned char displayID, unsigned char r);

#define RS       32
#define LCDDATA   8
#define LCDCLOCK 16
#define EN1       4
#define EN2      32

static unsigned int lptPort;
static char stuckinputs = 0;	      // if an input line is stuck, it will be ignored

static char lastkey = 0;		  // currently not in use, old keypad code

// Initialisation
int
hd_init_serialLpt (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port)
{
	unsigned char enableLines = EN1 | EN2;

	// Reserve the port registers
	lptPort = port;
	port_access(lptPort);
	port_access(lptPort+1);
	port_access(lptPort+2);

	hd44780_functions->senddata = lcdserLpt_HD44780_senddata;
	hd44780_functions->readkeypad = lcdserLpt_HD44780_readkeypad;

	if (have_keypad) {
		driver->getkey = lcdserLpt_HD44780_getkey;
	}
	// Clear the shiftregister
	rawshift (0);

	// setup the lcd in 4 bit mode
	shiftreg (enableLines, 3);
	hd44780_functions->uPause (4100);

	shiftreg (enableLines, 3);
	hd44780_functions->uPause (100);

	shiftreg (enableLines, 3);
	hd44780_functions->uPause (40);

	shiftreg (enableLines, 2);
	hd44780_functions->uPause (40);

	hd44780_functions->senddata (0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);

	common_init ();

	/*
	// set display type functions
	lcdserLpt_HD44780_senddata (0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
	// clear
	lcdserLpt_HD44780_senddata (0, RS_INSTR, CLEAR);
	hd44780_functions->uPause (1600);
	// Now turn on the display
	lcdserLpt_HD44780_senddata (displayID, RS_INSTR, ONOFFCTRL | DISPON | CURSOROFF | CURSORNOBLINK);
	hd44780_functions->uPause (1600);
	*/

	return 0;
}

void
lcdserLpt_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines;
	unsigned char portControl = 0;
	unsigned char h = ch >> 4, l = ch & 15;

	if (displayID == 1)
		enableLines = EN1;
	else if (displayID == 2)
		enableLines = EN2;
	else
		enableLines = EN1 | EN2;

	if (flags == RS_DATA)
		portControl = RS;
	else
		portControl = 0;

	shiftreg (enableLines, portControl | h);
	shiftreg (enableLines, portControl | l);

	/* TODO: instead of delay read keys here */
}

char
lcdserLpt_HD44780_getkey ()
{
	// TODO: a keypad scan that does not use shift register
	// define a key table
	char keytr[] = "ABCDEFGH";
	int n;

	// disabled now !
	return 0;

	rawshift (0);					//send all line on shift register low
	hd44780_functions->uPause (1);
	if ((port_in (lptPort + 1) & 32) == 0)		//test if line back is low - if not return(0)
	{									 //else
		//start walking a single zero across the eight lines
		for (n = 7; n >= 0; n--) {
			rawshift (255 - (1 << n));
			hd44780_functions->uPause (1);

			if ((port_in (lptPort + 1) & 32) == 0)  // check if line back is low if yes debounce
			{
				if (lastkey == keytr[n])
					return (0);
				//      printf("key is %c %d\n",keytr[n],n);
				lastkey = keytr[n];
				return (keytr[n]);  	//return correct key code.
			}
		}
	}				 	//else fall out and return(0) [too transient a keypress]
	lastkey = 0;
	return (0);
}

unsigned char lcdserLpt_HD44780_readkeypad (unsigned int YData)
{
	return 0;
}

/* this function sends r out onto the shift register */
void
rawshift (unsigned char r)
{
	int i;
	for (i = 7; i >= 0; i--) {						/* MSB first      */
		port_out (lptPort, ((r >> i) & 1) * LCDDATA);			/*set up data   */
		port_out (lptPort, (((r >> i) & 1) * LCDDATA) | LCDCLOCK);	/*rising edge of clock   */
	}
}

// enableLines = value on parallel port to toggle the correct display
void
shiftreg (unsigned char enableLines, unsigned char r)
{
	//unsigned char mr;
	//mr = (r & 0xF0) | ((r<<3) & 0x0F) | ((r<<1) & 0x04) |
	//	      ((r>>1) & 0x02) | ((r>>3) & 0x01);

	rawshift (r | 0x80);			// highest bit always set to 1 for Clear
	port_out (lptPort, enableLines);	//latch it, to correct display
	hd44780_functions->uPause (1);
	port_out (lptPort, 0);
	hd44780_functions->uPause (3);
}
