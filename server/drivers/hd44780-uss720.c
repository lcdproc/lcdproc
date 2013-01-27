/** \file server/drivers/hd44780-uss720.c
 * \c uss720 connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 8 bit-mode to be connected to a uss720 based Belkin F5U002.
 */

/*
 * Copyright (c)  2008 Eric Pooch <epooch@cox.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * This is a uss720 driver module for Hitachi HD44780 based LCD displays
 * connected to a uss720 USB-to-IEEE 1284 Bridge chip in 8 bit mode.
 * The uss720 chip is found in many (but not all)
 * Belkin F5U002 USB->parallel printer adapters.
 *
 * Applicable Data Sheets:
 * - http://digilander.libero.it/demarchidaniele/qcamvc/uss-720.pdf
 *
 * Wiring is same as winamp connection.
 * Uncomment //#define USS720_MAC_USB_LCD_KIT_WIRING 1 below to use a
 * Mac USB LCD Kit instead.
 *
 */

#include "hd44780-uss720.h"
#include "hd44780-low.h"
#include "lpt-port.h"
#include "report.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

enum {
	SSPMode = 0,
	PS2Mode,
	HHCMode,
	ECPMode,
	EPPMode,
} /*ECRMode*/;


enum {
	StatusIndex = 0,
	ControlIndex,
	ExtControlIndex,
	USSSTZControlIndex,
	DataIndex,
	EPPIndex,
	USSSTZSetupIndex,
} /*RegisterDataByteIndex*/;

enum {
	DataRegAdr = 0,
	StatusRegAdr,
	ControlRegAdr,
	EPPAddressRegAdr,
	EPPDataRegAdr,
	ECPCommandRegAdr,
	ExtControlRegAdr,
	USSSTZControlRegAdr,
	USSSTZSetupRegAdr,
} /*RegisterAddress*/;


/* connection type specific functions to be exposed using pointers in init() */
void uss720_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void uss720_HD44780_backlight(PrivateData *p, unsigned char state);
void uss720_HD44780_close(PrivateData *p);
static void uss720_HD44780_uPause(PrivateData *p, int usecs);

//#define USS720_MAC_USB_LCD_KIT_WIRING 1
#ifdef USS720_MAC_USB_LCD_KIT_WIRING
/* Mac OS X USB LCD Kit wiring */
#define EN1	STRB
#define EN2	INIT
#define EN3	0
#define RS	SEL
#define BL	INIT
#define VCC	LF
#else
/* winamp wiring */
#define EN1	STRB
#define EN2	SEL
#define EN3	LF
#define RW	LF
#define RS	INIT
#define BL	SEL
#define LE	LF
#define VCC	0
#endif

static const unsigned char EnMask[] = { EN1, EN2, EN3 };

