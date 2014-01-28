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
 * 		 2010-2014 Martin Jones <martin@brasskipper.org.uk>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
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
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* LCDproc includes */
#include "lcd.h"
#include "lcd_lib.h"
#include "adv_bignum.h"
#include "report.h"
#include "picolcd.h"
#include "timing.h"

#define NUM_CCs         8	/* max. number of custom characters */
#define KEY_BUFFER_SIZE 8	/* size of the key ring buffer */

#define ANSI_ESCAPES		/* Use color to make (IR) debug easier to read */
#ifdef ANSI_ESCAPES
#define	TEXT_NORMAL		"\033[0m"
#define	TEXT_RED		"\033[31m"
#else
#define	TEXT_NORMAL		""
#define	TEXT_RED		""
#endif

#ifdef HAVE_LIBUSB_1_0
/**
 * This structure holds the keys reported in a key event.
 */
typedef struct {
	unsigned char high_key;	/**< Set if a single key is pressed or it holds the first key if two keys are pressed.*/
	unsigned char low_key;	/**< Holds the second key pressed (if any). */
} keys;
#endif

/*
 * The current decision to use a separate select statement for libusb was
 * made to restrict the use of hardware related libraries to the drivers.
 * Ultimately it would be better if all socket and time event processing was
 * in one place. See:
 * http://libusb.sourceforge.net/api-1.0/group__asyncio.html#asyncevent
 *
 * If using a single select statement for all event processing libusb-1.0
 * initialisation, deinitialisation and event processing needs to be
 * performed by LCDd core code.  See comments in picoLCD_init(),
 * picoLCD_close() and picoLCD_get_key(). It may be convenient to use the
 * default context.
 */
#undef USE_LIBUSB_SINGLE_SELECT

/**
 * Multiple buffers are needed to ensure that no USB transfer is missed. Ideally
 * a single select statement with appropriate timeout should be used, in this
 * case double buffering is sufficient. When distributed select statements are
 * used processing of the USB signals is performed by
 * libusb_handle_events_timeout() in picoLCD_get_key(). This is called at 32Hz
 * (PROCESS_FREQ) i.e. every 31.25ms which is not really fast enough as the
 * picoLCD USB transfers can occur every 10ms. This may cause buffer overrun
 * problems for long bursts of IR data, to avoid problems there must be more
 * than 3 buffers.
 */
#ifdef USE_LIBUSB_SINGLE_SELECT
#define USB_BUFFERS 2
#else
#define USB_BUFFERS 4
#endif

#ifdef HAVE_LIBUSB_1_0
/**
 * This structure holds the data for a USB transfer.
 */
typedef struct usb_transfer_data {
	/** structure for the details of the asynchronous USB transfer */
	struct libusb_transfer *transfer;
	/** transfer status */
	int status;
	/** Pointer to driver private data */
	Driver *drvthis;
	/** data buffer for the USB transfer */
	unsigned char buffer[PICOLCD_MAX_DATA_LEN];
}UsbTransferData;
#endif

/** Private data for the picoLCD driver */
typedef struct picolcd_private_data {
	USB_DEVICE_HANDLE *lcd;
	int width;
	int height;
	int cellwidth;
	int cellheight;
	int contrast;
	int backlight;
	int brightness;
	int offbrightness;
	int keylights;
	int key_light[KEYPAD_LIGHTS];
	int linklights;
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
	unsigned char *resptr;
	struct timeval lastmsg;
	int lastval;
	int lirc_time_us;
	int flush_threshold;
#ifdef HAVE_LIBUSB_1_0
	/* Pointer to libusb 1.0 session */
	libusb_context *lib_ctx;
	/* structure for the details of the USB transfer */
	UsbTransferData input_transfer[USB_BUFFERS];
	/* buffer for the key press data */
	keys key_buffer[KEY_BUFFER_SIZE];
	int key_read_index;	/* Read index in the key_buffer */
	int key_write_index;	/* Write index in the key_buffer */
	keys reported_keys;	/* keys reported during last get_key */
	int key_repeat_delay;
	int key_repeat_interval;
	struct timeval *key_wait_time;
#else
	int key_timeout;
#endif
} PrivateData;

