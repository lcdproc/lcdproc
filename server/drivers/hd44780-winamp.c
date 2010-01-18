/** \file server/drivers/hd44780-winamp.c
 * \c winamp connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 8 bit-mode to be connected to a single PC parallel port.
 */

/*
 * Copyright (c)  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1999-2000 Benjamin Tse <blt@Comports.com>
 *                  - modular driver
 *
 *		  Modified July 2000 by Charles Steinkuehler for enhanced
 *		  performance and reduced CPU usage
 *		    - provided required setup time for RS valid to E high
 *		    - 1 uS call to uPause removed as it is unnecessary
 *
 *                2001 Joris Robijn <joris@robijn.net>
 *                  - Keypad support
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * The connections are:
 * printer port   LCD
 * D0 (2)	  D0 (7)
 * D1 (3)	  D1 (8)
 * D2 (4)	  D2 (9)
 * D3 (5)	  D3 (10)
 * D4 (6)	  D4 (11)
 * D5 (7)	  D5 (12)
 * D6 (8)	  D6 (13)
 * D7 (9)	  D7 (14)
 * nSTRB (1)      EN (6)
 * nLF   (14)     nRW (5) (EN3 6 - LCD 3) (optional)
 * INIT  (16)     RS (4)
 * nSEL  (17)     EN2 (6 - LCD 2) (optional)
 *
 * Backlight
 * SEL  (17)	  backlight (optional, see documentation)
 *
 * Additional output latch strobe
 * Not part of the hd44780 itself - addl '373/4, '574 chip or similar
 * (optional, see documentation (eventually))
 * nLF  (14)	  LE
 *
 * Keypad connection (optional):
 * Some diodes and resistors are needed, see further documentation.
 * printer port   keypad
 * D0 (2)	  Y1
 * D1 (3)	  Y2
 * D2 (4)	  Y3
 * D3 (5)	  Y4
 * D4 (6)	  Y5
 * D5 (7)	  Y6
 * D6 (8)	  Y7
 * D7 (9)	  Y8
 * nACK   (10)    X1
 * BUSY   (11)    X2
 * PAPEREND (12)  X3
 * SELIN  (13)    X4
 * nFAULT (15)    X5
 */

#include "hd44780-winamp.h"
#include "hd44780-low.h"
#include "lpt-port.h"
#include "port.h"
#include "report.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void lcdwinamp_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdwinamp_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcdwinamp_HD44780_readkeypad(PrivateData *p, unsigned int YData);
void lcdwinamp_HD44780_output(PrivateData *p, int data);

// Compile time mapping of control lines
// For expert users only!
#define EN1	STRB
#define EN2	SEL
#define EN3	LF
#define RW	LF
#define RS	INIT
#define BL	SEL
#define LE	LF

static const unsigned char EnMask[] = { EN1, EN2, EN3 };


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_winamp(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	// Safety check against common configuration errors
	if (p->numDisplays == 2) {
		if (p->have_backlight && (EnMask[1] == BL)) {
			report(RPT_ERR, "hd_init_winamp: backlight must be on different pin than 2nd controller");
			report(RPT_ERR, "hd_init_winamp: please change connection mapping in hd44780-winamp.c");
			return -1;
		}
		if (p->have_backlight && p->have_output) {
			report(RPT_ERR, "hd_init_winamp: backlight and output not possible with 2 controllers");
			return -1;
		}
	}
	else if (p->numDisplays == 3 && (p->have_backlight || p->have_output)) {
		report(RPT_ERR, "hd_init_winamp: backlight or output not possible with 3 controllers");
		return -1;
	}

	// Reserve the port registers
	if (port_access_multiple(p->port,3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X: %s",
				drvthis->name, p->port, strerror(errno));
		return -1;
	}

	hd44780_functions->senddata = lcdwinamp_HD44780_senddata;
	hd44780_functions->backlight = lcdwinamp_HD44780_backlight;
	hd44780_functions->readkeypad = lcdwinamp_HD44780_readkeypad;

	// setup the lcd in 8 bit mode
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 4100);
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 100);

	common_init(p, IF_8BIT);

	if (p->have_keypad) {
		// Remember which input lines are stuck
		p->stuckinputs = lcdwinamp_HD44780_readkeypad(p, 0);
	}

	// Write new value to output port latches
	hd44780_functions->output = lcdwinamp_HD44780_output;

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
lcdwinamp_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl;

	if (flags == RS_DATA)
		portControl = RS;
	else
		portControl = 0;

	portControl |= p->backlight_bit;

	if (displayID == 0)
		enableLines = EnMask[0]
		| ((p->numDisplays >= 2) ? EnMask[1] : 0)
		| ((p->numDisplays == 3) ? EnMask[2] : 0);
	else
		enableLines = EnMask[displayID - 1];

	// 40 nS setup time for RS valid to EN high, so set RS
	port_out(p->port + 2, portControl ^ OUTMASK);

	// Output the actual data
	port_out(p->port, ch);

	if (p->delayBus) p->hd44780_functions->uPause(p, 1);

	// then set EN high
	port_out(p->port + 2, (enableLines|portControl) ^ OUTMASK);

	if (p->delayBus) p->hd44780_functions->uPause(p, 1);

	// 80 nS setup from valid data to EN low will be met without any delay
	// unless you are running a REALLY FAST ISA bus (like 75 MHZ!)
	// 230 nS minimum E high time provided by ISA bus delays as well...

	// ABOVE TEXT ignored now, using delays if delayBus is specified

	// Set EN low and we're done...
	port_out(p->port + 2, portControl ^ OUTMASK);

	// 10 nS data hold time provided by the length of ISA write for EN
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void lcdwinamp_HD44780_backlight(PrivateData *p, unsigned char state)
{
	p->backlight_bit = (state ? 0 : BL);

	port_out(p->port + 2, p->backlight_bit ^ OUTMASK);
}


/**
 * Read keypress.
 * \param p      Pointer to driver's private data structure.
 * \param YData  Bitmap of rows / lines to enable.
 * \return       Bitmap of the pressed keys.
 */
unsigned char lcdwinamp_HD44780_readkeypad(PrivateData *p, unsigned int YData)
{
	unsigned char readval;

	// 8 bits output
	// Convert the positive logic to the negative logic on the LPT port
	port_out(p->port, ~YData & 0x00FF);

	if (p->delayBus) p->hd44780_functions->uPause(p, 1);

	// Read inputs
	readval = ~ port_in(p->port + 1) ^ INMASK;

	// And convert value back (MSB first).
	return (((readval & FAULT) / FAULT <<4) |		/* pin 15 */
		((readval & SELIN) / SELIN <<3) |		/* pin 13 */
		((readval & PAPEREND) / PAPEREND <<2) |		/* pin 12 */
		((readval & BUSY) / BUSY <<1) |			/* pin 11 */
		((readval & ACK) / ACK )) & ~p->stuckinputs;	/* pin 10 */
}


/**
 * Set output port.
 * \param p      Pointer to driver's private data structure.
 * \param data   Value the output port shall be set to.
 */
void lcdwinamp_HD44780_output(PrivateData *p, int data)
{
	// Setup data bus
	port_out(p->port, data);
	// Strobe the latch (374 latches on rising edge, 373 on trailing. No matter)
	port_out(p->port + 2, (LE | p->backlight_bit) ^ OUTMASK);
        if (p->delayBus) p->hd44780_functions->uPause(p, 1);
	port_out(p->port + 2, (p->backlight_bit) ^ OUTMASK);
        if (p->delayBus) p->hd44780_functions->uPause(p, 1);
}
