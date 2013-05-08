/** \file server/drivers/glcd-picolcdgfx.c
 * Driver for the picoLCD Graphics 256x64 from mini-box.com.  Based on lcd4linux
 * driver by Nicu Pavel.
 */

/*-
 * Copyright (c) 2009 Nicu Pavel <napvel@mini-box.com>
 * Copyright (c) 2012 Scott Meahrg <ssmeharg@gmail.com>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <usb.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"

#define picoLCDGfx_VENDOR  0x04d8
#define picoLCDGfx_DEVICE  0xc002

/* picoLCD Graphics Commands */
#define PICOLCDGFX_IN_KEY_STATE		0x11

#define PICOLCDGFX_OUT_GPO		0x81

#define PICOLCDGFX_OUT_BACKLIGHT	0x91
#define PICOLCDGFX_OUT_CONTRAST		0x92
#define PICOLCDGFX_OUT_INIT_DISPLAY	0x93
#define PICOLCDGFX_OUT_CMD		0x94
#define PICOLCDGFX_OUT_DATA		0x95
#define PICOLCDGFX_OUT_CMD_DATA		0x96

#define PICOLCDGFX_MAX_DATA_LEN 24

#define PICOLCDGFX_DEF_KEYTIMEOUT	125
#define PICOLCDGFX_DEF_INVERTED		0

/* Display properties */
#define PICOLCDGFX_WIDTH	256
#define PICOLCDGFX_HEIGHT	64

/** Private data for the picolcdgfx connection type */
typedef struct glcd_picolcdgfx_data {
	usb_dev_handle *lcd;
	unsigned char inverted;
	int keytimeout;

	unsigned char *backingstore;
} CT_picolcdgfx_data;

/* Prototypes */
void glcd_picolcdgfx_blit(PrivateData *p);
void glcd_picolcdgfx_close(PrivateData *p);
unsigned char glcd_picolcdgfx_pollkeys(PrivateData *p);
void glcd_picolcdgfx_set_backlight(PrivateData *p, int state);
void glcd_picolcdgfx_set_contrast(PrivateData *p, int value);

/* Local functions */
static int
picolcdgfx_read(usb_dev_handle * lcd, unsigned char *data, int size, int timeout)
{
	return usb_interrupt_read(lcd, USB_ENDPOINT_IN + 1, (char *) data, size, timeout);
}

