/** \file server/drivers/hd44780-4bit.c
 * \c 4bit connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 4 bit-mode to be connected to a single PC parallel port.
 */

/* Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
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
 * D0 (2)	  Y1
 * D1 (3)	  Y2
 * D2 (4)	  Y3
 * D3 (5)	  Y4
 * D4 (6)	  Y5
 *
 * With a backlight or three displays, 4 additional rows are possible:
 * nSTRB (1)      Y6
 * nLF  (14)      Y7
 * INIT (16)      Y8
 * nSEL (17)      Y9
 *
 * With only two controllers and no backlight, 5 additional rows are possible:
 * D5 (7)	  Y6
 * nSTRB (1)      Y7
 * nLF  (14)      Y8
 * INIT (16)      Y9
 * nSEL (17)      Y10
 *
 * Colums return lines are the same for all configurations:
 * nACK (10)      X0
 * BUSY (11)      X1
 * PAPEREND (12)  X2
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

void lcdstat_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdstat_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcdstat_HD44780_readkeypad(PrivateData *p, unsigned int YData);

#define RS	0x10
#define RW	0x20
#define EN1	0x40
#define EN2	0x80
#define EN3	0x20
#define BL	0x20
// note that the above bits are all meant for the data port of LPT

static const unsigned char EnMask[] = { EN1, EN2, EN3, STRB, LF, INIT, SEL };

#define ALLEXT  (STRB|LF|INIT|SEL)
// The above bits are on the control port of LPT


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_4bit(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	int enableLines = EN1 | EN2 | ((p->numDisplays == 3) ? EnMask[2] : 0);

	// Reserve the port registers
	if (port_access_multiple(p->port,3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X: %s",
				drvthis->name, p->port, strerror(errno));
		return -1;
	}

	hd44780_functions->senddata = lcdstat_HD44780_senddata;
	hd44780_functions->backlight = lcdstat_HD44780_backlight;
	hd44780_functions->readkeypad = lcdstat_HD44780_readkeypad;

	// powerup the lcd now
	port_out(p->port + 2, 0 ^ OUTMASK);
	port_out(p->port, 0x03);
	if (p->delayBus) hd44780_functions->uPause(p, 1);

	/* We'll now send 0x03 a coulpe of times,
	 * which is in fact (FUNCSET | IF_8BIT) >> 4 */
	port_out(p->port, enableLines | 0x03);
	port_out(p->port + 2, ALLEXT ^ OUTMASK);
	if (p->delayBus) hd44780_functions->uPause(p, 1);
	port_out(p->port, 0x03);
	port_out(p->port + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause(p, 15000);

	port_out(p->port, enableLines | 0x03);
	port_out(p->port + 2, ALLEXT ^ OUTMASK);
	if (p->delayBus) hd44780_functions->uPause(p, 1);
	port_out(p->port, 0x03);
	port_out(p->port + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause(p, 5000);

	port_out(p->port, enableLines | 0x03);
	port_out(p->port + 2, ALLEXT ^ OUTMASK);
	if (p->delayBus) hd44780_functions->uPause(p, 1);
	port_out(p->port, 0x03);
	port_out(p->port + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause(p, 100);

	port_out(p->port, enableLines | 0x03);
	port_out(p->port + 2, ALLEXT ^ OUTMASK);
	if (p->delayBus) hd44780_functions->uPause(p, 1);
	port_out(p->port, 0x03);
	port_out(p->port + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause(p, 100);

	// now in 8-bit mode...  set 4-bit mode
	port_out(p->port, 0x02);
	if (p->delayBus) hd44780_functions->uPause(p, 1);

	port_out(p->port, enableLines | 0x02);
	port_out(p->port + 2, ALLEXT ^ OUTMASK);
	if (p->delayBus) hd44780_functions->uPause(p, 1);
	port_out(p->port, 0x02);
	port_out(p->port + 2, 0 ^ OUTMASK);
	hd44780_functions->uPause(p, 100);

	// Set up two-line, small character (5x8) mode
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause(p, 40);

	common_init(p, IF_4BIT);

	if (p->have_keypad) {
		// Remember which input lines are stuck
		p->stuckinputs = lcdstat_HD44780_readkeypad(p, 0);
	}

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
lcdstat_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl = 0;
	unsigned char h = (ch >> 4) & 0x0f;     // high and low nibbles
	unsigned char l = ch & 0x0f;

	if (flags == RS_INSTR)
		portControl = 0;
	else //if (flags == RS_DATA)
		portControl = RS;

	portControl |= p->backlight_bit;

	if (displayID <= 3) {
		if (displayID == 0) {
			enableLines = EnMask[0] | EnMask[1]
			| ((p->numDisplays == 3) ? EnMask[2] : 0);
		} else {
			enableLines = EnMask[displayID - 1];
		}

		port_out(p->port, portControl | h);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port, enableLines | portControl | h);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port, portControl | h);

		port_out(p->port, portControl | l);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port, enableLines | portControl | l);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port, portControl | l);
	}

	if (p->numDisplays > 3) {
		if (displayID == 0) {
			enableLines = ALLEXT;
		} else {
			enableLines = EnMask[(displayID - 1)];
		}

		port_out(p->port, portControl | h);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port + 2, enableLines ^ OUTMASK);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port + 2, 0 ^ OUTMASK);

		port_out(p->port, portControl | l);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port + 2, enableLines ^ OUTMASK);
		if (p->delayBus) p->hd44780_functions->uPause(p, 1);
		port_out(p->port + 2, 0 ^ OUTMASK);
	}
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void lcdstat_HD44780_backlight(PrivateData *p, unsigned char state)
{
	p->backlight_bit = ((!p->have_backlight||state)?0:BL);

	port_out(p->port, p->backlight_bit);
}


/**
 * Read keypress.
 * \param p      Pointer to driver's private data structure.
 * \param YData  Bitmap of rows / lines to enable.
 * \return       Bitmap of the pressed keys.
 */
unsigned char lcdstat_HD44780_readkeypad(PrivateData *p, unsigned int YData)
{
	unsigned char readval;

	/* If at most two controllers and NO backlight, 10 bits may be used */
	if ((p->numDisplays <= 2) && (!p->have_backlight)) {
		port_out(p->port, ~YData & 0x003F);
		port_out(p->port + 2, (((~YData & 0x03C0) >> 6)) ^ OUTMASK);
	}
	else {
		/*
		 * Else ignore D5, because it may be either backlight or EN3.
		 * Note: If three displays are used, have_backlight must be
		 * set to 'no' in the config.
		 */
		port_out(p->port, (~YData & 0x001F) | p->backlight_bit);
		/*
		 * With at most three controllers or backlight 9 bits may be
		 * used.
		 */
		if (p->numDisplays<=3) {
			port_out(p->port + 2, (((~YData & 0x01E0) >> 5)) ^ OUTMASK);
		}
	}
	if (p->delayBus) p->hd44780_functions->uPause(p, 1);

	/* Read inputs */
	readval = ~ port_in(p->port + 1) ^ INMASK;

	/* Put port back into idle state for backlight */
	port_out(p->port, p->backlight_bit);

	/* And convert value back (MSB first) */
	return (((readval & FAULT) / FAULT <<4) |		/* pin 15 */
		((readval & SELIN) / SELIN <<3) |		/* pin 13 */
		((readval & PAPEREND) / PAPEREND <<2) |		/* pin 12 */
		((readval & BUSY) / BUSY <<1) |			/* pin 11 */
		((readval & ACK) / ACK)) & ~p->stuckinputs;	/* pin 10 */
}
