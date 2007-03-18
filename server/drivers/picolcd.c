/*
 * picoLCD driver for lcdPROC
 *
 * (c) 2007 NitroSecurity, Inc.
 * Written by Gatewood Green <woody@nitrosecurity.com> or <woody@linif.org>
 * (c) 2007 Peter Marschall - adapted coding style and reporting to LCDproc
 *
 * License: GPL (same as usblcd and lcdPROC)
 *
 * picoLCD: http://www.mini-box.com/picoLCD-20x2-OEM  
 * Can be purchased separately or preinstalled in units such as the 
 * M300 http://www.mini-box.com/Mini-Box-M300-LCD
 *
 * This driver (key lables and arrangement) is based on the M300 implementation 
 * of the picoLCD
 *
 * The picoLCD is usb connected and is driven (currently) via userspace 
 * libraries using the Mini-box.com usblcd library (not to be confused with the 
 * Linux usblcd module which does NOT support this device).  The usblcd library 
 * rides atop libusb and libhid (both of which are required for this driver to 
 * operate).
 *
 *   libusb: http://libusb.sf.net
 *   libhid: http://libhid.alioth.debian.org
 *   usblcd: http://www.mini-box.com/picoLCD-20x2-OEM
 * 
 * The usblcd library is very haphazardly written and directly writes to 
 * stdout and stderr instead of returning the result for most functions 
 * (including read_events).  Eventually it would be a good idea to eliminate 
 * the need for usblcd and drive the hardware via libusb and libhid directly.  
 * Such a conversion has the opportunity to provide meaningful return values 
 * for all fucntions (instead of stab and hope) and allow for use of multiple 
 * picoLCD devices.
 *
 * Due to the way libusblcd's read_events prints keys to stderr instead of 
 * returning a struct or some such, you will find my own get_key_events below.
 *
 * ### WARNING ###: libusblcd.so sets a handler for SIGTERM.  Because most 
 * applications would set up their signal handling early on (before calling 
 * new_usblcd_operations()), this can result in a condition that will prevent
 * a handler your application installed from executing.  If your handler was 
 * responsible for cleaning up logs, syncing, etc, it can result in lost data. 
 * 
 */

/* lcdPROC includes */
#include "lcd.h"
#include "picolcd.h"

/* Debug mode: un-comment to turn on debugging messages in the server */
/* #define DEBUG 1 */

#include "report.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* These three includes are the Mini-box.com libusblcd (usblcd) and company. */
#include <usblcd.h>
#include <widgets.h>
#include <usblcd_util.h>

/* Various odds and ends */
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/* 12 keys plus a 0 placeholder */
#define KEYPAD_MAX 13
#define KEYPAD_LIGHTS 6

#define DEFAULT_CONTRAST  1000 /* Full */
#define DEFAULT_BACKLIGHT 1    /* On */
#define DEFAULT_KEYLIGHTS 1    /* On */
#define DEFAULT_TIMEOUT   500  /* Half second */


/* PrivateData struct */
typedef struct pd {
	usblcd_operations *lcd; // Reference to the LCD instance
	int  width;
	int  height;
	int  key_timeout;
	int  contrast;
	int  backlight;
	int  keylights;
	int  key_light[KEYPAD_LIGHTS];
	char *key_matrix[KEYPAD_MAX];
	char *info;
	unsigned char *framebuf;
	unsigned char *lstframe;
} PrivateData;

static char * keymap[KEYPAD_MAX] = {
	NULL,
	"Plus",
	"Minus",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"Left",
	"Right",
	"Up",
	"Down",
	"Enter"
};

/* Private function definitions */
void get_key_event  (usblcd_operations *self, lcd_packet *packet, int timeout);
void set_key_lights (usblcd_operations *self, int keys[], int state);

/* lcd_logical_driver Variables */
MODULE_EXPORT char *api_version       = API_VERSION;
MODULE_EXPORT int  stay_in_foreground = 0;
MODULE_EXPORT int  supports_multiple  = 1;
MODULE_EXPORT char *symbol_prefix     = "picoLCD_";

