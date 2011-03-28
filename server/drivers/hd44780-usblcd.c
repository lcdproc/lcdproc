/** \file server/drivers/hd44780-usblcd.c
 * \c ucdlcd connection type of \c hd44780 driver for Hitachi HD44780 based LCD
 * displays. This driver does only work with Linux and requires a kernel driver
 * available since 2.4.20-pre7.
 *
 * See http://www.usblcd.de for hardware and documentation.
 */

/*
 * Copyright (C)  2002 Adams IT Services <info@usblcd.de>
 *
 * This driver is based on pic-an-lcd driver. See file hd44780-serial.c for
 * additional Copyrights.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Modified February 2009 by Markus Dolze to use API v0.5.
 */

#include "hd44780-usblcd.h"
#include "hd44780-low.h"
#include "report.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define DEFAULT_DEVICE  "/dev/usb/lcd"

#define IOCTL_GET_HARD_VERSION  1
#define IOCTL_GET_DRV_VERSION	2

void usblcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void usblcd_HD44780_backlight(PrivateData *p, unsigned char state);
void usblcd_HD44780_close(PrivateData *p);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error opening device.
 * \retval -2      Error reading or unsupported driver version.
 * \retval -3      Error reading or unsupported hardware version.
 */
int
hd_init_usblcd(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	char device[256] = DEFAULT_DEVICE;
	char buf[128];
	int major, minor;

	/* Get device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "HD44780: USBLCD: using device: %s", device);

	/* Set up io port correctly, and open it... */
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "HD44780: USBLCD: could not open device %s (%s)", device, strerror(errno));
		return -1;
	}

	/* Read and check kernel driver version */
	memset(buf, 0, 128);
	if (ioctl(p->fd, IOCTL_GET_DRV_VERSION, buf) != 0) {
		report(RPT_ERR, "IOCTL failed, could not get Driver Version");
		return -2;
	}
	report(RPT_INFO, "Driver Version: %s", buf);

	if (sscanf(buf, "USBLCD Driver Version %d.%d", &major, &minor) != 2) {
		report(RPT_ERR, "Could not read Driver Version");
		return -2;
	}
	if (major != 1) {
		report(RPT_ERR, "Driver Version not supported");
		return -2;
	}

	/* Read and check hardware version */
	memset(buf, 0, 128);
	if (ioctl(p->fd, IOCTL_GET_HARD_VERSION, buf) != 0) {
		report(RPT_ERR, "IOCTL failed, could not get Hardware Version");
		return -3;
	};
	report(RPT_INFO, "Hardware Version: %s", buf);
	if (sscanf(buf, "%d.%d", &major, &minor) != 2) {
		report(RPT_ERR, "Could not read Hardware Version");
		return -3;
	};
	if (major != 1) {
		report(RPT_ERR, "Hardware Version not supported");
		return -3;
	}

	/* Set local functions */
	p->hd44780_functions->senddata = usblcd_HD44780_senddata;
	p->hd44780_functions->backlight = usblcd_HD44780_backlight;
	p->hd44780_functions->close = usblcd_HD44780_close;

	common_init(p, IF_8BIT);
	return 0;
}

/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
usblcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	static const char instr_byte = 0;

	if (flags == RS_DATA) {
		/*
		 * Escape data byte 0x00 by sending it twice because it is
		 * used as instruction byte
		 */
		if (ch == '\0')
			write(p->fd, &ch, 1);
		write(p->fd, &ch, 1);
	} else {
		/* Instructions are sent prepending 0x00 as escape sequence */
		write(p->fd, &instr_byte, 1);
		write(p->fd, &ch, 1);
	}
}

/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
usblcd_HD44780_backlight(PrivateData *p, unsigned char state)
{
	static const char instr_byte = 0;
	static const char bl_on = 0x21;
	static const char bl_off = 0x20;

	write(p->fd, &instr_byte, 1);

	if (state == BACKLIGHT_OFF)
		write(p->fd, &bl_off, 1);
	else
		write(p->fd, &bl_on, 1);
}

/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
usblcd_HD44780_close(PrivateData *p)
{
	if (p->fd >= 0)
		close(p->fd);
}

/* EOF */
