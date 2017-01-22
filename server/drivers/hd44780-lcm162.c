/*
 * 8-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 8 bit-mode to be connected to a single
 * PC parallel port.
 *
 * Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *                2001 Joris Robijn <joris@robijn.net>
 *		  2009 Holger Rasch <rasch@bytemine.net>
 *
 * Created modular driver Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Based on the code in the lcdtime package which uses the LCD
 * controller in its 8-bit mode.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/*
 * Copied from "hd44780-ext8.c" and modified for lcm-162 module
 * found in nexcom nsaXXXX models.
 */

#include "hd44780-lcm162.h"
#include "hd44780-low.h"
#include "lpt-port.h"
#include "port.h"
#include "shared/report.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void lcm162_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcm162_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcm162_HD44780_readkeypad(PrivateData *p, unsigned int YData);
void lcm162_HD44780_output(PrivateData *p, int data);

/* lcm-162 specific mapping parport <-> hd44780 signals */
#define	RS	SEL
#define	RW	INIT
#define	EN1	LF


// initialise the driver
int
hd_init_lcm162(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	// Reserve the port registers
	if (port_access_multiple(p->port,3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X: %s",
				drvthis->name, p->port, strerror(errno));
		return -1;
	}

	hd44780_functions->senddata = lcm162_HD44780_senddata;
	hd44780_functions->backlight = lcm162_HD44780_backlight;
	hd44780_functions->readkeypad = lcm162_HD44780_readkeypad;

	// setup the lcd in 8 bit mode
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 4100);
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 100);
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause(p, 40);

	common_init (p, IF_8BIT);

	if (p->have_keypad)
		p->stuckinputs = 0;	/* unused here */

	// Writes new value to the "bargraph" latches
	hd44780_functions->output = 0;
	return 0;
}

// lcm162_HD44780_senddata
void
lcm162_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl;

	// Only one controller is supported
	enableLines = EN1;

	if (flags == RS_INSTR)
		portControl = 0;
	else //if (iflags == RS_DATA)
		portControl = RS;

	portControl |= p->backlight_bit;

	port_out(p->port + 2, portControl ^ OUTMASK);
	port_out(p->port, ch);
	if (p->delayBus) p->hd44780_functions->uPause(p, 1);
	port_out(p->port + 2, (enableLines|portControl) ^ OUTMASK);
	if (p->delayBus) p->hd44780_functions->uPause(p, 1);
	port_out(p->port + 2, portControl ^ OUTMASK);
}

void lcm162_HD44780_backlight(PrivateData *p, unsigned char state)
{
	/* ignore config, there is no backlight control on the lcm-162 */
	return;
}

unsigned char lcm162_HD44780_readkeypad(PrivateData *p, unsigned int YData)
{
	unsigned char readval;

	if (YData) return 0;	/* no keymatrix */

	readval = port_in(p->port + 1);
	if (readval & ACK) {	/* a key is pressed */
		/* 4 keys encoded in signals PAPEREND and FAULT */
		/* map them to low nibble (multiple keys not supported) */
		return 1 << (
			((readval & FAULT) / FAULT << 1) |
			((readval & PAPEREND) / PAPEREND)
		);
	} else
		return 0;
}