/* lcd_logical_driver Manditory functions */
MODULE_EXPORT int  picoLCD_init(Driver *drvthis) {
	PrivateData *pd;
	int x;

	pd = (PrivateData *) malloc(sizeof(PrivateData));

	if (! pd)
		return -1;

	if (drvthis->store_private_ptr(drvthis, pd))
		return -1;

	pd->lcd = new_usblcd_operations();
	pd->lcd->init(pd->lcd);
	pd->width  = 20; /* hard coded (mfg spec) */
	pd->height = 2;  /* hard coded (mfg spec) */
	pd->info = "picoLCD: Supports the LCD as installed on the M300 (http://www.mini-box.com/Mini-Box-M300-LCD) ";

	for (x = 0; x < KEYPAD_LIGHTS; x++)
		pd->key_light[x] = 1; /* individual lights on */

	pd->contrast     = drvthis->config_get_int( drvthis->name, "Contrast",   0, DEFAULT_CONTRAST );
	pd->backlight    = drvthis->config_get_bool(drvthis->name, "BackLight",  0, DEFAULT_BACKLIGHT);
	pd->keylights    = drvthis->config_get_bool(drvthis->name, "KeyLights",  0, DEFAULT_KEYLIGHTS); /* key lights with LCD Backlight? */
	pd->key_timeout  = drvthis->config_get_int( drvthis->name, "KeyTimeout", 0, DEFAULT_TIMEOUT  );

	/* These allow individual lights to be disabled */
	pd->key_light[0] = drvthis->config_get_bool(drvthis->name, "Key0Light",  0, 1); /* Directional PAD */
	pd->key_light[1] = drvthis->config_get_bool(drvthis->name, "Key1Light",  0, 1); /* F1 */
	pd->key_light[2] = drvthis->config_get_bool(drvthis->name, "Key2Light",  0, 1); /* F2 */
	pd->key_light[3] = drvthis->config_get_bool(drvthis->name, "Key3Light",  0, 1); /* F3 */
	pd->key_light[4] = drvthis->config_get_bool(drvthis->name, "Key4Light",  0, 1); /* F4 */
	pd->key_light[5] = drvthis->config_get_bool(drvthis->name, "Key5Light",  0, 1); /* F5 */

	for (x = 0; x < KEYPAD_MAX; x++)
		pd->key_matrix[x] = keymap[x];

	pd->framebuf = (unsigned char *) malloc(pd->width * pd->height + 1);
	if (pd->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuf.\n", __FUNCTION__);
		return -1;
	}
	memset(pd->framebuf, ' ', pd->width * pd->height);
	pd->framebuf[pd->width * pd->height] = 0;

	pd->lstframe = (unsigned char *) malloc(pd->width * pd->height + 1);
	if (pd->lstframe == NULL) {
		report(RPT_ERR, "%s: unable to create lstframe.\n", __FUNCTION__);
		return -1;
	}
	memset(pd->lstframe, ' ', pd->width * pd->height);
	pd->lstframe[pd->width * pd->height] = 0;

	if (pd->backlight)
		picoLCD_backlight(drvthis, 1);
		if (! pd->keylights)
			set_key_lights(pd->lcd, pd->key_light, 0);
	else
		picoLCD_backlight(drvthis, 0);

	picoLCD_set_contrast(drvthis, pd->contrast);

	report(RPT_INFO, "picolcd: init complete\n", __FUNCTION__);

	return 0;
}

MODULE_EXPORT void picoLCD_close(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

	pd->lcd->close(pd->lcd); /* This actually does not do anything in usblcd (yet?) */

	debug(RPT_DEBUG, "picolcd: close complete\n");
}

/* lcd_logical_driver Essential output functions */
MODULE_EXPORT int  picoLCD_width(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

	return pd->width;
}

MODULE_EXPORT int  picoLCD_height(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

	return pd->height;
}

MODULE_EXPORT void picoLCD_clear(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

	memset(pd->framebuf, ' ', pd->width * pd->height);

	debug(RPT_DEBUG, "picolcd: clear complete\n");
}

