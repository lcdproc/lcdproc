/**
 * LCDd driver for the Futaba TOSD-5711BB LED display,
 * as found in Elonex Artisan/FIC Spectra media centre cases
 */

/*- Copyright (C)
 *- 2014   Blackeagle         email: gm(dot)blackeagle(at)gmail(dot)com
 *- 2015/6 Alex Wood (AJW107) email: thetewood(at)gmail(dot)com
 *-
 *- CREDITS
 *- Steve Williams for the original driver code and the inspiration
 *- to write this.
 *-
 *- V E R S I O N - 1.3
 *-
 *- C H A N G E L O G
 *- =================
 *-
 *- 23/12/2016 - Improved function descriptions
 *-              Aligned more with lcdproc code style, function name changes
 *-              GNU indent format (first time used, agh)
 *-              Improved comments here and in .h file
 *-              Changed RPT_ERR to RPT_WARNINGerr if an error occurs, but
 *-              code still continesg on
 *-              Added a few todo's for future reference
 *-              Removed goto in init_driver, never comfortable with it
 *-              Removed global futaba driver struct and added to private data
 *-              Added alternate usb config for some reason (picolcd has it, but
 *-              comments it never works
 *-              Confiureged LIBUSB 1.0 to give log messages
 *-              Use futaba_shutdown (maybe futaba_close) instead of many
 *-              discrete libusb close statements
 *-              (by AJW107)
 *-
 *- 22/12/16 - Corrected some debug/error messages and version number
 *-            (by AJW107)
 *-
 *- 22/12/16 - More strictly defined the Icon array int type to uint64_t.
 *-            This is to prevent compiler warnings when bit shift operations
 *-            are condicted on it.  (by AJW107)
 *-
 *- 22/12/16 - Included support for the libusb0.1 library (originally only
 *-            lisusb1.0 was supported).
 *-            Added a slower retry if reset fails during init (by AJW107)
 *-
 *- 22/12/16 - Updated code to be in line with lcdproc coding style(by AJW107)
 *-
 *- 29/08/15 - Added all remaining extra icon symbols (including colons for
 *-            time display)
 *-            Added explaination of all codes (4 are unknown)
 *-            Cleaned up code formatting (by AJW107)
 *-
 *- 24/03/14 - Attempt to add some custom icon support. Definitely working
 *-            - Volume (the Word), play, pause, stop, mute Music & photo
 *-            Icons light but do not extinguish !! (by blackeagle)
 *-
 *- 25/03/14 - Finished adding custom icon support with the XBMC LCDproc addon
 *-            and my custom futaba extension. (by blackeagle)
 *-
 *- 26/03/14 - Added support to XBMC client for FWD/REW icons.  Volume bar
 *-            algorithm tweaked to display 0 bars when muted. (by blackeagle)
 *-
 *- 27/03/14 - As the display doesnt support ':', added a routine to swap
 *-            this to '-' for the clock display (by blackeagle)
 *-
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or any later
 * version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lcd.h"
#include "futaba.h"
#include "errno.h"
#include "shared/report.h"

#define SET_REPORT 0x09

#define VENDOR_ID 0x0547
#define PRODUCT_ID 0x7000
//futabaDriver_t g_futabaDriver;

// ----------------------------------------------------------------------

/** private data for the futaba TOSD-5711BB LED driver */
typedef struct futaba_private_data {
	int width;			/* display width in characters */
	int height;			/* display height in characters */
	char *framebuf;			/* frame buffer */
	char *old_framebuf;		/* old framebuffer */
	int is_busy;			/* busy flag for futaba_flush */
	uint64_t old_icon_map;		/* futaba icon map */
	USB_DEVICE_HANDLE *my_handle;	/* usb device handle */
	futabaDriver_t *device;		/* data structure to store info about the futaba device */
#ifdef HAVE_LIBUSB_1_0
	libusb_context *ctx;		/* usb context */
#endif
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "futaba_";

/**
 * Send the report to the display.
 * \param my_handle  Pointer to usb handle.
 * \param my_report  Pointer to the report holding the string.
 * \return	     0 on success, -1 on error.
 */
int
futaba_send_report(USB_DEVICE_HANDLE * my_handle, futabaReport_t * my_report)
{
	int retVal = -1;
	uint8_t *p_rep = (uint8_t *) my_report;

	retVal = USB_CONTROL_TRANSFER(my_handle,
#ifdef HAVE_LIBUSB_1_0
				LIBUSB_DT_HID,	// Request Type (LIBUSB1.0)
#else
				USB_DT_HID,	// Request Type (USB0.1)
#endif
				SET_REPORT,	// Request
				0x0200,		// Report Type OUTPUT | ID 0
				0,		// Endpoint
#ifdef HAVE_LIBUSB_1_0
				p_rep,		// Data (LIBUSB1.0)
#else
				(char *) p_rep,	// Data (USB0.1)
#endif
				sizeof(futabaReport_t),	// Length
				5000);

	return (retVal != sizeof(futabaReport_t));
}

 /**
 * Send the data to the display.
 * \param drvthis  Pointer to driver structure.
 * \param string   The string to send.
 * \return         0 on success, -1 on error.
 */
int
futaba_send_string(Driver *drvthis)
{
	int len;
	int i, n;
	futabaReport_t my_report;
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;
	char string[p->width * p->height];

	memset(&my_report, 0, sizeof(futabaReport_t));

	p->is_busy = 1;

	// get our string
	for (i = 0; i < p->height; i++) {
		memcpy(string, p->framebuf + (i * p->width), p->width);
		string[p->width] = '\0';

		/* swap all occurances of the ':' char for '-'
		 * TODO: Keep the ':' chars and use the ':' in between each
		 *       Segment of the display */
		len = strlen(string);
		for (n = 0; n < len; n++) {
			if (string[n] == ':') {
				string[n] = '-';
			}
		}

		len = strlen(string);

		my_report.opcode = FUTABA_OPCODE_STRING;
		my_report.param1 = 0x04;
		my_report.type.str.startPos = 0x01;
		my_report.type.str.len = len;

		// Convert to UPPER case as that's all the display uses
		for (n = 0; n < len; n++) {
			my_report.type.str.string[n] = toupper(string[n]);
		}

		if (len > 7) {
			while (len >= 7) {
				if (futaba_send_report(p->my_handle, &my_report)) {
					report(RPT_ERR, "[%s] Failed to send report",
					       drvthis->name);
					return -1;
					usleep(500000);
				}
				my_report.type.str.startPos--;
				len--;
			}
		}
		else {
			my_report.type.str.len = 7;
			futaba_send_report(p->my_handle, &my_report);
		}
	}

	p->is_busy = 0;
	return 0;
}

/**
 * Internal function to the usb handle for the driver, initalise the display
 * make sure the OS isn't trying to use the display as well
 * \param drvthis   Pointer to driver structure.
 * \return          0 on success, -1 on error.
 */
int
futaba_init_driver(Driver *drvthis)
{
	int retVal = -1;
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;

#ifdef HAVE_LIBUSB_1_0
	/* somewhere to store the return codes from libusb 1.0 functions to check against and display in logs */
	int error = 0;

	/* libusb1.0 code */
	if (p->ctx == NULL) {
		error = libusb_init(&p->ctx);
		if (error) {
			report(RPT_ERR, "LIBUSB1.0: [%s] USB init Failed with Error [%d]",
			       drvthis->name, error);
			retVal = -1;
		}
		else {
			debug(RPT_INFO, "LIBUSB1.0: [%s] USB init succeded", drvthis->name);
		}
	}

	/* set verbosity for LIBUSB 1.0 logging */
	libusb_set_debug(p->ctx, 3);

	// Get a handle to our device
	if ((p->my_handle = libusb_open_device_with_vid_pid(p->ctx,
							    VENDOR_ID,
							    PRODUCT_ID)) == NULL) {
		report(RPT_ERR, "LIBUSB1.0: [%s] open failed, no device found", drvthis->name);
		return -1;
	}
	else {
		debug(RPT_INFO, "LIBUSB1.0: [%s] open succeded, device found", drvthis->name);
		retVal = 0;
	}

	if (libusb_kernel_driver_active(p->my_handle, 0) == 1) {
		debug(RPT_DEBUG, "LIBUSB1.0: [%s] Kernel driver found. Detatching it...",
		      drvthis->name);

		error = libusb_detach_kernel_driver(p->my_handle, 0);
		if (error) {
			report(RPT_ERR,
			       "LIBUSB1.0: [%s] Can't detach kernel driver with error [%d]",
			       drvthis->name, error);
			return -1;
		}
		else {
			debug(RPT_INFO, "LIBUSB1.0: [%s] Detaching successful", drvthis->name);
			retVal = 0;
		}

		/*-
	         *- FIXME: Is this the libusb-1.0 equivalent to
		 *-    if (usb_set_altinterface(p->lcd, 0) < 0)
		 *-        report(RPT_WARNING, "%s: unable to set alternate configuration", drvthis->name);
		 * I always get error -5 (LIBUSB_ERROR_NOT_FOUND the requested alternate
		 * setting does not exist). Is this needed? Has it ever worked?
		 * lsusb reports one configuration with one interface and no alternate settings.
		 */
		usleep(100);
		error = libusb_set_interface_alt_setting(p->my_handle, 1, 0);
		if (error) {
			report(RPT_WARNING, "LIBUSB1.0: [%s] unable to aquire alternate usb settings error [%d]",drvthis->name, error);
		}
	}
	else {
		debug(RPT_DEBUG, "LIBUSB1.0: [%s] no kernel driver found", drvthis->name);
		retVal = 0;
	}

#else /* HAVE_LIBUSB_1_0 */
	/* libusb0.1 code */
	struct usb_bus *bus;
	struct usb_device *dev;
	int found_dev = 0;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		for (dev = bus->devices; dev != NULL; dev = dev->next) {
			if ((dev->descriptor.idVendor == VENDOR_ID) &&
			    (dev->descriptor.idProduct == PRODUCT_ID)) {
				report(RPT_INFO,
				       "USB0.1: [%s] found display on bus [%s] device [%s]",
				       drvthis->name, bus->dirname, dev->filename);
				p->my_handle = usb_open(dev);
				found_dev = 1;
			}
			if ( found_dev == 1){
				break;
			}
		}
		if ( found_dev == 1){
			break;
		}
	}
	if (p->my_handle != NULL) {
		debug(RPT_DEBUG, "USB0.1: [%s] opening device succeeded", drvthis->name);
		retVal = 0;
#ifdef LIBUSB_HAS_GET_DRIVER_NP
		char driver[1024];

		if (usb_get_driver_np(p->my_handle, 0, driver, sizeof(driver)) == 0) {
			report(RPT_WARNING,
			       "USB0.1: [%s] interface 0 already claimed by [%s] - detaching",
			       drvthis->name, driver);
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
			if (usb_detach_kernel_driver_np(p->my_handle, 0) < 0) {
				report(RPT_WARNING, "USB0.1: [%s] unable to detach [%s] driver",
				       drvthis->name, driver);
				// probably should return -1 here, but let's
				// give it a go still
			}
			else {
				debug(RPT_INFO, "USB0.1: [%s] detached [%s] driver", drvthis->name,
				      driver);
				retVal = 0;
			}
#endif /* LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP */
		}

		usleep(100);
		if (usb_set_altinterface(p->my_handle, 0) < 0) {
			report(RPT_WARNING, "USB0.1: [%s] unable to set alternate configuration", drvthis->name);
		}
#endif /* LIBUSB_HAS_GET_DRIVER_NP */
	}
	else {
		report(RPT_ERR, "USB0.1: [%s] no device found", drvthis->name);
		return -1;
	}
#endif /* HAVE_LIBUSB_1_0 */

	return retVal;
}

