/** \file server/drivers/hd44780-usb4all.c
 * USB-4-all connection type of hd44780 driver for Hitachi HD44780 based
 * LCD displays connected to an USB-4-all controller.
 *
 * USB-4-all is PIC based USB controller, see
 * http://www.sprut.de/electronic/pic/projekte/usb4all/usb4all.htm
 *
 * It supports
 * - up to two displays with each up to 2x40 or 4x20 lines/characters.
 * - contrast control (PWM1)
 * - brightness control (PWM2)
 * - 4x4 keypad (RA0 - RA3 - X lines, RA4, RA5, RC6, RC7 - Y lines)
 *
 * Only the MCD version is supported!
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <usb.h>

#include "hd44780-usb4all.h"
#include "hd44780-low.h"
#include "report.h"

/* connection type specific functions to be exposed using pointers in init() */
void usb4all_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void usb4all_HD44780_close(PrivateData *p);
void usb4all_HD44780_backlight(PrivateData *p, unsigned char state);
void usb4all_HD44780_set_contrast(PrivateData *p, unsigned char value);
unsigned char usb4all_HD44780_readkeypad(PrivateData *p, unsigned int Ydata);

/**
 * Send a command with data to usb4all controller and reads the answer.
 * Even if some inputs should be read a command to do this has to send before.
 * \param p            pointer to private data structure
 * \param data_out     pointer to data send to controller
 * \param data_in      pointer to buffer where read in data is stored
 * \return             number of bytes read in
 */
int
usb4all_data_io(PrivateData *p, tx_buffer * data_out, tx_buffer * data_in)
{
	int res;

	if (p->usbMode == MODE_BULK) {
		res = usb_bulk_write(p->usbHandle,
				     p->usbEpOut,
				     (char *)data_out->buffer,
				     data_out->use_count,
				     TIMEOUT_WRITE);
	}
	else {
		res = usb_interrupt_write(p->usbHandle,
					  p->usbEpOut,
					  (char *)data_out->buffer,
					  data_out->use_count,
					  TIMEOUT_WRITE);
	}

	if (res < 0) {
		p->hd44780_functions->drv_report(RPT_WARNING,
		   "usb4all_data_io: unable to send, result = %d ...", res);
		return -1;
	}

	/* if a reset command is send the usb4all controller doesn't answer */
	if (data_out->buffer[0] == USB4ALL_RESET) {
		return 0;
	}

	if (res != data_out->use_count) {
		p->hd44780_functions->drv_report(RPT_WARNING,
						 "usb4all_data_io: Want to send %d bytes but currently only %d bytes was send!?",
						 data_out->use_count, res);
		return -1;
	}

	if (p->usbMode == MODE_BULK) {
		res = usb_bulk_read(p->usbHandle,
				    p->usbEpIn,
				    (char *)data_in->buffer,
				    USB4ALL_RX_MAX,
				    TIMEOUT_READ);
	}
	else {
		res = usb_interrupt_read(p->usbHandle,
					 p->usbEpIn,
					 (char *)data_in->buffer,
					 USB4ALL_RX_MAX,
					 TIMEOUT_READ);
	}

	return res;
}

/**
 * Set the pwm value on the pwm channel
 * \param p            pointer to private date structure
 * \param pwm_channel  the pwm channel
 * \param value        the high-part of each pwm cylce
 */
void
usb4all_set_pwm_value(PrivateData *p, unsigned char pwm_channel, unsigned char value)
{
	p->tx_buf.buffer[0] = 0x56 + pwm_channel;
	p->tx_buf.buffer[1] = 0x02;
	p->tx_buf.buffer[2] = value;
	p->tx_buf.buffer[3] = 0x00;
	p->tx_buf.use_count = 4;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);
}

/**
 * Init the usb-4-all controller part for PWM
 * The PWM outputs can be used to control bightness and contrast
 * \param p            pointer to private date structure
 * \param pwm_channel  channel to initialize (1 or 2)
 */
void
usb4all_init_pwm(PrivateData *p, unsigned char pwm_channel)
{
	p->hd44780_functions->drv_report(RPT_INFO,
		"init usb4all controller for PWM channel %d ", pwm_channel);

	p->tx_buf.buffer[0] = 0x056 + pwm_channel;
	p->tx_buf.buffer[1] = 0x01;
	p->tx_buf.buffer[2] = 0x05;
	p->tx_buf.use_count = 3;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);
}

/**
 * Init the usb-4-all controller part for digital I/O (keypad)
 * \param p          pointer to private date structure
 */
