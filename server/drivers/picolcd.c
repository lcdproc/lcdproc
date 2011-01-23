/** \file server/drivers/picolcd.c
 * LCDd \c picolcd driver for the Mini-Box.com USB LCD picoLCD displays.
 *
 * Currently the following devices are supported:
 * \li picoLCD 20x2 (OEM), stand-alone version http://www.mini-box.com/picoLCD-20x2-OEM
 *     or pre-installed as in http://www.mini-box.com/Mini-Box-M300-LCD.
 * \li picoLCD-4x20-sideshow, http://www.mini-box.com/picoLCD-4x20-sideshow.
 *
 * The picoLCD is USB connected and is driven via userspace using libusb
 * library (http://www.libusb.org/).
 */

/*-
 * Copyright (c) 2007 NitroSecurity, Inc.
 * 		 2007-2008 Peter Marschall
 * 		 2007-2008 Mini-Box.com, Nicu Pavel <npavel@ituner.com>
 * 		 2008 Jack Cleaver
 * 		 2009 Andries van Schie
 * 		 2010 Martin Jones <martin.t.jones@virgin.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/*-
 * Driver history:
 *
 * 2007 NitroSecurity, Inc.
 *  - First version written by Gatewood Green <woody@nitrosecurity.com>
 * 2007-2008 Peter Marschall
 *  - adapted coding style and reporting to LCDproc
 * 2007 Mini-Box.com, Nicu Pavel <npavel@ituner.com>
 *  - removed libusblcd and hid dependency
 *  - added vbar, hbar, custom char, bignum support
 * 2008 Jack Cleaver
 *  - add LIRC connection
 * 2008 Mini-Box.com Nicu Pavel <npavel@mini-box.com>
 *  - Added support for 4x20 picoLCD
 * 2009 Andries van Schie
 *  - Bugfix RC-5 for picoLCD 20x2
 *  - Changed to dynamic IR sync(space) injection, by timing time between end
 *    and start pulse.
 *  - Queueing IR data to prevent timeouts by LIRC (sending by timeout)
 *  - Removed usb_clear_halt, because it breaks picoLCD 20x2 (1.57) communication
 * 2010 Martin Jones <martin.t.jones@virgin.net>
 *  - Use module output function to control key LEDs.
 * 2011 Markus Dolze
 *  - Clean-up includes and move all defaults to header file.
 *  - Fix heartbeat and bignum by mapping character 0 to 8.
 *  - Fix backlight and contrast handling (by M. T. Jones).
 *  - Add more icons (by M. T. Jones).
 *  - Add OffBrightness for 20x4 (idea by S. Crane), IMPORTANT: The meaning
 *    of 'bklight_min' changed.  It now is the maximum value allowed if
 *    brightness is set off.
 *  - Add range checking for config settings.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Various odds and ends */
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <usb.h>

/* LCDproc includes */
#include "lcd.h"
#include "lcd_lib.h"
#include "adv_bignum.h"
#include "report.h"
#include "picolcd.h"

#define NUM_CCs         8 /* max. number of custom characters */

/* PrivateData struct */
typedef struct picolcd_private_data {
	usb_dev_handle *lcd;
	int  width;
	int  height;
	int  cellwidth;
	int  cellheight;
	int  key_timeout;
	int  contrast;
	int  backlight;
	int  brightness;
	int  offbrightness;
	int  keylights;
	int  key_light[KEYPAD_LIGHTS];
	CGmode ccmode;
	char *info;
	unsigned char *framebuf;
	unsigned char *lstframe;
	/* device info struct */
	picolcd_device *device;
	/* For communicating with LIRC */
	int IRenabled;
	int lircsock;
	struct sockaddr_in lircserver;
	/* IR transcode results */
	unsigned char result[512];
	unsigned char* resptr;
	struct timeval lastmsg;
	int lastval;
	int flush_threshold;
} PrivateData;

/* Private function definitions */
static void picolcd_send(usb_dev_handle *lcd, unsigned char *data, int size);
static void picolcd_20x2_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data);
static void picolcd_20x4_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data);
static void picolcd_20x2_set_char(Driver *drvthis, int n, unsigned char *dat);
static void picolcd_20x4_set_char(Driver *drvthis, int n, unsigned char *dat);
static void get_key_event(usb_dev_handle *lcd, lcd_packet *packet, int timeout);
static void set_key_lights(usb_dev_handle *lcd, int keys[], int state);
static void picolcd_lircsend(Driver *drvthis);
static void ir_transcode(Driver *drvthis, unsigned char *data, unsigned int cbdata);

/**
 * Table describing various features of known picoLCD devices and pointers
 * to low-level functions.
 */
