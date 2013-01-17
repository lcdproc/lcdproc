/** \file server/drivers/glcd-glcd2usb.c
 * Driver for the glcd2usb device by Till Harbaum, a graphic lcd display
 * interface based on AVR USB driver. Based on his driver for lcd4linux.
 *
 * \note  This connection type uses the screen size as reported by the glcd2usb
 *        device. Any size configured in LCDd.conf will be ignored.
 * \note  The glcd2usb with KS108 firmware uses a vertical (aka paged) frame
 *        buffer layout. Other layouts are not supported by this driver.
 *
 * \todo  Keypad input
 */

/*-
 * Copyright (c) 2007 Till Harbaum
 *               2011 Markus Dolze <bsdfan@nurfuerspam.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <usb.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"
#include "glcd-glcd2usb.h"

/* USB Vendor and Product ID used for glcd2usb device */
#define GLCD2USB_VID	0x1c40
#define GLCD2USB_PID	0x0525

/** Data local to the glcd2usb connection type */
typedef struct glcd_glcd2usb_data {
	usb_dev_handle *device;
	unsigned char *paged_buffer;	/**< frame buffer in paged format */
	unsigned char *dirty_buffer;	/**< buffer indicating changed bytes */
	union {
		unsigned char bytes[132];
		/** when reading this buffer can be accessed as structure */
		display_info_t display_info;
	} tx_buffer;		/**< buffer to accumulate bytes to be sent */
} CT_glcd2usb_data;


/*-------------------------- USB HID IMPLEMENTATION -----------------------*/

/*
 * Note: The glcd2usb device implements a HID device. My first try to use
 * libhid failed, because glcd2usb uses report descriptors that totally confuse
 * libhid's parser. Therefor we need to roll our own. The code below has been
 * copied from the LCD4Linux implementation of this driver.
 */

/* Error codes */
#define USB_ERROR_NONE      0
#define USB_ERROR_ACCESS    1
#define USB_ERROR_NOTFOUND  2
#define USB_ERROR_BUSY      16
#define USB_ERROR_IO        5

/* HID report types */
#define USB_HID_REPORT_TYPE_INPUT   1
#define USB_HID_REPORT_TYPE_OUTPUT  2
#define USB_HID_REPORT_TYPE_FEATURE 3

/* USB request types */
#define USBRQ_HID_GET_REPORT  0x01
#define USBRQ_HID_SET_REPORT  0x09

/**
 * Sends a 'set report' request to the default control end point of glcd2usb
 * device. At most 128 bytes may be sent (plus 4 additional control bytes).
 *
 * \param device      USB device handle of the device to talk to
 * \param reportType  A HID report type
 * \param buffer      Pointer to data to write
 * \param len         Amount of data that shall be written
 * \return  0 on success, USB_ERROR_IO otherwise
 */
static int
usbSetReport(usb_dev_handle * device, int reportType, unsigned char *buffer, int len)
{
	int bytesSent;

	/*
	 * the write command needs some tweaking regarding allowed report
	 * lengths
	 */
	if (buffer[0] == GLCD2USB_RID_WRITE) {
		const int allowed_lengths[] = {4 + 4, 8 + 4, 16 + 4, 32 + 4, 64 + 4, 128 + 4};
		int i = 0;

		if (len > 128 + 4)
			fprintf(stderr, "%d bytes usb report is too long \n", len);

		while (allowed_lengths[i] != (128 + 4) && allowed_lengths[i] < len)
			i++;

		len = allowed_lengths[i];
		buffer[0] = GLCD2USB_RID_WRITE + i;
	}

	bytesSent = usb_control_msg(device, USB_TYPE_CLASS | USB_RECIP_INTERFACE |
				    USB_ENDPOINT_OUT, USBRQ_HID_SET_REPORT,
				    reportType << 8 | buffer[0], 0, (char *)buffer, len, 1000);

	if (bytesSent != len) {
		if (bytesSent < 0)
			fprintf(stderr, "Error sending message: %s", usb_strerror());
		return USB_ERROR_IO;
	}
	return 0;
}


/**
 * Sends a 'get report' request to the default control end point of glcd2usb
 * device. The amount of data read is stored in parameter len.
 *
 * \param device        USB device handle of the device to talk to
 * \param reportType    A HID report type (input / output / feature)
 * \param reportNumber  The ReportID to be read
 * \param buffer        Pointer to buffer to read the data into. The buffer MUST
 *                      be allocated beforehand and be at least of 'len' size.
 * \param len           Pointer to an integer with the number of bytes to read
 *                      at most. Upon return, this hold the number of bytes
 *                      actually read or -1 if an error occurred.
 * \return  0 on success, USB_ERROR_IO otherwise
 */