/**
 * Internal function to set up usb interface and reset display.
 * \param drvthis  Pointer to driver structure.
 * \return         0 on success.
 */
int
futaba_start_driver(Driver *drvthis)
{
	/* somewhere to store the return codes from libusb 1.0 functions to check against and display in logs */
	int error;
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;

	if (p == NULL) {
		report(RPT_ERR, "[%s] unable to initalise private data, is NULL. Could be out of memory?", drvthis->name);
		return -1;
	}

	if (futaba_init_driver(drvthis)) {
		report(RPT_ERR, "[%s] Failed to init driver", drvthis->name);
		futaba_shutdown(drvthis);
		return -1;
	}
	else {
		debug(RPT_INFO, "[%s] RESET Device", drvthis->name);
		error = USB_RESET_DEVICE(p->my_handle);
		if (error) {
			report(RPT_WARNING, "[%s] RESET Failed with error [%d], retrying ...",
			       drvthis->name, error);
			USB_CLOSE_DEVICE(p->my_handle);
			// try again, slower, just incase
			futaba_init_driver(drvthis);
			usleep(500000);
			error = USB_RESET_DEVICE(p->my_handle);
			usleep(500000);
			if (error) {
				report(RPT_ERR, "[%s] Failed to re-init driver", drvthis->name);
				futaba_shutdown(drvthis);
				return -1;
			}
			else {
				debug(RPT_INFO, "[%s] re-init succeeded", drvthis->name);
			}
		}
		else {
			debug(RPT_INFO, "[%s] reset succeded", drvthis->name);
		}
	}

#ifdef HAVE_LIBUSB_1_0
	/* LIBUSB 1.0 code */
	// Claim the interface for us
	error = libusb_claim_interface(p->my_handle, 0);
	if (error) {
		report(RPT_ERR, "LIBUSB1.0: [%s] Failed to claim interface with error [%d]",
		       drvthis->name, error);
		futaba_shutdown(drvthis);
		return -1;
	}
	else {
		debug(RPT_INFO, "LIBUSB1.0: [%s] device successfully claimed", drvthis->name);
	}

#else /* HAVE_LIBUSB_1_0 */
	/* USB 0.1 code */
	if (usb_claim_interface(p->my_handle, 0) < 0) {
		report(RPT_ERR, "USB0.1: [%s] cannot claim interface!", drvthis->name);
		futaba_shutdown(drvthis);
		return -1;
	}
	else {
		debug(RPT_INFO, "USB0.1: [%s] claimed interface", drvthis->name);
	}
#endif /* HAVE_LIBUSB_1_0 */
	return 0;
}

