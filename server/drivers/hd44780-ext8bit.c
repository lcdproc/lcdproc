/*
 * 8-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 8 bit-mode to be connected to a single
 * PC parallel port.
 *
 * Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *
 * The connections are:
 * printer port	  LCD
 * D0 (2)      	  D0 (7)
 * D1 (3)	  D1 (8)
 * D2 (4)	  D2 (9)
 * D3 (5)	  D3 (10)
 * D4 (6)	  D4 (11)
 * D5 (7)	  D5 (12)
 * D6 (8)	  D6 (13)
 * D7 (9)	  D7 (14)
 * nSEL  (17)     -
 * nSTRB (1)      RS (4)
 * nLF   (14)     RW (5) (LCD2 - 6) (optional - pull all LCD RW low)
 * INIT  (16)     EN (6)
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
#include "port.h"

#include "lcd_sem.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata

void lcdtime_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);

#define nRS 	nSTRB
#define nRW 	nLF
#define EN1     INIT
#define METER_OFF nSEL

#define SETUPLCD (METER_OFF | nRS)
#define CTRL    (nRW | METER_OFF | nRS)
#define CHAR    (nRW | METER_OFF)

static unsigned int lptPort;
static int semid;

// initialise the driver
int
hd_init_ext8bit (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port)
{
	lptPort = port;
	semid = sem_get ();

	hd44780_functions->senddata = lcdtime_HD44780_senddata;

	if (port_access (port + 2)) {
		fprintf (stderr, "HD44780_init: failed (%s)\n", strerror (errno));
		return -1;
	}

	common_init (IF_8bit);
	return 0;
}

// lcdtime_HD44780_senddata
void
lcdtime_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char dispID = 0, portControl;

	if (displayID == 1)
		dispID |= EN1;
	else								  //if (displayID == 0)
		dispID |= EN1;

	if (flags == RS_DATA)
		portControl = CHAR;
	else
		portControl = CTRL;

	sem_wait (semid);
	port_out (lptPort + 2, SETUPLCD);
	port_out (lptPort, ch);
	port_out (lptPort + 2, dispID | portControl);
	hd44780_functions->uPause (1);
	port_out (lptPort + 2, portControl);
	sem_signal (semid);
}
