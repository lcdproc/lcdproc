/** \file server/drivers/hd44780-lis2.c
 * \c lis2 and \c mplay connection types of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 */

/*
 * Driver for VLsystems LIS2 and MPlay displays.
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
 *		  2007, Sean Meiners <sean.lcdproc@ssttr.com>
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

#define DEFAULT_DEVICE	"/dev/ttyUSB0"


/* bitrate conversion */
extern unsigned int **bitrate_conversion;
extern int convert_bitrate(unsigned int conf_bitrate, size_t *bitrate);

void lis2_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lis2_HD44780_close(PrivateData *p);

static void clearScreen(int fd);
static void gotoXY(int fd, unsigned char x, unsigned char y);
static void setMPlayCustomChars(int fd, CGram *chars);
static void setLIS2CustomCharRow(int fd, unsigned char custom, unsigned char row, unsigned char data);
static char readMPlayTemps(int fd, int *temps, int num);
static void initMPlayFans(int fd);
static void setMPlayFans(int fd, unsigned char *fans, int num);
static void setLIS2Fans(int fd, int fan1, int fan2, int fan3, int fan4);
static void writeChar(int fd, unsigned char code);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
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

#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
#else
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	/* Set timeouts */
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;

	if (p->connectiontype == HD44780_CT_LIS2) {
		/* Set port speed to 19200 baud */
		cfsetospeed(&portset, B19200);
		cfsetispeed(&portset, B0);
	}
	else {	// p->connectiontype == HD44780_CT_MPLAY
	       /* Get bitrate */
		unsigned int conf_bitrate;
		size_t bitrate;

		conf_bitrate = drvthis->config_get_int(drvthis->name, "Speed", 0, 38400);
		if (convert_bitrate(conf_bitrate, &bitrate)) {
			report(RPT_ERR, "HD44780: lis2: invalid configured bitrate speed");
			return -1;
		}
		report(RPT_INFO, "HD44780: lis2: using speed: %d", conf_bitrate);

		cfsetospeed(&portset, bitrate);
		cfsetispeed(&portset, bitrate);
	}

	/* Set TCSANOW mode of serial device */
	tcsetattr(p->fd, TCSANOW, &portset);

	p->hd44780_functions->senddata = lis2_HD44780_senddata;
	p->hd44780_functions->close = lis2_HD44780_close;

	common_init(p, IF_8BIT);

	return 0;
}


/**
 * Close the device.
 * \param p          Pointer to driver's private data structure.
 */
void
lis2_HD44780_close(PrivateData *p) {
	if (p->fd >= 0) {
		close(p->fd);
	}
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void lis2_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
static int mode = 0;
static unsigned char charNum = 0;
static unsigned char rowNum = 0;

	if (flags == RS_DATA) {	// RS_DATA: we have data
		if (mode == SETCHAR) {
			if (p->connectiontype == HD44780_CT_LIS2) {
				if (rowNum < p->cellheight)
					// define row for custom char
					setLIS2CustomCharRow(p->fd, charNum, rowNum, ch);
				else
					mode = 0;	// reset mode
			}

			// increase row counter
			rowNum++;

			if (p->connectiontype == HD44780_CT_MPLAY) {
				if (rowNum == p->cellheight) {
					// set all custom chars
					setMPlayCustomChars(p->fd, p->cc);
					// give enough time for the commands to exec
					p->hd44780_functions->uPause(p, 40);

					mode = 0;	// reset mode
				}
			}
		}
		else {
			if (p->connectiontype == HD44780_CT_LIS2) {
				// offset given char by one to avoid collisions with command prefix
				// TODO: check if this is correct
				if (ch < 7)
					ch++;
			}
			else {	// p->connectiontype == HD44780_CT_MPLAY
				// 0x00 is the command prefix so we can't use it.
				// Luckily, there are 16 custom char positions & the
				// second 8 are identical to the first 8 so we can
				// just offset it
				if (ch < NUM_CCs)
					ch += NUM_CCs;
			}

			// simply write data/character byte
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

			if (p->connectiontype == HD44780_CT_LIS2) {
				// extract character to set from given info
				charNum = (ch & ~SETCHAR) / 8;

				// offset it by one to avoid collisions with command prefix
				// TODO: check if this is correct
				charNum++;

				// restrict it to range [1, 7]
				// TODO: check if this is correct
				if (charNum == 8)
					charNum = 7;
			}
			// MPlay devices don't need the char num;
			// they always set all custom characters

			// reset row number
			rowNum = 0;
		}
	 	else
			// simply write instruction byte
			write(p->fd, &ch, 1);
	}
}