/* Private function definitions */
static void picolcd_send(USB_DEVICE_HANDLE *lcd, unsigned char *data, int size);
static void picolcd_20x2_write(USB_DEVICE_HANDLE *lcd, const int row, const int col, const unsigned char *data);
static void picolcd_20x4_write(USB_DEVICE_HANDLE *lcd, const int row, const int col, const unsigned char *data);
static void picolcd_20x2_set_char(Driver *drvthis, int n, unsigned char *dat);
static void picolcd_20x4_set_char(Driver *drvthis, int n, unsigned char *dat);
static void set_key_lights(USB_DEVICE_HANDLE *lcd, int keys[], int state);
static void picolcd_lircsend(Driver *drvthis);
static void ir_transcode(Driver *drvthis, unsigned char *data, unsigned int cbdata);
#ifdef HAVE_LIBUSB_1_0
static void free_usb_transfers(Driver *drvthis);
static void key_buffer_put(Driver *drvthis, unsigned char high_key, unsigned char low_key);
static void usb_cb_input(struct libusb_transfer *transfer);
#else
static void get_key_event(USB_DEVICE_HANDLE *lcd, lcd_packet *packet, int timeout);
#endif

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
MODULE_EXPORT int
picoLCD_init(Driver *drvthis)
{
	PrivateData *p;
#ifdef HAVE_LIBUSB_1_0
	int error = 0;
	int i;
#else
	struct usb_bus *bus;
	struct usb_device *dev;
#endif
	const char *lirchost;
	int lircport;
	int id;
	int tmp;

	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;

	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	p->lcd = NULL;
	p->device = NULL;

#ifdef HAVE_LIBUSB_1_0
#ifndef USE_LIBUSB_SINGLE_SELECT
	/*
	 * When using a single select statement libusb-1.0 should be
	 * initialised once for all USB drivers so code equivalent to that
	 * below should be somewhere common, before init_drivers() perhaps.
	 */
	error = libusb_init(&p->lib_ctx);
	if (error) {
		report(RPT_ERR, "%s: libusb_init error %d", drvthis->name, error);
		return -1;
	}
	libusb_set_debug(p->lib_ctx, 3);
#endif

	p->key_read_index = 0;
	p->key_write_index = 0;

	/*
	 * Try to find picolcd device the new way, this opens the first
	 * picoLCD found if you need to handle more than one picoLCD you'll
	 * need something more sophisticated.
	 */
	for (id = 0; picolcd_device_ids[id].device_name != NULL; id++) {
		report(RPT_INFO, "%s: looking for device %s ", drvthis->name,
			picolcd_device_ids[id].device_name);
		p->lcd = libusb_open_device_with_vid_pid(p->lib_ctx,
					   picolcd_device_ids[id].vendor_id,
					  picolcd_device_ids[id].device_id);
		if ((p->lcd) != NULL) {
			p->device = &picolcd_device_ids[id];
			debug(RPT_INFO, "%s: opening device %s succeeded", drvthis->name,
			      picolcd_device_ids[id].device_name);
			break;
		}
	}
	if (p->lcd == NULL) {
		report(RPT_ERR, "%s: no device found", drvthis->name);
		return -1;
	}

	if (libusb_kernel_driver_active(p->lcd, 0) == 1) {
		debug(RPT_DEBUG, "%s: libusb_kernel_driver_active returned true", drvthis->name);
		error = libusb_detach_kernel_driver(p->lcd, 0);
		if (error) {
			report(RPT_ERR, "%s: libusb_detach_kernel_driver error %d", drvthis->name, error);
			return -1;
		}
	}
	else {
		debug(RPT_DEBUG, "%s: libusb_kernel_driver_active returned false", drvthis->name);
	}

	error = libusb_claim_interface(p->lcd, 0);
	if (error) {
		report(RPT_ERR, "%s: libusb_claim_interface error %d", drvthis->name, error);
		return -1;
	}
	/*-
	 * FIXME: Is this the libusb-1.0 equivalent to
	 *    if (usb_set_altinterface(p->lcd, 0) < 0)
	 *        report(RPT_WARNING, "%s: unable to set alternate configuration", drvthis->name);
	 * I always get error -5 (LIBUSB_ERROR_NOT_FOUND the requested alternate
	 * setting does not exist). Is this needed? Has it ever worked?
	 * lsusb reports one configuration with one interface and no alternate settings.
	 */
	error = libusb_set_interface_alt_setting(p->lcd, 1, 0);
	if (error) {
		report(RPT_WARNING, "%s: libusb_set_interface_alt_setting error %d", drvthis->name, error);
	}

	/* Set-up USB input transfer data structures */
	for (i = 0; i < USB_BUFFERS; i++)
		p->input_transfer[i].transfer = NULL;
	for (i = 0; i < USB_BUFFERS; i++)
	{
		UsbTransferData *utdp = &p->input_transfer[i];

		utdp->drvthis = drvthis;
		utdp->transfer = libusb_alloc_transfer(0);
		if (utdp->transfer == NULL) {
			report(RPT_ERR, "%s: libusb_alloc_transfer failed", drvthis->name);
			free_usb_transfers(drvthis);
			return -1;
		}
		libusb_fill_interrupt_transfer(utdp->transfer,
				p->lcd,
				LIBUSB_ENDPOINT_IN + 1,
				utdp->buffer,
				sizeof(utdp->buffer),
				usb_cb_input,
				(void *)utdp,
				0);
		utdp->status = libusb_submit_transfer(utdp->transfer);
		if (utdp->status) {
			report(RPT_ERR, "%s: libusb_submit_transfer error %d",
					drvthis->name, utdp->status);
			free_usb_transfers(drvthis);
			return -1;
		}
	}

#else				/* The libusb 0.1 way */

	/* Try to find picolcd device */
	usb_init();
	usb_find_busses();
	usb_find_devices();

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
#endif	/* HAVE_LIBUSB_1_0 */

	/* if the device has a init sequence send it to device */
	picolcd_send(p->lcd, p->device->initseq, PICOLCD_MAX_DATA_LEN);

	p->width = p->device->width;
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
	p->linklights = drvthis->config_get_bool(drvthis->name, "LinkLights", 0, DEFAULT_LINKLIGHTS);

	/* allow individual lights to be set */
	for (tmp = 0; tmp < KEYPAD_LIGHTS; tmp++) {
		char configkey[32];

		sprintf(configkey, "Key%dLight", tmp);
		p->key_light[tmp] = drvthis->config_get_bool(drvthis->name, configkey, 0, 1);
	}

#ifdef HAVE_LIBUSB_1_0
	/* Aquire persistent time structure */
	p->key_wait_time = malloc(sizeof(struct timeval));
	if (p->key_wait_time == NULL) {
		report(RPT_ERR, "%s: error allocating memory", drvthis->name);
		return -1;
	}
	timerclear(p->key_wait_time);

	/* Get key auto repeat delay */
	tmp = drvthis->config_get_int(drvthis->name, "KeyRepeatDelay", 0, DEFAULT_REPEAT_DELAY);
	if (tmp < 0 || tmp > 3000) {
		report(RPT_WARNING, "%s: KeyRepeatDelay must be between 0-3000; using default %d",
			drvthis->name, DEFAULT_REPEAT_DELAY);
		tmp = DEFAULT_REPEAT_DELAY;
	}
	p->key_repeat_delay = tmp;

	/* Get key auto repeat interval */
	tmp = drvthis->config_get_int(drvthis->name, "KeyRepeatInterval", 0, DEFAULT_REPEAT_INTERVAL);
	if (tmp < 0 || tmp > 3000) {
		report(RPT_WARNING, "%s: KeyRepeatInterval must be between 0-3000; using default %d",
			drvthis->name, DEFAULT_REPEAT_INTERVAL);
		tmp = DEFAULT_REPEAT_INTERVAL;
	}
	p->key_repeat_interval = tmp;

	report(RPT_WARNING, "%s: Key repeat: delay %d, interval %d",
	       drvthis->name, p->key_repeat_delay, p->key_repeat_interval);

	p->reported_keys.high_key = 0;
	p->reported_keys.low_key = 0;

#else
	/* Get Timeout for USB read of key presses */
	tmp = drvthis->config_get_int(drvthis->name, "KeyTimeout", 0, DEFAULT_TIMEOUT);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: KeyTimeout must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_TIMEOUT);
		tmp = DEFAULT_TIMEOUT;
	}
	p->key_timeout = tmp;