picolcd_device picolcd_device_ids[] = {
	{
		.device_name  = "picoLCD20x2",
		.description  = "Driver for picoLCD 20x2 OEM and picoLCD found on M200/M300 cases",
		.vendor_id    = 0x04d8,
		.device_id    = 0x0002,
		.bklight_max  = 1,
		.bklight_min  = 0,
		.contrast_max = 40,
		.contrast_min = 0,
		.width        = 20,
		.height       = 2,
		.write        = picolcd_20x2_write,
		.cchar        = picolcd_20x2_set_char,
		/* all keymap labels must be shorter than KEYPAD_LABEL_MAX */
		.keymap       = { NULL,  "Plus", "Minus", "F1", "F2", "F3", "F4", "F5",
				  "Left", "Right", "Up", "Down", "Enter" },
		.initseq      = {},
	},
	{
		.device_name  = "picoLCD20x4",
		.description  = "Driver for picoLCD 20x4 desktop LCD",
		.vendor_id    = 0x04d8,
		.device_id    = 0xc001,
		.bklight_max  = 100,
		.bklight_min  = 100,
		.contrast_max = 1,
		.contrast_min = 0,
		.width        = 20,
		.height       = 4,
		.write        = picolcd_20x4_write,
		.cchar        = picolcd_20x4_set_char,
		/* all keymap labels must be shorter than KEYPAD_LABEL_MAX */
		.keymap       = { NULL, "Escape", "F1", "F2", "F3", "Home", "Down",
				  "Enter", "Up", "", "", "", ""},
		.initseq      = { 0x94, 0x00, 0x07, 0x00, 0x32, 0x30, 0x00, 0x32,
				  0x30, 0x00, 0x32, 0x30, 0x00, 0x32, 0x38, 0x00,
				  0x32, 0x06, 0x00, 0x32, 0x0C, 0x07, 0xD0, 0x01},
	},
	{}, /* End list */
};

/* lcd_logical_driver Variables */
MODULE_EXPORT char *api_version       = API_VERSION;
MODULE_EXPORT int  stay_in_foreground = 0;
MODULE_EXPORT int  supports_multiple  = 1;
MODULE_EXPORT char *symbol_prefix     = "picoLCD_";

