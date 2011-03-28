/** \file server/drivers/hd44780-bwct-usb.c
 * \c bwctusb connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * See http://www.bwct.de/lcd.html
 */

/* Copyright (c) 2004, Bernd Walter <bernd@bwct.de>
 * Contributions:
 *   Copyright (c) 2004-7, Peter Marschall <peter@adpm.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 */

#include "hd44780-bwct-usb.h"
#include "hd44780-low.h"
#include "report.h"

#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


// connection type specific functions to be exposed using pointers in init()
void bwct_usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void bwct_usb_HD44780_set_contrast(PrivateData *p, unsigned char value);
void bwct_usb_HD44780_close(PrivateData *p);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_bwct_usb(Driver *drvthis)
{
  PrivateData *p = (PrivateData*) drvthis->private_data;

  struct usb_bus *bus;
  //char device_manufacturer[LCD_MAX_WIDTH+1] = "";
  char device_serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;
  char serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;

  p->hd44780_functions->senddata = bwct_usb_HD44780_senddata;
  p->hd44780_functions->close = bwct_usb_HD44780_close;
  p->hd44780_functions->set_contrast = bwct_usb_HD44780_set_contrast;

  /* Read config file's contents: serial number and contrast */

  strncpy(serial, drvthis->config_get_string(drvthis->name, "SerialNumber",
                                             0, DEFAULT_SERIALNO), sizeof(serial));
  serial[sizeof(serial)-1] = '\0';
  if (*serial != '\0') {
    report(RPT_INFO, "hd_init_bwct_usb: Using serial number: %s", serial);
  }

  /* try to find USB device */
#if 0
  usb_debug = 2;
#endif

  usb_init();
  usb_find_busses();
  usb_find_devices();

  p->usbHandle = NULL;
  p->usbIndex = 0;
  for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
    struct usb_device *dev;

    for (dev = bus->devices; dev != NULL; dev = dev->next) {
      int c;

      /* Check if this device is a BWCT device */
      if (dev->descriptor.idVendor != BWCT_USB_VENDORID) {
        continue;
      }
      /* Loop through all of the configurations */
      for (c = 0; c < dev->descriptor.bNumConfigurations; c++) {
        /* Loop through all of the interfaces */
        for (p->usbIndex = 0; p->usbIndex < dev->config[c].bNumInterfaces; p->usbIndex++) {
          int a;

          /* Loop through all of the alternate settings */
          for (a = 0; a < dev->config[c].interface[p->usbIndex].num_altsetting; a++) {
            /* Check if this interface is a BWCT lcd */
            if (((dev->config[c].interface[p->usbIndex].altsetting[a].bInterfaceClass == 0xFF) &&
                 (dev->config[c].interface[p->usbIndex].altsetting[a].bInterfaceSubClass == 0x01)) ||
                (dev->descriptor.idProduct == BWCT_USB_PRODUCTID)) {

              /* BWCT device found; try to find its description and serial number */
              p->usbHandle = usb_open(dev);
              if (p->usbHandle == NULL) {
                report(RPT_WARNING, "hd_init_bwct_usb: unable to open device");
                // return -1;                /* it's better to continue */
              }
              else {
                /* get device information & check for serial number */
                //if (usb_get_string_simple(p->usbHandle, dev->descriptor.iManufacturer,
                //                          manufacturer, LCD_MAX_WIDTH) <= 0)
                //  *manufacturer = '\0';
                //manufacturer[sizeof(manufacturer)-1] = '\0';

                //if (usb_get_string_simple(p->usbHandle, dev->descriptor.iProduct,
                //                          product, LCD_MAX_WIDTH) <= 0)
                //  *product = '\0';
                //product[sizeof(product)-1] = '\0';

                if (usb_get_string_simple(p->usbHandle, dev->descriptor.iSerialNumber,
                                          device_serial, LCD_MAX_WIDTH) <= 0)
                  *device_serial = '\0';
                device_serial[sizeof(device_serial)-1] = '\0';
                if ((*serial != '\0') && (*device_serial == '\0')) {
                  report(RPT_ERR, "hd_init_bwct_usb: unable to get device's serial number");
                  usb_close(p->usbHandle);
                  return -1;
                }

                /* succeed if no serial was given in the config or the 2 numbers match */
                if ((*serial == '\0') || (strcmp(serial, device_serial) == 0))
                  goto done;

                usb_close(p->usbHandle);
                p->usbHandle = NULL;
              }
            }
          }
        }
      }
    }
  }

  done:
  if (p->usbHandle != NULL) {
    debug(RPT_DEBUG, "hd_init_bwct_usb: opening device succeeded");

    errno = 0;
    if (usb_set_configuration(p->usbHandle, p->usbIndex) < 0) {
      report(RPT_WARNING, "hd_init_bwct_usb: unable to set configuration: %s",
             strerror(errno));
    }

    errno = 0;
    if (usb_claim_interface(p->usbHandle, p->usbIndex) < 0) {
#if defined(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP)
      report(RPT_WARNING, "hd_init_bwct_usb: interface may be claimed by "
                          "kernel driver, attempting to detach it");

      errno = 0;
      if ((usb_detach_kernel_driver_np(p->usbHandle, p->usbIndex) < 0) ||
          (usb_claim_interface(p->usbHandle, p->usbIndex) < 0)) {
        report(RPT_ERR, "hd_init_bwct_usb: unable to re-claim interface: %s",
	       strerror(errno));
        usb_close(p->usbHandle);
        return -1;
      }
#else
      report(RPT_ERR, "hd_init_bwct_usb: unable to claim interface: %s",
             strerror(errno));
      usb_close(p->usbHandle);
      return -1;
#endif
    }
  }
  else {
    report(RPT_ERR, "hd_init_bwct_usb: no (matching) BWCT device found");
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
bwct_usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
int type = (flags == RS_DATA) ? BWCT_LCD_DATA : BWCT_LCD_CMD;

  usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, type, ch, p->usbIndex, NULL, 0, 1000);
}


void
bwct_usb_HD44780_close(PrivateData *p)
{
  if (p->usbHandle != NULL) {
    usb_close(p->usbHandle);
    p->usbHandle = NULL;
  }
}


/**
 * Change LCD contrast.
 * \param p          Pointer to driver's private data structure.
 * \param value     New contrast value (one byte).
 */
void
bwct_usb_HD44780_set_contrast(PrivateData *p, unsigned char value)
{
  if (usb_control_msg(p->usbHandle, USB_TYPE_VENDOR, BWCT_LCD_SET_CONTRAST,
                      value, p->usbIndex, NULL, 0, 1000) < 0)
    p->hd44780_functions->drv_report(RPT_WARNING, "bwct_usb_HD44780_set_contrast: setting contrast failed");
}


/* EOF */