MODULE_EXPORT void picoLCD_flush(Driver *drvthis) {
	PrivateData   *pd = drvthis->private_data;
	unsigned char *fb = pd->framebuf;
	unsigned char *lf = pd->lstframe;
	static char   text[48];
	int           i, line, offset;

	debug(RPT_DEBUG, "picolcd: flush started\n");

	for (line = 0; line < pd->height; line++) { 
		memset(text, 0, 48); 
		offset = line * pd->width;
		fb     = pd->framebuf + offset;
		lf     = pd->lstframe + offset;

		for (i = 0; i < pd->width; i++) {
			if (*fb++ != *lf++) {
				strncpy(text, pd->framebuf + offset, pd->width);
				pd->lcd->settext(pd->lcd, line, 0, text);
				memcpy(pd->lstframe + offset, pd->framebuf + offset, pd->width);

				debug(RPT_DEBUG, "picolcd: flush wrote line %d (%s)\n", line + 1, text);

				break;
			}
		}
	}

	debug(RPT_DEBUG, "picolcd: flush complete\n\t(%s)\n\t(%s)\n", pd->framebuf, pd->lstframe);
}

MODULE_EXPORT void picoLCD_string(Driver *drvthis, int x, int y, char *str) {
	PrivateData *pd = drvthis->private_data;
	char *dest;
	int  len;

	debug(RPT_DEBUG, "picolcd: string start (%s)\n", str);

	if (y < 1 || y > pd->height)
		return;
	if (x < 1 || x > pd->width)
		return;

	len = strlen(str);
	if (len + x > pd->width) {
		debug(RPT_DEBUG, "picolcd: string overlength (>%d). Start: %d Length: %d (%s)\n", pd->width, x, len ,str);

		len = pd->width - x; /* Copy what we can */
	}

	x--; y--; /* Convert 1-based to 0-based */
	dest = pd->framebuf + (y * pd->width + x);
	memcpy(dest, str, len * sizeof(char));

	debug(RPT_DEBUG, "picolcd: string complete (%s)\n", str);
}

MODULE_EXPORT void picoLCD_chr(Driver *drvthis, int x, int y, char chr) {
	PrivateData *pd = drvthis->private_data;
	char *dest;

	debug(RPT_DEBUG, "picolcd: chr start (%c)\n", chr);

	if (y < 1 || y > pd->height)
		return;
	if (x < 1 || x > pd->width)
		return;

	x--; y--; /* Convert 1-based to 0-based */
	dest = pd->framebuf + (y * pd->width + x);
	memcpy(dest, &chr, sizeof(char));

	debug(RPT_DEBUG, "picolcd: chr complete (%c)\n", chr);
}

/* lcd_logical_driver Essential input functions */

MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;
	lcd_packet *keydata;
	char *keystr;
	int  keys_read = 0;
	int  key_pass  = 0;
	int  two_keys  = 0;

	debug(RPT_DEBUG, "picolcd: get_key start (timeout %d)\n", pd->key_timeout);

	keydata = malloc(sizeof(lcd_packet));

	while (! keys_read) {
		get_key_event(pd->lcd, keydata, pd->key_timeout);
		debug(RPT_DEBUG, "picolcd: get_key got an event\n");

		if (keydata->type == IN_REPORT_KEY_STATE) {
			if (! keydata->data[1] && key_pass) {
				debug(RPT_DEBUG, "picolcd: get_key got all clear\n");
				/* Got a <0, 0> key-up event after reading a valid key press event */
				keys_read++; /* All clear */
			} else if (! keydata->data[2] && ! two_keys) {
				debug(RPT_DEBUG, "picolcd: get_key got one key\n");
				/* We got one key (but not after a two key event and before and all clear) */
				keystr = pd->key_matrix[keydata->data[1]];
			} else {
				/* We got two keys */
				debug(RPT_DEBUG, "picolcd: get_key got two keys\n");
				two_keys++;
				sprintf(keystr, "%s+%s", pd->key_matrix[keydata->data[1]], pd->key_matrix[keydata->data[2]]);
			}

			key_pass++; /* This hack allows us to deal with receiving left over <0,0> first */
		} else {
			debug(RPT_DEBUG, "picolcd: get_key got non-key data or timeout\n");
			/* We got IR or otherwise bad data */
			return NULL;
		}

	}

   free(keydata);

	debug(RPT_DEBUG, "picolcd: get_key complete (%s)\n", keystr);

	if (! strlen(keystr))
		return NULL;

	return keystr;

