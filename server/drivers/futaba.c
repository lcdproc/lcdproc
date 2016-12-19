/**
 * LCDd driver for the Futaba TOSD-5711BB LED display,
 * as found in Elonex Artisan/FIC Spectra media centre cases
 */

/* Copyright (C) 2014 Blackeagle  email: gm(dot)blackeagle(at)gmail(dot)com
 * Additions by Alex Wood (2015)  email: thetewood(at)gmail(dot)com
 *
 * CREDITS to Steve Williams for the original driver code and the
 * inspiration to write this.
 *
 * V E R S I O N  - 1.2
 *
 * C H A N G E L O G
 * =================
 *
 * 29/08/15 - Added all remaining extra icon symbols (including colons for time display)
 *            Added explaination of all codes (4 are unknown)
 *            Cleaned up code formatting
 *            (by AJW)
 *
 * 24/03/14 - Attempt to add some custom icon support.
 *
 * Definitely working - Volume (the Word), play, pause, stop, mute
 * Music & photo Icons light but do not extinguish !!
 *
 * 25/03/14 -  Finished adding custom icon support with the XBMC
 * 				LCDproc addon and my custom futaba extension.
 *
 * 26/03/14 -  Added support to XBMC client for FWD/REW icons.
 * 			   Volume bar algorithm tweaked to display 0 bars
 * 			   when muted.
 *
 * 27/03/14 -  As the display doesnt support ':', added a routine
 * 			   to swap this to '-' for the clock display
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <ctype.h>
#include "lcd.h"
#include "futaba.h" 
#include "errno.h"
#include "report.h"

#define SET_REPORT 0x09

#define VENDOR_ID 0x0547
#define PRODUCT_ID 0x7000
futabaDriver_t g_futabaDriver;

//----------------------------------------------------------------------

/** private data for the \* futaba TOSD-5711BB LED driver */
typedef struct futaba_private_data {
	int width;				/**< display width in characters */
	int height;				/**< display height in characters */
	char *framebuf;				/**< frame buffer */
	char *old_framebuf;			/** < old framebuffer */
	int is_busy;				/** < busy flag for futaba_flush  */
	int old_icon_map;			/** < futaba icon map  */
	libusb_device_handle *my_handle;	/** < usb device handle  */
	libusb_context *ctx;			/** < usb context  */
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "futaba_";

/**
 * Send the report to the display.
 * \param my_report  Pointer to the report holding the string.
 * \param my_handle  Pointer to usb handle.
 * \retval 	     0 on success, -1 on error.
 */
 int futabaSendReport(libusb_device_handle *my_handle, futabaReport_t *my_report)
{
	int retVal = -1;
	uint8_t *p_rep = (uint8_t *)my_report;

	retVal  = libusb_control_transfer(
					my_handle,
					LIBUSB_DT_HID,		// Request Type
					SET_REPORT,		// Request
					0x0200,			// Report Type OUTPUT | ID 0
					0,			// Endpoint
					p_rep,			// Data to go
					sizeof(futabaReport_t),	// Length
					5000);

	return(retVal != sizeof(futabaReport_t));
}

 /**
 * Send the data to the display.
 * \param drvthis  Pointer to driver structure.
 * \param string   The string to send.
 * \return         0 on success, -1 on error.
 */
 int futaba_send_string(Driver *drvthis)
{
	int len;
	int n, i;
	futabaReport_t my_report;
	PrivateData *p = drvthis->private_data;
	char string[p->width * p->height];

	memset(&my_report,0,sizeof(futabaReport_t));

	p->is_busy = 1;

	// get our string
	for (i = 0; i < p->height; i++) {
		memcpy(string, p->framebuf + (i * p->width), p->width);
		string[p->width] = '\0';

		// swap the : chars for -
		len = strlen (string);

		for(n=0; n < len; n++)
		{
			if (string[n] == ':')
				string[n] ='-';
		}

		len = strlen (string);

		my_report.opcode = FUTABA_OPCODE_STRING;
		my_report.param1 = 0x04;
		my_report.type.str.startPos = 0x01;
		my_report.type.str.len = len;

		// Convert to UPPER case as that's all the display uses
		for(n=0; n < len; n++)
		{
			my_report.type.str.string[n] = toupper(string[n]);
		}

		if( len > 7)
		{
			while( len >= 7)
			{
				if( futabaSendReport(p->my_handle, &my_report) )
				{
					fprintf(stderr,"Failed in futabaSendString()\n");
					return -1;
					usleep(500000);
				}
				my_report.type.str.startPos--;
				len--;
			}
		}
		else
		{
			my_report.type.str.len = 7;
			futabaSendReport(p->my_handle,&my_report);
		}
	}

	p->is_busy = 0;
	return 0;
}

/**
 * Initalize the usb data and internal handles.
 * \param drvthis   Pointer to driver structure.
 * \param my_driver Pointer to futaba driver structure.
 * \return          0 on success, -1 on error.
 */
int futabaInitDriver( futabaDriver_t *my_driver, Driver *drvthis)
{
	int retVal = -1;

	if( my_driver->ctx == NULL )
	{
		if( libusb_init(&my_driver->ctx) < 0 )
		{
			report(RPT_DEBUG, "%s: USB init Failed", drvthis->name);
			retVal--;
		}
	}

	// Get a handle to our device
	if( (my_driver->my_handle = libusb_open_device_with_vid_pid(
				my_driver->ctx,
				VENDOR_ID,
				PRODUCT_ID)) == NULL )
	{
		report(RPT_DEBUG, "%s: LibUsb open failed", drvthis->name);
	}
	else
	{
	        retVal++;
	}

	if( libusb_kernel_driver_active(my_driver->my_handle,0) )
	{
		report(RPT_DEBUG, "%s: Kernel driver found. Detatching it...", drvthis->name);

		if( libusb_detach_kernel_driver(my_driver->my_handle,0) )
		{
			report(RPT_DEBUG, "%s: Can't detach kernel driver", drvthis->name);
			retVal--;
		}
	}
	return retVal;
}

/**
 * Internal routine to set up usb interface and reset display.
 * \param drvthis  Pointer to driver structure.
 * \return         0 on success.
 */
int futaba_start_driver(Driver *drvthis)
{
	int n;
	futabaDriver_t *my_driver = &g_futabaDriver;
	PrivateData *p = drvthis->private_data;

	if( futabaInitDriver(my_driver,drvthis) )
	{
		report(RPT_DEBUG, "%s: Failed to init driver", drvthis->name);
		futaba_shutdown(drvthis);
	}
	else
	{
		report(RPT_DEBUG, "%s: RESET Device", drvthis->name);
		if( (n = libusb_reset_device(my_driver->my_handle)) )
		{
			report(RPT_DEBUG, "%s: RESET Failed", drvthis->name);
			libusb_close(my_driver->my_handle);
			if( futabaInitDriver(my_driver, drvthis) )
			{
				report(RPT_DEBUG, "%s: Failed to re-init driver", drvthis->name);
				futaba_shutdown(drvthis);
			}
		}
	}

	// Claim the interface for us
	if( (n = libusb_claim_interface(my_driver->my_handle,0)) )
	{
		report(RPT_DEBUG, "%s: Failed to claim interface", drvthis->name);
		libusb_close(my_driver->my_handle);
		libusb_exit(my_driver->ctx);
		exit(-1);
	}
	else
	{ // Write the usb handle and context to the private_data
		p->my_handle = my_driver->my_handle;
		p->ctx = my_driver->ctx;
	}
	return 0;
}

/**
 * Internal function to cleanly exit.
 * \param drvthis  Pointer to driver structure.
 */
void futaba_shutdown(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	libusb_release_interface(p->my_handle,0);
	libusb_close(p->my_handle);

	libusb_exit(p->ctx);
	// printf("Futaba display driver shutdown complete\n");
}

 /**
 * Initalize the private data and call futaba_start_driver.
 * \param drvthis  Pointer to driver structure.
 * \return         0 on success, -1 on error.
 */

MODULE_EXPORT int
futaba_init (Driver *drvthis)
{
	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */

	p->width = 7;
	p->height = 1;
        p->old_icon_map = 0;
	p->framebuf = (char *)malloc(p->width * p->height);

	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}

	p->old_framebuf = (char *)malloc(p->width * p->height);
	if (p->old_framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create old_framebuffer", drvthis->name);
		return -1;
	}

	/* write to the file */
	futaba_start_driver(drvthis);
	memset(p->framebuf, ' ', p->width * p->height);
	memcpy(p->old_framebuf, p->framebuf,p->width * p->height);
	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->framebuf != NULL)
			free(p->framebuf);
		if (p->old_framebuf)
			free(p->old_framebuf);
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

	futaba_shutdown(drvthis);
	return;
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
futaba_width (Driver *drvthis)
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
futaba_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->height;
}

