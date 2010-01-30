/** \file server/drivers/hd44780-serialLpt.c
 * \c serialLPT connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in it's 4 bit-mode through a 4094 shift register and supports a keypad.
 */

/* Copyright (c)  1999 Andrew McMeikan <andrewm@engineer.com>
 *		modular driver 1999 Benjamin Tse <blt@Comports.com>
 *
 *              2001 Joris Robijn <joris@robijn.net>
 *                - Keypad support
 *                - Changed for 2 line wire control
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
 * Q1 (4)	  Y1
 * Q2 (5)	  Y2
 * Q3 (6)	  Y3
 * Q4 (7)	  Y4
 * Q5 (14)	  Y5
 * Q6 (13)	  Y6
 * Q7 (12)	  Y7
 * The 'output' of the keys should be connected to the following LPT pins:
 * nACK   (10)    X1
 * BUSY   (11)    X2
 * PAPEREND (12)  X3
 * SELIN  (13)    X4
 * nFAULT (15)    X5
 * If you want to use as few LPT lines as possible, only use X0.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-serialLpt.h"
#include "hd44780-low.h"
#include "lpt-port.h"
#include "port.h"
#include "report.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

// Hardware specific functions
void lcdserLpt_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdserLpt_HD44780_backlight(PrivateData *p, unsigned char state);

unsigned char lcdserLpt_HD44780_scankeypad(PrivateData *p);

static void rawshift(PrivateData *p, unsigned char r);
static void shiftreg(PrivateData *p, unsigned char displayID, unsigned char r);

#define RS       32
#define LCDDATA   8
#define LCDCLOCK 16
#define EN1       4
#define EN2      32


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_serialLpt(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;
	unsigned char enableLines = EN1 | EN2;

	// Reserve the port registers
	if (port_access_multiple(p->port,3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X: %s",
				drvthis->name, p->port, strerror(errno));
		return -1;
	}

	hd44780_functions->senddata = lcdserLpt_HD44780_senddata;
	hd44780_functions->backlight = lcdserLpt_HD44780_backlight;
	hd44780_functions->scankeypad = lcdserLpt_HD44780_scankeypad;

	// setup the lcd in 4 bit mode
	shiftreg(p, enableLines, 3);
	hd44780_functions->uPause(p, 15000);

	shiftreg(p, enableLines, 3);
	hd44780_functions->uPause(p, 5000);

	shiftreg(p, enableLines, 3);
	hd44780_functions->uPause(p, 100);

	shiftreg(p, enableLines, 3);
	hd44780_functions->uPause(p, 100);

	shiftreg(p, enableLines, 2);
	hd44780_functions->uPause(p, 100);

	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause(p, 40);

	common_init(p, IF_4BIT);

	return 0;
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
lcdserLpt_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
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

	shiftreg(p, enableLines, portControl | h);
	shiftreg(p, enableLines, portControl | l);

	// Restore line status for backlight
	port_out(p->port, p->backlight_bit);
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
lcdserLpt_HD44780_backlight(PrivateData *p, unsigned char state)
{
	// Store new state
	p->backlight_bit = (state?LCDDATA:0);

	// Set line status for backlight
	port_out(p->port, p->backlight_bit);
}


/**
 * Read keypress.
 * \param p  Pointer to driver's private data structure.
 * \return   Bitmap of the pressed keys.
 */
