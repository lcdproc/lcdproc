/* driver module for Hitachi HD44780 based LCD displays
 * connected to using the lcd2usb interface
 * (http://www.harbaum.org/till/lcd2usb)
 *
 * Copyright (C) 2007 Peter Marschall <peter@adpm.de>
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


/* USB device handle & interface index we write to */
static usb_dev_handle *lcd2usb;


// initialize the driver
int
hd_init_lcd2usb(Driver *drvthis)
{
  PrivateData *p = (PrivateData*) drvthis->private_data;

  struct usb_bus *bus;
  //char device_manufacturer[LCD_MAX_WIDTH+1] = "";
  int contrast = -1;	/* illegal contrast value (to detect errors) */
  int brightness = -1;	/* illegal brightness value (to detect errors) */

  p->hd44780_functions->senddata = lcd2usb_HD44780_senddata;
  p->hd44780_functions->backlight = lcd2usb_HD44780_backlight;
  p->hd44780_functions->scankeypad = lcd2usb_HD44780_scankeypad;
  p->hd44780_functions->close = lcd2usb_HD44780_close;

  /* Read config file's contents: contrast */

  contrast = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
  brightness = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);

  /* try to find USB device */
#if 0
  usb_debug = 2;
#endif

  usb_init();
  usb_find_busses();
  usb_find_devices();

  lcd2usb = NULL;
  for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
    struct usb_device *dev;

    for (dev = bus->devices; dev != NULL; dev = dev->next) {

      /* Check if this device is a LCD2USB device */
      if ((dev->descriptor.idVendor == LCD2USB_VENDORID) &&
	  (dev->descriptor.idProduct == LCD2USB_PRODUCTID)) {

        /* LCD2USB device found; try to find its description */
        lcd2usb = usb_open(dev);
        if (lcd2usb == NULL) {
          report(RPT_WARNING, "hd_init_lcd2usb: unable to open device");
          // return -1;                /* it's better to continue */
        }
        else {
          /* get device information & check for serial number */
          //if (usb_get_string_simple(lcd2usb, dev->descriptor.iManufacturer,
          //                          manufacturer, LCD_MAX_WIDTH) <= 0)
          //  *manufacturer = '\0';
          //manufacturer[sizeof(manufacturer)-1] = '\0';

          //if (usb_get_string_simple(lcd2usb, dev->descriptor.iProduct,
          //                          product, LCD_MAX_WIDTH) <= 0)
          //  *product = '\0';
          //product[sizeof(product)-1] = '\0';

          //usb_close(lcd2usb);
          //lcd2usb = NULL;
        }
      }
    }
  }

  if (lcd2usb != NULL) {
    debug(RPT_DEBUG, "hd_init_lcd2usb: opening device succeeded");

  }
  else {
    report(RPT_ERR, "hd_init_lcd2usb: no (matching) LCD2USB device found");
    return -1;
  }

  common_init(p, IF_4BIT);

  /* set contrast */
  if ((0 <= contrast) && (contrast <= 1000)) {
    int res = usb_control_msg(lcd2usb, USB_TYPE_VENDOR, LCD2USB_SET_CONTRAST,
                              (contrast * 255) / 1000, 0, NULL, 0, 1000);
    if (res < 0)
      report(RPT_WARNING, "hd_init_lcd2usb: setting contrast failed");
  } else {
    report(RPT_INFO, "hd_init_lcd2usb: Using default contrast value");
  }

  /* set brightness */
  if ((0 <= brightness) && (brightness <= 1000)) {
    int res = usb_control_msg(lcd2usb, USB_TYPE_VENDOR, LCD2USB_SET_BRIGHTNESS,
                              (brightness * 255) / 1000, 0, NULL, 0, 1000);
    if (res < 0)
      report(RPT_WARNING, "hd_init_lcd2usb: setting brightness failed");
  } else {
    report(RPT_INFO, "hd_init_lcd2usb: Using default brightness value");
  }

  return 0;
}


// lcd2usb_HD44780_senddata
void
lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
int type = (flags == RS_DATA) ? LCD2USB_DATA : LCD2USB_CMD;
int id = (displayID == 0) ? LCD2USB_CTRL_BOTH
                          : ((displayID == 1) ? LCD2USB_CTRL_0 : LCD2USB_CTRL_1);

    usb_control_msg(lcd2usb, USB_TYPE_VENDOR, (type | id), ch, 0, NULL, 0, 1000);
}


void
lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state)
{
}


unsigned char
lcd2usb_HD44780_scankeypad(PrivateData *p)
{
  return 0;
}

void
lcd2usb_HD44780_close(PrivateData *p)
{
  if (lcd2usb != NULL) {
    usb_close(lcd2usb);
    lcd2usb = NULL;
  }
}

/* EOF */