/* lcd_logical_driver mandatory functions */

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int picoLCD_init(Driver *drvthis)
{
	PrivateData *p;
	int x;
	struct usb_bus *bus;
	struct usb_device *dev;
	const char *lirchost;
	int lircport;
	int id;
	int tmp;

	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;

	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Try to find picolcd device */
	usb_init();
	usb_find_busses();
	usb_find_devices();

	p->lcd = NULL;
	p->device = NULL;

	for (id = 0; picolcd_device_ids[id].device_name != NULL; ++id) {
		report(RPT_INFO, "%s: looking for device %s ",
			drvthis->name, picolcd_device_ids[id].device_name);
		for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
			for (dev = bus->devices; dev != NULL; dev = dev->next) {
				if ((dev->descriptor.idVendor == picolcd_device_ids[id].vendor_id) &&
				     (dev->descriptor.idProduct == picolcd_device_ids[id].device_id)) {
					report(RPT_INFO, "%s: found %s on bus %s device %s",
						drvthis->name, picolcd_device_ids[id].device_name,
						bus->dirname, dev->filename);

					p->lcd = usb_open(dev);
					p->device = &picolcd_device_ids[id];
					goto done;
				}
			}
		}
	}
	done:

	if (p->lcd != NULL) {
		debug(RPT_DEBUG, "%s: opening device succeeded", drvthis->name);

#ifdef LIBUSB_HAS_GET_DRIVER_NP
		char driver[1024];

		if (usb_get_driver_np(p->lcd, 0, driver, sizeof(driver)) == 0) {
			report(RPT_WARNING, "%s: interface 0 already claimed by '%s' - detaching",
				drvthis->name, driver);
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
			if ((usb_detach_kernel_driver_np(p->lcd, 0) < 0))
				report(RPT_ERR, "%s: unable to detach %s driver", drvthis->name, driver);
#endif
		}
#endif
		if (usb_claim_interface(p->lcd, 0) < 0) {
			report(RPT_ERR, "%s: cannot claim interface!", drvthis->name);
			usb_close(p->lcd);
			return -1;
		}
		usleep(100);
		if (usb_set_altinterface(p->lcd, 0) < 0)
			report(RPT_WARNING, "%s: unable to set alternate configuration", drvthis->name);
	}
	else {
		report(RPT_ERR, "%s: no device found", drvthis->name);
		return -1;
	}

	/* if the device has a init sequence sent it to device */
	picolcd_send(p->lcd, p->device->initseq, PICOLCD_MAX_DATA_LEN);

	p->width  = p->device->width;
	p->height = p->device->height;
	p->info = p->device->description;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;
	p->ccmode = standard;

	/* set contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_CONTRAST);
		tmp = DEFAULT_CONTRAST;
	}
	p->contrast = tmp;

	/* set brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* set brightness while display is off */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_OFFBRIGHTNESS);
		tmp = DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

	/* Backlight and key lights enable/disable */
	p->backlight = drvthis->config_get_bool(drvthis->name, "Backlight", 0, DEFAULT_BACKLIGHT);
	p->keylights = drvthis->config_get_bool(drvthis->name, "KeyLights", 0, DEFAULT_KEYLIGHTS);

	/* allow individual lights to be set */
	for (x = 0; x < KEYPAD_LIGHTS; x++) {
		char configkey[32];

		sprintf(configkey, "Key%dLight", x);
		p->key_light[x] = drvthis->config_get_bool(drvthis->name, configkey, 0, 1);
	}

	/* Get Timeout for USB read of key presses */
	tmp = drvthis->config_get_int(drvthis->name, "KeyTimeout", 0, DEFAULT_TIMEOUT);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: KeyTimeout must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_TIMEOUT);
		tmp = DEFAULT_TIMEOUT;
	}
	p->key_timeout = tmp;

	/* Allocate and clear frame buffers */
	p->framebuf = (unsigned char *) malloc(p->width * p->height + 1);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuf", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);
	p->framebuf[p->width * p->height] = '\0';

	p->lstframe = (unsigned char *) malloc(p->width * p->height + 1);
	if (p->lstframe == NULL) {
		report(RPT_ERR, "%s: unable to create lstframe", drvthis->name);
		return -1;
	}
	memset(p->lstframe, ' ', p->width * p->height);
	p->lstframe[p->width * p->height] = '\0';

	/* Apply config settings to the display */
	if (p->backlight)
		picoLCD_backlight(drvthis, 1);
	else
		picoLCD_backlight(drvthis, 0);

	if (p->keylights)
		set_key_lights(p->lcd, p->key_light, 1);
	else
		set_key_lights(p->lcd, p->key_light, 0);

	picoLCD_set_contrast(drvthis, p->contrast);

	/* setup LIRC */
	lirchost = drvthis->config_get_string(drvthis->name, "LircHost", 0, NULL);
	lircport = drvthis->config_get_int(drvthis->name, "LircPort", 0, DEFAULT_LIRCPORT);
	/* LIRC is only enabled if a hostname is set */
	p->IRenabled = (lirchost != NULL && *lirchost != '\0') ? 1 : 0;

	tmp = drvthis->config_get_int(drvthis->name, "LircFlushThreshold", 0, DEFAULT_FLUSH_THRESHOLD_JIFFY);
	/* Prevent small 'foolish' values these will disable the check also! */
	if (p->flush_threshold < 16) {
		report(RPT_WARNING, "%s: flush threshold to small - disabled");
		tmp = 0x8000; /* Disabled, send check will always fail! */
	}
	else if (p->flush_threshold > 0x7FFF) {
		report(RPT_WARNING, "%s: flush threshold to large, using default");
		tmp = DEFAULT_FLUSH_THRESHOLD_JIFFY;
	}
	p->flush_threshold = tmp;

	/* Simulate that the last value send was a PULSE,
	 * so we start with sending a SPACE to make LIRC happy
	 */
	p->lastval = 0;
	p->resptr  = p->result;
	gettimeofday(&p->lastmsg, NULL);

	if (p->IRenabled) {
		/* Initialize communication with LIRC */
		struct hostent *hostinfo = gethostbyname(lirchost);

		if (hostinfo == NULL) {
			report (RPT_ERR, "%s: unknown LIRC host %s", drvthis->name, lirchost);
			return -1;
		}
		if ((p->lircsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			report(RPT_ERR, "%s: failed to create socket to send data to LIRC", drvthis->name);
			return -1;
		}

		/* Restrict LircPort to usable values */
		if ((lircport < 1) || (lircport > 0xFFFF)) {
			report(RPT_WARNING, "%s: invalid LircPort, using default");
			lircport = DEFAULT_LIRCPORT;
		}

		/* Construct the server sockaddr_in structure */
		memset(&p->lircserver, 0, sizeof(p->lircserver));		/* Clear struct */
		p->lircserver.sin_family = AF_INET;				/* Internet/IP */
		p->lircserver.sin_addr = *(struct in_addr *) hostinfo->h_addr;	/* IP address */
		p->lircserver.sin_port = htons(lircport);			/* server port */

		report(RPT_INFO, "%s: IR events will be sent to LIRC on %s:%d, with flush threshold=%d",
			drvthis->name, lirchost, lircport, p->flush_threshold);
	}

	report(RPT_INFO, "%s: init complete", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void picoLCD_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	usb_release_interface(p->lcd, 0);
	usb_close(p->lcd);

	debug(RPT_DEBUG, "%s: close complete", drvthis->name);
}


/* lcd_logical_driver Essential output functions */

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int  picoLCD_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int  picoLCD_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void picoLCD_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;

	debug(RPT_DEBUG, "%s: clear complete", drvthis->name);
}


