/** \file server/drivers/hd44780-usb4all.h
 * USB-4-all connection type of hd44780 driver for Hitachi HD44780 based
 * LCD displays connected to an USB-4-all controller.
 */

/*-
 * Copyright (C) 2012 Thomas Mohaupt <thomas.mohaupt@gmail.com>
 *               2010 Roland Wundrig <http://wundrig.de/>
 *
 * Some code parts are adapted from Roland Wundrig linux demo code
 * (part of usb4all package)
 *
 * This file is released under the GNU General Public License version 2.
 * Refer to the COPYING file distributed with this package.
 */

#ifndef HD_USB4ALL_H
#define HD_USB4ALL_H

#include "lcd.h"		/* for Driver */

/* vendor and product id */
#define USB4ALL_VENDORID   0x004D8
#define USB4ALL_PRODUCTID  0x0FF0B

/* USB read/write timeouts */
#define TIMEOUT_READ   1000
#define TIMEOUT_WRITE  1000

/* USB transfer modes */
#define MODE_INT   0x04		/* usb interface : interrup mode */
#define MODE_BULK  0x08		/* usb interface : bulk mode */

#define USB4ALL_TX_MAX  64	/* usb4all tx buffer size */
#define USB4ALL_RX_MAX  16	/* usb4all rx buffer size */

/* assignment of PWM channels */
#define USB4ALL_PWM_CONTRAST    0x01	/* PWM channel used for contrast */
#define USB4ALL_PWM_BRIGHTNESS  0x02	/* PWM channel used for brightness */

#define USB4ALL_RESET   0xFF

/* initialise this particular driver */
int hd_init_usb4all(Driver *drvthis);

#endif
