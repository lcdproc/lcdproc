// This file contains defines to control the LPT port.
// A lot of this was originally in hd44780-low.h

#ifndef LPT_PORT_H
#define LPT_PORT_H


/* Output data
 * Write to baseaddress+0
 */
// Straight-forward use, no defines needed for this...


/* Control output lines
 * Write to baseaddress+2
 */
#define nSTRB 	0x01	/* negative logic */
#define STRB 	0x01
#define nLF 	0x02
#define LF 	0x02
#define INIT 	0x04	/* the only positive logic output line */
#define nSEL 	0x08
#define SEL 	0x08
#define ENIRQ	0x10	/* Enable IRQ via ACK line */
#define ENBI	0x20	/* Enable bi-directional port */

#define OUTMASK	0x0B	/* SEL, LF and STRB are hardware inverted */
			/* Use this mask only for the control output lines */
			/* XOR with this mask ( ^ OUTMASK ) */


/* Control input lines
 * Read from baseaddress+1
 */
#define nFAULT	0x08
#define FAULT	0x08
#define SELIN	0x10
#define PAPEREND  0x20
#define nACK	0x40
#define ACK	0x40
#define BUSY	0x80
#define IRQ	0x02

#define INMASK	0x84	/* BUSY input and the IRQ indicator are inverted */
			/* Use this mask only for the control input lines */
			/* XOR with this mask ( ^ OUTMASK ) */

#endif