// hardware specific functions common to all device types

/**
 * Clear device screen.
 * \param fd  File handle to write to.
 */
static void clearScreen(int fd)
{
	writeChar(fd, 0);
	writeChar(fd, 0xA0);
}


/** Move cursor to given character coordinates.
 * \param fd  File handle to write to.
 * \param x   Character column to set cursor to.
 * \param y   Character row to set cursor to.
 */
static void gotoXY(int fd, unsigned char x, unsigned char y)
{
	writeChar(fd, 0);
	writeChar(fd, 0xA1 + y);
	writeChar(fd, x);
	writeChar(fd, 0xA7);
}


// hardware specific functions specific for a special device type

/**
 * Set a custom character, valid on MPlay devices only.
 * \param fd     File handle to write to.
 * \param chars  Pixel definition of all custom chars.
 */
static void setMPlayCustomChars(int fd, CGram *chars)
{
	// chars *must* be 8x8
	int i, row;

	writeChar(fd, 0);
	writeChar(fd, 0xAD);

	for (i = 0; i < NUM_CCs; i++) {
	        for (row = 0; row < 8; row++) {	// 8 == p->cellheight
		        writeChar(fd, chars[i].cache[row]);
		}
	}
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
	writeChar(fd, 0xAB);
	writeChar(fd, custom);
	writeChar(fd, row);
	writeChar(fd, data);
}


/**
 * Read temperatures, valid only on MPlay devices.
 * \param fd     File handle to read from.
 * \param temps  Pointer to pre-allocated array to store the temperatures.
 * \param num    Length of temperature array.
 * \retval 0     Error.
 * \retval 1     Temperatures read successfully.
 */
static char readMPlayTemps(int fd, int *temps, int num)
{
	unsigned char raw[num];
	int i;

	writeChar(fd, 0);
	writeChar(fd, 0xAF);

	if (read(fd, raw, num) == num) {
	        for (i = 0; i < num; i++)
		        temps[i] = (int) raw[i] - 150;
		return 1;
	}

	return 0;
}


/**
 * initialize fans, valid only on MPlay devices.
 * \param fd     File handle to write to.
 */
static void initMPlayFans(int fd)
{
	writeChar(fd, 0);
	writeChar(fd, 0xA4);
	writeChar(fd, 0x7D);
}


/**
 * Set fans, valid on MPlay devices only.
 * \param fd    File handle to write to.
 * \param fans  Array holding fan information
 * \param num   Length of fan array.
 */
static void setMPlayFans(int fd, unsigned char *fans, int num)
{
	int i;

	writeChar(fd, 0);
	writeChar(fd, 0xAC);

	for (i = 0; i < num; i++)
	        writeChar(fd, fans[i]);
}


/**
 * Set fans, valid on LIS2 devices only. ???
 * \param fd    File handle to write to.
 * \param fan1  Data for 1st fan.
 * \param fan2  Data for 2nd fan.
 * \param fan3  Data for 3rd fan.
 * \param fan4  Data for 4th fan.
 */
static void setLIS2Fans(int fd, int fan1, int fan2, int fan3, int fan4)
{
	writeChar(fd, 0);
	writeChar(fd, 0xAE);
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


// basic functions

/**
 * Write a character.
 * \param fd    File handle to write to.
 * \param code  Character to write.
 */
static void writeChar(int fd, unsigned char code)
{
	write(fd, &code, 1);
}

/* EOF */