#endif

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

	p->lirc_time_us = drvthis->config_get_bool(drvthis->name, "LircTime_us", 0, DEFAULT_LIRC_TIME_us);

	tmp = drvthis->config_get_int(drvthis->name, "LircFlushThreshold", 0, DEFAULT_FLUSH_THRESHOLD);
	/*
	 * Enforce a sensible minimum. Only want to flush on gaps between IR
	 * bursts not the spaces between marks.
	 */
	if (tmp < 1000) {
		report(RPT_WARNING, "%s: flush threshold to small (%d) , using default", drvthis->name, tmp);
		tmp = DEFAULT_FLUSH_THRESHOLD;
	}
	if (p->lirc_time_us) {
		/*
		 * Values greater than 32.767ms will disable the flush.
		 */
		if (tmp > 32727) {
			report(RPT_WARNING, "%s: flush threshold to large (%d), disabled", drvthis->name, tmp);
		}
	}
	else {
		/*
		 * Scale between microseconds and jiffies (1/16384s)
		 * Values greater than 1999.938ms will disable the flush.
		 */
		if (0x7FFF * 15625 / 256 < tmp) {
			report(RPT_WARNING, "%s: flush threshold to large (%d), disabled", drvthis->name, tmp);
			tmp = 0x8000;
		}
		else {
			tmp = tmp * 256 / 15625;
		}
	}
	p->flush_threshold = tmp;

	/*
	 * Simulate that the last value send was a MARK, so we start with
	 * sending a SPACE to make LIRC happy
	 */
	p->lastval = 0;
	p->resptr = p->result;
	gettimeofday(&p->lastmsg, NULL);

	if (p->IRenabled) {
		/* Initialize communication with LIRC */
		struct hostent *hostinfo = gethostbyname(lirchost);

		if (hostinfo == NULL) {
			report(RPT_ERR, "%s: unknown LIRC host %s", drvthis->name, lirchost);
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

		report(RPT_INFO, "%s: IR events will be sent to LIRC on %s:%d, with flush threshold=%d, time unit: %s",
			drvthis->name, lirchost, lircport, p->flush_threshold, p->lirc_time_us ? "us" : "1/16384s");
	}

	report(RPT_INFO, "%s: init complete", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
picoLCD_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p != NULL) {
#ifdef HAVE_LIBUSB_1_0
		int error;

		free_usb_transfers(drvthis);

		error = libusb_release_interface(p->lcd, 0);
		if (error) {
			report(RPT_ERR, "%s: usb_release_interface error %d", drvthis->name, error);
		}

		/* FIXME: Does it make sense to re-attach a kernel driver? */
		error = libusb_attach_kernel_driver(p->lcd, 0);
		if (error) {
			report(RPT_ERR, "%s: libusb_attach_kernel_driver error %d", drvthis->name, error);
		}

		libusb_close(p->lcd);
		if (p->key_wait_time != NULL)
			free(p->key_wait_time);
#ifndef USE_LIBUSB_SINGLE_SELECT
		/*
		 * When using a single select statement the code below should
		 * be called after closing all open devices and before the
		 * application terminates this should be somewhere common,
		 * after drivers_unload_all() perhaps.
		 */
		libusb_exit(p->lib_ctx);
#endif
#else	/* The libusb 0.1 way */
		usb_release_interface(p->lcd, 0);
		usb_close(p->lcd);
#endif
		if (p->framebuf != NULL)
			free(p->framebuf);
		if (p->lstframe != NULL)
			free(p->lstframe);
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

	debug(RPT_DEBUG, "%s: close complete", drvthis->name);
}


/* lcd_logical_driver Essential output functions */

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
picoLCD_width(Driver *drvthis)
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
picoLCD_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
picoLCD_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;

	debug(RPT_DEBUG, "%s: clear complete", drvthis->name);
}


/**
 *  Flush data in screen buffer to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
picoLCD_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char *fb = p->framebuf;
	unsigned char *lf = p->lstframe;
	static unsigned char text[48];
	int i, line, offset;

	debug(RPT_DEBUG, "%s: flush started", drvthis->name);

	for (line = 0; line < p->height; line++) {
		memset(text, 0, sizeof(text));
		offset = line * p->width;
		fb = p->framebuf + offset;
		lf = p->lstframe + offset;

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
 *
 * The picoLCD uses a HD44780UA00 which provides ASCII and Japanese fonts.
 *
 * Characters 0x20-0x7F are ASCII with the following substitutions:
 *	| Index | ASCII | Substitute       |
 *	| :---: | :---: | :--------------- |
 *	| 0x5C  |   \   | Yen              |
 *	| 0x7E  |   ~   | Rightwards Arrow |
 *	| 0x7F  |  DEL  | Leftwards Arrow  |
 *
 * Characters 0xA0-0xDF are half-width Katakana. They map to
 * the Unicode block starting at U+FF60 and ending at U+FF9F.
 *
 * Characters 0xE0-0xFF are various Greek letters and symbols.
 */