/**
 *  Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void picoLCD_flush(Driver *drvthis)
{
	PrivateData   *p = drvthis->private_data;
	unsigned char *fb = p->framebuf;
	unsigned char *lf = p->lstframe;
	static unsigned char   text[48];
	int           i, line, offset;

	debug(RPT_DEBUG, "%s: flush started", drvthis->name);

	for (line = 0; line < p->height; line++) {
		memset(text, 0, sizeof(text));
		offset = line * p->width;
		fb     = p->framebuf + offset;
		lf     = p->lstframe + offset;

		for (i = 0; i < p->width; i++) {
			if (*fb++ != *lf++) {
				strncpy((char *)text, (char *)p->framebuf + offset, p->width);
				p->device->write(p->lcd, line, 0, text);
				memcpy(p->lstframe + offset, p->framebuf + offset, p->width);

				debug(RPT_DEBUG, "%s: flush wrote line %d (%s)",
					drvthis->name, line + 1, text);

				break;
			}
		}
	}

	debug(RPT_DEBUG, "%s: flush complete\n\t(%s)\n\t(%s)",
		drvthis->name, p->framebuf, p->lstframe);
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void picoLCD_string(Driver *drvthis, int x, int y, unsigned char string[])
{
	PrivateData *p = drvthis->private_data;
	unsigned char *dest;
	int  len;

	debug(RPT_DEBUG, "%s: string start (%s)", drvthis->name, string);

	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	x--; y--; /* Convert 1-based to 0-based */

	len = strlen((char *)string);
	if (len + x > p->width) {
		debug(RPT_DEBUG, "%s: string overlength (>%d). Start: %d Length: %d (%s)",
			drvthis->name, p->width, x, len, string);

		len = p->width - x; /* Copy what we can */
	}

	dest = p->framebuf + (y * p->width + x);
	memcpy(dest, string, len * sizeof(unsigned char));

	debug(RPT_DEBUG, "%s: string complete (%s)", drvthis->name, string);
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void picoLCD_chr(Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: chr start (%c)", drvthis->name, c);

	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	x--; y--; /* Convert 1-based to 0-based */

	/*
	 * Map NUL to character 8 to avoid problems with string handling
	 * functions used elsewhere. The custom characters stored in pos. 0-7
	 * are repeated in pos. 8-15 (this is a feature of the controller
	 * used).
	 */
	if (c == 0)
		c = 8;

	p->framebuf[y * p->width + x] = c;

	debug(RPT_DEBUG, "%s: chr complete (%c)", drvthis->name, c);
}

/* lcd_logical_driver User-defined character functions */

