/** \file hd44780-lis2.c
 * \c lis2 connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 */

/*
 * VLSystems LIS2 driver
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
 *		  2007, Peter Marschall <peter@adpm.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
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

static void setLIS2CustomCharRow(int fd, unsigned char custom, unsigned char row, unsigned char data);
static void setLIS2Fan(int fd, int fan1, int fan2, int fan3, int fan4);
static void gotoXY(int fd, unsigned char x, unsigned char y);

static void writeChar(int fd, unsigned char code);


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
	report(RPT_INFO, "HD44780: lis2: Using device: %s", device);

	// Set up io port correctly, and open it...
	p->fd = open(device, O_RDWR | O_NOCTTY);
	if (p->fd == -1) {
		report(RPT_ERR, "HD44780: lis2: could not open device %s (%s)",
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

	/* Set port speed to 19200 baud */
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


/**
 * Set one pixel row of a custom character, valid on LIS2 devices only.
 * \param fd      File handle to write to.
 * \param custom  Custom character to set (from 0 to 7).
 * \param row     Index of pixel row to set (from (0 [=top] to 7 [=bottom]).
 * \param data    Pixel data: 5 bit coding from 0 to 31 from right to left (16/8/4/2/1).
 */
static void setLIS2CustomCharRow(int fd, unsigned char custom, unsigned char row, unsigned char data)
{
	writeChar(fd, 0);
	writeChar(fd, 171);
	writeChar(fd, custom);
	writeChar(fd, row);
	writeChar(fd, data);
}


/**
 * ??? Set fans, valid on LIS2 devices only. ???
 * \param fd    File handle to write to.
 * \param fan1  Data for 1st fan.
 * \param fan2  Data for 2nd fan.
 * \param fan3  Data for 3rd fan.
 * \param fan4  Data for 4th fan.
 */
static void setLIS2Fan(int fd, int fan1, int fan2, int fan3, int fan4)
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


/** Move cursor to given character coordinates.
 * \param fd  File handle to write to.
 * \param x   Character column to set cursor to.
 * \param y   Character row to set cursor to.
 */
static void gotoXY(int fd, unsigned char x, unsigned char y)
{
	writeChar(fd, 0);
	writeChar(fd, 161 + y);
	writeChar(fd, x);
	writeChar(fd, 167);
}


/**
 * Write a character.
 * \param fd  File handle to write to.
 * \param ch  Character to write.
 */
static void writeChar(int fd, unsigned char code)
{
	write(fd, &code, 1);
}


void lis2_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
static int mode = 0;
static unsigned char charNum = 0;
static unsigned char rowNum = 0;

	if (flags == RS_DATA) {	// RS_DATA: we have data
		if (mode == SETCHAR) {
			// define row for custom char (reset mode to normal if illegal row)
			if (rowNum < p->cellheight)
				setLIS2CustomCharRow(p->fd, charNum, rowNum, ch);
			else
				mode = 0;

			// increase row counter
			rowNum++;
		}
		else {
			// offset given char by one to avoid collisions with command prefix
			// TODO: check if this is correct
			if (ch < 7)
				ch++;

			// simply write character byte
			write(p->fd, &ch, 1);
		}
	}
	else {	// RS_INSTR: we have an instruction
		if ((ch & POSITION) != 0) {
			unsigned char divisor = (p->ext_mode) ? 0x20 : 0x40;

			// get mangled position by stripping POSITION flag from given data
			ch &= ~POSITION;

			// goto X & Y coordinates from un-mangled position
			gotoXY(p->fd, ch % divisor, ch / divisor);
		}
		else if ((ch & SETCHAR) != 0) {
			mode = SETCHAR;

			// extract character to set from given info
			charNum = (ch & ~SETCHAR) / 8;
			
			// offset it by one to avoid collisions with command prefix 
			// TODO: check if this is correct
			charNum++;

			// restrict it to range [1, 7]
			// TODO: check if this is correct
			if (charNum == 8)
				charNum = 7;

			// reset row number
			rowNum = 0;
		}
	 	else
			// simply write through instruction byte
			write(p->fd, &ch, 1);
	}
}


void lis2_HD44780_backlight(PrivateData *p, unsigned char state)
{
	/* No backlight control */
}


unsigned char lis2_HD44780_scankeypad(PrivateData *p)
{
	return '\0';
}


