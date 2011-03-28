/** \file server/drivers/hd44780-usbtiny.c
 * \c USBtiny connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * http://www.xs4all.nl/~dicks/avr/usbtiny/
 */

/*-
 * Copyright (C) 2007 Peter Marschall <peter@adpm.de>
 *               2010 Siarhei Herasimchuk <sergby@tut.by>
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "hd44780-usbtiny.h"
#include "hd44780-low.h"
#include "report.h"


/* connection type specific functions to be exposed using pointers in init() */
void usbtiny_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void usbtiny_HD44780_close(PrivateData *p);


/**
 * Pseudo (empty) uPause function.
 * Override the uPause function as a usbtiny device takes care of timing itself.
 * \param p      Pointer to PrivateData structure (unused)
 * \param usecs  Number of micro-seconds to sleep (ignored)
 */
void
usbtiny_HD44780_uPause(PrivateData *p, int usecs)
{
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_usbtiny(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	struct usb_bus *bus;

	p->hd44780_functions->senddata = usbtiny_HD44780_senddata;
	p->hd44780_functions->close = usbtiny_HD44780_close;

	/* try to find USB device */
	usb_init();
	usb_find_busses();
	usb_find_devices();

	p->usbHandle = NULL;
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev != NULL; dev = dev->next) {

			/* Check if this device is a usbtiny device */
			if ((dev->descriptor.idVendor == USBTINY_VENDORID) &&
			 (dev->descriptor.idProduct == USBTINY_PRODUCTID)) {

				/* USBtiny device found; try to open it */
				p->usbHandle = usb_open(dev);
				if (p->usbHandle == NULL) {
					report(RPT_WARNING, "hd_init_usbtiny: unable to open device");
					/*
					 * Do not exit here because other
					 * (possibly working) devices may be
					 * connected.
					 */
				}
				else {
					report(RPT_INFO, "hd_init_usbtiny: USBtiny device found");
				}
			}
		}
	}

	/* No device found at all */
	if (p->usbHandle == NULL) {
		report(RPT_ERR, "hd_init_usbtiny: no (matching) USBtiny device found");
		return -1;
	}

	common_init(p, IF_4BIT);

	/* replace uPause with empty one after initialization */
	p->hd44780_functions->uPause = usbtiny_HD44780_uPause;

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
usbtiny_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	int type = (flags == RS_DATA) ? USBTINY_DATA : USBTINY_INSTR;
	char buffer[1];
	buffer[0] = (char) ch;

	usb_control_msg(p->usbHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, type, 0, 0, buffer, 1, 1000);
}


/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
usbtiny_HD44780_close(PrivateData *p)
{
	if (p->usbHandle != NULL) {
		usb_close(p->usbHandle);
		p->usbHandle = NULL;
	}
}

/* EOF */