/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8 (=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void picoLCD_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;

	p->device->cchar(drvthis, n, dat);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int picoLCD_get_free_chars (Driver *drvthis)
{
	return NUM_CCs;
}

/* lcd_logical_driver Extended output functions */

/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void picoLCD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i] = 0xFF;
			picoLCD_set_char(drvthis, i, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void picoLCD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		p->ccmode = hbar;

		memset(hBar, 0x00, sizeof(hBar));

		for (i = 1; i <= p->cellwidth; i++) {
			/* fill pixel columns from left to right. */
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			picoLCD_set_char(drvthis, i, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void picoLCD_num (Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	if (p->ccmode != bignum) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		p->ccmode = bignum;

		do_init = 1;
	}

	/* Lib_adv_bignum does everything needed to show the big numbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     symbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int picoLCD_icon (Driver *drvthis, int x, int y, int icon)
{
	static unsigned char heart_open[] =
	{
		b_______,
		b___X_X_,
		b__X_X_X,
		b__X___X,
		b__XX_XX,
		b___X_X_,
		b____X__,
		b_______
	};

	static unsigned char heart_filled[] =
	{
		b_______,
		b___X_X_,
		b__XXXXX,
		b__XXXXX,
		b__XXXXX,
		b___XXX_,
		b____X__,
		b_______
	};

	static unsigned char checkbox_gray[] =
	{
		b__X_X_X,
		b_______,
		b__X___X,
		b____X__,
		b__X___X,
		b_______,
		b__X_X_X,
		b_______
	};

	static unsigned char checkbox_off[] =
	{
		b__XXXXX,
		b__X___X,
		b__X___X,
		b__X___X,
		b__X___X,
		b__X___X,
		b__XXXXX,
		b_______
	};

	static unsigned char checkbox_on[] =
	{
		b__XXXXX,
		b__X___X,
		b__XX_XX,
		b__X_X_X,
		b__XX_XX,
		b__X___X,
		b__XXXXX,
		b_______
	};

	switch (icon) {
		case ICON_BLOCK_FILLED:
			picoLCD_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
			picoLCD_set_char(drvthis, 0, heart_filled);
			picoLCD_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			picoLCD_set_char(drvthis, 0, heart_open);
			picoLCD_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_LEFT:
			picoLCD_chr(drvthis, x, y, 127);
			break;
		case ICON_ARROW_RIGHT:
			picoLCD_chr(drvthis, x, y, 126);
			break;
		case ICON_CHECKBOX_GRAY:
			picoLCD_set_char(drvthis, 5, checkbox_gray);
			picoLCD_chr(drvthis, x, y, 5);
			break;
		case ICON_CHECKBOX_ON:
			picoLCD_set_char(drvthis, 6, checkbox_on);
			picoLCD_chr(drvthis, x, y, 6);
			break;
		case ICON_CHECKBOX_OFF:
			picoLCD_set_char(drvthis, 7, checkbox_off);
			picoLCD_chr(drvthis, x, y, 7);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/* lcd_logical_driver Essential input functions */

/**
 * Handle input from keyboard.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key
 */
MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	lcd_packet keydata;
	char *keystr = NULL;
	int  keys_read = 0;
	int  key_pass  = 0;
	int  two_keys  = 0;

	debug(RPT_DEBUG, "%s: get_key start (timeout %d)",
		drvthis->name, p->key_timeout);

	while (! keys_read) {
		get_key_event(p->lcd, &keydata, p->key_timeout);
		debug(RPT_DEBUG, "%s: get_key got an event", drvthis->name);

		if (keydata.type == IN_REPORT_KEY_STATE) {
			if (! keydata.data[1] && key_pass) {
				debug(RPT_DEBUG, "%s: get_key got all clear", drvthis->name);
				/* Got a <0, 0> key-up event after reading a valid key press event */
				keys_read++; /* All clear */
			}
			else if (! keydata.data[2] && ! two_keys) {
				debug(RPT_DEBUG, "%s: get_key got one key", drvthis->name);
				/* We got one key (but not after a two key event and before and all clear) */
				keystr = p->device->keymap[keydata.data[1]];
			}
			else {
				/* We got two keys */
				static char keybuf[2 * KEYPAD_LABEL_MAX + 1];

				debug(RPT_DEBUG, "%s: get_key got two keys", drvthis->name);
				two_keys++;
				sprintf(keybuf, "%s+%s", p->device->keymap[keydata.data[1]],
							 p->device->keymap[keydata.data[2]]);
				keystr = keybuf;
			}

			key_pass++; /* This hack allows us to deal with receiving left over <0,0> first */
		}
		else if (p->IRenabled && keydata.type == IN_REPORT_IR_DATA) {
			debug(RPT_NOTICE, "%s: get_key irdata, length=%d bytes",
				drvthis->name, keydata.data[1]);

			/* transcoded data is queued and send when complete or by a timeout */
			ir_transcode(drvthis, keydata.data + 2, keydata.data[1]);
		}
		else {
			debug(RPT_DEBUG, "%s: get_key got non-key/ir data or timeout", drvthis->name);
			if (p->result < p->resptr) {
				debug(RPT_INFO, "picolcd: timeout %d send lirc data now", p->key_timeout);
				/* Send data maybe is enough for LIRC */
				picolcd_lircsend(drvthis);
			}
			/* We got IR or otherwise bad data */
			return NULL;
		}

	}

	debug(RPT_DEBUG, "%s: get_key complete (%s)", drvthis->name, keystr);

	if ((keystr != NULL) && (strlen(keystr) > 0))
		return keystr;

	return NULL;

/*
 * Due to how key events are reported, we need to keep reading key presses
 * until we get the all clear (all keys up) event.
 *
 * Key events come back in such a way to report up to two simultanious keys
 * pressed.  The highest numbered key always comes back as the first key and
 * the lower numbered key follows.  If only one key was pressed, the second
 * key is 0.  I will refer to a key event as: <high key, low key>.
 *
 * The picoLCD also sends key-up events.
 *
 * On a single key press, the return is <keynum, 0>.  The key-up event is a
 * read that returns <0, 0> (all clear).  On a dual key press, if one key is
 * released later than the other key, the first key-up event is
 * <remainingkey, 0>.  This will be followed by a final "all clear" key-up
 * <0, 0>.  If both keys are release simultaniously, then after <hk, lk>,
 * you will receive <0, 0>.  If the keys are pressed down in a staggard
 * fashion, you will receive <first key, 0> followed by <hk, lk> followed by
 * key-up events as already detailed.
 *
 * What this means is that we need to keep reading key presses until we get
 * the <0, 0> all clear.
 *
 * For keymapping see the picolcd_device structs.
 */

}

/* lcd_logical_driver Hardware functions */

/**
 * Get current display contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         Stored contrast in promille.
 */
MODULE_EXPORT int picoLCD_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change display contrast.
 * \param drvthis   Pointer to driver structure.
 * \param promille  New contrast value in promille.
 */
MODULE_EXPORT void picoLCD_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[2] = { 0x92 }; /* set contrast id */

	/*
	 * Higher values for promille will result in less contrast. So reverse
	 * the meaning of promille.
	 */
	if (promille > 0 && promille <= 1000) {
		p->contrast = promille;

		if (p->device->contrast_max == 1)
			packet[1] = 0x00; /* picoLCD20x4 permits contrast as 0/1 value */
		else {
			int inv = 1000 - promille;
			packet[1] =  inv * p->device->contrast_max / 1000;
		}
	}
	else if (promille > 1000) {	/* Should not really happen */
		p->contrast = 1000;
		packet[1] = p->device->contrast_min;
	}
	else if (promille <= 0) {
		p->contrast = 0;
		packet[1] = p->device->contrast_max;
	}

	picolcd_send(p->lcd, packet, 2);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return         Stored brightness in promille.
 */
MODULE_EXPORT int picoLCD_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Set on/off brightness.
 * \param drvthis   Pointer to driver structure.
 * \param state     Brightness state (on/off) for which we want to store the value.
 * \param promille  New brightness in promille.
 */
MODULE_EXPORT void picoLCD_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	if (promille < 0 || promille > 1000)
		return;

	if (state == BACKLIGHT_ON)
		p->brightness = promille;
	else
		p->offbrightness = promille;
}


/**
 * Turn the backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param state    New backlight status.
 */
MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[2] = { 0x91 }; /* set backlight id */
	int s;

	if (state == BACKLIGHT_ON) {
		s = p->brightness / 10;
		if (s > p->device->bklight_max)
			s = p->device->bklight_max;
		packet[1] = (unsigned char) s;
		picolcd_send(p->lcd, packet, 2);
		/* Only enable key lights if enabled by user */
		if (p->keylights)
			set_key_lights(p->lcd, p->key_light, state);
	}
	else if (state == BACKLIGHT_OFF) {
		s = p->offbrightness / 10;
		if (s > p->device->bklight_min)
			s = p->device->bklight_min;
		packet[1] = (unsigned char) s;
		picolcd_send(p->lcd, packet, 2);
		/* Always turn ley lights off */
		set_key_lights(p->lcd, p->key_light, state);
	}
}


