/*
 * "PIC-an-LCD" serial driver module for Hitachi HD44780 based LCD displays.
 *
 * Copyright (c)  1997, Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *		  1998, Richard Rognlie <rrognlie@gamerz.net>
 * 		  1999, Ethan Dicks
 *		  1999-2000, Benjamin Tse <blt@Comports.com>
 *		  2001, Rene Wagner
 *		  2001-2002, Joris Robijn <joris@robijn.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * See the PIC-an-LCD documentation for data on how it should be connected
 * to the computer.
 * http://dalewheat.com/products/PIC-an-LCD/index.html
 *
 */

#include "hd44780-picanlcd.h"
#include "hd44780-low.h"

#include "report.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>

#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void picanlcd_HD44780_senddata (PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void picanlcd_HD44780_backlight (PrivateData *p, unsigned char state);
unsigned char picanlcd_HD44780_scankeypad (PrivateData *p);

// PIC-an-LCD ASCII commands
//#define PICANLCD_HOME '\001'
//#define PICANLCD_CLR  '\014'
#define PICANLCD_LED  '\025'
#define PICANLCD_LCDI '\021'
#define PICANLCD_LCDD '\022'

#define DEFAULT_DEVICE		"/dev/lcd"

// initialise the driver
int
hd_init_picanlcd (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;

	struct termios portset;
	char device[256] = DEFAULT_DEVICE;

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string ( drvthis->name , "device" , 0 , DEFAULT_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	report (RPT_INFO,"HD44780: PIC-an-LCD: Using device: %s", device);

	// Set up io port correctly, and open it...
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "HD44780: PIC-an-LCD: could not open device %s (%s)\n", device, strerror(errno));
		return -1;
	}

	/* Get serial device parameters */
	tcgetattr(p->fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw( &portset );
#else
	/* The hard way */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	/* Set port speed to 9600 baud */
	cfsetospeed(&portset, B9600);

	/* Set TCSANOW mode of serial device */
	tcsetattr(p->fd, TCSANOW, &portset);

	p->hd44780_functions->senddata = picanlcd_HD44780_senddata;
	p->hd44780_functions->backlight = picanlcd_HD44780_backlight;
	p->hd44780_functions->scankeypad = picanlcd_HD44780_scankeypad;

	common_init (p);

	return 0;
}

// picanlcd_HD44780_senddata
void
picanlcd_HD44780_senddata (PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	static const char data_byte = PICANLCD_LCDD;
	static const char instr_byte = PICANLCD_LCDI;

	if (flags == RS_DATA) {
		// Do we need a DATA indicator byte ?
		if( ch < 32 ) {
			write( p->fd, &data_byte, 1 );
		}
		write( p->fd, &ch, 1 );
	}
	else {
		write( p->fd, &instr_byte, 1 );
		write( p->fd, &ch, 1 );
	}
}

void
picanlcd_HD44780_backlight (PrivateData *p, unsigned char state)
{
	/* Anyone wants to implement this ? */
}

unsigned char
picanlcd_HD44780_scankeypad (PrivateData *p)
{
	return 0;
}
