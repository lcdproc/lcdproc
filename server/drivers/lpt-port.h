/*
 * This file contains defines to control the LPT port.
 * A lot of this was originally in hd44780-low.h
 *
 * Moved from the HD44780 driver by Joris Robijn.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *		  2001 Mark Haemmerling <mail@markh.de>
 *		  2000 Charles Steinkuehler <cstein@newtek.com>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 */

#ifndef LPT_PORT_H
#define LPT_PORT_H


/* Output data
 * Write to baseaddress+0
 */
/* Straight-forward use, no defines needed for this... */


/* Control output lines
 * Write to baseaddress+2
 */
#define nSTRB 	0x01	/* pin 1; negative logic */
#define STRB 	0x01
#define nLF 	0x02	/* pin 14 */
#define LF 	0x02
#define INIT 	0x04	/* pin 16; the only positive logic output line */
#define nSEL 	0x08	/* pin 17 */
#define SEL 	0x08
#define ENIRQ	0x10	/* Enable IRQ via ACK line (don't enable this without
			 * setting up interrupt stuff too) */
#define ENBI	0x20	/* Enable bi-directional port (is nice to play with!
			 * I first didn't know a SPP could do this) */

#define OUTMASK	0x0B	/* SEL, LF and STRB are hardware inverted */
			/* Use this mask only for the control output lines */
			/* XOR with this mask ( ^ OUTMASK ) */


/* Control input lines
 * Read from baseaddress+1
 */
#define nFAULT	0x08	/* pin 15 */
#define FAULT	0x08
#define SELIN	0x10	/* pin 13 */
#define PAPEREND  0x20	/* pin 12 */
#define nACK	0x40	/* pin 10 */
#define ACK	0x40
#define BUSY	0x80	/* pin 11 */
#define IRQ	0x02

#define INMASK	0x84	/* BUSY input and the IRQ indicator are inverted */
			/* Use this mask only for the control input lines */
			/* XOR with this mask ( ^ INMASK ) */

#endif