unsigned char lcdserLpt_HD44780_scankeypad(PrivateData *p)
{
	// Unfortunately just bit shifting does not work with the 2-wire version...

	unsigned char keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned char readval, inputs_zero;
	int i;
	unsigned int scancode = 0;

	// While scanning the keypad, the 2-wire version will place the
	// character 0xFF on the current cursor position. Therefor we first
	// set the cursor position to home, do the keypad reading and
	// afterwards restore the first character (on all connected displays).
	//
	// I could not prevent this, while staying compatible with both
	// wiring versions. Joris.
	//
	// (Positioning the cursor out of screen does not work either :( )

	// Set cursor position
	p->hd44780_functions->senddata(p, 0, RS_INSTR, POSITION | 0);
	p->hd44780_functions->uPause(p, 40);

	// Clear the shiftregister, needed for 3-wire version
	rawshift(p, 0);
	p->hd44780_functions->uPause(p, 1);

	readval = ~ port_in(p->port + 1) ^ INMASK;

	// And convert value back (MSB first).
	inputs_zero =  (((readval & FAULT) / FAULT <<4) |	/* pin 15 */
			((readval & SELIN) / SELIN <<3) |	/* pin 13 */
			((readval & PAPEREND) / PAPEREND <<2) |	/* pin 12 */
			((readval & BUSY) / BUSY <<1) |		/* pin 11 */
			((readval & ACK) / ACK ));		/* pin 10 */


	if (inputs_zero == 0) {
		// No keys were pressed

		// Restore line status for backlight.
		port_out(p->port, p->backlight_bit);
		return 0;
	}

	// Scan the keypad while sending the first half of the command (high nibble)
	for (i = 7; i >= 0; i--) {				/* MSB first  */
		port_out(p->port, LCDDATA);			/*set up data */
		port_out(p->port, LCDDATA | LCDCLOCK);		/*rising edge of clock */

		p->hd44780_functions->uPause(p, 1);

		if (!scancode) {
			// Read input line(s)
			readval = ~ port_in(p->port + 1) ^ INMASK;

			// And convert value back (MSB first).
			keybits = (((readval & FAULT) / FAULT <<4) |		/* pin 15 */
				((readval & SELIN) / SELIN <<3) |		/* pin 13 */
				((readval & PAPEREND) / PAPEREND <<2) |		/* pin 12 */
				((readval & BUSY) / BUSY <<1) |			/* pin 11 */
				((readval & ACK) / ACK ));			/* pin 10 */

			if (keybits != inputs_zero) {
				shiftingbit = 1;
				for (shiftcount=0; shiftcount<KEYPAD_MAXX && !scancode; shiftcount++) {
					if ((keybits ^ inputs_zero) & shiftingbit) {
						// Found !
						scancode = ((8-i)<<4) | (shiftcount+1);
					}
					shiftingbit <<= 1;
				}
			}
		}
	}

	// Wait for 2-wire version to clear the latch...
	p->hd44780_functions->uPause(p, 6);

	// And again for the second half of the command (low nibble).
	// Needed for 2-wire version.
	rawshift(p, 0xFF);

	// Wait 6us for 2-wire version to clear the latch and wait for
	// the data to be processed
	p->hd44780_functions->uPause(p, 40);

	// Restore the screen state
	// Move back to home cursor position
	p->hd44780_functions->senddata(p, 0, RS_INSTR, POSITION | 0);
	p->hd44780_functions->uPause(p, 40);

	// Output the corect byte
	p->hd44780_functions->senddata(p, 1, RS_DATA,
				p->framebuf[0]);
	// ... and second display if connected ...
	if (p->numDisplays>1) {
		p->hd44780_functions->senddata(p, 2, RS_DATA,
				p->framebuf[ p->width * p->dispVOffset[2-1] ]);
	}
	p->hd44780_functions->uPause(p, 40);

	// No need to restore the line for backlight, already done by senddata.

	return scancode;
}

/* this function sends r out onto the shift register */
static void
rawshift(PrivateData *p, unsigned char r)
{
	int i;

	for (i = 7; i >= 0; i--) {						/* MSB first      */
		port_out(p->port, ((r >> i) & 1) * LCDDATA);			/*set up data   */
		port_out(p->port, (((r >> i) & 1) * LCDDATA) | LCDCLOCK);	/*rising edge of clock   */
	}
}

// enableLines = value on parallel port to toggle the correct display
static void
shiftreg(PrivateData *p, unsigned char enableLines, unsigned char r)
{
	rawshift(p, r | 0x80);			// highest bit always set to 1 for Clear for 2-wire version
	port_out(p->port, enableLines);	// latch it, to correct display
	p->hd44780_functions->uPause(p, 1);
	port_out(p->port, 0);			// for 3-wire version
	p->hd44780_functions->uPause(p, 5);		// wait for 2-wire version to clear the latch...
}
