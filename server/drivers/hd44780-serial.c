/** \file hd44780-serial.c
 * Connection types \c picanlcd, \c lcdserializer, \c los-panel, \c vdr-lcd,
 * \c vdr-wakeup, \c pertelian, ... of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 */

/* Copyright (C) 2006-2007  Matteo Pillon <matteo.pillon@gmail.com>
 *
 * Some parts are based on the original pic-an-lcd driver code
 *  Copyright (C) 1997, Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *                1998, Richard Rognlie <rrognlie@gamerz.net>
 *                1999, Ethan Dicks
 *                1999-2000, Benjamin Tse <blt@Comports.com>
 *                2001, Rene Wagner
 *                2001-2002, Joris Robijn <joris@robijn.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "hd44780-serial.h"
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

#define SERIAL_IF serial_interfaces[p->serial_type]

/* bitrate conversion */
unsigned int bitrate_conversion[][2] = {
	{ 50, B50 },
	{ 75, B75 },
	{ 110, B110 },
	{ 134, B134 },
	{ 150, B150 },
	{ 200, B200 },
	{ 300, B300 },
	{ 600, B600 },
	{ 1200, B1200 },
	{ 1800, B1800 },
	{ 2400, B2400 },
	{ 4800, B4800 },
	{ 9600, B9600 },
	{ 19200, B19200 },
	{ 38400, B38400 },
	{ 57600, B57600 },
	{ 115200, B115200 }
#if defined(B230400)
	, { 230400, B230400 }
#endif	
#if defined(B460800)	
	, { 460800, B460800 }
#endif
#if defined(B500000)
	, { 500000, B500000 }
#endif
#if defined(B576000)
	, { 576000, B576000 }
#endif
#if defined(B921600)
	, { 921600, B921600 }
#endif
#if defined(B1000000)
	, { 1000000, B1000000 }
#endif
#if defined(B1152000)
	, { 1152000, B1152000 }
#endif
#if defined(B1500000)
	, { 1500000, B1500000 }
#endif
#if defined(B2000000)
	, { 2000000, B2000000 }
#endif
#if defined(B2500000)
	, { 2500000, B2500000 }
#endif
#if defined(B3000000)
	, { 3000000, B3000000 }
#endif
#if defined(B3500000)
	, { 3500000, B3500000 }
#endif
#if defined(B4000000)
	, { 4000000, B4000000 }
#endif
};

int convert_bitrate(unsigned int conf_bitrate, size_t *bitrate) {
	int counter;
	for (counter = 0; counter < sizeof(bitrate_conversion)/(2*sizeof(unsigned int)); counter++)
		if (bitrate_conversion[counter][0] == conf_bitrate) {
			*bitrate = (size_t) bitrate_conversion[counter][1];
			return 0;
		}
	return 1;
}

static int lastdisplayID;

void serial_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void serial_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char serial_HD44780_scankeypad(PrivateData *p);
void serial_HD44780_close(PrivateData *p);