/**
 * Set output port(s). If the keypad is connected this controls the key lights.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
MODULE_EXPORT void  picoLCD_output(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	int x;
	int m;

	for (x = 0, m = 1; x < KEYPAD_LIGHTS; x++, m <<= 1) {
		p->key_light[x] = state & m;
	}
	set_key_lights(p->lcd, p->key_light, 1);
}


/* lcd_logical_driver Informational functions */

/**
 * Provide general information about the LCD/VFD display.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT char *picoLCD_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->info;
}


/* Private functions */

/**
 * Transcode from picoLCD USB format to LIRC UDP format.
 * LIRC UDP packets expect 16-bit intervals, with MSB set for space.
 * Intervals are measured in jiffies (1/16384 s).
 * PicoLCD USB packets contain 16-bit intervals, with value negated
 * for space. Intervals are in microseconds.
 * PicoLCD presents the bytes in network order, and they must be put back
 * in that order for transmission via UDP.
 * One jiffy == 61 us. 537 us == 9j.
 *
 * \param drvthis   Pointer to driver structure [used for debug() and report()].
 * \param data      Buffer of integers to be transcoded.
 * \param cbdata    Buffer of integers to be transcoded.
 *
 * \note The picoLCD introduces two issues:
 * \note 1. Every read contains a maximum of 10 samples (20 bytes),
 *    sending the converted samples direct to LIRC will lead to timeouts,
 *    in LIRC while we are still waiting for the rest of the samples.
 *    To fix this I queue the samples and send it when a sync is detected or by a timeout.
 * \note 2. The sync (long space) are not send by the picoLCD.
 *    To fix this we look for a pulse at the end of the last message and a pulse at the
 *    begin new message, we then flush the queue and start with a (sync) space, with
 *    the duration of the time between the last and current message.
 *
 * \note To make LIRC happy I send the queued samples with the sync space a the begin,
 * and not at the end (the next 'calculated' sync is put at the begin of the next message),
 * this is because LIRC requires a space at the begin but will solves the missing space
 * with a timeout at the end.
 */