MODULE_EXPORT void
picoLCD_string(Driver *drvthis, int x, int y, unsigned char string[])
{
	PrivateData *p = drvthis->private_data;
	unsigned char *dest;
	int len;

	debug(RPT_DEBUG, "%s: string start (%s)", drvthis->name, string);

	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	x--;
	y--;			/* Convert 1-based to 0-based */

	len = strlen((char *)string);
	if (len + x > p->width) {
		debug(RPT_DEBUG, "%s: string overlength (>%d). Start: %d Length: %d (%s)",
			drvthis->name, p->width, x, len, string);

		len = p->width - x;	/* Copy what we can */
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
MODULE_EXPORT void
picoLCD_chr(Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: chr start (%c)", drvthis->name, c);

	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width))
		return;

	x--;
	y--;			/* Convert 1-based to 0-based */

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
MODULE_EXPORT void
picoLCD_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;

	p->device->cchar(drvthis, n, dat);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
picoLCD_get_free_chars(Driver *drvthis)
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
MODULE_EXPORT void
picoLCD_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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
			/* add pixel line per pixel line ... */
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
MODULE_EXPORT void
picoLCD_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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
			memset(hBar, 0x1F & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
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
MODULE_EXPORT void
picoLCD_num(Driver *drvthis, int x, int num)
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
	/*
	 * Lib_adv_bignum does everything needed to show the big numbers.
	 * Use an offset so that custom character zero is available for the heart-beat
	 */
	lib_adv_bignum(drvthis, x, num, 1, do_init);
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
MODULE_EXPORT int
picoLCD_icon(Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;

	static unsigned char heart_open[] =
	{
		b_______,
		b___X_X_,
		b__X_X_X,
		b__X___X,
		b__X___X,
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

	static unsigned char bar_right[] =
	{
		b____XXX,
		b____XXX,
		b____XXX,
		b____XXX,
		b____XXX,
		b____XXX,
		b____XXX,
		b_______
	};

	static unsigned char bar_left[] =
	{
		b__XXX__,
		b__XXX__,
		b__XXX__,
		b__XXX__,
		b__XXX__,
		b__XXX__,
		b__XXX__,
		b_______
	};
	static unsigned char triangle_right[] =
	{
		b__X____,
		b__XX___,
		b__XXX__,
		b__XXXX_,
		b__XXX__,
		b__XX___,
		b__X____,
		b_______
	};

	static unsigned char triangle_left[] =
	{
		b______X,
		b_____XX,
		b____XXX,
		b___XXXX,
		b____XXX,
		b_____XX,
		b______X,
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

	/* Icons from the display's character map ROM */
	switch (icon) {
	    case ICON_BLOCK_FILLED:
		picoLCD_chr(drvthis, x, y, 255);
		return 0;
	    case ICON_ARROW_LEFT:
		picoLCD_chr(drvthis, x, y, 127);
		return 0;
	    case ICON_ARROW_RIGHT:
		picoLCD_chr(drvthis, x, y, 126);
		return 0;
	}

	/*
	 * Icons loaded to the display's custom character RAM. Horizontal &
	 * vertical bars do not use RAM address zero so the heart can be used
	 * in these modes. Big numbers use an offset to avoid address zero.
	 */
	if ((p->ccmode != icons)
	    && !((p->ccmode == hbar || p->ccmode == vbar || p->ccmode != bignum)
		 && (icon == ICON_HEART_FILLED || icon == ICON_HEART_OPEN))) {
		if (p->ccmode != standard) {
			/* Combined custom character modes not supported */
			report(RPT_WARNING, "%s: icon: cannot combine two modes using user-defined characters",
			       drvthis->name);
			return -1;	/* Let the core do the icon */
		}
		p->ccmode = icons;
	}

	switch (icon) {
	    case ICON_HEART_FILLED:
		picoLCD_set_char(drvthis, 0, heart_filled);
		picoLCD_chr(drvthis, x, y, 0);
		break;
	    case ICON_HEART_OPEN:
		picoLCD_set_char(drvthis, 0, heart_open);
		picoLCD_chr(drvthis, x, y, 0);
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
		case ICON_SELECTOR_AT_LEFT:
			picoLCD_set_char(drvthis, 2, triangle_right);
			picoLCD_chr(drvthis, x, y, 2);
			break;
		case ICON_SELECTOR_AT_RIGHT:
			picoLCD_set_char(drvthis, 1, triangle_left);
			picoLCD_chr(drvthis, x, y, 1);
			break;
		/* Icons below are two characters wide */
		case ICON_STOP:       /* should look like  []  */
			picoLCD_set_char(drvthis, 4, bar_right);
			picoLCD_chr(drvthis, x, y, 4);
			picoLCD_set_char(drvthis, 3, bar_left);
			picoLCD_chr(drvthis, x+1, y, 3);
			break;
		case ICON_PAUSE:      /* should look like  ||  */
			picoLCD_set_char(drvthis, 3, bar_left);
			picoLCD_chr(drvthis, x, y, 3);
			picoLCD_set_char(drvthis, 4, bar_right);
			picoLCD_chr(drvthis, x+1, y, 4);
			break;
		case ICON_PLAY:       /* should look like  >   */
			picoLCD_set_char(drvthis, 2, triangle_right);
			picoLCD_chr(drvthis, x, y, 2);
			picoLCD_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_PLAYR:      /* should look like  <   */
			picoLCD_set_char(drvthis, 1, triangle_left);
			picoLCD_chr(drvthis, x, y, 1);
			picoLCD_chr(drvthis, x+1, y, ' ');
			break;
		case ICON_FF:         /* should look like  >>  */
			picoLCD_set_char(drvthis, 2, triangle_right);
			picoLCD_chr(drvthis, x, y, 2);
			picoLCD_chr(drvthis, x+1, y, 2);
			break;
		case ICON_FR:         /* should look like  <<  */
			picoLCD_set_char(drvthis, 1, triangle_left);
			picoLCD_chr(drvthis, x, y, 1);
			picoLCD_chr(drvthis, x+1, y, 1);
			break;
		case ICON_NEXT:       /* should look like  >|  */
			picoLCD_set_char(drvthis, 2, triangle_right);
			picoLCD_chr(drvthis, x, y, 2);
			picoLCD_set_char(drvthis, 3, bar_left);
			picoLCD_chr(drvthis, x+1, y, 3);
			break;
		case ICON_PREV:       /* should look like  |<  */
			picoLCD_set_char(drvthis, 4, bar_right);
			picoLCD_chr(drvthis, x, y, 4);
			picoLCD_set_char(drvthis, 1, triangle_left);
			picoLCD_chr(drvthis, x+1, y, 1);
			break;
		case ICON_REC:        /* should look like  () */
			/* There aren't enough custom characters to draw a circle so use a diamond. */
			picoLCD_set_char(drvthis, 1, triangle_left);
			picoLCD_chr(drvthis, x, y, 1);
			picoLCD_set_char(drvthis, 2, triangle_right);
			picoLCD_chr(drvthis, x+1, y, 2);
			break;
	    default:
		return -1;	/* Let the core do other icons */
	}
	return 0;
}


/* lcd_logical_driver Essential input functions */

/**
 * Handle input from keyboard.
 *
 * Key events come back in such a way as to report up to two simultaneous keys
 * pressed.  The highest numbered key always comes back as the first key and
 * the lower numbered key follows.  If only one key was pressed, the second
 * key is 0.  The picoLCD also sends key-up events (both key parameters zero).
 *
 * On a single key press, the return is <keynum, 0>.  The key-up event is a
 * read that returns <0, 0> (all clear).  On a dual key press, if one key is
 * released later than the other key, the first key-up event is <remaining key,
 * 0>.  This will be followed by a final "all clear" key-up <0, 0>.  If both
 * keys are released simultaneously, then after <high key, low key>, you will
 * receive <0, 0>.  If the keys are pressed down in a staggered fashion, you
 * will receive <first key, 0> followed by <high key, low key> followed by
 * key-up events as already detailed.
 *
 * For keymapping see the picolcd_device structs.
 *
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key
 */
MODULE_EXPORT char *
picoLCD_get_key(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char *keystr = NULL;

#ifdef HAVE_LIBUSB_1_0
	int high_key;
	int low_key;
	struct timeval current_time, delay_time;

#ifndef USE_LIBUSB_SINGLE_SELECT
	/*
	 * When using a single select statement polling for libusb-1.0 events
	 * needs to be integrated into the LCDd main loop. See:
	 * http://libusb.sourceforge.net/api-1.0/group__poll.html#pollmain
	 */
	struct timeval timeout;
	/*
	 * FIXME: It is not efficient to call this at 32Hz, it is only needed
	 * if a key has been pressed or IR data has been received.
	 *
	 * Process any outstanding USB events for our session.
	 */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	libusb_handle_events_timeout(p->lib_ctx, &timeout);
#endif

	/*
	 * Read any key events from the buffer and report, do not wait for
	 * key up events so that the main loop timing is not disrupted; thus
	 * the behaviour is somewhat different from the previous version.
	 */
	if (p->key_read_index == p->key_write_index) {
		/* No new key, check if it is time to repeat a key */
		if (p->reported_keys.high_key && timerisset(p->key_wait_time)) {
			gettimeofday(&current_time, NULL);

			/* Time for repeated key report? */
			if (timercmp(&current_time,p->key_wait_time,>)) {
				high_key = p->reported_keys.high_key;
				low_key = p->reported_keys.low_key;

				/* Set timer for next key */
				delay_time.tv_sec  = p->key_repeat_interval / 1000;
				delay_time.tv_usec = (p->key_repeat_interval % 1000) * 1000;
				timeradd(&current_time,&delay_time,p->key_wait_time);
			}
			else {
				return NULL;
			}
		}
		else {
			return NULL;
		}
	}
	else {
		/* Get new key data */
		high_key = p->key_buffer[p->key_read_index].high_key;
		low_key = p->key_buffer[p->key_read_index].low_key;
		debug(RPT_DEBUG, "%s: got %d, %d from key_buffer %d",
		      drvthis->name, high_key, low_key, p->key_read_index);

		/* Advance read buffer, wrapping around if necessary */
		p->key_read_index++;
		if (KEY_BUFFER_SIZE <= p->key_read_index)
			p->key_read_index = 0;

		/* Store the reported keys for repeat */
		p->reported_keys.high_key = high_key;
		p->reported_keys.low_key = low_key;
		/* Set the time for repeated key press if enabled */
		if (p->key_repeat_delay > 0) {
			gettimeofday(&current_time, NULL);
			delay_time.tv_sec  = p->key_repeat_delay / 1000;
			delay_time.tv_usec = (p->key_repeat_delay % 1000) * 1000;
			timeradd(&current_time,&delay_time,p->key_wait_time);
		}
	}

	if (low_key) {		/* Two keys have been pressed */
		static char keybuf[2 * KEYPAD_LABEL_MAX + 1];
		/*
		 * The order here is important for clients that are
		 * interested in multi-key presses. The key pairs are
		 * reported in the opposite order to their position in the
		 * key-map thus if a client wants to be informed when keys F1
		 * & F2 are both pressed it will have to send the command
		 * "client_add_key [-exclusively|-shared] F2+F1". It would be
		 * more logical to change the order but this is consistent
		 * with the previous version.
		 */
		sprintf(keybuf, "%s+%s", p->device->keymap[high_key],
			p->device->keymap[low_key]);
		keystr = keybuf;
	}
	else {			/* Only one key pressed */
		keystr = p->device->keymap[high_key];
	}

	debug(RPT_DEBUG, "%s: get_key complete (%s)", drvthis->name, keystr);

	if ((keystr != NULL) && (strlen(keystr) > 0))
		return keystr;
	else
		return NULL;

#else	/* the libusb 0.1 way */

	lcd_packet keydata;
	int  keys_read = 0;
	int  key_pass  = 0;
	int  two_keys  = 0;

	debug(RPT_DEBUG, "%s: get_key start (timeout %d)",
		drvthis->name, p->key_timeout);

	/*
	 * Due to how key events are reported, we need to keep reading key
	 * presses until we get the all clear (all keys up) event.
	 */
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
#endif
}

/* lcd_logical_driver Hardware functions */

/**
 * Get current display contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         Stored contrast in promille.
 */
MODULE_EXPORT int
picoLCD_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change display contrast.
 * \param drvthis   Pointer to driver structure.
 * \param promille  New contrast value in promille.
 */
MODULE_EXPORT void
picoLCD_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[2] = {0x92};	/* set contrast id */

	/*
	 * Higher values for promille will result in less contrast. So
	 * reverse the meaning of promille.
	 */
	if (promille > 0 && promille <= 1000) {
		p->contrast = promille;

		if (p->device->contrast_max == 1) {
			/* picoLCD20x4 permits contrast as 0/1 value */
			packet[1] = 0x00;
		}
		else {
			int inv = 1000 - promille;
			packet[1] = inv * p->device->contrast_max / 1000;
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
MODULE_EXPORT int
picoLCD_get_brightness(Driver *drvthis, int state)
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
MODULE_EXPORT void
picoLCD_set_brightness(Driver *drvthis, int state, int promille)
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
MODULE_EXPORT void
picoLCD_backlight(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[2] = {0x91};	/* set backlight id */
	int s;

	if (state == BACKLIGHT_ON) {
		s = p->brightness / 10;
		if (s > p->device->bklight_max)
			s = p->device->bklight_max;
		packet[1] = (unsigned char) s;
		picolcd_send(p->lcd, packet, 2);
		if (p->linklights) {
			/* Only enable key lights if enabled by user */
			if (p->keylights)
				set_key_lights(p->lcd, p->key_light, state);
		}
	}
	else if (state == BACKLIGHT_OFF) {
		s = p->offbrightness / 10;
		if (s > p->device->bklight_min)
			s = p->device->bklight_min;
		packet[1] = (unsigned char) s;
		picolcd_send(p->lcd, packet, 2);
		if (p->linklights) {
			/* Always turn key lights off */
			set_key_lights(p->lcd, p->key_light, state);
		}
	}
}


/**
 * Set output port(s). If the keypad is connected this controls the key lights.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
MODULE_EXPORT void
picoLCD_output(Driver *drvthis, int state)
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
MODULE_EXPORT char *
picoLCD_get_info(Driver *drvthis)
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
 * for mark. Intervals are in microseconds.
 * PicoLCD presents the bytes in network order, and they must be put back
 * in that order for transmission via UDP.
 * One jiffy == 61 us. 537 us == 9j.
 *
 * \param drvthis   Pointer to driver structure [used for debug() and report()].
 * \param data      Buffer of integers to be transcoded.
 * \param cbdata    Length of data to be transcoded.
 *
 * \note The picoLCD introduces two issues:
 *
 * \note 1. Every read contains a maximum of 10 samples (20 bytes), sending the
 *       converted samples direct to LIRC will lead to timeouts, in LIRC
 *       while we are still waiting for the rest of the samples. To fix this I
 *       queue the samples and send it when a sync is detected or by a timeout.
 *
 * \note 2. The sync (long space) are not send by the picoLCD. To fix this we
 *       look for a mark at the end of the last message and a mark at the
 *       begin new message, we then flush the queue and start with a (sync)
 *       space, with the duration of the time between the last and current
 *       message.
 *
 * \note To make LIRC happy I send the queued samples with the sync space a the
 *       begin, and not at the end (the next 'calculated' sync is put at the
 *       begin of the next message), this is because LIRC requires a space at
 *       the begin but will solves the missing space with a timeout at the end.
 */
static void
ir_transcode(Driver *drvthis, unsigned char *data, unsigned int cbdata)
{
	PrivateData *p = drvthis->private_data;
	int i;
	int cIntervals = cbdata >> 1;
	long w = (data[1] << 8) | data[0];
	struct timeval now;

	/* Check for odd buffer length (invalid buffer) */
	if (cbdata & 1) {
		report(RPT_WARNING, "picolcd: buffer invalid length (%d)", cbdata);
		return;
	}

	/* Get time needed to calculate the time between 2 IR data messages */
	gettimeofday(&now, 0);

#ifdef DEBUG
	debug(RPT_DEBUG, "picolcd: received %d IR samples", cIntervals);
	{
		unsigned char *ptr = data;
		int c = cIntervals;
		char logbuf[cbdata * 5];
		char *logptr = logbuf;
		while (c--) {
			unsigned int val = *ptr++;
			val |= *ptr++ << 8;
			logptr += sprintf(logptr, " %s%04x",
						(0x7fff < val) ? TEXT_RED : TEXT_NORMAL,
						(0x7fff < val) ? 0x10000 - val : val);
		}
		debug(RPT_DEBUG, "picolcd: data:%s" TEXT_NORMAL, logbuf);
	}
#endif

	/* Check for a missing SPACE since the last message */
	debug(RPT_DEBUG, "picolcd: last %04x first %04x", p->lastval, (-w & 0xFFFF));
	if (((p->lastval & 0x8000) == 0) && ((-w & 0x8000) == 0)) {
		/*
		 * Calculate the time passed from the last IR message to now
		 * and use that time for the missing space (sync)
		 */
		struct timeval time_gap;
		int gap = 0x7FFF;

		timersub(&now, &p->lastmsg, &time_gap);

		if (p->resptr != p->result) {
			/* previous message is complete send it without the added space */
			debug(RPT_INFO, "picolcd: missing space detected, flushing queue before adding sync");
			picolcd_lircsend(drvthis);
		}
		else {
			debug(RPT_INFO, "picolcd: missing space detected, adding timed space to buffer");
		}

		if (p->lirc_time_us) {
			/*
			 * When sending times in microseconds a single word gives a range up
			 * to 32767us. I am not aware of anything that uses IR pulse or space
			 * times as long as 32ms so the pulse and space times can just be
			 * copied from the buffer see below. The gap between transmissions can
			 * be very long so may need special encoding. LIRC processes times in
			 * microseconds using 24 bits so send a zero word as a flag followed
			 * by a three byte time sent as four bytes to keep the buffer length
			 * even. 24 bits gives a range up to 16777215 so we limit to 16s for
			 * simplicity.
			 */
			if (time_gap.tv_sec >= 16) {
				debug(RPT_INFO, "picolcd: IR transmission gap: 8000 00F42400");
				*p->resptr++ = 0x00;	/* zero */
				*p->resptr++ = 0x80;	/* with space bit */
				*p->resptr++ = 0x00;	/* 16s as 24 bit value */
				*p->resptr++ = 0x24;
				*p->resptr++ = 0xf4;
				*p->resptr++ = 0x00;
			}
			else {
				gap = (time_gap.tv_sec * 1000000 + time_gap.tv_usec);
				if (gap > 0x7FFF) {
					/* Send as 24 bits */
					debug(RPT_INFO, "picolcd: IR transmission gap: 8000 %08x", gap);
					*p->resptr++ = 0x00;	/* zero */
					*p->resptr++ = 0x80;	/* with space bit */
					*p->resptr++ = (unsigned char)(gap & 0xff);
					*p->resptr++ = (unsigned char)((gap >> 8) & 0xff);
					*p->resptr++ = (unsigned char)((gap >> 16) & 0xff);
					*p->resptr++ = 0x00;
				}
				else {
					/* Send a 16 bit space */
					gap |= 0x8000;
					debug(RPT_INFO, "picolcd: IR transmission gap: %04x", gap);
					*p->resptr++ = (unsigned char)(gap & 0xff);
					*p->resptr++ = (unsigned char)((gap >> 8) & 0xff);
				}
			}
		}
		else {	/* Intervals measured in jiffies (1/16384 s). */
			/*
			 * Prevent the overflow (2 secs = 32678 jiffies), but allow
			 * 2.99 seconds to reach the max
			 */
			if (time_gap.tv_sec >= 2) {
				/*
				 * microseconds to jiffies (same as (16384/1000000)
				 * but no possible int32 overflow)
				 */
				gap = ((time_gap.tv_sec * 1000000 + time_gap.tv_usec) * 256) / 15625;
			}
			/* Saturate on 15 bit overflow */
			if (gap >= 0x8000) {
				gap = 0x7FFF;
			}
			/* Make it a space */
			gap |= 0x8000;

			debug(RPT_INFO, "picolcd: injecting space %04x between %04x and %04x",
				gap, p->lastval, -w & 0xFFFF);
			*p->resptr++ = (unsigned char)(gap & 0xff);
			*p->resptr++ = (unsigned char)((gap >> 8) & 0xff);
		}
	}
	/* Check if there is enough space left in buffer to store all new samples */
	else if (cbdata >= (&p->result[sizeof(p->result)] - p->resptr)) {
		/* This should never happen but just to be sure. */
		report(RPT_WARNING, "picolcd: buffer almost full send lirc data now");
		picolcd_lircsend(drvthis);
	}
	for (i = 0; i < cIntervals; i++) {
		w = *data++;
		w |= *data++ << 8;

		if (w & 0x8000) {	/* Mark */
			/* picoLCD uses negative for mark so negate. E.g. 0xDCA1 (-9055) -> 9055. */
			w = 0x10000 - w;
			if (!p->lirc_time_us) {
				/* scale: orig is usec, new is jiffy. E.g. 9055usec = 148 jiffy. */
				w = (w * 16384 / 1000000) & 0xFFFF;
			}
		}
		else {	/* Space */
			if (!p->lirc_time_us) {
				/* Scale */
				w = w * 16384 / 1000000;
			}
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
	/*
	 * Look for a short buffer (a full buffer has 10 samples) with a
	 * terminal mark
	 */
	if ((cIntervals < 10) && ((w & 0x8000) == 0)) {
		debug(RPT_INFO, "picolcd: IR data end detected sending lirc data now");
		picolcd_lircsend(drvthis);
	}
}

/**
 * Send any queued IR samples to LIRC
 * \param drvthis  Pointer to driver structure.
 */
static void
picolcd_lircsend(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int len = p->resptr - p->result;
	if (len > 0) {
#ifdef DEBUG
		debug(RPT_INFO, "picolcd: sending LIRCD %d samples", len / 2);
		{
			unsigned char *ptr = p->result;
			unsigned char *endptr = ptr + len;
			/* every 2 bytes become 5 bytes " xxxx" */
			char logbuf[sizeof(p->result) * 3];
			char *logptr = logbuf;
			while (ptr < endptr) {
				unsigned int val = *ptr++;
				val |= *ptr++ << 8;
				logptr += sprintf(logptr, " %04x", val);
			}
			debug(RPT_DEBUG, "picolcd: data:%s", logbuf);
		}
#endif
		if (sendto(p->lircsock, p->result, len, 0,
			   (struct sockaddr *)&(p->lircserver), sizeof(p->lircserver)) == -1) {
			/* Ignore not connected errors when lirc has gone away */
			if (errno != ECONNREFUSED) {
				report(RPT_WARNING, "picolcd: failed to send IR data, reason: %s", strerror(errno));
			}
		}
		else {
			debug(RPT_DEBUG, "picolcd: sent %d bytes to lirc(udp)", len);
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
static void
picolcd_send(USB_DEVICE_HANDLE * lcd, unsigned char *data, int size)
{
	if ((lcd == NULL) && (data == NULL))
		return;
#ifdef HAVE_LIBUSB_1_0
	int error = 0;
	int transferred = 0;
	unsigned int timeout = 1000;	/* milliseconds */
	error = libusb_interrupt_transfer(lcd, LIBUSB_ENDPOINT_OUT + 1, data, size, &transferred, timeout);
	if (error) {
		/* can't use report here */
		fprintf(stderr, "libusb_interrupt_transfer error %d, sent %d of %d bytes\n",
			error, transferred, size);
	}
#else
	usb_interrupt_write(lcd, USB_ENDPOINT_OUT + 1, (char *)data, size, 1000);
#endif
}


/**
 * Write function for 20x4 desktop displays.
 * \param lcd   pointer to device handle
 * \param row   Row to place the string at
 * \param col   ignored
 * \param data  pointer to NUL terminated string
 */
static void
picolcd_20x4_write(USB_DEVICE_HANDLE * lcd, const int row, const int col, const unsigned char *data)
{
	unsigned char packet[64] = {0x95, 0x01, 0x00, 0x01};
	unsigned char lineset[4][6] = {
		{0x94, 0x00, 0x01, 0x00, 0x64, 0x80},
		{0x94, 0x00, 0x01, 0x00, 0x64, 0xC0},
		{0x94, 0x00, 0x01, 0x00, 0x64, 0x94},
		{0x94, 0x00, 0x01, 0x00, 0x64, 0xD4}
	};
	int len = strlen((char *)data);

	/* Cut off at display width */
	if (len > 20)
		len = 20;

	/* Send command to select row */
	switch (row) {
	    case 0:
		picolcd_send(lcd, lineset[0], 6);
		break;
	    case 1:
		picolcd_send(lcd, lineset[1], 6);
		break;
	    case 2:
		picolcd_send(lcd, lineset[2], 6);
		break;
	    case 3:
		picolcd_send(lcd, lineset[3], 6);
		break;
	    default:
		picolcd_send(lcd, lineset[0], 6);
		break;
	}

	/* Fill in an send packet */
	packet[4] = len;
	memcpy(packet + 5, data, len);
	picolcd_send(lcd, packet, 5 + len);
}


/**
 * Write function for 20x2 OEM displays.
 * \param lcd   pointer to device handle
 * \param row   Row to place the string at
 * \param col   Column to place the string at
 * \param data  pointer to NUL terminated string
 */
static void
picolcd_20x2_write(USB_DEVICE_HANDLE * lcd, const int row, const int col, const unsigned char *data)
{
	unsigned char packet[64] = {0x98};
	int len = strlen((char *)data);

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
 * Custom character define function for 20x2 OEM displays.
 * \param drvthis  Pointer to driver structure
 * \param n        Index of custom character to update
 * \param dat      Pointer to array of pixel data
 */
static void
picolcd_20x2_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[10] = {0x9c};	/* define character */
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (dat == NULL)
		return;

	packet[1] = n;		/* Custom char to define. */

	for (row = 0; row < p->cellheight; row++) {
		packet[row + 2] = dat[row] & mask;
	}

	picolcd_send(p->lcd, packet, 10);
}


/**
 * Custom character define function for 20x4 desktop displays.
 * \param drvthis  Pointer to driver structure
 * \param n        Index of custom character to update
 * \param dat      Pointer to array of pixel data
 */
static void
picolcd_20x4_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (dat == NULL)
		return;

	unsigned char command[6] = {
		OUT_REPORT_CMD, 0x00, 0x01, 0x00, 0x64, 0x40 + 8 * n
	};			/* 0x94 */
	unsigned char data[13] = {
		OUT_REPORT_DATA, 0x01, 0x00, 0x01, 0x08,
		dat[0], dat[1], dat[2], dat[3],
		dat[4], dat[5], dat[6], dat[7]
	};			/* 0x95 */

	picolcd_send(p->lcd, command, 6);
	picolcd_send(p->lcd, data, 13);
}

#ifndef HAVE_LIBUSB_1_0
/**
 * Read a key or IR event from the display into one packet.
 * \param lcd      Pointer to device handle
 * \param packet   Pointer to packet structure which is filled with data
 *                 read from the display
 * \param timeout  Read timeout in ms
 */
static void
get_key_event(usb_dev_handle * lcd, lcd_packet * packet, int timeout)
{
	int ret;

	memset(packet->data, 0, 255);
	packet->type = 0;

	ret = usb_interrupt_read(lcd, USB_ENDPOINT_IN + 1, (char *)packet->data, PICOLCD_MAX_DATA_LEN, timeout);
	if (ret > 0) {
		/* Set packet type */
		switch (packet->data[0]) {
		    case IN_REPORT_KEY_STATE:
			packet->type = IN_REPORT_KEY_STATE;
			break;
		    case IN_REPORT_IR_DATA:
			packet->type = IN_REPORT_IR_DATA;
			break;
		    default:
			packet->type = 0;
		}
	}
}
#endif


/**
 * Set lights for individual keys.
 * \param lcd    Pointer to device handle
 * \param keys   Array indicating which key number to turn on
 * \param state  0 to turn all LEDs off, 1 to turn them on according to
 *               values set in 'keys' array
 */
static void
set_key_lights(USB_DEVICE_HANDLE * lcd, int keys[], int state)
{
	unsigned char packet[2] = {0x81};	/* set led */
	unsigned int leds = 0;
	int i;

	if (state) {
		/* Only LEDs we want on */
		for (i = 0; i < KEYPAD_LIGHTS; i++)
			if (keys[i])
				leds |= (1 << i);
			else
				leds &= ~(1 << i);
	}
	else {
		/* All LEDs off */
		leds = 0;
	}

	packet[1] = leds;
	picolcd_send(lcd, packet, 2);
}


#ifdef HAVE_LIBUSB_1_0
/**
 * Free the USB transfer data.
 *
 * \param drvthis   Pointer to driver structure
 */
static void
free_usb_transfers(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i;

	for (i = 0; i < USB_BUFFERS; i++) {
		if (p->input_transfer[i].transfer != NULL) {
			if (p->input_transfer[i].status == LIBUSB_SUCCESS) {
				/* Need to cancel transfer before it is freed */
				libusb_cancel_transfer(p->input_transfer[i].transfer);
				while (p->input_transfer[i].status != LIBUSB_TRANSFER_CANCELLED) {
					struct timeval timeout;
					/*
					 * Wait for the cancellation to complete, the
					 * call-back will then have freed the
					 * transfer.
					 */
					report(RPT_INFO, "%s: waiting for usb transfer %d to be cancelled", drvthis->name, i);
					timeout.tv_sec = 1;
					timeout.tv_usec = 0;
					libusb_handle_events_timeout(p->lib_ctx, &timeout);
				}
			}
			else {
				libusb_free_transfer(p->input_transfer[i].transfer);
				p->input_transfer[i].transfer = NULL;
			}
		}
	}
}

/**
 * Store key press and release events in a buffer ready for the get key function.
 * If the buffer is full key codes are discarded.
 *
 * \param drvthis   Pointer to driver structure
 * \param high_key  Highest numbered key pressed
 * \param low_key   The second key if pressed
 */
static void
key_buffer_put(Driver *drvthis, unsigned char high_key, unsigned char low_key)
{
	PrivateData *p = drvthis->private_data;
	int space;

	space = ((p->key_read_index > p->key_write_index) ? 0 : KEY_BUFFER_SIZE) + p->key_read_index - p->key_write_index;

	/* Store events if there is space or if it's a key-up event */
	if ((space > 1) || ((space == 1) && (high_key == 0) && (low_key == 0))) {
		debug(RPT_DEBUG, "%s: key_buffer put %d, %d @ %d",
		      drvthis->name, high_key, low_key, p->key_write_index);
		/* Store the keys */
		p->key_buffer[p->key_write_index].high_key = high_key;
		p->key_buffer[p->key_write_index].low_key = low_key;
		/* Advance write index, wrap around if necessary) */
		p->key_write_index++;
		if (p->key_write_index >= KEY_BUFFER_SIZE)
			p->key_write_index = 0;
	}
}

/**
 * Call-back for USB input. Either calls key_buffer_put to process key events
 * or ir_trancode to process events from IR receiver.
 *
 * \param transfer  Structure containing the USB data
 */
static void
usb_cb_input(struct libusb_transfer *transfer)
{
	static const char *status[] = {
		"COMPLETED", "ERROR", "TIMED_OUT", "CANCELLED", "STALL",
		"NO_DEVICE", "OVERFLOW"
	};
	UsbTransferData *p = (UsbTransferData *)transfer->user_data;
	Driver *drvthis = p->drvthis;
	PrivateData *p_data = drvthis->private_data;

	if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
		report(RPT_ERR, "%s: input transfer status: %s", drvthis->name, status[transfer->status]);
		p->status = transfer->status;
		libusb_free_transfer(transfer);
		p->transfer = NULL;
		return;
	}

	switch (transfer->buffer[0]) {
	    case IN_REPORT_KEY_STATE:
		debug(RPT_INFO, "%s: USB input call-back key", drvthis->name);
		key_buffer_put(drvthis, transfer->buffer[1], transfer->buffer[2]);
		break;
	    case IN_REPORT_IR_DATA:
		debug(RPT_INFO, "%s: USB input call-back IR length %i", drvthis->name, transfer->buffer[1]);
		if (p_data->IRenabled)
			ir_transcode(drvthis, &transfer->buffer[2], transfer->buffer[1]);
		break;
	    default:
		report(RPT_ERR, "%s: input transfer unexpected data %d", drvthis->name, transfer->buffer[0]);
		break;
	}

	/* Re-transmit the input request transfer */
	p->status = libusb_submit_transfer(transfer);
	if (p->status != LIBUSB_SUCCESS)
		report(RPT_ERR, "%s: input transfer submit status %d", drvthis->name, p->status);
}
#endif

/* EOF */