static int
usbGetReport(usb_dev_handle * device, int reportType, int reportNumber, unsigned char *buffer, int *len)
{
	*len = usb_control_msg(device, USB_TYPE_CLASS | USB_RECIP_INTERFACE |
			       USB_ENDPOINT_IN, USBRQ_HID_GET_REPORT,
			       reportType << 8 | reportNumber, 0, (char *)buffer, *len, 1000);

	if (*len < 0) {
		fprintf(stderr, "Error sending message: %s", usb_strerror());
		return USB_ERROR_IO;
	}
	return 0;
}


/**
 * Converts an error code into a user readable string.
 * \param errCode  The error code
 * \return  Pointer to error message
 */
static char *
usbErrorMessage(int errCode)
{
	static char buffer[80];

	switch (errCode) {
	    case USB_ERROR_ACCESS:
		return "Access to device denied";
	    case USB_ERROR_NOTFOUND:
		return "The specified device was not found";
	    case USB_ERROR_BUSY:
		return "The device is used by another application";
	    case USB_ERROR_IO:
		return "Communication error with device";
	    default:
		sprintf(buffer, "Unknown USB error %d", errCode);
		return buffer;
	}
	/* NOTREACHED */
	return NULL;
}

/*------------------- Connection type functions below ---------------------*/

/**
 * API: Close device descriptors and free resources
 */
void
glcd2usb_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_glcd2usb_data *ctd = (CT_glcd2usb_data *) p->ct_data;

		if (ctd->device != 0)
			usb_close(ctd->device);
		if (ctd->paged_buffer != NULL)
			free(ctd->paged_buffer);
		if (ctd->dirty_buffer != NULL)
			free(ctd->dirty_buffer);
		free(ctd);
	}
}


/**
 * API: Set the backlight brightness
 */