static void ir_transcode(Driver *drvthis, unsigned char* data, unsigned int cbdata)
{
	PrivateData *p = drvthis->private_data;
	int i;
	int cIntervals = cbdata >> 1;
	long w = (data[1] << 8) | data[0];
	struct timeval now;

	/* Check for odd buffer length (invalid buffer) */
	if (cbdata & 1) {
		return;
	}

	/* Get time needed to calculate the time between 2 ir data messages */
	gettimeofday(&now, 0);

	/* Check for a missing SPACE since the last message */
	debug(RPT_INFO, "picolcd: last 0x04x first %04x", p->lastval, (-w & 0xFFFF));
	if (((p->lastval & 0x8000) == 0) && ((-w & 0x8000) == 0)) {
		/* Calculate the time passed from the last ir message to now
		 * and use that time for the missing space (sync) */
		int secs = now.tv_sec - p->lastmsg.tv_sec;
		int gap = 0x7FFF;

		/* previous message is complete send it, without the added space */
		debug(RPT_INFO, "picolcd: missing sync detected, flushing queue before adding sync");
		picolcd_lircsend(drvthis);

		/* Prevent the overflow (2 secs = 32678 jiffies), but allow 2.99 seconds to reach the max */
		if (secs <= 2) {
			/* microseconds to jiffies (same as (16384/1000000) but no possible int32 overflow) */
			gap = ((now.tv_usec - p->lastmsg.tv_usec + secs * 1000000) * 256) / 15625;
			/* Check overflow */
			if (gap >= 0x8000) {
				gap = 0x7FFF;
			}
		}
		/* Make it a space */
		gap |= 0x8000;

		debug(RPT_INFO, "picolcd: injecting space %04hx between %04hx and %04hx",
			gap, p->lastval, -w & 0xFFFF);
		*p->resptr++ = (unsigned char)(gap & 0xff);
		*p->resptr++ = (unsigned char)((gap >> 8) & 0xff);
	}
	/* Check if there is enough space left in buffer to store all new samples */
	else if (cbdata >= (&p->result[sizeof(p->result)] - p->resptr)) {
		/* This should never happen but just to be sure. */
		debug(RPT_INFO, "picolcd: buffer almost full send lirc data now");
		picolcd_lircsend(drvthis);
	}
	for (i = 0; i < cIntervals; i++) {
		w = *data++;
		w |= *data++ << 8;

		if (w & 0x8000) {
			/* IF w is negative THEN negate. E.g. 0xDCA1 (-9055) -> 9055. */
			w = 0x10000 - w;
			/* scale: orig is usec, new is jiffy. E.g. 9055usec = 148 jiffy. */
			w = (w * 16384/ 1000000) & 0xFFFF;
		}
		else {
			/* Scale */
			w = w * 16384 / 1000000;
			if (w >= p->flush_threshold) {
				report(RPT_INFO, "picolcd: detected sync space sending lirc data now");
				picolcd_lircsend(drvthis);
			}
			/* Set the space bit */
			w |= 0x8000;
		}
		*p->resptr++ = (unsigned char)(w & 0xff);
		*p->resptr++ = (unsigned char)((w >> 8) & 0xff);
	}
	p->lastval = w;
	p->lastmsg = now;
	/* Look for a short buffer (a full buffer has 10 samples) with a terminal PULSE */
	if ((cIntervals < 10) && ((w & 0x8000) == 0)) {
		debug(RPT_INFO, "picolcd: IR data end detected sending lirc data now");
		picolcd_lircsend(drvthis);
	}
}

/**
 * Send any queued IR samples to LIRC
 * \param drvthis  Pointer to driver structure.
 */
static void picolcd_lircsend(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int len = p->resptr - p->result;
	if (len > 0) {
#ifdef DEBUG
		debug(RPT_INFO, "picolcd: sending LIRCD %d samples", len/2);
		{
			unsigned char *ptr = p->result;
			unsigned char *endptr = ptr + len;
			char logbuf[sizeof(p->result) * 3]; /* every 2 bytes become 5 bytes " xxxx" */
			char* logptr = logbuf;
			while (ptr < endptr) {
				unsigned int val = *ptr++;
				val |= *ptr++ << 8;
				logptr += sprintf(logptr, " %04x", val);
			}
			debug(RPT_DEBUG, "picolcd: data:%s", logbuf);
		}
#endif
		if (sendto(p->lircsock, p->result, len, 0,
			(struct sockaddr *) &(p->lircserver),  sizeof(p->lircserver)) == -1) {
			/* Ignore not connected errors when lirc has gone away */
			if (errno != ECONNREFUSED) {
				report(RPT_WARNING, "picolcd: failed to send IR data, reason: %s", strerror(errno));
			}
		} else {
			debug(RPT_DEBUG, "picolcd: send %d bytes to lirc(udp)", len);
		}
		p->resptr = p->result;
	}
}


/**
 * Send raw data to the display using low level usb_interrupt_write.
 * \param lcd   pointer to device handle
 * \param data  pointer to data packet to send
 * \param size  number of bytes to send
 */
