/*
 * 4-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 4 bit-mode to be connected to a single
 * 8 bit-port.
 *
 * Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *
 * The connections are:
 * printer port   LCD
 * D0 (2)	  D4 (11)
 * D1 (3)	  D5 (12)
 * D2 (4)	  D6 (13)
 * D3 (5)	  D7 (14)
 * D4 (6)	  RS (4)
 * D5 (7)	  RW (5) (LCD3 - 6) (optional - pull all LCD RW low)
 * D6 (8)	  EN (6)
 * D7 (9)	  EN2 (LCD2 - 6) (optional)
 *
 * Backlight
 * D5 (7)	  backlight (optional, see documentation)
 *
 * Extended interface (including LCD3 above)
 * D5 (7)	  EN3
 * STR (1)	  EN4
 * LF (14)	  EN5
 * INIT (16)      EN6
 * SEL (17)       EN7
 *
 * Keypad connection (optional):
 * Some diodes and resistors are needed, see further documentation.
 * printer port   keypad
 * D0 (2)	  Y0
 * D1 (3)	  Y1
 * D2 (4)	  Y2
 * D3 (5)	  Y3
 * D4 (6)	  Y4
 * D5 (7)	  Y5
 * nSTRB (1)      Y6
 * nLF  (14)      Y7
 * INIT (16)      Y8
 * nSEL (17)      Y9
 * nACK (10)      X0
 * BUSY (11)      X1
 * PAPEREND (12   X2
 * SELIN (13)     X3
 * nFAULT (15)    X4
 *
 * Added support for up to 7 LCDs Jan 2000, Benjamin Tse
 * Created modular driver Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Based on Matthias Prinke's <m.prinke@trashcan.mcnet.de> lcd4.c module
 * in lcdstat. This was in turn based on Benjamin Tse's lcdtime package
 * which uses the LCD controller in its 8-bit mode.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-4bit.h"
#include "hd44780.h"

#include "port.h"
#include "shared/str.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void lcdstat_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdstat_HD44780_backlight (unsigned char state);
unsigned char lcdstat_HD44780_readkeypad (unsigned int YData);

#define RS	0x10
#define RW	0x20
#define EN1	0x40
#define EN2	0x80
#define EN3	0x20
#define BL	0x20
// note that the above bits are all meant for the data port of LPT

static unsigned char EnMask[] = { EN1, EN2, EN3, STRB, LF, INIT, SEL };

#define ALLEXT  (STRB|LF|INIT|SEL)
// The above bits are on the control port of LPT

static unsigned int lptPort;
static char stuckinputs = 0;	// if an input line is stuck, it will be ignored
static char backlight_bit = 0;	// default to low to enable three displays

// initialisation function
int
hd_init_4bit (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port)
{
	int enableLines = EN1 | EN2;

	// Reserve the port registers
	lptPort = port;
	port_access(lptPort);
	port_access(lptPort+1);
	port_access(lptPort+2);

	hd44780_functions->senddata = lcdstat_HD44780_senddata;
	hd44780_functions->backlight = lcdstat_HD44780_backlight;
	hd44780_functions->readkeypad = lcdstat_HD44780_readkeypad;

	// powerup the lcd now
	if (extIF) {
		enableLines |= EN3;
		port_out (lptPort + 2, 0 ^ OUTMASK);
	}

	port_out (lptPort, 0x03);
	if( delayBus ) hd44780_functions->uPause (1);
	port_out (lptPort, enableLines | 0x03);
	if (extIF)
		port_out (lptPort + 2, ALLEXT ^ OUTMASK);
	if( delayBus ) hd44780_functions->uPause (1);
	port_out (lptPort, 0x03);
	if (extIF)
		port_out (lptPort + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause (4100);

	port_out (lptPort, enableLines | 0x03);
	if (extIF)
		port_out (lptPort + 2, ALLEXT ^ OUTMASK);
	if( delayBus ) hd44780_functions->uPause (1);
	port_out (lptPort, 0x03);
	if (extIF)
		port_out (lptPort + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause (100);

	port_out (lptPort, enableLines | 0x03);
	if (extIF)
		port_out (lptPort + 2, ALLEXT ^ OUTMASK);
	if( delayBus ) hd44780_functions->uPause (1);
	port_out (lptPort, 0x03);
	if (extIF)
		port_out (lptPort + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause (40);

	// now in 8-bit mode...  set 4-bit mode
	port_out (lptPort, enableLines | 0x02);
	if (extIF)
		port_out (lptPort + 2, ALLEXT ^ OUTMASK);
	if( delayBus ) hd44780_functions->uPause (1);
	port_out (lptPort, 0x02);
	if (extIF)
		port_out (lptPort + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause (40);

	common_init ();

	if (have_keypad) {
		// Remember which input lines are stuck
		stuckinputs = lcdstat_HD44780_readkeypad (0);
	}

	return 0;
}

// lcdstat_HD44780_senddata
void
lcdstat_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl = 0;
	unsigned char h = (ch >> 4) & 0x0f;     // high and low nibbles
	unsigned char l = ch & 0x0f;

	if (flags == RS_INSTR)
		portControl = 0;
	else //if (flags == RS_DATA)
		portControl = RS;

	portControl |= backlight_bit;

	if (displayID <= 3) {
		if (displayID == 0)
			enableLines = EnMask[0] | EnMask[1] | EnMask[2];
		else
			enableLines = EnMask[displayID - 1];

		port_out (lptPort, portControl | h);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort, enableLines | portControl | h);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort, portControl | h);

		port_out (lptPort, portControl | l);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort, enableLines | portControl | l);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort, portControl | l);
	}

	if (extIF && (displayID == 0 || displayID >= 4)) {
		if (displayID == 0)
			enableLines = ALLEXT;
		else
			enableLines = EnMask[(displayID - 1)];

		port_out (lptPort, portControl | h);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort + 2, enableLines ^ OUTMASK);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort + 2, 0 ^ OUTMASK);

		port_out (lptPort, portControl | l);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort + 2, enableLines ^ OUTMASK);
		if( delayBus ) hd44780_functions->uPause (1);
		port_out (lptPort + 2, 0 ^ OUTMASK);
	}
}

void lcdstat_HD44780_backlight (unsigned char state)
{
	backlight_bit = (state?0:0x20);	// D5 line

	port_out (lptPort, backlight_bit);
}

unsigned char lcdstat_HD44780_readkeypad (unsigned int YData)
{
	unsigned char readval;

	// 10 bits output or 6 bits if >=3 displays
	// Convert the positive logic to the negative logic on the LPT port
	port_out (lptPort, ~YData & 0x003F );
	if (!extIF) {
		port_out (lptPort + 2, ( ((~YData & 0x03C0) << 6 )) ^ OUTMASK);
	}
	if( delayBus ) hd44780_functions->uPause (1);

	// Read inputs
	readval = ~ port_in (lptPort + 1) ^ INMASK;

	// Put port back into idle state for backlight
	port_out (lptPort, backlight_bit ^ OUTMASK);

	// And convert value back.
	return ( (readval >> 4 & 0x03) | (readval >> 5 & 0x04) | (readval >> 3 & 0x08) | (readval << 1 & 0x10) ) & ~stuckinputs;
}