static void
picolcdgfx_write(usb_dev_handle * lcd, unsigned char *data, int size)
{
	usb_interrupt_write(lcd, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}

/**
 * API: Initialize the connection type driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
int
glcd_picolcdgfx_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	CT_picolcdgfx_data *ct_data;

	struct usb_bus *busses, *bus;
	struct usb_device *dev;
	char driver[1024];
	char product[1024];
	char manufacturer[1024];
	char serialnumber[1024];
	int ret;

	report(RPT_INFO, "GLCD/picolcdgfx: intializing");

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd_picolcdgfx_blit;
	p->glcd_functions->close = glcd_picolcdgfx_close;
	p->glcd_functions->poll_keys = glcd_picolcdgfx_pollkeys;
	p->glcd_functions->set_backlight = glcd_picolcdgfx_set_backlight;
	p->glcd_functions->set_contrast = glcd_picolcdgfx_set_contrast;

	/* Allocate memory structures */
	ct_data = (CT_picolcdgfx_data *) calloc(1, sizeof(CT_picolcdgfx_data));
	if (ct_data == NULL) {
		report(RPT_ERR, "GLCD/picolcdgfx: error allocating connection data");
		return -1;
	}
	p->ct_data = ct_data;

	/* Fix display size to 256x64 */
	p->framebuf.layout = FB_TYPE_VPAGED;
	p->framebuf.px_width = PICOLCDGFX_WIDTH;
	p->framebuf.px_height = PICOLCDGFX_HEIGHT;

	/* Since the display is fixed to 256x64 we have to recalculate. */
	p->framebuf.size = (PICOLCDGFX_HEIGHT / 8) * PICOLCDGFX_WIDTH;

	ct_data->backingstore = malloc(p->framebuf.size);
	if (ct_data->backingstore == NULL) {
		report(RPT_ERR, "GLCD/picolcdgfx: unable to allocate backing store");
		return -1;
	}

	/* framebuf is initialized with 0x00 so initialize the backingstore with
	 * 0xFF so the first call to _blit will draw the entire screen.
	 * */
	memset(ct_data->backingstore, 0xFF, p->framebuf.size);

	/* Get key timeout */
	ct_data->keytimeout = drvthis->config_get_int(drvthis->name,
						      "picolcdgfx_KeyTimeout", 0,
						      PICOLCDGFX_DEF_KEYTIMEOUT);

	/* Get inverted option */
	if (drvthis->config_get_bool(drvthis->name, "picolcdgfx_Inverted", 0, PICOLCDGFX_DEF_INVERTED))
		ct_data->inverted = 0xFF;
	else
		ct_data->inverted = 0;

	ct_data->lcd = NULL;

	report(RPT_DEBUG, "GLCD/picolcdgfx: scanning for picoLCD 256x64...");

	usb_init();
	usb_find_busses();
	usb_find_devices();
	busses = usb_get_busses();

	for (bus = busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == picoLCDGfx_VENDOR) &&
			    (dev->descriptor.idProduct == picoLCDGfx_DEVICE)) {
				report(RPT_DEBUG,
				       "GLCD/picolcdgfx: found picoLCDGraphics on bus %s device %s",
				       bus->dirname, dev->filename);

				ct_data->lcd = usb_open(dev);

				ret = usb_get_driver_np(ct_data->lcd, 0, driver, sizeof(driver));

				if (ret == 0) {
					report(RPT_DEBUG,
					       "GLCD/picolcdgfx: interface 0 already claimed by '%s'",
					       driver);
					report(RPT_DEBUG,
					       "GLCD/picolcdgfx: attempting to detach driver...");
					if (usb_detach_kernel_driver_np(ct_data->lcd, 0) < 0) {
						report(RPT_ERR,
						       "GLCD/picolcdgfx: usb_detach_kernel_driver_np() failed!");
						return -1;
					}
				}

				usb_set_configuration(ct_data->lcd, 1);
				usleep(100);

				if (usb_claim_interface(ct_data->lcd, 0) < 0) {
					report(RPT_ERR,
					       "GLCD/picolcdgfx: usb_claim_interface() failed!");
					return -1;
				}

				usb_set_altinterface(ct_data->lcd, 0);
				usb_get_string_simple(ct_data->lcd, dev->descriptor.iProduct,
						      product, sizeof(product));
				usb_get_string_simple(ct_data->lcd, dev->descriptor.iManufacturer,
						      manufacturer, sizeof(manufacturer));
				usb_get_string_simple(ct_data->lcd, dev->descriptor.iSerialNumber,
						      serialnumber, sizeof(serialnumber));

				report(RPT_INFO,
				       "GLCD/picolcdgfx: Manufacturer='%s' Product='%s' SerialNumber='%s'",
				       manufacturer, product, serialnumber);

				debug(RPT_DEBUG, "GLCD/picolcdgfx: init() done");

				return 0;
			}
		}
	}

	report(RPT_ERR, "GLCD/picolcdgfx: could not find a picoLCDGraphics");
	return -1;
}