void
usb4all_init_keypad(PrivateData *p)
{
	p->hd44780_functions->drv_report(RPT_INFO, "init usb4all controller for keypad");

	/* Input  lines (X) RA0, RA1, RA2, RA3 */
	p->tx_buf.buffer[0] = 0x50;
	p->tx_buf.buffer[1] = 0x04;
	p->tx_buf.buffer[2] = 0x0F;
	p->tx_buf.buffer[3] = 0x00;
	p->tx_buf.buffer[4] = 0x00;
	p->tx_buf.use_count = 5;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);

	/* Output lines (Y) RA4, RA5, RC6, RC7 */
	p->tx_buf.buffer[0] = 0x50;
	p->tx_buf.buffer[1] = 0x05;
	p->tx_buf.buffer[2] = 0x30;
	p->tx_buf.buffer[3] = 0x00;
	p->tx_buf.buffer[4] = 0xC0;
	p->tx_buf.use_count = 5;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);
}

/**
 * Init the usb-4-all controller part for LCD displays
 * \param p          pointer to private date structure
 * \param displayID  display to init
 * \param rows       number of lines
 * \param cols       number of characters
 */
void
usb4all_init_lcd(PrivateData *p, unsigned char displayID, unsigned char rows, unsigned char cols)
{
	p->hd44780_functions->drv_report(RPT_INFO,
		      "init usb4all controller for display %d ", displayID);

	p->tx_buf.buffer[0] = 0x054 + displayID;
	p->tx_buf.buffer[1] = 0x01;
	p->tx_buf.buffer[2] = rows;
	p->tx_buf.buffer[3] = cols;
	p->tx_buf.use_count = 4;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);
}

/**
 * Init the usb-4-all controller
 * \param p          pointer to private date structure
 */
void
usb4all_init(PrivateData *p)
{
	usb4all_init_lcd(p, 1, p->dispVOffset[0], p->width);
	if (p->numDisplays == 2) {
		usb4all_init_lcd(p, 2, p->dispVOffset[1], p->width);
	}

	usb4all_init_pwm(p, USB4ALL_PWM_CONTRAST);
	if (p->have_backlight) {
		usb4all_init_pwm(p, USB4ALL_PWM_BRIGHTNESS);
	}

	if (p->have_keypad) {
		usb4all_init_keypad(p);
	}
}

/**
 * Identifies the usb transfer mode and endpoint addresses.
 * Values are stored in the corresponding private data
 * fields (usbMode, usbEpIn and usbEpOut).
 * \param p      Pointer to PrivateData structure
 * \param iface  Pointer to USB interface descriptor
 */