/**
 * Internal function to cleanly exit.  It should free up memory, but if it
 * fails to do this, it will just continue without checking.
 * \param drvthis  Pointer to driver structure.
 */
void
futaba_shutdown(Driver *drvthis)
{
	/* no need to check for failure, just try it */
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;
#ifdef HAVE_LIBUSB_1_0
	/* LIBUSB 1.0 version of code */
	int error;

	error = libusb_release_interface(p->my_handle, 0);
	if (error) {
		report(RPT_ERR, "LIBUSB1.0: [%s] usb interface release failed with error [%d]",
		       drvthis->name, error);
	}

	/* FIXME: Does it make sense to re-attach a kernel driver? picolcd
	*  does it, but questions it's value, so maybe we should too, even
        * though we aren't keeping track if a kernel driver is used initially */
	error = libusb_attach_kernel_driver(p->my_handle, 0);
	if (error) {
		report(RPT_WARNING,
		       "LIBUSB1.0: [%s] failed to re-attach to kernel driver (not serious, may never have been a kernel driver initialy) with error [%d]",
		       drvthis->name, error);
	}

	libusb_close(p->my_handle);

	libusb_exit(p->ctx);
#else /* The libusb 0.1 way */
	usb_release_interface(p->my_handle, 0);
	usb_close(p->my_handle);
#endif
	debug(RPT_INFO, "[%s] USB connection closed", drvthis->name);
}

 /**
 * Initalize the driver, usb connection and data structures.
 * \param drvthis  Pointer to driver structure.
 * \return         0 on success, -1 on error.
 */