/**
 * API: Write the framebuffer to the display
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_picolcdgfx_blit(PrivateData *p)
{
	CT_picolcdgfx_data *ct_data = (CT_picolcdgfx_data *) p->ct_data;

	/* send command + data */
	unsigned char cmd3[64] = {PICOLCDGFX_OUT_CMD_DATA};
	/* send data only */
	unsigned char cmd4[64] = {PICOLCDGFX_OUT_DATA};

	int offset;
	int index;
	unsigned char cs, line;		/* controller and page */

	for (cs = 0; cs < 4; cs++) {
		unsigned char chipsel = (cs << 2);
		for (line = 0; line < 8; line++) {
			offset = line * PICOLCDGFX_WIDTH + cs * 64;
			if (memcmp((p->framebuf.data) + offset, (ct_data->backingstore) + offset, 64) == 0)
				continue;

			cmd3[0] = PICOLCDGFX_OUT_CMD_DATA;
			cmd3[1] = chipsel;
			cmd3[2] = 0x02;
			cmd3[3] = 0x00;
			cmd3[4] = 0x00;
			cmd3[5] = 0xb8 | line;
			cmd3[6] = 0x00;
			cmd3[7] = 0x00;
			cmd3[8] = 0x40;
			cmd3[9] = 0x00;
			cmd3[10] = 0x00;
			cmd3[11] = 32;

			cmd4[0] = PICOLCDGFX_OUT_DATA;
			cmd4[1] = chipsel | 0x01;
			cmd4[2] = 0x00;
			cmd4[3] = 0x00;
			cmd4[4] = 32;

			for (index = 0; index < 32; index++) {
				cmd3[12 + index] = *((p->framebuf.data) + offset + index) ^ ct_data->inverted;
			}

			for (index = 32; index < 64; index++) {
				cmd4[5 + (index - 32)] = *((p->framebuf.data) + offset + index) ^ ct_data->inverted;
			}

			picolcdgfx_write(ct_data->lcd, cmd3, 44);
			picolcdgfx_write(ct_data->lcd, cmd4, 37);
		}
	}

	memcpy(ct_data->backingstore, p->framebuf.data, p->framebuf.size);
}

/**
 * API: Release low-level resources.
 */
void
glcd_picolcdgfx_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_picolcdgfx_data *ct_data = (CT_picolcdgfx_data *) p->ct_data;

		if (ct_data->lcd != NULL) {
			usb_release_interface(ct_data->lcd, 0);
			usb_close(ct_data->lcd);
		}

		if (ct_data->backingstore != NULL)
			free(ct_data->backingstore);

		free(p->ct_data);
		p->ct_data = NULL;
	}
}


/**
 * API: Poll for any pressed keys. Converts the bitmap of keys pressed into a
 * scancode (1-6) for each pressed key.
 */
unsigned char
glcd_picolcdgfx_pollkeys(PrivateData *p)
{
	CT_picolcdgfx_data *ct_data = (CT_picolcdgfx_data *) p->ct_data;
	unsigned char rv = 0;

	int ret;
	unsigned char rx_packet[PICOLCDGFX_MAX_DATA_LEN];
	ret = picolcdgfx_read(ct_data->lcd, rx_packet, PICOLCDGFX_MAX_DATA_LEN,
			      ct_data->keytimeout);

	if ((ret > 0) && (rx_packet[0] == PICOLCDGFX_IN_KEY_STATE)) {
		switch (rx_packet[1]) {
		case 0x01:
			rv = 6;		/* Escape */
			break;
		case 0x02:
			rv = 3;		/* Left */
			break;
		case 0x05:
			rv = 1;		/* Up */
			break;
		case 0x06:
			rv = 5;		/* Enter */
			break;
		case 0x07:
			rv = 2;		/* Down */
			break;
		}
	}

	return rv;
}


/**
 * API: Set the backlight brightness.
 */
void
glcd_picolcdgfx_set_backlight(PrivateData *p, int state)
{
	CT_picolcdgfx_data *ct_data = (CT_picolcdgfx_data *) p->ct_data;
	int promille = (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
	unsigned char cmd[2];

	cmd[0] = PICOLCDGFX_OUT_BACKLIGHT;
	cmd[1] = (promille * 255 / 1000);

	picolcdgfx_write(ct_data->lcd, cmd, 2);
}


/**
 * API: Change LCD contrast.
 */
void
glcd_picolcdgfx_set_contrast(PrivateData *p, int value)
{
	CT_picolcdgfx_data *ct_data = (CT_picolcdgfx_data *) p->ct_data;
	unsigned char cmd[2] = {PICOLCDGFX_OUT_CONTRAST};

	/* I believe hardware contrast values are 200 to 255.  Where a higher
	 * contrast setting sends a lower value to the deivce. (S. Meharg) */
	unsigned char val = ((1000 - value) * (255 - 200)) / 1000 + 200;

	cmd[1] = val;
	picolcdgfx_write(ct_data->lcd, cmd, 2);
}