/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);
}

/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
futaba_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	/* If driver is busy scrolling, do nothing
	 * This shouldn't occur as LCDd should be sending
	 * the correct sized strings */

	if (p->is_busy == 1)
		return;

	for (i = 0; i < p->height; i++) {
		int offset = i * p->width;

		/* Backing-store based implementation:
		 * Only put it on the screen if it's not already there
		 * Otherwise the display flickers badly !!   */

		if (memcmp(p->old_framebuf+offset, p->framebuf+offset, p->width) != 0) {
			memcpy(p->old_framebuf+offset, p->framebuf+offset, p->width);
			futaba_send_string(drvthis);
		}
	}
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
futaba_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--; y--; // Convert 1-based coords to 0-based...

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)	// no write left of left border
			p->framebuf[(y * p->width) + x] = string[i];
	}
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
futaba_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--; x--;
	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[x] = c;
}

/**
 * Change the display contrast.
 * Display is LED so we ignore it.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
futaba_set_contrast (Driver *drvthis, int promille)
{
	//PrivateData *p = drvthis->private_data;
	debug(RPT_DEBUG, "Contrast: %d command ignored as not supported", promille);
}

/**
 * Turn the display backlight on or off.
 * Display is LED so we ignore it.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
futaba_backlight (Driver *drvthis, int on)
{
	//PrivateData *p = drvthis->private_data;
	debug(RPT_DEBUG, "Backlight %s command ignored as not supported", (on) ? "ON" : "OFF");
}

/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
futaba_get_info (Driver *drvthis)
{
    	//PrivateData *p = drvthis->private_data;
        static char *info_string = "Futaba TOSD-5711BB Driver v1.1 (c) Blackeagle 2014";
	return info_string;
}

/**
 *  Controls the custom icons.
 *
 * The bits of the \c icon_map value map to icons as this (a bit set '1' means the
 * icon is switched on, a bit set '0' means the icon is turned off):
 *
 * Icon 				Bit	Code
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
 * \param drvthis  pointer to driver structure
 * \param state	   integer with bits representing LED states
 */
