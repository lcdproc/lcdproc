/** \file server/drivers/hd44780-lcd2usb.c
 * \c lcd2usb connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * See http://www.harbaum.org/till/lcd2usb.
 */

/* Copyright (C) 2007 Peter Marschall <peter@adpm.de>
 *               2007 Markus Dolze 
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-lcd2usb.h"

#include "report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


// connection type specific functions to be exposed using pointers in init()
void lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcd2usb_HD44780_scankeypad(PrivateData *p);
void lcd2usb_HD44780_close(PrivateData *p);
void lcd2usb_HD44780_set_contrast(PrivateData *p, unsigned char value);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval -1  Error.
 */
int
hd_init_lcd2usb(Driver *drvthis)
{
  PrivateData *p = (PrivateData*) drvthis->private_data;

  struct usb_bus *bus;

  p->hd44780_functions->senddata = lcd2usb_HD44780_senddata;
  p->hd44780_functions->backlight = lcd2usb_HD44780_backlight;
  p->hd44780_functions->scankeypad = lcd2usb_HD44780_scankeypad;
  p->hd44780_functions->close = lcd2usb_HD44780_close;
  p->hd44780_functions->set_contrast = lcd2usb_HD44780_set_contrast;

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

        /* LCD2USB device found; try to find its description */
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

  common_init(p, IF_4BIT);

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
lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
  int type = (flags == RS_DATA) ? LCD2USB_DATA : LCD2USB_CMD;
  int id = (displayID == 0) ? LCD2USB_CTRL_BOTH
                          : ((displayID == 1) ? LCD2USB_CTRL_0 : LCD2USB_CTRL_1);

  usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, (type | id), ch, 0, NULL, 0, 1000);
}


/**
 * Turn the LCD backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state)
{
  // Get backlight brightness.
  int promille = (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

  // And set it (converted from [0,1000] -> [0,255]).
  usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, LCD2USB_SET_BRIGHTNESS,
                   (promille * 255) / 1000, 0, NULL, 0, 1000);
}


/**
 * Change LCD contrast.
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
 * Set on/off brightness.
 * The value is not actually transmitted to the display. lcd2usb_hd44780_backlight
 * has to be called to do this.
 * \param drvthis   Pointer to driver structure.
 * \param state     Brightness state (on/off) for which we want to store the value.
 * \param promille  New brightness in promille.
 */
void
lcd2usb_set_brightness(Driver *drvthis, int state, int promille)
{
  PrivateData *p = drvthis->private_data;

  // Check if value within range
  if (promille < 0 || promille > 1000)
    return;

  /* store the software value */
  if (state == BACKLIGHT_ON)
    p->brightness = promille;
  else
    p->offbrightness = promille;
}


/**
 * Read keypress.
 * \param p  Pointer to driver's private data structure.
 * \return  Bitmap of the pressed keys.
 */
unsigned char
lcd2usb_HD44780_scankeypad(PrivateData *p)
{
  unsigned char       buffer[2];
  int                 nBytes;

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
}

/* EOF */