void
glcd2usb_backlight(PrivateData *p, int state)
{
	int err = 0;
	CT_glcd2usb_data *ctd = (CT_glcd2usb_data *) p->ct_data;
	int promille = (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	ctd->tx_buffer.bytes[0] = GLCD2USB_RID_SET_BL;
	ctd->tx_buffer.bytes[1] = promille * 255 / 1000;

	p->glcd_functions->drv_debug(RPT_DEBUG, "glcd2usb_backlight: new value = %d",
		ctd->tx_buffer.bytes[1]);

	if ((err = usbSetReport(ctd->device, USB_HID_REPORT_TYPE_FEATURE, ctd->tx_buffer.bytes, 2)) != 0) {
		p->glcd_functions->drv_report(RPT_ERR, "Error freeing display: %s\n", usbErrorMessage(err));
	}
}


/**
 * API: Transfer an image to the glcd2usb device. This function and its update
 * algorithm are copied from the LCD4Linux driver as it does its job well.
 */
void
glcd2usb_blit(PrivateData *p)
{
	CT_glcd2usb_data *ctd = (CT_glcd2usb_data *) p->ct_data;
	int r;
	int i, j;
	int err;
	int pos;

	p->glcd_functions->drv_debug(RPT_DEBUG, "glcd2usb_blit: starting");

	/* Reset the dirty buffer */
	memset(ctd->dirty_buffer, 0x00, p->framebuf.size);

	/*
	 * Step 1: Compare the content of the secondary buffer with the frame
	 * buffer and copy the differences. For each different byte, set the
	 * flag in the dirty buffer.
	 */
	for (pos = 0; pos < p->framebuf.size; pos++) {
		if (ctd->paged_buffer[pos] != p->framebuf.data[pos]) {
			ctd->paged_buffer[pos] = p->framebuf.data[pos];
			ctd->dirty_buffer[pos] = 1;
		}
	}

	/*
	 * Step 2: Short gaps of unchanged bytes in fact increase the
	 * communication overhead. So we eliminate them here.
	 */
	for (j = -1, i = 0; i < p->framebuf.size; i++) {
		if (ctd->dirty_buffer[i] && j >= 0 && i - j <= 4) {
			/* found a clean gap <= 4 bytes: mark it dirty */
			for (r = j; r < i; r++)
				ctd->dirty_buffer[r] = 1;
		}

		/* if this is dirty, drop the saved position */
		if (ctd->dirty_buffer[i])
			j = -1;

		/* save position of this clean entry if no position saved yet */
		if (!ctd->dirty_buffer[i] && j < 0)
			j = i;
	}

	/* Step 3: Send the changes. */
	ctd->tx_buffer.bytes[0] = 0;
	for (i = 0; i < p->framebuf.size; i++) {
		if (ctd->dirty_buffer[i]) {
			/* Start a new packet */
			if (!ctd->tx_buffer.bytes[0]) {
				ctd->tx_buffer.bytes[0] = GLCD2USB_RID_WRITE;
				ctd->tx_buffer.bytes[1] = i % 256;
				ctd->tx_buffer.bytes[2] = i / 256;
				ctd->tx_buffer.bytes[3] = 0;	/* length */
			}
			/* Append the dirty byte and increase size */
			ctd->tx_buffer.bytes[4 + ctd->tx_buffer.bytes[3]++] = ctd->paged_buffer[i];
		}

		/*
		 * Send the data if we hit a clean byte, are at the end of
		 * the frame or reached the maximum payload for a write
		 * request.
		 */
		if (!ctd->dirty_buffer[i] || i == p->framebuf.size - 1 || ctd->tx_buffer.bytes[3] == 128) {
			/* Only write if there IS something to be written */
			if (ctd->tx_buffer.bytes[0] == GLCD2USB_RID_WRITE && ctd->tx_buffer.bytes[3] > 0) {
				err = usbSetReport(ctd->device, USB_HID_REPORT_TYPE_FEATURE,
						   ctd->tx_buffer.bytes, ctd->tx_buffer.bytes[3] + 4);
				if (err)
					p->glcd_functions->drv_report(RPT_ERR, "glcd2usb_blit: error in transfer");
				/* Start a new packet the next time */
				ctd->tx_buffer.bytes[0] = 0;
			}
		}
	}
}


/**
 * API: Poll for any pressed keys. Converts the bitmap of keys pressed into a
 * scancode (1-4) for each pressed key.
 * \note Only single key presses are detected.
 */
unsigned char
glcd2usb_poll_keys(PrivateData *p)
{
	CT_glcd2usb_data *ctd = (CT_glcd2usb_data *) p->ct_data;
	unsigned char keycode = 0;
	int err = 0, len = 2;
	int i;

	if ((err = usbGetReport(ctd->device, USB_HID_REPORT_TYPE_FEATURE,
				GLCD2USB_RID_GET_BUTTONS, ctd->tx_buffer.bytes, &len)) != 0) {
		p->glcd_functions->drv_report(RPT_ERR, "glcd2usb_poll_keys: Error getting button state: %s",
					      usbErrorMessage(err));
		return 0;
	}

	for (i = 0; i < 4; i++) {
		if (ctd->tx_buffer.bytes[1] & (1 << i)) {
			keycode = i + 1;
			break;
		}
	}

	return keycode;
}


/**
 * API: Initialize glcd2usb connection type.
 */
int
glcd2usb_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	CT_glcd2usb_data *ctd;

	static int didUsbInit = 0;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *handle = NULL;
	int err = 0;
	int rval, retries = 3;
	int len;

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd2usb_blit;
	p->glcd_functions->close = glcd2usb_close;
	p->glcd_functions->set_backlight = glcd2usb_backlight;
	p->glcd_functions->poll_keys = glcd2usb_poll_keys;

	/* Allocate memory structures */
	ctd = (CT_glcd2usb_data *) calloc(1, sizeof(CT_glcd2usb_data));
	if (ctd == NULL) {
		report(RPT_ERR, "%s/glcd2usb: error allocating connection data", drvthis->name);
		return -1;
	}
	p->ct_data = ctd;

	/*
	 * Try to find and open a device. Only the first device found will be
	 * recognized.
	 */
	if (!didUsbInit) {
		usb_init();
		didUsbInit = 1;
	}

	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		for (dev = bus->devices; dev != NULL; dev = dev->next) {

			if (dev->descriptor.idVendor == GLCD2USB_VID
			    && dev->descriptor.idProduct == GLCD2USB_PID) {

				handle = usb_open(dev);
				if (!handle) {
					report(RPT_WARNING, "%s/glcd2usb: cannot open USB device: %s",
					       drvthis->name, usb_strerror());
					continue;
				}
				else {
					goto found_dev;
				}
			}
		}
	}