/* 
 * Due to how key events are reported, we need to keep reading key presses 
 *	until we get the all clear (all keys up) event.
 * 
 * Key events come back in such a way to report up to two simultanious keys 
 * pressed.  The highest numbered key always comes back as the first key and 
 * the lower numbered key follows.  If only one key was pressed, the second 
 * key is 0.  I will refer to a key event as: <high key, low key>.
 *
 * Key ID numbers:
 *        0 = (no key)
 *        1 = + (plus)
 *        2 = - (minus)
 *        3 = F1
 *        4 = F2
 *        5 = F3
 *        6 = F4
 *        7 = F5
 *        8 = Left
 *        9 = Right
 *       10 = Up
 *       11 = Down
 *       12 = Enter
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
 */
	
}

/* lcd_logical_driver Extended output functions */

/* lcd_logical_driver User-defined character functions */

/* lcd_logical_driver Hardware functions */
/*MODULE_EXPORT int picoLCD_get_contrast(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

}*/

MODULE_EXPORT int  picoLCD_set_contrast(Driver *drvthis, int promille) {
	PrivateData *pd = drvthis->private_data;
	int inv; /* The hardware seems to go dark on higher values, so we turn it around */

	if (promille <= 1000 && promille > 0) {
		inv = 1000 - promille;
		pd->lcd->contrast(pd->lcd, (int) (inv / 1000 * 40));
		return 0;
	} else if (promille > 1000) {
		pd->lcd->contrast(pd->lcd,  0);
		return 0;
	} else if (promille <= 0) {
		pd->lcd->contrast(pd->lcd,  40);
		return 0;
	} else {
		return -1;
	}
}

/*MODULE_EXPORT int picoLCD_get_brightness(Driver *drvthis, int state) {
	PrivateData *pd = drvthis->private_data;

}*/

/*MODULE_EXPORT int  picoLCD_set_brightness(Driver *drvthis, int state, int promille) {
	PrivateData *pd = drvthis->private_data;

}*/

MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int state) {
	PrivateData *pd = drvthis->private_data;

	if (state == 0) {
		pd->lcd->backlight(pd->lcd, state);
		set_key_lights(pd->lcd, pd->key_light, state);
		return;
	}

	if (state == 1) {
		pd->lcd->backlight(pd->lcd, state);
		if (pd->keylights)
			set_key_lights(pd->lcd, pd->key_light, state);
		return;
	}

	return;
}

/*MODULE_EXPORT int  picoLCD_output(Driver *drvthis, int state) {
	PrivateData *pd = drvthis->private_data;

}*/

/* lcd_logical_driver Informational functions */
MODULE_EXPORT char *picoLCD_get_info(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;

	return pd->info;
}

/* Private functions */
void get_key_event (usblcd_operations *self, lcd_packet *packet, int timeout) {
	int ret;

	memset(packet->data, 0, 255);
	packet->type = 0;
	ret = usb_interrupt_read(self->hid->hiddev->handle, USB_ENDPOINT_IN + 1, packet->data, _USBLCD_MAX_DATA_LEN, timeout);
	if (ret > 0) {
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

void set_key_lights (usblcd_operations *self, int keys[], int state) {
	if (state) {
		/* Only LEDs we want on */
		if (keys[0])
			self->setled(self, 0, 1);
		if (keys[1])
			self->setled(self, 1, 1);
		if (keys[2])
			self->setled(self, 2, 1);
		if (keys[3])
			self->setled(self, 3, 1);
		if (keys[4])
			self->setled(self, 4, 1);
		if (keys[5])
			self->setled(self, 5, 1);
	} else {
		/* All LEDs off */
		self->setled(self, 0, 0);
		self->setled(self, 1, 0);
		self->setled(self, 2, 0);
		self->setled(self, 3, 0);
		self->setled(self, 4, 0);
		self->setled(self, 5, 0);
	}
}