MODULE_EXPORT void
futaba_output (Driver *drvthis, int icon_map)
{
	PrivateData *p = drvthis->private_data;
	int icons_changed = icon_map ^ p->old_icon_map;
        /* Apart from the codes in the array below:
         * Volume Bar Codes: 0x02 to 0x0C are each line of the volume bar
         * Unknown Codes:    0x0D, 0x1C, 0x1D, 0x27 where not SEEN to do anything (but may do)
         * Segment Codes:    0x38 to 0x99 contol each of the 14 segments of each character
         * Unused Codes:     0x9A onwards just produce garbage */
	const char Icon[FUTABA_ICON_ARRAY_LENGTH] = {0x01, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
	futabaReport_t my_report;
	int i, numBars, the_volume, n;

	memset(&my_report,0,sizeof(futabaReport_t));

	my_report.opcode = FUTABA_OPCODE_SYMBOL;
	my_report.param1 = 0x02;
	my_report.type.sym.count=1; /* set just one icon with each call */

	for (i=0; i<FUTABA_ICON_ARRAY_LENGTH; i++)
	{
		if (icons_changed & (1 << i))
		{
			my_report.type.sym.symbol[0].symName = Icon[i];
			my_report.type.sym.symbol[0].state = ((icon_map & (1 << i)) < 1) ? 0 : 1;;
			futabaSendReport(p->my_handle, &my_report);
		}
	}

	the_volume = (icon_map >> FUTABA_ICON_ARRAY_LENGTH) & 0x0F ; /* this is 0 - 10 */
	/* Only write to the display if the volume has changed */
	if (the_volume != ((p->old_icon_map >> FUTABA_ICON_ARRAY_LENGTH) & 0x0F ))
	{
		memset(&my_report,0,sizeof(futabaReport_t));
		numBars = (the_volume*FUTABA_VOLUME_BARS)/10;
		my_report.opcode = FUTABA_OPCODE_SYMBOL;
		my_report.param1 = 0x02;
		my_report.type.sym.count = FUTABA_VOLUME_BARS;

		for(n=0; n < FUTABA_VOLUME_BARS; n++)
		{
			my_report.type.sym.symbol[n].symName = FUTABA_VOLUME_START+n;

			if( n <= numBars)
			{ /* if we don't do this we always light one bar ! */
				if (the_volume != 0)
					my_report.type.sym.symbol[n].state = FUTABA_SYM_ON;
				else
					my_report.type.sym.symbol[n].state = 0;
			}
		}
		futabaSendReport(p->my_handle, &my_report);
	}
	p->old_icon_map = icon_map;
}