static void picolcd_send(usb_dev_handle *lcd, unsigned char *data, int size)
{
	if ((lcd == NULL) && (data == NULL))
		return;

	usb_interrupt_write(lcd, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}


/**
 * Write function for 20x4 desktop displays
 * \param lcd   pointer to device handle
 * \param row   Row to place the string at
 * \param col   ignored
 * \param data  pointer to NUL terminated string
 */
static void picolcd_20x4_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data)
{
	unsigned char packet[64] = { 0x95, 0x01, 0x00, 0x01 };
	unsigned char lineset[4][6] = {
		{ 0x94, 0x00, 0x01, 0x00, 0x64, 0x80 },
		{ 0x94, 0x00, 0x01, 0x00, 0x64, 0xC0 },
		{ 0x94, 0x00, 0x01, 0x00, 0x64, 0x94 },
		{ 0x94, 0x00, 0x01, 0x00, 0x64, 0xD4 }
	};
	int len = strlen((char *) data);

	/* Cut off at display width */
	if (len > 20)
		len = 20;

	/* Send command to select row */
	switch (row) {
		case 0:  picolcd_send(lcd, lineset[0], 6);  break;
		case 1:  picolcd_send(lcd, lineset[1], 6);  break;
		case 2:  picolcd_send(lcd, lineset[2], 6);  break;
		case 3:  picolcd_send(lcd, lineset[3], 6);  break;
		default: picolcd_send(lcd, lineset[0], 6);  break;
	}

	/* Fill in an send packet */
	packet[4] = len;
	memcpy(packet + 5, data, len);
	picolcd_send(lcd, packet, 5 + len);
}


/**
 * Write function for 20x2 OEM displays
 * \param lcd   pointer to device handle
 * \param row   Row to place the string at
 * \param col   ignored
 * \param data  pointer to NUL terminated string
 */
static void picolcd_20x2_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data)
{
	unsigned char packet[64] = { 0x98 };
	int len = strlen((char *) data);

	/*
	 * FIXME: Is it possible that data is written beyond the end of
	 * the selected row? Shouldn't cut off happen at (20 - col)?
	 */
	if (len > 20)
		len = 20;

	/* prepare and send packet */
	packet[1] = row;
	packet[2] = col;
	packet[3] = len;

	memcpy(packet + 4, data, len);

	picolcd_send(lcd, packet, 4 + len);
}


/**
 * Custom character define function for 20x2 OEM displays
 * \param drvthis  Pointer to driver structure
 * \param n        Index of custom character to update
 * \param dat      Pointer to array of pixel data
 */
static void picolcd_20x2_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[10] = { 0x9c };   /* define character */
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	packet[1] = n;	/* Custom char to define. */

	for (row = 0; row < p->cellheight; row++) {
		packet[row + 2] = dat[row] & mask;
	}

	picolcd_send(p->lcd, packet, 10);
}


/**
 * Custom character define function for 20x4 desktop displays
 * \param drvthis  Pointer to driver structure
 * \param n        Index of custom character to update
 * \param dat      Pointer to array of pixel data
 */
static void picolcd_20x4_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	unsigned char command[6] = { OUT_REPORT_CMD, 0x00, 0x01, 0x00, 0x64, 0x40+8*n }; /* 0x94 */
	unsigned char data[13] = { OUT_REPORT_DATA, 0x01, 0x00, 0x01, 0x08,
					dat[0], dat[1], dat[2], dat[3],
					dat[4], dat[5], dat[6], dat[7]};                 /* 0x95 */

	picolcd_send(p->lcd, command, 6);
	picolcd_send(p->lcd, data, 13);
}


/**
 * Read a key or IR event from the display into one packet.
 * \param lcd      pointer to device handle
 * \param packet   Pointer to packet structure which is filled with data
 *                 read from the display
 * \param timeout  Read timeout in ms
 */
static void get_key_event(usb_dev_handle *lcd, lcd_packet *packet, int timeout)
{
	int ret;

	memset(packet->data, 0, 255);
	packet->type = 0;

	ret = usb_interrupt_read(lcd, USB_ENDPOINT_IN + 1, (char *)packet->data, PICOLCD_MAX_DATA_LEN, timeout);
	if (ret > 0) {
		/* Set packet type */
		switch (packet->data[0]) {
			case IN_REPORT_KEY_STATE: {
				packet->type = IN_REPORT_KEY_STATE;
			} break;
			case IN_REPORT_IR_DATA: {
				packet->type = IN_REPORT_IR_DATA;
			} break;
			default: {
				packet->type = 0;
			}
		}
	}
}


/**
 * Set lights for individual keys
 * \param lcd    pointer to device handle
 * \param keys   array indicating which key number to turn on
 * \param state  0 to turn all LEDs off, 1 to turn them on according to
 *               values set in 'keys' array
 */
static void set_key_lights(usb_dev_handle *lcd, int keys[], int state)
{
	unsigned char packet[2] = { 0x81 }; /* set led */
	unsigned int leds = 0;
	int i;

	if (state) {
		/* Only LEDs we want on */
		for (i = 0; i < KEYPAD_LIGHTS; i++)
			if (keys[i])
				leds |= (1 << i);
			else
				leds &= ~ (1 << i);
	}
	else {
		/* All LEDs off */
		leds = 0;
	}

	packet[1] = leds;
	picolcd_send(lcd, packet, 2);
}

/* EOF */
