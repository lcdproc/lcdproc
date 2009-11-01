/** \file server/drivers/serialVFD_io.c
 * Hardware dependent routines for the \c serialVFD driver.
 * Here are the routines for initializing, writing to, and closing the
 * serial resp. parallel port for the \c serialVFD driver.
 */

/* 	This file is part the LCDproc driver for various serial VFD Devices.

	It contains the hardwaredependent commands ach characterset.

	If you want to add a new device to the driver add a new section
	to the displaytype-switch-case in the init-function, add a new load_...
	function below and fill it with the corrrect commands for the display.
	(Try wich displaytype works best with your display, copy and modify
	it's section that is the easiest way I guess.)

	Copyright (C) 2006 Stefan Herdler

	2006-05-15 Version 0.3: everything should work (not all hardware tested!)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

*/


#include "port.h"
#include "serialVFD_io.h"
#include "serialVFD.h"
#include "lcd.h"

#define WR_on  0x10
#define WR_off 0x11
#define Busy   0x80
#define LPTPORT 0x378

#define MAXBUSY 300

void
serialVFD_write_serial (Driver *drvthis, unsigned char *dat, size_t length)
{
	PrivateData *p = drvthis->private_data;

	if (length <= 0)
		return;

	write(p->fd,dat,length);
}

void
serialVFD_write_parallel (Driver *drvthis, unsigned char *dat, size_t length)
{
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	PrivateData *p = drvthis->private_data;
	int i_para, j_para;

	if (length <= 0)
		return;

	for (i_para = 0; i_para < length; i_para++) {
		port_out(p->port, dat[i_para]);

		if (p->para_wait > 2)	// some displays need a little time to rest
			port_in(p->port+1);

		port_out(p->port+2, WR_on);

		if (p->para_wait > 1)	// some displays need a little time to rest
			port_in(p->port+1);

		port_out(p->port+2, WR_off);

		if (p->para_wait > 0)	// some displays need a little time to rest
			port_in(p->port+1);

		for (j_para = 0; j_para < MAXBUSY; j_para++) {
			if ((port_in(p->port+1)) & Busy)
				break;
		}

		for (j_para = 3; j_para < p->para_wait; j_para++) // some displays need a little longer time to rest
			port_in(p->port+1);

	}
#endif
}

int
serialVFD_init_serial (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	struct termios portset;

	/* Set up io port correctly, and open it...*/
	debug( RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (p->fd == -1) {
		report(RPT_ERR, "%s: open() of %s failed (%s)", __FUNCTION__, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw( &portset );
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	// Set port speed
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, B0);

	// Do it...
	tcsetattr(p->fd, TCSANOW, &portset);
	return 0;
}

int
serialVFD_init_parallel (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	debug(RPT_DEBUG, "%s: Opening parallelport at: 0x%X", __FUNCTION__, p->port);
	if (port_access_multiple(p->port,3)) {
		report(RPT_ERR, "%s: port_access_multiple() of 0x%X failed (%s)", __FUNCTION__, p->port, strerror(errno));
		return -1;
	}
	return 0;
#else
	report(RPT_ERR, "%s: LCDproc was compiled without PCstyle LPT support", __FUNCTION__);
	return -1;
#endif
}

void
serialVFD_close_serial (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p->fd >= 0)
		close(p->fd);
}

void
serialVFD_close_parallel (Driver *drvthis)
{
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: Closing parallelport at: 0x%X", __FUNCTION__, p->port);
	if (port_deny_multiple(p->port,3)) {
		report(RPT_ERR, "%s: port_deny_multiple() of 0x%X failed (%s)", __FUNCTION__, p->port, strerror(errno));
	}
#endif
}
