/*
 * 8-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 8 bit-mode to be connected to a single
 * PC parallel port.
 *
 * Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *                2001 Joris Robijn <joris@robijn.net>
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
 * nSTRB (1)      RS (4)
 * nLF   (14)     RW (5) (LCD2 - 6) (optional - pull all LCD RW low)
 * INIT  (16)     EN (6)
 *
 * Backlight
 * SEL  (17)	  backlight (optional, see documentation)
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
 * D6 (7)	  Y6
 * D7 (7)	  Y7
 * nSTRB  (1)     Y8
 * nSEL   (17)    Y9
 * nACK   (10)    X0
 * BUSY   (11)    X1
 * PAPEREND (12)  X2
 * SELIN  (13)    X3
 * nFAULT (15)    X4
 *
 * Created modular driver Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Based on the code in the lcdtime package which uses the LCD
 * controller in its 8-bit mode.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-ext8bit.h"
#include "hd44780-low.h"
#include "lpt-port.h"

#include "port.h"
#include "lcd_sem.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void lcdtime_HD44780_senddata (PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdtime_HD44780_backlight (PrivateData *p, unsigned char state);
unsigned char lcdtime_HD44780_readkeypad (PrivateData *p, unsigned int YData);

#define RS	STRB
#define RW	LF
#define EN1	INIT
#define BL	SEL

static int semid;

// initialise the driver
int
hd_init_ext8bit (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	semid = sem_get ();

	// Reserve the port registers
	port_access(p->port);
	port_access(p->port+1);
	port_access(p->port+2);

	hd44780_functions->senddata = lcdtime_HD44780_senddata;
	hd44780_functions->backlight = lcdtime_HD44780_backlight;
	hd44780_functions->readkeypad = lcdtime_HD44780_readkeypad;

	// setup the lcd in 8 bit mode
	hd44780_functions->senddata (p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause (p, 4100);
	hd44780_functions->senddata (p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause (p, 100);
	hd44780_functions->senddata (p, 0, RS_INSTR, FUNCSET | IF_8BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause (p, 40);

	common_init (p);

	if (p->have_keypad) {
		// Remember which input lines are stuck
		p->stuckinputs = lcdtime_HD44780_readkeypad (p, 0);
	}
	return 0;
}

// lcdtime_HD44780_senddata
void
lcdtime_HD44780_senddata (PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl;

	// Only one controller is supported
	enableLines = EN1;

	if (flags == RS_INSTR)
		portControl = 0;
	else //if (iflags == RS_DATA)
		portControl = RS;

	portControl |= p->backlight_bit;

	sem_wait (semid);
	port_out (p->port + 2, portControl ^ OUTMASK);
	port_out (p->port, ch);
	if( p->delayBus ) p->hd44780_functions->uPause (p, 1);
	port_out (p->port + 2, (enableLines|portControl) ^ OUTMASK);
	if( p->delayBus ) p->hd44780_functions->uPause (p, 1);
	port_out (p->port + 2, portControl ^ OUTMASK);
	sem_signal (semid);
}

void lcdtime_HD44780_backlight (PrivateData *p, unsigned char state)
{
	p->backlight_bit = (state?0:SEL);

	// Semaphores not needed because backlight will not go together with
	// the bacrgraph anyway...
	port_out (p->port + 2, p->backlight_bit ^ OUTMASK);
}

unsigned char lcdtime_HD44780_readkeypad (PrivateData *p, unsigned int YData)
{
	unsigned char readval;

	sem_wait (semid);

	// 10 bits output or 8 bits if >=3 displays
	// Convert the positive logic to the negative logic on the LPT port
	port_out (p->port, ~YData & 0x00FF );
	if (!p->extIF) {
		port_out (p->port + 2, ( ((~YData & 0x0100) >> 8) | ((~YData & 0x0200) >> 6)) ^ OUTMASK);
	}
	if( p->delayBus ) p->hd44780_functions->uPause (p, 1);

	// Read inputs
	readval = ~ port_in (p->port + 1) ^ INMASK;

	// Put port back into idle state for backlight
	port_out (p->port, p->backlight_bit ^ OUTMASK);
	sem_signal (semid);

	// And convert value back (MSB first).
	return (((readval & FAULT) / FAULT <<4) |		/* pin 15 */
		((readval & SELIN) / SELIN <<3) |		/* pin 13 */
		((readval & PAPEREND) / PAPEREND <<2) |		/* pin 12 */
		((readval & BUSY) / BUSY <<1) |			/* pin 11 */
		((readval & ACK) / ACK )) & ~p->stuckinputs;	/* pin 10 */
}
