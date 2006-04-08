/*
 * USB driver module for Hitachi HD44780 based LCD displays.
 * http://www.bwct.de/lcd.html
 *
 * Copyright (c) 2004, Bernd Walter <bernd@bwct.de>
 * Contributions:
 *   Copyright (c) 2004, Peter Marschall <peter@adpm.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 */

#include "hd44780-bwct-usb.h"

#include "report.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <usb.h>

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


/* USB device handle & interface index we write to */
static usb_dev_handle *bwct_usb;
static int bwct_usb_i;


// initialize the driver
int
hd_init_bwct_usb (Driver *drvthis)
{
  PrivateData *p = (PrivateData*) drvthis->private_data;
  struct usb_bus *busses;
  struct usb_bus *bus;
  char device_manufacturer[LCD_MAX_WIDTH+1] = "";
  char device_serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;
  char serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;
  int contrast = -1;	/* illegal contrast value (to detect errors) */

  p->hd44780_functions->senddata = bwct_usb_HD44780_senddata;
  p->hd44780_functions->backlight = bwct_usb_HD44780_backlight;
  p->hd44780_functions->scankeypad = bwct_usb_HD44780_scankeypad;

  /* Read config file's contents: serial number and contrast */

  strncpy(serial, drvthis->config_get_string(drvthis->name, "SerialNumber",
                                             0, DEFAULT_SERIALNO), sizeof(serial));
  serial[sizeof(serial)-1] = '\0';
  if (*serial != '\0') {
    report(RPT_INFO, "hd_init_bwct_usb: Using serial number: %s", serial);
  }

  contrast = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);

  /* try to find USB device */
#if 0
  usb_debug = 2;
#endif

  usb_init();
  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();

  bwct_usb = NULL;
  for (bus = busses; bus; bus = bus->next) {
    struct usb_device *dev;

    for (dev = bus->devices; dev; dev = dev->next) {
      int c;

      /* Check if this device is a BWCT device */
      if (dev->descriptor.idVendor != 0x03DA) {
        continue;
      }
      /* Loop through all of the configurations */
      for (c = 0; c < dev->descriptor.bNumConfigurations; c++) {
        /* Loop through all of the interfaces */
        for (bwct_usb_i = 0; bwct_usb_i < dev->config[c].bNumInterfaces; bwct_usb_i++) {
          int a;

          /* Loop through all of the alternate settings */
          for (a = 0; a < dev->config[c].interface[bwct_usb_i].num_altsetting; a++) {
            /* Check if this interface is a BWCT ulcd */
            if (((dev->config[c].interface[bwct_usb_i].altsetting[a].bInterfaceClass == 0xFf) &&
                 (dev->config[c].interface[bwct_usb_i].altsetting[a].bInterfaceSubClass == 0x01)) ||
                (dev->descriptor.idProduct == 0x0002)) {

              /* BWCT device found; try to find its description and serial number */
              bwct_usb = usb_open(dev);
              if (bwct_usb == NULL) {
                report(RPT_WARNING, "hd_init_bwct_usb: unable to open device");
                // return -1;                /* it's better to continue */
              }
              else {
                /* get device information & check for serial number */
                //if (usb_get_string_simple(bwct_usb, dev->descriptor.iManufacturer,
                //                          manufacturer, LCD_MAX_WIDTH) <= 0)
                //  *manufacturer = '\0';
                //manufacturer[sizeof(manufacturer)-1] = '\0';

                //if (usb_get_string_simple(bwct_usb, dev->descriptor.iProduct,
                //                          product, LCD_MAX_WIDTH) <= 0)
                //  *product = '\0';
                //product[sizeof(product)-1] = '\0';

                if (usb_get_string_simple(bwct_usb, dev->descriptor.iSerialNumber,
                                          device_serial, LCD_MAX_WIDTH) <= 0)
                  *device_serial = '\0';
                device_serial[sizeof(device_serial)-1] = '\0';
                if ((*serial != '\0') && (*device_serial == '\0')) {
                  report(RPT_ERR, "hd_init_bwct_usb: unable to get device's serial number");
                  usb_close(bwct_usb);
                  return -1;
                }

                /* succeed if no serial was given in the config or the 2 numbers match */
                if ((*serial == '\0') || (strcmp(serial, device_serial) == 0))
                  goto done;

                usb_close(bwct_usb);
                bwct_usb = NULL;
              }
            }
          }
        }
      }
    }
  }

  done:
  if (bwct_usb != NULL) {
    debug(RPT_DEBUG, "hd_init_bwct_usb: opening device succeeded");

    if (usb_claim_interface(bwct_usb, bwct_usb_i) < 0) {
#if defined(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP)
      if ((usb_detach_kernel_driver_np(bwct_usb, bwct_usb_i) < 0) ||
          (usb_claim_interface(bwct_usb, bwct_usb_i) < 0)) {
        usb_close(bwct_usb);
        report(RPT_ERR, "hd_init_bwct_usb: unable to re-claim interface");
        return -1;
      }
#else
      usb_close(bwct_usb);
      report(RPT_ERR, "hd_init_bwct_usb: unable to claim interface");
      return -1;
#endif
    }
  }
  else {
    report(RPT_ERR, "hd_init_bwct_usb: no (matching) BWCT device found");
    return -1;
  }

  common_init(p, IF_4BIT);

  /* set contrast */
  if ((0 <= contrast) && (contrast <= 1000)) {
    int res = usb_control_msg(bwct_usb, USB_TYPE_VENDOR, VENDOR_LCD_CONTRAST,
                              (contrast * 255) / 1000, bwct_usb_i, NULL, 0, 1000);
    if (res < 0)
      report(RPT_WARNING, "hd_init_bwct_usb: setting contrast failed");
  } else {
    report(RPT_INFO, "hd_init_bwct_usb: Using default contrast value");
  }

  return 0;
}


// bwct_usb_HD44780_senddata
void
bwct_usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
int type = (flags == RS_DATA) ? VENDOR_LCD_DATA : VENDOR_LCD_CMD;

    usb_control_msg(bwct_usb, USB_TYPE_VENDOR, type, ch, bwct_usb_i, NULL, 0, 1000);
}


void
bwct_usb_HD44780_backlight(PrivateData *p, unsigned char state)
{
}


unsigned char
bwct_usb_HD44780_scankeypad(PrivateData *p)
{
  return 0;
}
