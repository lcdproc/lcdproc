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
 * Q7 (12)	  Y6
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
#include "lpt-port.h"

#include "port.h"
#include "shared/str.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Hardware specific functions
void lcdserLpt_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdserLpt_HD44780_backlight (unsigned char state);

unsigned char lcdserLpt_HD44780_scankeypad ();
void rawshift (unsigned char r);
void shiftreg (unsigned char displayID, unsigned char r);

#define RS       32
#define LCDDATA   8
#define LCDCLOCK 16
#define EN1       4
#define EN2      32

static unsigned int lptPort;
static char backlight_bit = 0;

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
	hd44780_functions->backlight = lcdserLpt_HD44780_backlight;
	hd44780_functions->scankeypad = lcdserLpt_HD44780_scankeypad;

	// setup the lcd in 4 bit mode
	shiftreg (enableLines, 3);
	hd44780_functions->uPause (15000);

	shiftreg (enableLines, 3);
	hd44780_functions->uPause (5000);

	shiftreg (enableLines, 3);
	hd44780_functions->uPause (100);

	shiftreg (enableLines, 3);
	hd44780_functions->uPause (100);

	shiftreg (enableLines, 2);
	hd44780_functions->uPause (100);

	hd44780_functions->senddata (0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);

	common_init ();

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

	// Restore line status for backlight
	port_out (lptPort, backlight_bit );
}

void
lcdserLpt_HD44780_backlight (unsigned char state)
{
	// Store new state
	backlight_bit = (state?LCDDATA:0);

	// Set line status for backlight
	port_out (lptPort, backlight_bit );
}

unsigned char lcdserLpt_HD44780_scankeypad ()
{
	// Unfortunately just bit shifting does not work with the 2-wire version...

	unsigned char keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned char readval, inputs_zero;
	int i;
	unsigned int scancode = 0;

	// While scanning the keypad, the 2-wire version executes the 0xFF
	// command. This command sets the cursor position, so it's harmless.
	// I could not prevent this, while staying compatible with both
	// wiring versions.

	// Clear the shiftregister, needed for 3-wire version
	rawshift(0);
	hd44780_functions->uPause (2);

	readval = ~ port_in (lptPort + 1) ^ INMASK;
	inputs_zero = ( (readval >> 4 & 0x03) | (readval >> 5 & 0x04) | (readval >> 3 & 0x08) | (readval << 1 & 0x10) );

	if( inputs_zero == 0 ) {
		// No keys were pressed

		// Restore line status for backlight.
		port_out (lptPort, backlight_bit );
		return 0;
	}

	// Scan the keypad while sending the first half of the command (high nibble)
	for (i = 7; i >= 0; i--) {				/* MSB first  */
		port_out (lptPort, LCDDATA);			/*set up data */
		port_out (lptPort, LCDDATA | LCDCLOCK);		/*rising edge of clock */

		hd44780_functions->uPause (2);

		if( !scancode ) {
			// Read input line(s)
			readval = ~ port_in (lptPort + 1) ^ INMASK;
			keybits = ( (readval >> 4 & 0x03) | (readval >> 5 & 0x04) | (readval >> 3 & 0x08) | (readval << 1 & 0x10) );
			if( keybits != inputs_zero ) {
				shiftingbit = 1;
				for (shiftcount=0; shiftcount<KEYPAD_MAXX && !scancode; shiftcount++) {
					if ( (keybits ^ inputs_zero ) & shiftingbit) {
						// Found !
						scancode = ((8-i)<<4) | (shiftcount+1);
					}
					shiftingbit <<= 1;
				}
			}
		}
	}

	// Wait for 2-wire version to clear the latch...
	hd44780_functions->uPause (6);

	// And again for the second half of the command (low nibble).
	// Needed for 2-wire version.
	rawshift (0xFF);

	// Wait for 2-wire version to clear the latch...
	hd44780_functions->uPause (6);

	// Restore line status for backlight.
	port_out (lptPort, backlight_bit );

	return scancode;
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
	rawshift (r | 0x80);			// highest bit always set to 1 for Clear for 2-wire version
	port_out (lptPort, enableLines);	// latch it, to correct display
	hd44780_functions->uPause (1);
	port_out (lptPort, 0);			// for 3-wire version
	hd44780_functions->uPause (5);		// wait for 2-wire version to clear the latch...
}