void
usb4all_determine_usb_params(PrivateData *p, struct usb_interface_descriptor *iface)
{
	p->usbMode = -1;

	if ((iface->endpoint[0].bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT &&
	    (iface->endpoint[1].bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT) {
		p->usbMode = MODE_INT;
	}
	if ((iface->endpoint[0].bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK &&
	    (iface->endpoint[1].bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {
		p->usbMode = MODE_BULK;
	}

	if (p->usbMode == -1) {
		p->hd44780_functions->drv_report(RPT_ERR,
				  "Unsupported USB_ENDPOINT_TYPE = %d / %d",
		   iface->endpoint[0].bmAttributes & USB_ENDPOINT_TYPE_MASK,
		  iface->endpoint[1].bmAttributes & USB_ENDPOINT_TYPE_MASK);
		return;
	}

	if (iface->endpoint[0].bEndpointAddress & USB_ENDPOINT_DIR_MASK) {
		p->usbEpIn = iface->endpoint[0].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK;
		p->usbEpOut = iface->endpoint[1].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK;
	}
	else {
		p->usbEpIn = iface->endpoint[1].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK;
		p->usbEpOut = iface->endpoint[0].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK;
	}
}

/**
 * Pseudo (empty) uPause function.
 * Override the uPause function as a usb4all device takes care of timing itself.
 * \param p      Pointer to PrivateData structure (unused)
 * \param usecs  Number of micro-seconds to sleep (ignored)
 */
void
usb4all_HD44780_uPause(PrivateData *p, int usecs)
{
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_usb4all(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	struct usb_bus *bus;

	p->hd44780_functions->senddata = usb4all_HD44780_senddata;
	p->hd44780_functions->close = usb4all_HD44780_close;

	p->hd44780_functions->set_contrast = usb4all_HD44780_set_contrast;
	p->hd44780_functions->backlight = usb4all_HD44780_backlight;
	p->hd44780_functions->readkeypad = usb4all_HD44780_readkeypad;

	/* try to find USB device */
	usb_init();
	usb_find_busses();
	usb_find_devices();

	p->usbHandle = NULL;
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev != NULL; dev = dev->next) {

			/* Check if this device is a usb4all device */
			if ((dev->descriptor.idVendor == USB4ALL_VENDORID) &&
			 (dev->descriptor.idProduct == USB4ALL_PRODUCTID)) {

				/* USB-4-all device found; try to open it */
				p->usbHandle = usb_open(dev);
				if (p->usbHandle == NULL) {
					report(RPT_WARNING, "hd_init_usb4all: unable to open device");
					/*
					 * Do not exit here because other
					 * (possibly working) devices may be
					 * connected.
					 */
				}
				else {
					report(RPT_INFO, "hd_init_usb4all: USB-4-all device found");
					usb4all_determine_usb_params(p, &dev->config[0].interface[0].altsetting[0]);
				}
			}
		}
	}

	/* No device found at all */
	if (p->usbHandle == NULL) {
		report(RPT_ERR, "hd_init_usb4all: no (matching) USB-4-all device found");
		return -1;
	}

	if (p->usbMode == -1) {
		report(RPT_ERR, "hd_init_usb4all: unknown usb mode");
		return -1;
	}

	/* allocate and initialize buffer */
	if ((p->tx_buf.buffer = malloc(USB4ALL_TX_MAX)) == NULL) {
		report(RPT_ERR, "hd_init_usb4all: could not allocate send buffer");
		usb4all_HD44780_close(p);
		return -1;
	}

	if ((p->rx_buf.buffer = malloc(USB4ALL_RX_MAX)) == NULL) {
		report(RPT_ERR, "hd_init_usb4all: could not allocate receive buffer");
		usb4all_HD44780_close(p);
		return -1;
	}

	common_init(p, IF_4BIT);

	/* replace uPause with empty one after initialization */
	p->hd44780_functions->uPause = usb4all_HD44780_uPause;

	/* initialize usb-4-all controller */
	usb4all_init(p);

	return 0;
}

/**
 * Send a single byte of data to the LCD.
 * \param p          pointer to private date structure
 * \param displayID  display to send data to (0 = all displays)
 * \param flags      data or instruction command (RS_DATA | RS_INSTR)
 * \param ch         character to display or instruction value
 */
void
usb4all_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	if (displayID == 0) {
		usb4all_HD44780_senddata(p, 1, flags, ch);
		if (p->numDisplays == 2) {
			usb4all_HD44780_senddata(p, 2, flags, ch);
		}
		return;
	}

	p->tx_buf.buffer[0] = 0x54 + displayID;
	p->tx_buf.buffer[1] = (flags == RS_DATA) ? 0x02 : 0x03;
	p->tx_buf.buffer[2] = ch;
	p->tx_buf.use_count = 3;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);
}

/**
 * Turn display backlight on or off.
 * Backlight is turned on or off by toggeling between the brightness
 * and offbrightness values.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
usb4all_HD44780_backlight(PrivateData *p, unsigned char state)
{
	/* get backlight brightness */
	int promille = (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	p->hd44780_functions->drv_debug(RPT_DEBUG,
	    "usb4all_HD44780_backlight: Setting backlight to %d", promille);

	/* and set it (converted from [0,1000] -> [255,0]) */
	usb4all_set_pwm_value(p, USB4ALL_PWM_BRIGHTNESS, ((1000 - promille) * 255) / 1000);
}

/**
 * Change display contrast.
 * \param p      Pointer to driver's private data structure.
 * \param value  New contrast value (one byte).
 */
void
usb4all_HD44780_set_contrast(PrivateData *p, unsigned char value)
{
	p->hd44780_functions->drv_debug(RPT_DEBUG,
	     "usb4all_HD44780_set_contrast: Setting contrast to %d", value);
	usb4all_set_pwm_value(p, USB4ALL_PWM_CONTRAST, value);
}

/**
 * Read the keypad
 * \param p      Pointer to driver's private data structure.
 * \param Ydata  the up to 4 bits that should be put on the Y side of the matrix
 * \return       the up to 4 bits that are read out on the X side of the matrix
 */
unsigned char
usb4all_HD44780_readkeypad(PrivateData *p, unsigned int Ydata)
{
	/* adjust and invert bits */
	Ydata = ((Ydata << 4) ^ 0xF0);

	/* Output lines (Y) are RA4, RA5, RC6, RC7 */
	p->tx_buf.buffer[0] = 0x50;
	p->tx_buf.buffer[1] = 0x02;
	p->tx_buf.buffer[2] = Ydata & 0x30;
	p->tx_buf.buffer[3] = 0x00;
	p->tx_buf.buffer[4] = Ydata & 0xC0;
	p->tx_buf.use_count = 5;

	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);

	/* Read RA3 RA2 RA1 RA0 */
	p->tx_buf.buffer[0] = 0x50;
	p->tx_buf.buffer[1] = 0x03;
	p->tx_buf.use_count = 2;
	usb4all_data_io(p, &p->tx_buf, &p->rx_buf);

	/* filter and invert the result */
	return ((p->rx_buf.buffer[2] & 0x0F) | 0xF0) ^ 0xFF;
}

/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
usb4all_HD44780_close(PrivateData *p)
{
	if (p->usbHandle != NULL) {
		if (p->have_backlight) {
			usb4all_HD44780_backlight(p, BACKLIGHT_OFF);
		}

		usb_close(p->usbHandle);
		p->usbHandle = NULL;
	}

	if (p->tx_buf.buffer != NULL) {
		free(p->tx_buf.buffer);
		p->tx_buf.buffer = NULL;
	}

	if (p->rx_buf.buffer != NULL) {
		free(p->rx_buf.buffer);
		p->rx_buf.buffer = NULL;
	}

}

/* EOF */