/* local helper functions not exposed */
int uss720_get_1284_register(usb_dev_handle *usbHandle, unsigned int regIndex, unsigned char *value);
int uss720_set_1284_register(usb_dev_handle *usbHandle, unsigned int regAddress, unsigned char value);
int uss720_set_1284_mode(usb_dev_handle *usbHandle, unsigned int mode);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_uss720(Driver *drvthis)
{
    unsigned int vendor_id, product_id;
	struct usb_bus *bus;

	PrivateData *p = (PrivateData*) drvthis->private_data;

	p->hd44780_functions->senddata = uss720_HD44780_senddata;
	p->hd44780_functions->backlight = uss720_HD44780_backlight;
	p->hd44780_functions->close = uss720_HD44780_close;
	p->hd44780_functions->uPause = uss720_HD44780_uPause;

    /* load config */
    vendor_id = drvthis->config_get_int(drvthis->name, "VendorID", 0, 0x1293);
    product_id = drvthis->config_get_int(drvthis->name, "ProductID", 0, 0x0002);

	/* Hardware Init */

	/* try to find USB device */
	usb_init();
	usb_find_busses();
	usb_find_devices();

	p->usbHandle = NULL;
	p->usbIndex = 0;
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev != NULL; dev = dev->next) {

			/* Check if this device is a uss720 device */
			if (dev->descriptor.idVendor != vendor_id || dev->descriptor.idProduct != product_id) {
				continue;
			}

			/* uss720 device found; try to open it */
			p->usbHandle = usb_open(dev);
			if (p->usbHandle == NULL) {
				report(RPT_WARNING, "hd_init_uss720: unable to open device");
				continue;
			}

			debug(RPT_DEBUG, "hd_init_uss720: opening device succeeded");

			errno = 0;
			if (usb_claim_interface(p->usbHandle, p->usbIndex) < 0) {
#if defined(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP)
				report(RPT_WARNING, "hd_init_uss720: interface may be claimed by "
					   "kernel driver, attempting to detach it");

				errno = 0;
				if ((usb_detach_kernel_driver_np(p->usbHandle, p->usbIndex) < 0) ||
					(usb_claim_interface(p->usbHandle, p->usbIndex) < 0)) {
					report(RPT_ERR, "hd_init_uss720: unable to re-claim interface: %s",
						   strerror(errno));
					usb_close(p->usbHandle);
					continue;
				}
#else
				report(RPT_ERR, "hd_init_uss720: unable to claim interface: %s",
					   strerror(errno));
				usb_close(p->usbHandle);
				continue;
#endif
			}
			/* Use the alternate, vendor-specific, interface so we have access to the registers */

			errno = usb_set_altinterface(p->usbHandle, 2);
			if (errno) {
				report(RPT_WARNING, "hd_init_uss720: unable to set alt interface: %s", strerror(errno));
				usb_close(p->usbHandle);
				continue;
			}
			/* Make sure the port is in SSP (Standard) mode for full software control. */
			errno = uss720_set_1284_mode(p->usbHandle, SSPMode);
			if (errno) {
				report(RPT_WARNING, "hd_init_uss720: unable to set SSP mode: %d", errno);//strerror(errno));
				/* probably not fatal, so try to keep going with this device */
			}

			/* init the HD44780 LCD */
			common_init(p, IF_8BIT);

			return 0;
		}
	}
	report(RPT_ERR, "hd_init_uss720: no (matching) USS720 device found");
	return -1;
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
uss720_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char enableLines = 0, portControl;

	if (flags == RS_DATA)
		portControl = RS;	/* enable RS for data */
	else
		portControl = 0;	/* disable RS for instruction */

	portControl |= VCC;		/* keep the power on */
	portControl |= p->backlight_bit;

	if (displayID == 0)
		enableLines = EnMask[0]
		| ((p->have_backlight) ? 0 : EnMask[1])
		| ((p->numDisplays == 3) ? EnMask[2] : 0);
	else
		enableLines = EnMask[displayID - 1];

	uss720_set_1284_register(p->usbHandle, ControlRegAdr, portControl ^ OUTMASK);

	uss720_set_1284_register(p->usbHandle, DataRegAdr, ch);
	p->hd44780_functions->uPause(p, 1);

	uss720_set_1284_register(p->usbHandle, ControlRegAdr, (portControl | enableLines ) ^ OUTMASK);
	p->hd44780_functions->uPause(p, 1);

	uss720_set_1284_register(p->usbHandle, ControlRegAdr, portControl ^ OUTMASK);
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
uss720_HD44780_backlight(PrivateData *p, unsigned char state)
{
	p->backlight_bit = (state?0:BL);
}


/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
uss720_HD44780_close(PrivateData *p)
{
  if (p->usbHandle != NULL) {
    usb_close(p->usbHandle);
    p->usbHandle = NULL;
  }
}


/**
 * Pause as needed (= a long time).
 * \param p      Pointer to driver's private data structure.
 * \param usecs  Pause duration.
 */
static void
uss720_HD44780_uPause(PrivateData *p, int usecs)
{
	usleep(10*usecs);
}

/* === Basic Register Methods === */

int
uss720_get_1284_register(usb_dev_handle *usbHandle, unsigned int regIndex, unsigned char *value)
{
	int res = 0;
	unsigned char regData[128];

	res = usb_control_msg(usbHandle,
		/* bmRequestType */ 0xc0,/*USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,*/
		/* bRequest      */ 3,
	        /* wValue        */ (unsigned int)regIndex<<8,
        	/* wIndex        */ 0,
	        /* Data          */ (char *)regData,
        	/* wLength       */ 7,
                	            10000 );

	if (res) {

	} else if (value && (regIndex < 8)) {
		*value = regData[(int)regIndex];
	}

	return res;
}


int
uss720_set_1284_register(usb_dev_handle *usbHandle, unsigned int regAddress, unsigned char value)
{
	return usb_control_msg(usbHandle,
		/* bmRequestType */ 0x40,/*USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,*/
		/* bRequest      */ 4,
	        /* wValue        */ ((unsigned int)regAddress<<8 | (unsigned int) value),
        	/* wIndex        */ 0,
	        /* Data          */ NULL,
        	/* wLength       */ 0,
                	            10000 );
}

/* === Extended Methods === */

int
uss720_set_1284_mode(usb_dev_handle *usbHandle, unsigned int mode)
{
	unsigned char regByte = 0x00;
	int res = 0;

	res = uss720_get_1284_register(usbHandle, 3, &regByte);

	regByte &= ~0x01;

	res = uss720_set_1284_register(usbHandle, 7, regByte);
	if (res) return res;

	res = uss720_get_1284_register(usbHandle, 2, &regByte);

	regByte &= ~(mode<<5);	/* Mask off the mode bits 7-5. */
	regByte |= (mode<<5);	/* Set the mode bits 7-5. */

	return uss720_set_1284_register(usbHandle, 6, regByte);
}