MODULE_EXPORT int
futaba_init(Driver *drvthis)
{
	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL) {
		report(RPT_ERR, "[%s] Out of memory creating Private Data", drvthis->name);
		return -1;
	}

	/* initalise the private data structure */
	if (drvthis->store_private_ptr(drvthis, p)) {
		report(RPT_ERR, "%s: Error creating pointer to Private Data [%d]", drvthis->name,
		       p);
		return -1;
	}

	/* initialize private data */
	p->device = NULL;
	p->width = 7;
	p->height = 1;
	p->old_icon_map = 0;

	/* all frame buffers shout be initalised but empty */
	p->framebuf = (char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "[%s] unable to create framebuffer", drvthis->name);
		return -1;
	}

	p->old_framebuf = (char *) malloc(p->width * p->height);
	if (p->old_framebuf == NULL) {
		report(RPT_ERR, "[%s] unable to create old_framebuffer", drvthis->name);
		return -1;
	}

	/* write to the file */
	futaba_start_driver(drvthis);
	memset(p->framebuf, ' ', p->width * p->height);
	memcpy(p->old_framebuf, p->framebuf, p->width * p->height);
	debug(RPT_INFO, "[%s] init() succeded", drvthis->name);
	return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_close(Driver *drvthis)
{
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		// terminate usb driver
		futaba_shutdown(drvthis);
		// clean up data structures used and release memory
		if (p->framebuf != NULL) {
			debug(RPT_INFO, "[%s] Freeing framebuffer memory", drvthis->name);
			free(p->framebuf);
		}

		if (p->old_framebuf) {
			debug(RPT_INFO, "[%s] Freeing memory from old framebuffer", drvthis->name);
			free(p->old_framebuf);
		}
		debug(RPT_INFO, "[%s] Freeing Private Data structure from memory", drvthis->name);
		free(p);
	}
	debug(RPT_INFO, "[%s] Private Data already empty/NULL", drvthis->name);
	drvthis->store_private_ptr(drvthis, NULL);

	debug(RPT_DEBUG, "[%s] USB driver close complete", drvthis->name);
	return;
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
futaba_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->width;
}

