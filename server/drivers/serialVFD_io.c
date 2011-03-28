/** \file server/drivers/serialVFD_io.c
 * Hardware dependent routines for the \c serialVFD driver.
 * Here are the routines for initializing, writing to, and closing the
 * serial resp. parallel port for the \c serialVFD driver.
 */

/*-
 * Copyright (C) 2006 Stefan Herdler
 *
 * This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
 * driver. It may contain parts of other drivers of this package too.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * 2006-05-16 Version 0.3: everything should work (not all hardware tested!)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "lcd.h"
#include "report.h"
#include "lpt-port.h"
#include "port.h"
#include "serialVFD_io.h"
#include "serialVFD.h"

/*
 * XXX: ENIRQ used to be set, although no IRQ handler is defined. As the
 * schematic in the user guide shows BUSY connected to ACK, leave it as is.
 */
#define WR_on  ((ENIRQ | STRB) ^ OUTMASK)	/* STRB (1) is active low */
#define WR_off (ENIRQ ^ OUTMASK)

#define MAXBUSY 300

/**
 * Write bytes to the serial port.
 * \param drvthis  Pointer to driver
 * \param dat      Pointer to array storing the data
 * \param length   Number of bytes to write
 */
void
serialVFD_write_serial (Driver *drvthis, unsigned char *dat, size_t length)
{
	PrivateData *p = drvthis->private_data;

	if (length <= 0)
		return;

	write(p->fd,dat,length);
}

/**
 * Write bytes to the parallel port.
 *
 * Timing can be modified by inserting additional delays according to the
 * \c PortWait setting in \c LCDd.conf: \n
 * Portwait = 0; no delays, only BUSY bit checked\n
 * Portwait = 1; additional WR off to check BUSY delay\n
 * Portwait = 2; as 1 + additional WR pulse width\n
 * Portwait = 3; as 2 + additional WR setup delay\n
 * Portwait >= 4; as 3 + additional wait delay to next command
 *
 * \param drvthis  Pointer to driver
 * \param dat      Pointer to array storing the data
 * \param length   Number of bytes to write
 */
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

		/* data to WR enable delay */
		if (p->para_wait > 2)
			port_in(p->port+1);

		port_out(p->port+2, WR_on);

		/* additional WR pulse width */
		if (p->para_wait > 1)
			port_in(p->port+1);

		port_out(p->port+2, WR_off);

		/* additional BUSY delay time */
		if (p->para_wait > 0)
			port_in(p->port+1);

		/*
		 * Check MAXBUSY times if the BUSY bit is set. Note: BUSY bit
		 * on parallel port is hardware inverted, but VFD is ready
		 * if its BUSY bit is cleared (thus high on PC).
		 */
		for (j_para = 0; j_para < MAXBUSY; j_para++) {
			if ((port_in(p->port+1)) & BUSY)
				break;
		}

		/* wait time of next WR */
		for (j_para = 3; j_para < p->para_wait; j_para++)
			port_in(p->port+1);

	}
#endif
}

/**
 * Open a serial port according to the settings in \c serialVFD_private_data.
 * \param  drvthis  Pointer to driver
 * \return -1 on error, 0 on success
 */
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

	/* Set port speed */
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);
	return 0;
}

/**
 * Open a parallel port according to the settings in \c serialVFD_private_data.
 * \param  drvthis  Pointer to driver
 * \return -1 on error, 0 on success
 */
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

/**
 * Close serial port.
 * \param  drvthis  Pointer to driver
 */
void
serialVFD_close_serial (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p->fd >= 0)
		close(p->fd);
}

/**
 * Close parallel port.
 * \param  drvthis  Pointer to driver
 */
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
