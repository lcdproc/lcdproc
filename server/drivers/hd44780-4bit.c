/*
 * 4-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 4 bit-mode to be connected to a single
 * 8 bit-port. 
 *
 * Copyright (c)  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *
 * The connections are:
 * printer port	  LCD
 * D0 (2)      	  D4 (11)
 * D1 (3)	  D5 (12)
 * D2 (4)	  D6 (13)
 * D3 (5)	  D7 (14)
 * D4 (6)	  RS (4)
 * D5 (7)	  RW (5) (LCD3 - 6) (optional - pull all LCD RW low)
 * D6 (8)	  EN (6)
 * D7 (9)	  EN2 (LCD2 - 6) (optional)
 *
 * Extended interface (including LCD3 above)
 * STR (1)	  EN4
 * LF (14)	  EN5
 * INIT (16)	  EN6
 * SEL (17)	  EN7
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
#include "port.h"

#include "../../shared/str.h"
#include <sys/perm.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically just
// HD44780_senddata

void lcdstat_HD44780_senddata (unsigned char displayID, unsigned char flags, unsigned char ch);


#define RS 	0x10
#define RW 	0x20
#define EN1 	0x40
#define EN2	0x80
#define EN3	0x20

static unsigned char EnMask[] = { EN1, EN2, EN3, STRB, LF, INIT, SEL };
static int EnMaskSize = sizeof (EnMask) / sizeof (unsigned char);

static unsigned int lptPort;
static int extIF = 0;		// non-zero if extended interface

// initialisation function
int
hd_init_4bit (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port)
{
   // TODO: remove magic numbers below
   char *argv[64];
   int argc;
   int i;
   int displayID = EN1 | EN2;

   lptPort = port;

   hd44780_functions->senddata = lcdstat_HD44780_senddata;

   // parse command-line arguments
   argc = get_args (argv, args, 64);

   for (i = 0; i < argc; ++i) {
      if (strcmp (argv[i], "-e") == 0 || strcmp (argv[i], "--extended") == 0) {
	 extIF = 1;
	 if ((ioperm (port + 2, 1, 255)) == -1) {
	    fprintf (stderr, "HD44780_init: failed (%s)\n", strerror (errno));
	    return -1;
	 }
      }
   }

   // powerup the lcd now
   if (extIF) {
      displayID |= EN3;
      port_out (lptPort + 2, 0 ^ OUTMASK);
   }

   port_out (lptPort, displayID | 0x03);
   if (extIF)
      port_out (lptPort + 2, (EnMask[3] | EnMask[4] | EnMask[5] | EnMask[6]) ^ OUTMASK);
   hd44780_functions->uPause (1);
   port_out (lptPort, 0x03);
   if (extIF)
      port_out (lptPort + 2, 0 ^ OUTMASK);
   hd44780_functions->uPause (4100);

   port_out (lptPort, displayID | 0x03);
   if (extIF)
      port_out (lptPort + 2, (EnMask[3] | EnMask[4] | EnMask[5] | EnMask[6]) ^ OUTMASK);
   hd44780_functions->uPause (1);
   port_out (lptPort, 0x03);
   if (extIF)
      port_out (lptPort + 2, 0 ^ OUTMASK);
   hd44780_functions->uPause (100);

   port_out (lptPort, displayID | 0x03);
   if (extIF)
      port_out (lptPort + 2, (EnMask[3] | EnMask[4] | EnMask[5] | EnMask[6]) ^ OUTMASK);
   hd44780_functions->uPause (1);
   port_out (lptPort, 0x03);
   if (extIF)
      port_out (lptPort + 2, 0 ^ OUTMASK);
   hd44780_functions->uPause (40);

   // now in 8-bit mode...  set 4-bit mode
   port_out (lptPort, displayID | 0x02);
   if (extIF)
      port_out (lptPort + 2, (EnMask[3] | EnMask[4] | EnMask[5] | EnMask[6]) ^ OUTMASK);
   hd44780_functions->uPause (1);
   port_out (lptPort, 0x02);
   if (extIF)
      port_out (lptPort + 2, 0 ^ OUTMASK);
   hd44780_functions->uPause (40);

   common_init (IF_4bit);
   return 0;
}


// lcdstat_HD44780_senddata
void
lcdstat_HD44780_senddata (unsigned char displayID, unsigned char iflags, unsigned char ch)
{
   unsigned char dispID = 0, flags = 0;
   unsigned char h = (ch >> 4) & 0x0f;	// high and low nibbles
   unsigned char l = ch & 0x0f;

   if (iflags == RS_INSTR)
      flags = 0;
   else				//if (iflags == RS_DATA)
      flags = RS;

   if (displayID <= 3) {
      if (displayID == 0)
	 dispID = EnMask[0] | EnMask[1] | EnMask[2];
      else
	 dispID = EnMask[displayID - 1];

      port_out (lptPort, flags | h);
      hd44780_functions->uPause (2);
      port_out (lptPort, dispID | flags | h);
      hd44780_functions->uPause (4);
      port_out (lptPort, flags | h);

      port_out (lptPort, dispID | flags | l);
      hd44780_functions->uPause (4);
      port_out (lptPort, flags | l);
   }

   if (extIF && (displayID == 0 || displayID >= 4)) {
      if (displayID == 0)
	 dispID = EnMask[3] | EnMask[4] | EnMask[5] | EnMask[6];
      else
	 dispID = EnMask[(displayID - 1) % EnMaskSize];

      port_out (lptPort, flags | h);
      hd44780_functions->uPause (2);
      port_out (lptPort + 2, dispID ^ OUTMASK);
      hd44780_functions->uPause (4);
      port_out (lptPort + 2, 0 ^ OUTMASK);

      port_out (lptPort, flags | l);
      port_out (lptPort + 2, dispID ^ OUTMASK);
      hd44780_functions->uPause (4);
      port_out (lptPort + 2, 0 ^ OUTMASK);
   }
}
