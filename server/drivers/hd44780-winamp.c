/*
 * "Winamp" 8-bit driver module for Hitachi HD44780 based LCD displays.
 * The LCD is operated in it's 8 bit-mode to be connected to a single
 * PC parallel port.
 *
 * Copyright (c)  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  modular driver 1999-2000 Benjamin Tse <blt@Comports.com> 
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
 * nSTRB (1)      EN (6)
 * nLF   (14)     nRW (5) (EN3 6 - LCD 3) (optional)
 * INIT  (16)     RS (4)
 * nSEL  (17)     EN2 (6 - LCD 2) (optional)
 *
 * Created modular driver Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-winamp.h"
#include "port.h"

#include "../../shared/str.h"
#include <sys/perm.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void lcdwinamp_HD44780_senddata(unsigned char displayID, 
				unsigned char flags, 
				unsigned char ch);

#define EN1     STRB
#define EN2	SEL
#define EN3	LF
#define RS	INIT

static unsigned char EnMask[] = { EN1, EN2, EN3 };
static int EnMaskSize = sizeof(EnMask) / sizeof(unsigned char);

static unsigned int lptPort;
static int extIF = 0;		// non-zero if extended interface


// initialise the driver
int hd_init_winamp(HD44780_functions *hd44780_functions, 
		   lcd_logical_driver *driver, 
		   char *args, 
		   unsigned int port)
{
   // TODO: remove magic numbers below
  char *argv[64];
  int argc;
  int i;
  
  hd44780_functions->senddata = lcdwinamp_HD44780_senddata;
  lptPort = port;

  // parse command-line arguments
  argc = get_args(argv, args, 64);

  for (i = 0; i < argc; ++i)
    if (strcmp(argv[i], "-e") == 0 ||
	strcmp(argv[i], "--extended") == 0)
      extIF = 1;

  if ((ioperm(lptPort + 2, 1, 255)) == -1) {
    fprintf(stderr, "HD44780_init: failed (%s)\n", strerror(errno)); 
    return -1;
  }

  common_init(IF_8bit);
  return 0;
}


// lcdwinamp_HD44780_senddata
void lcdwinamp_HD44780_senddata(unsigned char displayID, 
				unsigned char flags, 
				unsigned char ch)
{
  unsigned char dispID = 0, portControl;

  if (flags == RS_DATA)
    portControl = RS;
  else
    portControl = 0;

  if (displayID == 0)
    dispID = EnMask[0] | EnMask [1] | ((extIF) ? EnMask[2] : 0);
  else
    dispID = EnMask[displayID - 1];
	
  port_out(lptPort, ch);
  port_out(lptPort + 2, (dispID | portControl) ^ OUTMASK);
  hd44780_functions->uPause(1);
  port_out(lptPort + 2, portControl ^ OUTMASK);
}

