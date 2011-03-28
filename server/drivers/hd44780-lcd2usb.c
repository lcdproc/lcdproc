/** \file server/drivers/hd44780-lcd2usb.c
 * \c lcd2usb connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * See http://www.harbaum.org/till/lcd2usb/.
 */

/*-
 * Copyright (C) 2007 Peter Marschall <peter@adpm.de>
 * 		 2007-2010 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "hd44780-lcd2usb.h"
#include "hd44780-low.h"
#include "report.h"

/* connection type specific functions to be exposed using pointers in init() */
void lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcd2usb_HD44780_scankeypad(PrivateData *p);
void lcd2usb_HD44780_close(PrivateData *p);
void lcd2usb_HD44780_set_contrast(PrivateData *p, unsigned char value);
void lcd2usb_HD44780_flush(PrivateData *p);


/**
 * Pseudo (empty) uPause function.
 * Override the uPause function as a lcd2usb device takes care of timing itself.
 * \param p      Pointer to PrivateData structure (unused)
 * \param usecs  Number of micro-seconds to sleep (ignored)
 */
void
lcd2usb_HD44780_uPause(PrivateData *p, int usecs)
{
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_lcd2usb(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	struct usb_bus *bus;

	p->hd44780_functions->senddata = lcd2usb_HD44780_senddata;
	p->hd44780_functions->backlight = lcd2usb_HD44780_backlight;
	p->hd44780_functions->scankeypad = lcd2usb_HD44780_scankeypad;
	p->hd44780_functions->close = lcd2usb_HD44780_close;
	p->hd44780_functions->set_contrast = lcd2usb_HD44780_set_contrast;
	p->hd44780_functions->flush = lcd2usb_HD44780_flush;

	/* try to find USB device */
#if 0
	usb_debug = 2;
#endif

	usb_init();
	usb_find_busses();
	usb_find_devices();

	p->usbHandle = NULL;
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev != NULL; dev = dev->next) {

			/* Check if this device is a LCD2USB device */
			if ((dev->descriptor.idVendor == LCD2USB_VENDORID) &&
			 (dev->descriptor.idProduct == LCD2USB_PRODUCTID)) {

				/*
				 * LCD2USB device found; try to find its
				 * description
				 */
				p->usbHandle = usb_open(dev);
				if (p->usbHandle == NULL) {
					report(RPT_WARNING, "hd_init_lcd2usb: unable to open device");
				}
				else {
					/* read firmware version */
					unsigned char buffer[2];

					if (usb_control_msg(p->usbHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
							    LCD2USB_GET_FWVER, 0, 0, (char *)buffer, sizeof(buffer), 1000) == 2)
						report(RPT_INFO, "hd_init_lcd2usb: device with firmware version %d.%02d found", buffer[0], buffer[1]);
				}
			}
		}
	}

	if (p->usbHandle != NULL) {
		debug(RPT_DEBUG, "hd_init_lcd2usb: opening device succeeded");
	}
	else {
		report(RPT_ERR, "hd_init_lcd2usb: no (matching) LCD2USB device found");
		return -1;
	}

	/* allocate and initialize send buffer */
	if ((p->tx_buf.buffer = malloc(LCD2USB_MAX_CMD)) == NULL) {
		report(RPT_ERR, "hd_init_lcd2usb: could not allocate send buffer");
		lcd2usb_HD44780_close(p);
		return -1;
	}
	p->tx_buf.type = -1;
	p->tx_buf.use_count = 0;

	common_init(p, IF_4BIT);

	/* replace uPause with empty one after initialization */
	p->hd44780_functions->uPause = lcd2usb_HD44780_uPause;

	return 0;
}

/**
 * Send data or command to the display. The data/command is internally
 * queued.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	int type = (flags == RS_DATA) ? LCD2USB_DATA : LCD2USB_CMD;
	int id = (displayID == 0) ? LCD2USB_CTRL_BOTH
	: ((displayID == 1) ? LCD2USB_CTRL_0 : LCD2USB_CTRL_1);

	/* flush current buffer if target or command type are different */
	if ((p->tx_buf.type >= 0) && (p->tx_buf.type != (type | id)))
		lcd2usb_HD44780_flush(p);

	/* add new item to buffer */
	p->tx_buf.type = (type | id);
	p->tx_buf.buffer[p->tx_buf.use_count++] = ch;

	/* flush buffer if it's full */
	if (p->tx_buf.use_count == LCD2USB_MAX_CMD)
		lcd2usb_HD44780_flush(p);
}

/**
 * Actually send data or command to the display.
 * \param p  Pointer to driver's private data structure.
 */
void
lcd2usb_HD44780_flush(PrivateData *p)
{
	/* only if some data available */
	if (p->tx_buf.use_count == 0)
		return;

	/* construct and send message */
	usb_control_msg(p->usbHandle, USB_TYPE_VENDOR,
			p->tx_buf.type | (p->tx_buf.use_count - 1),
			p->tx_buf.buffer[0] | (p->tx_buf.buffer[1] << 8),
			p->tx_buf.buffer[2] | (p->tx_buf.buffer[3] << 8),
			NULL, 0, 1000);

	/* buffer is now free again. Not necessary to clear what's in it. */
	p->tx_buf.type = -1;
	p->tx_buf.use_count = 0;
}

/**
 * Turn display backlight on or off.
 * Backlight is turned on or off by toggeling between the brightness and
 * offbrightness values.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state)
{
	/* get backlight brightness */
	int promille = (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	p->hd44780_functions->drv_debug(RPT_DEBUG, "lcd2usb_HD44780_backlight: Setting backlight to %d", promille);

	/* and set it (converted from [0,1000] -> [0,255]) */
	if (usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, LCD2USB_SET_BRIGHTNESS,
			    (promille * 255) / 1000, 0, NULL, 0, 1000) < 0)
		p->hd44780_functions->drv_report(RPT_WARNING, "lcd2usb_HD44780_backlight: setting backlight failed");
}


/**
 * Change display contrast.
 * \param p      Pointer to driver's private data structure.
 * \param value  New contrast value (one byte).
 */
void
lcd2usb_HD44780_set_contrast(PrivateData *p, unsigned char value)
{
	if (usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, LCD2USB_SET_CONTRAST,
			    value, 0, NULL, 0, 1000) < 0)
		p->hd44780_functions->drv_report(RPT_WARNING, "lcd2usb_HD44780_set_contrast: setting contrast failed");
}


/**
 * Read keypress.
 * \param p  Pointer to driver's private data structure.
 * \return   Scancode of the key as returned by the lcd2usb device.
 */
unsigned char
lcd2usb_HD44780_scankeypad(PrivateData *p)
{
	unsigned char buffer[2];
	int nBytes;

	/* send control request and accept return value */
	nBytes = usb_control_msg(p->usbHandle,
		       USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
	      LCD2USB_GET_KEYS, 0, 0, (char *) buffer, sizeof(buffer), 1000);

	if (nBytes != -1) {
		return buffer[0];
	}

	return '\0';
}


/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
lcd2usb_HD44780_close(PrivateData *p)
{
	if (p->usbHandle != NULL) {
		usb_close(p->usbHandle);
		p->usbHandle = NULL;
	}
	if (p->tx_buf.buffer != NULL) {
		free(p->tx_buf.buffer);
		p->tx_buf.buffer = NULL;
	}
}

/* EOF */