/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
futaba_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->height;
}

/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);
	debug(RPT_DEBUG, "[%s] clear complete", drvthis->name);
}

/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	/* If driver is busy scrolling, do nothing This shouldn't occur as
	 * LCDd should be sending the correct sized strings */

	if (p->is_busy == 1) {
		debug(RPT_INFO, "[%s] is busy, unable to flush - this should not happen",
		      drvthis->name);
		return;
	}

	for (i = 0; i < p->height; i++) {
		int offset = i * p->width;

		/* Backing-store based implementation: Only put it on the
		 * screen if it's not already there Otherwise the display
		 * flickers badly !! */
		if (memcmp(p->old_framebuf + offset, p->framebuf + offset, p->width) != 0) {
			memcpy(p->old_framebuf + offset, p->framebuf + offset, p->width);
			futaba_send_string(drvthis);
		}
	}
	debug(RPT_DEBUG, "[%s] flush complete\n\t[%s]\n\t[%s]", drvthis->name, p->framebuf,
	      p->old_framebuf);
}

/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
futaba_string(Driver *drvthis, int x, int y, const char string[])
{
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;
	int i;

	debug(RPT_DEBUG, "[%s] string start [%s]", drvthis->name, string);

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((y < 0) || (y >= p->height)) {
		debug(RPT_DEBUG, "[%s] string y co-ordinates out of bounds [%d]>[%d]",
		      drvthis->name, y, p->height);
		return;
	}

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0) {
			/* only write text after the left border */
			p->framebuf[(y * p->width) + x] = string[i];
		}
	}
	debug(RPT_DEBUG, "[%s] string complete [%s]", drvthis->name, string);
}

/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
futaba_chr(Driver *drvthis, int x, int y, char c)
{
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "[%s] chr start [%c]", drvthis->name, c);

	y--;
	x--;
	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height)) {
		/* make sure co-ordinates are sensible */
		p->framebuf[x] = c;
	}
	else {
		debug(RPT_DEBUG, "[%s] co-ordinates (%d,%d) out of bounds (%d,%d) [%c]",
		      drvthis->name, x, y, p->width, p->height, c);
	}
	debug(RPT_DEBUG, "[%s] chr complete [%c]", drvthis->name, c);
}

/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
futaba_get_info(Driver *drvthis)
{
	// PrivateData *p = drvthis->private_data;
	static char *info_string =
		"Futaba TOSD-5711BB Driver v1.3 (c) Blackeagle 2014 & AJW107 2016";
	return info_string;
}

