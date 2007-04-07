/*
 * LIS2 Serializer driver
 * Modification of the code of the LCDSerializer driver
 *
 * Copyright (c)  1997, Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *		  1998, Richard Rognlie <rrognlie@gamerz.net>
 * 		  1999, Ethan Dicks
 *		  1999-2000, Benjamin Tse <blt@Comports.com>
 *		  2001, Rene Wagner
 *		  2001-2002, Joris Robijn <joris@robijn.net>
 *                2005, Pillon Matteo <matteo.pillon@email.it>
 *		  2005, Laurent ARNAL <laurent@clae.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * 
 * 
 *
 */

#include "hd44780-lis2.h"
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

#define DEFAULT_DEVICE		"/dev/ttyUSB0"


void lis2_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lis2_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lis2_HD44780_scankeypad(PrivateData *p);

static void SetMatrice(PrivateData *p,int fd, int matriceNum, int ligne, int point);
static void SetFan(int fd, int fan1, int fan2, int fan3, int fan4);
static void gotoXY(int fd, int x, int y);

static void writeChar(int fd, int code);


// initialise the driver
int hd_init_lis2(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;

	struct termios portset;
	char device[256] = DEFAULT_DEVICE;

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO, "HD44780: LCD Serializer: Using device: %s", device);

	// Set up io port correctly, and open it...
	p->fd = open(device, O_RDWR | O_NOCTTY);
	if (p->fd == -1) {
		report(RPT_ERR, "HD44780: LCD Serializer: could not open device %s (%s)",
				device, strerror(errno));
		return -1;
	}

	/* Get serial device parameters */
	tcgetattr(p->fd, &portset);

	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;

	/* Set port speed to 9600 baud */
	cfsetospeed(&portset, B19200);
	cfsetispeed(&portset, B0);

	/* Set TCSANOW mode of serial device */
	tcsetattr(p->fd, TCSANOW, &portset);

	p->hd44780_functions->senddata = lis2_HD44780_senddata;
	p->hd44780_functions->backlight = lis2_HD44780_backlight;
	p->hd44780_functions->scankeypad = lis2_HD44780_scankeypad;

	common_init(p, IF_8BIT);

	return 0;
}


static void SetMatrice(PrivateData *p, int fd, int matriceNum, int ligne, int point)
{
	// char from 0 to 7
	// line from 0 to 7 from top to bottom
	// pixel 0/1 is 5 bit coding from 0 to 31 from right to left (16/8/4/2/1)
	writeChar(p->fd,0);
	writeChar(p->fd,171);
	writeChar(p->fd, matriceNum);
	writeChar(fd, ligne);
	writeChar(fd, point);
}

static void SetFan(int fd, int fan1, int fan2, int fan3, int fan4)
{
	writeChar(fd, 0);
	writeChar(fd, 174);
	writeChar(fd, 0);
	writeChar(fd, 0);
	writeChar(fd, fan1);
	writeChar(fd, 0);
	writeChar(fd, fan2);
	writeChar(fd, 0);
	writeChar(fd, fan3);
	writeChar(fd, 0);
	writeChar(fd, fan4);
	writeChar(fd, 0);
	writeChar(fd, 0);
}

static void gotoXY(int fd, int x, int y)
{
	writeChar(fd, 0);
	writeChar(fd, 160+y);
	writeChar(fd, x);
	writeChar(fd, 167);
}

static void writeChar(int fd, int code)
{
	char buf = code;

	write(fd, &buf, 1);
}


static int mode = 0;
static int charNum = 0;
static int rowNum = 0;

void lis2_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	if (flags == RS_DATA) {
		if (mode == SETCHAR) {
			writeChar(p->fd, 0);
			writeChar(p->fd, 171);
			writeChar(p->fd, charNum);
			writeChar(p->fd, rowNum);
			writeChar(p->fd, ch);
			rowNum++;
			if (rowNum == p->cellheight) {
				mode = 0;
				rowNum = 0;
			}
		}
		else {
			if (ch < 7) ch++;
			write(p->fd, &ch, 1);
		}
	}
	else {
		if ((ch & POSITION) != 0) {
			int x = 0;
			int y = 0;
			int pos = 0;

			pos = (ch & ~POSITION);

			if (p->ext_mode) {				
				y = pos / 0x20;
				x = pos - (y * 0x20);
			}
			else {
				y = pos / 0x40;
				x = pos - (y * 0x40);
			}

			writeChar(p->fd, 0);
			writeChar(p->fd, 161 + y);
			writeChar(p->fd, x);
			writeChar(p->fd, 167);

		}
		else if ((ch & SETCHAR) != 0) {
			mode = SETCHAR;
			charNum = ((ch & ~SETCHAR)/8) + 1;
			if (charNum == 8) charNum = 7;
		}
	 	else write(p->fd, &ch, 1);
	}

}

void lis2_HD44780_backlight(PrivateData *p, unsigned char state)
{
	/* No backlight control */
}

unsigned char lis2_HD44780_scankeypad(PrivateData *p)
{
	return 0;
}