found_dev:
	if (handle) {
		debug(RPT_DEBUG, "%s/glcd2usb: opening device succeeded", drvthis->name);
	}
	else {
		report(RPT_ERR, "%s/glcd2usb: no GLCD2USB device found", drvthis->name);
		goto err_out;
	}

	if (usb_set_configuration(handle, 1))
		report(RPT_WARNING, "%s/glcd2usb: could not set configuration: %s",
		       drvthis->name, usb_strerror());

	/*
	 * now try to claim the interface and detach the kernel HID driver on
	 * Linux and other operating systems which support the call.
	 */
	while ((rval = usb_claim_interface(handle, 0)) != 0 && retries-- > 0) {
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
		if (usb_detach_kernel_driver_np(handle, 0) < 0) {
			report(RPT_WARNING, "%s/glcd2usb: could not detach kernel HID driver: %s",
			       drvthis->name, usb_strerror());
		}
#endif
	}
	if (rval != 0)
		report(RPT_WARNING, "%s/glcd2usb: could not claim interface", drvthis->name);

	/*
	 * Continue anyway, even if we could not claim the interface. Control
	 * transfers should still work.
	 */
	ctd->device = handle;

	/* Query device */
	memset(&(ctd->tx_buffer), 0, sizeof(ctd->tx_buffer));
	len = sizeof(display_info_t);

	if ((err = usbGetReport(ctd->device, USB_HID_REPORT_TYPE_FEATURE, GLCD2USB_RID_GET_INFO, ctd->tx_buffer.bytes, &len)) != 0) {
		report(RPT_ERR, "%s/glcd2usb: query display parameters: %s",
		       drvthis->name, usbErrorMessage(err));
		goto err_out;
	}

	if (len < (int)sizeof(ctd->tx_buffer.display_info)) {
		report(RPT_ERR, "%s/glcd2usb: incomplete display info report (%d instead of %d)",
		       drvthis->name, len, (int)sizeof(ctd->tx_buffer.display_info));
		goto err_out;
	}

	if (!(ctd->tx_buffer.display_info.flags & FLAG_VERTICAL_UNITS)) {
		report(RPT_ERR, "%s/glcd2usb: unsupported display layout", drvthis->name);
		goto err_out;
	}

	if (ctd->tx_buffer.display_info.width > GLCD_MAX_WIDTH
	    || ctd->tx_buffer.display_info.width <= 0
	    || ctd->tx_buffer.display_info.height > GLCD_MAX_HEIGHT
	    || ctd->tx_buffer.display_info.height <= 0) {
		report(RPT_ERR, "%s/glcd2usb: display size out of range: %dx%d",
		       drvthis->name, ctd->tx_buffer.display_info.width,
		       ctd->tx_buffer.display_info.height);
		goto err_out;
	}
	p->framebuf.layout = FB_TYPE_VPAGED;
	p->framebuf.px_width = ctd->tx_buffer.display_info.width;
	p->framebuf.px_height = ctd->tx_buffer.display_info.height;
	p->framebuf.size = (p->framebuf.px_height + 7) / 8 * p->framebuf.px_width;
	report(RPT_INFO, "%s/glcd2usb: using display size %dx%d", drvthis->name,
	       ctd->tx_buffer.display_info.width, ctd->tx_buffer.display_info.height);

	ctd->paged_buffer = malloc(p->framebuf.size);
	if (ctd->paged_buffer == NULL) {
		report(RPT_ERR, "%s/glcd2usb: cannot allocate memory", drvthis->name);
		goto err_out;
	}
	memset(ctd->paged_buffer, 0x55, p->framebuf.size);

	ctd->dirty_buffer = malloc(p->framebuf.size);
	if (ctd->dirty_buffer == NULL) {
		report(RPT_ERR, "%s/glcd2usb: cannot allocate memory", drvthis->name);
		goto err_out;
	}

	/* Allocate the display (turn off the 'whirl') */
	ctd->tx_buffer.bytes[0] = GLCD2USB_RID_SET_ALLOC;
	ctd->tx_buffer.bytes[1] = 1;
	if ((err = usbSetReport(ctd->device, USB_HID_REPORT_TYPE_FEATURE, ctd->tx_buffer.bytes, 2)) != 0) {
		report(RPT_ERR, "%s/glcd2usb: Error allocating display: %s",
		       drvthis->name, usbErrorMessage(err));
		goto err_out;
	}

	return 0;

err_out:
	glcd2usb_close(p);
	return -1;
}