// initialize the driver
int
hd_init_serial(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;

	struct termios portset;
	char device[256] = DEFAULT_DEVICE;

	/* READ CONFIG FILE */

	/* Get interface type */
	int i;

	p->serial_type = 0;
	for (i = 0; serial_interfaces[i].connectiontype != HD44780_CT_UNKNOWN; i++) {
		if (p->connectiontype == serial_interfaces[i].connectiontype) {
			p->serial_type = i;
			break;
		}
	}
	if (p->serial_type != i) {
		report(RPT_ERR, "HD44780: serial: unknown connection type");
		return -1;
	}

	/* Check if user knows the capabilities of his hardware ;-) */
	if (p->have_keypad && !(SERIAL_IF.keypad)) {
		report(RPT_ERR, "HD44780: serial: keypad is not supported by connection type");
		report(RPT_ERR, "HD44780: serial: check your configuration file and disable it");
		return -1;
	}
	if (p->have_backlight && !(SERIAL_IF.backlight)) {
		report(RPT_ERR, "HD44780: serial: backlight control is not supported by connection type");
		report(RPT_ERR, "HD44780: serial: check your configuration file and disable it");
		return -1;
	}

	/* Get bitrate */
	unsigned int conf_bitrate;
	size_t bitrate;

	conf_bitrate = drvthis->config_get_int(drvthis->name, "Speed", 0, SERIAL_IF.default_bitrate);
	if (conf_bitrate == 0)
		conf_bitrate = SERIAL_IF.default_bitrate;
	if (convert_bitrate(conf_bitrate, &bitrate)) {
		report(RPT_ERR, "HD44780: serial: invalid configured bitrate speed");
		return -1;
	}
	report(RPT_INFO,"HD44780: serial: using speed: %d", conf_bitrate);

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO,"HD44780: serial: using device: %s", device);

	/* Set up io port correctly, and open it... */
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "HD44780: serial: could not open device %s (%s)", device, strerror(errno));
		return -1;
	}

	/* Get serial device parameters */
	tcgetattr(p->fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
	portset.c_cflag |= CLOCAL;
#else
	/* The hard way */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	/* Set port speed */
	cfsetospeed(&portset, bitrate);
	cfsetispeed(&portset, B0);

	/* Set TCSANOW mode of serial device */
	tcsetattr(p->fd, TCSANOW, &portset);

	lastdisplayID = -1;

	/* Assign functions */
	p->hd44780_functions->senddata = serial_HD44780_senddata;
	p->hd44780_functions->backlight = serial_HD44780_backlight;
	p->hd44780_functions->scankeypad = serial_HD44780_scankeypad;
	p->hd44780_functions->close = serial_HD44780_close;

	/* Do initialization */
	if (SERIAL_IF.if_bits == 8) {
		report(RPT_INFO,"HD44780: serial: initializing with 8 bits interface");
		common_init(p, IF_8BIT);
	} else {
		report(RPT_INFO,"HD44780: serial: initializing with 4 bits interface");
		common_init(p, IF_4BIT);
	}

	return 0;
}

// serial_HD44780_senddata
void
serial_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	/* Filter illegally sent escape characters (for interfaces without data escape) */
	if (flags == RS_DATA && SERIAL_IF.data_escape == 0 && ch == SERIAL_IF.instruction_escape)
		ch='?';

	if (flags == RS_DATA) {
		/* Do we need a DATA indicator byte? */
		if ((SERIAL_IF.data_escape != '\0') &&
		    (((ch >= SERIAL_IF.data_escape_min) &&
		      (ch < SERIAL_IF.data_escape_max)) ||
		     (SERIAL_IF.multiple_displays && displayID != lastdisplayID))) {
			write(p->fd, &SERIAL_IF.data_escape + displayID, 1);
		}
		write(p->fd, &ch, 1);
	}
	else {
		write(p->fd, &SERIAL_IF.instruction_escape, 1);
		write(p->fd, &ch, 1);
	}
	lastdisplayID = displayID;
}

void
serial_HD44780_backlight(PrivateData *p, unsigned char state)
{
	unsigned char send[1];
	if (p->have_backlight) {
		if (SERIAL_IF.backlight_escape) {
			send[0] = SERIAL_IF.backlight_escape;
			write(p->fd, &send, 1);
		}
		if (SERIAL_IF.backlight_on && SERIAL_IF.backlight_off) {
			send[0] = state ? SERIAL_IF.backlight_on : SERIAL_IF.backlight_off;
		}
		else {
			send[0] = state ? 0 : 0xFF;
		}
		write(p->fd, &send, 1);
	}
}

unsigned char
serial_HD44780_scankeypad(PrivateData *p)
{
	unsigned char buffer = 0;
	char hangcheck = 100;

	read(p->fd, &buffer, 1);
	if (buffer == SERIAL_IF.keypad_escape) {
		while (hangcheck > 0) {
			/* Check if I can read another byte */
			if (read(p->fd, &buffer, 1) == 1) {
				return buffer;
			}
			hangcheck--;
		}
	}
	return '\0';
}

void
serial_HD44780_close(PrivateData *p)
{
	if (SERIAL_IF.end_code)
		write(p->fd, &SERIAL_IF.end_code, 1);
	close(p->fd);
}