/**
 *  Controls the custom icons.
 *
 * The bits of the \c icon_map value map to icons as this (a bit set '1' means the
 * icon is switched on, a bit set '0' means the icon is turned off):
 *
 * Icon					Bit	Code
 * Volume (the word)			0	0x01
 * shuffle				1	0x0E
 * mute					2	0x0F
 * phone				3	0x10 (unused)
 * rec					4	0x11
 * radio				5	0x12
 * dvd					6	0x13 (set on movie / tv show playback)
 * vcd					7	0x14
 * cd					8	0x15
 * music				9	0x16
 * photo				10	0x17
 * tv					11	0x18
 * disk in tray				12	0x19
 * 5.1					13	0x1A
 * 7.1					14	0x1B
 * repeat				15	0x1E
 * all					16	0x1F
 * rew					17	0x20
 * pause				18	0x21
 * play					19	0x22
 * timer				20	0x23
 * guide1				21	0x24 (A physically long word, so both)
 * guide2				22	0x25 (guide1 and guide2 need to be on)
 * home					23	0x26
 * eject				24	0x28
 * fwd					25	0x29
 * KHz					26	0x2A (Not Used)
 * MHz					27	0x2B (Not Used)
 * 1st Colon				28	0x2C
 * 1st Dot				29	0x2D
 * 2nd Colon				30	0x2E
 * 2nd Dot				31	0x2F
 * 3rd Colon				32	0x30
 * 3rd Dot				33	0x31
 * 4th Colon				34	0x32
 * 4th Dot				35	0x33
 * 5th Colon				36	0x34
 * 5th Dot				37	0x35
 * 6th Colon				38	0x36
 * 6th Dot				39	0x37
 * vol (decimal 1-10)			3A....3D (No actual codes, just 4 values put aside)
 *						 (to differentiate volume bar control)
 * \param drvthis   pointer to driver structure
 * \param icon_map  integer with bits representing LED states
 */
MODULE_EXPORT void
futaba_output(Driver *drvthis, uint64_t icon_map)
{
	/* store the private data in a variable to make the code easier to read (no a->b->c) */
	PrivateData *p = drvthis->private_data;
	uint64_t icons_changed = icon_map ^ p->old_icon_map;
	/*- Apart from the codes in the array below:
         *- Volume Bar Codes: 0x02 to 0x0C are each line of the volume bar
         *- Unknown Codes: 0x0D, 0x1C, 0x1D, 0x27 where not SEEN to do anything (but may do)
         *- Segment Codes: 0x38 to 0x99 contol each of the 14 segments of each character
	 *- Unused Codes: 0x9A onwards just produce garbage */
	const char Icon[FUTABA_ICON_ARRAY_LENGTH] =
		{ 0x01, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
		0x1a, 0x1b, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A,
		0x2B, 0x2C, 0x2D, 0x2E,
		0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
	};
	futabaReport_t my_report;
	int i, n;
	uint64_t numBars, the_volume;

	/* initalise report data structure */
	memset(&my_report, 0, sizeof(futabaReport_t));

	/* populate it */
	my_report.opcode = FUTABA_OPCODE_SYMBOL;
	my_report.param1 = 0x02;
	my_report.type.sym.count = 1;	/* set just one icon with each call */

	/* only change icons whose states have changed to reduce flicker */
	for (i = 0; i < FUTABA_ICON_ARRAY_LENGTH; i++) {
		if (icons_changed & (1 << i)) {
			my_report.type.sym.symbol[0].symName = Icon[i];
			my_report.type.sym.symbol[0].state = ((icon_map & (1 << i)) < 1) ? 0 : 1;;
			futaba_send_report(p->my_handle, &my_report);
			debug(RPT_INFO, "[%s] Icon no.%d [%c] updated", drvthis->name, i, Icon[i]);
		}
	}

	/* this is 0 -  10 */
	the_volume = (icon_map >> FUTABA_ICON_ARRAY_LENGTH) & 0x0F;

	/* Only write to the display if the volume has changed */
	if (the_volume != ((p->old_icon_map >> FUTABA_ICON_ARRAY_LENGTH) & 0x0F)) {
		memset(&my_report, 0, sizeof(futabaReport_t));
		numBars = (the_volume * FUTABA_VOLUME_BARS) / 10;
		my_report.opcode = FUTABA_OPCODE_SYMBOL;
		my_report.param1 = 0x02;
		my_report.type.sym.count = FUTABA_VOLUME_BARS;

		for (n = 0; n < FUTABA_VOLUME_BARS; n++) {
			my_report.type.sym.symbol[n].symName = FUTABA_VOLUME_START + n;

			if (n <= numBars) {
				/* if we don't do this we always light one bar ! */
				if (the_volume != 0) {
					my_report.type.sym.symbol[n].state = FUTABA_SYM_ON;
					debug(RPT_INFO, "[%s] volume changed", drvthis->name);
				}
				else {
					my_report.type.sym.symbol[n].state = 0;
				}
			}
		}
		/* all done, send to the display */
		futaba_send_report(p->my_handle, &my_report);
	}
	/* and off we go again */
	p->old_icon_map = icon_map;
}
