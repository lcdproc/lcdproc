/*
 * picoLCD driver for lcdPROC
 *
 * (c) 2007 NitroSecurity, Inc.
 * Written by Gatewood Green <woody@nitrosecurity.com> or <woody@linif.org>
 * (c) 2007 Peter Marschall - adapted coding style and reporting to LCDproc
 * (c) 2007 Mini-Box.com, Nicu Pavel <npavel@ituner.com> 
 *     - removed libusblcd and hid dependency
 *     - added vbar, hbar, custom char, bignum support
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
 * using libusb library.
 *
 *   libusb: http://libusb.sf.net
 * 
 */

/* lcdPROC includes */
#include "lcd.h"
#include "picolcd.h"
#include <usb.h>
/* Debug mode: un-comment to turn on debugging messages in the server */
/* #define DEBUG 1 */

#include "report.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

#define NUM_CCs         8 /* max. number of custom characters */ 

typedef enum {
	standard,	/* only char 0 is used for heartbeat */
	vbar,		/* vertical bars */
	hbar,		/* horizontal bars */
	custom,		/* custom settings */
	bignum,		/* big numbers */
	bigchar		/* big characters */
} CGmode;


/* PrivateData struct */
typedef struct pd {
	usb_dev_handle *lcd;
	int  width;
	int  height;
	int  cellwidth;
	int  cellheight;
	int  key_timeout;
	int  contrast;
	int  backlight;
	int  keylights;
	int  key_light[KEYPAD_LIGHTS];
	/* defineable characters */
	CGmode ccmode;
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
static usb_dev_handle *picolcd_open(void);
static void picolcd_send(usb_dev_handle *lcd, unsigned char *data, int size);
static void picolcd_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data);
static void get_key_event  (usb_dev_handle *lcd, lcd_packet *packet, int timeout);
static void set_key_lights (usb_dev_handle *lcd, int keys[], int state);

/* lcd_logical_driver Variables */
MODULE_EXPORT char *api_version       = API_VERSION;
MODULE_EXPORT int  stay_in_foreground = 0;
MODULE_EXPORT int  supports_multiple  = 1;
MODULE_EXPORT char *symbol_prefix     = "picoLCD_";

/* lcd_logical_driver mandatory functions */
MODULE_EXPORT int  picoLCD_init(Driver *drvthis) {
	PrivateData *pd;
	int x;

	pd = (PrivateData *) malloc(sizeof(PrivateData));

	if (! pd)
		return -1;

	if (drvthis->store_private_ptr(drvthis, pd))
		return -1;

	pd->lcd = picolcd_open();
	pd->width  = 20; /* hard coded (mfg spec) */
	pd->height = 2;  /* hard coded (mfg spec) */
	pd->info = "picoLCD: Supports the LCD as installed on the M300 (http://www.mini-box.com/Mini-Box-M300-LCD) ";
	pd->cellwidth = LCD_DEFAULT_CELLWIDTH;
	pd->cellheight = LCD_DEFAULT_CELLHEIGHT;
	pd->ccmode = standard;

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

	usb_release_interface(pd->lcd, 0);
	usb_close(pd->lcd);

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
	pd->ccmode = standard;

	debug(RPT_DEBUG, "picolcd: clear complete\n");
}

MODULE_EXPORT void picoLCD_flush(Driver *drvthis) {
	PrivateData   *pd = drvthis->private_data;
	unsigned char *fb = pd->framebuf;
	unsigned char *lf = pd->lstframe;
	static unsigned char   text[48];
	int           i, line, offset;

	debug(RPT_DEBUG, "picolcd: flush started\n");

	for (line = 0; line < pd->height; line++) { 
		memset(text, 0, 48); 
		offset = line * pd->width;
		fb     = pd->framebuf + offset;
		lf     = pd->lstframe + offset;

		for (i = 0; i < pd->width; i++) {
			if (*fb++ != *lf++) {
				strncpy((char *)text, (char *)pd->framebuf + offset, pd->width);
				picolcd_write(pd->lcd, line, 0, text);
				memcpy(pd->lstframe + offset, pd->framebuf + offset, pd->width);

				debug(RPT_DEBUG, "picolcd: flush wrote line %d (%s)\n", line + 1, text);

				break;
			}
		}
	}

	debug(RPT_DEBUG, "picolcd: flush complete\n\t(%s)\n\t(%s)\n", pd->framebuf, pd->lstframe);
}

MODULE_EXPORT void picoLCD_string(Driver *drvthis, int x, int y, unsigned char *str) {
	PrivateData *pd = drvthis->private_data;
	unsigned char *dest;
	int  len;

	debug(RPT_DEBUG, "picolcd: string start (%s)\n", str);

	if (y < 1 || y > pd->height)
		return;
	if (x < 1 || x > pd->width)
		return;

	len = strlen((char *)str);
	if (len + x > pd->width) {
		debug(RPT_DEBUG, "picolcd: string overlength (>%d). Start: %d Length: %d (%s)\n", pd->width, x, len ,str);

		len = pd->width - x; /* Copy what we can */
	}

	x--; y--; /* Convert 1-based to 0-based */
	dest = pd->framebuf + (y * pd->width + x);
	memcpy(dest, str, len * sizeof(char));

	debug(RPT_DEBUG, "picolcd: string complete (%s)\n", str);
}

MODULE_EXPORT void picoLCD_chr(Driver *drvthis, int x, int y, unsigned char chr) {
	PrivateData *pd = drvthis->private_data;
	unsigned char *dest;
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


MODULE_EXPORT void picoLCD_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *pd = drvthis->private_data;
	unsigned char packet[10] = { 0x9c };   /* define character */ 
	unsigned char mask = (1 << pd->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	packet[1] = n;	/* Custom char to define. */

	for (row = 0; row < pd->cellheight; row++) {
		packet[row + 2] = dat[row] & mask;
	}
	
	picolcd_send(pd->lcd, packet, 10);
}

MODULE_EXPORT int picoLCD_get_free_chars (Driver *drvthis)
{
	return NUM_CCs;
}


MODULE_EXPORT void picoLCD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *pd = drvthis->private_data;

	if (pd->ccmode != vbar) {
		unsigned char vBar[pd->cellheight];
		int i;

		if (pd->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}
		pd->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < pd->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[pd->cellheight - i] = 0xFF;
			picoLCD_set_char(drvthis, i, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, pd->cellheight, 0);
}



MODULE_EXPORT void picoLCD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *pd = drvthis->private_data;

	if (pd->ccmode != hbar) {
		unsigned char hBar[pd->cellheight];
		int i;

		if (pd->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		pd->ccmode = hbar;

		memset(hBar, 0x00, sizeof(hBar));

		for (i = 1; i <= pd->cellwidth; i++) {
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (pd->cellwidth - i)) - 1), sizeof(hBar)-1);
			picoLCD_set_char(drvthis, i, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, pd->cellwidth, 0);
}

MODULE_EXPORT void picoLCD_num (Driver *drvthis, int x, int num)
{
	PrivateData *pd = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	if (pd->ccmode != bignum) {
		if (pd->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		pd->ccmode = bignum;

		do_init = 1;
	}

	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


MODULE_EXPORT int picoLCD_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *pd = drvthis->private_data;
	
	/* 8x5 icons each number represents one row in binary */

	static unsigned char heart_open[] = 
	    { 0x0, 0xa, 0x15, 0x11, 0x1b, 0xa, 0x4, 0x0 };
		 

	static unsigned char heart_filled[] = 
	    { 0x0, 0xa, 0x1f, 0x1f, 0x1f, 0xe, 0x4, 0x0 };
		 
	switch (icon) {
		case ICON_BLOCK_FILLED:
			picoLCD_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
		        pd->ccmode = custom;
			picoLCD_set_char(drvthis, 7, heart_filled);
			picoLCD_chr(drvthis, x, y, 7);
			break;
		case ICON_HEART_OPEN:
		        pd->ccmode = custom;
			picoLCD_set_char(drvthis, 7, heart_open);
			picoLCD_chr(drvthis, x, y, 7);
			break;
		case ICON_ARROW_LEFT:
			picoLCD_chr(drvthis, x, y, 127);
			break;
		case ICON_ARROW_RIGHT:
			picoLCD_chr(drvthis, x, y, 126);
			break;

		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}

/* lcd_logical_driver Essential input functions */

MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis) {
	PrivateData *pd = drvthis->private_data;
	lcd_packet *keydata;
	char *keystr = NULL;
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
	unsigned char packet[2] = { 0x92 }; /* set contrast id */
	
	if (promille <= 1000 && promille > 0) {
		inv = 1000 - promille;
		packet[1] =  inv / 1000 * 40;
	} else if (promille > 1000) {
		packet[1] = 0;
	} else if (promille <= 0) {
		packet[1] = 40;
	} else {
		return -1;
	}
	
	picolcd_send(pd->lcd, packet, 2);
	return 0;
	
	
}

/*MODULE_EXPORT int picoLCD_get_brightness(Driver *drvthis, int state) {
	PrivateData *pd = drvthis->private_data;

}*/

/*MODULE_EXPORT int  picoLCD_set_brightness(Driver *drvthis, int state, int promille) {
	PrivateData *pd = drvthis->private_data;

}*/

MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int state) {
	PrivateData *pd = drvthis->private_data;
	unsigned char packet[2] = { 0x91 }; /* set backlight id */
	
	if (state == 0) {
		packet[1] = state; 
		picolcd_send(pd->lcd, packet, 2);
		set_key_lights(pd->lcd, pd->key_light, state);
		return;
	}

	if (state == 1) {
		packet[1] = state;
		picolcd_send(pd->lcd, packet, 2);
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

static usb_dev_handle *picolcd_open(void)
{
    
	usb_dev_handle *lcd;
        struct usb_bus *busses, *bus;
	struct usb_device *dev;
        char driver[1024];
        int ret;
    
	lcd = NULL;

        debug(RPT_DEBUG, "picolcd: scanning for devices...\n");

	usb_init();
	usb_find_busses();
	usb_find_devices();
	busses = usb_get_busses();

	for (bus = busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == picoLCD_VENDOR) && (dev->descriptor.idProduct == picoLCD_DEVICE)) {
				debug(RPT_DEBUG, "Found picoLCD on bus %s device %s \n", bus->dirname, dev->filename);
				lcd = usb_open(dev);
				ret = usb_get_driver_np(lcd, 0, driver, sizeof(driver));
				if (ret == 0) {
					debug(RPT_DEBUG, "Interface 0 already claimed by '%s' attempting to detach driver...\n", driver);
					if (usb_detach_kernel_driver_np(lcd, 0) < 0) {
						debug(RPT_DEBUG, "Failed to detach '%s' driver !\n", driver);
						return NULL;
					}
				}

				usb_set_configuration(lcd, 1);
				usleep(100);

				if (usb_claim_interface(lcd, 0) < 0) {
					debug(RPT_DEBUG, "Failed to claim interface !\n");
					return NULL;
				}

				usb_set_altinterface(lcd, 0);
				return lcd;
			}
		}
	}
	
	debug(RPT_DEBUG, "Could not find a picoLCD !\n");
	return NULL;
}

static void picolcd_send(usb_dev_handle *lcd, unsigned char *data, int size)
{
        usb_interrupt_write(lcd, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}

static void picolcd_write(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data)
{
	unsigned char packet[64];
        int len, i;
    
        len = strlen((char *)data);
        if (len > 20) len = 20;

	packet[0] = 0x98;
        packet[1] = row;
        packet[2] = col;
        packet[3] = len;

	i = 4;
        while (len--) {
		packet[i++] = *data++;
        }

	picolcd_send(lcd, packet, i);
}


void get_key_event (usb_dev_handle *lcd, lcd_packet *packet, int timeout) {
	int ret;

	memset(packet->data, 0, 255);
	packet->type = 0;
	ret = usb_interrupt_read(lcd, USB_ENDPOINT_IN + 1, (char *)packet->data, PICOLCD_MAX_DATA_LEN, timeout);
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

void set_key_lights (usb_dev_handle *lcd, int keys[], int state) {

        unsigned char packet[2] = { 0x81 }; /* set led */
	unsigned int leds = 0; 
        int i;
    
	if (state) {
		/* Only LEDs we want on */
		for (i = 0; i < picoLCD_MAX_LEDS; i++) 
			if(keys[i]) 
				leds |= 1 << i;
			else 
				leds &= ~ (1 << i);
	} else {
		/* All LEDs off */
		leds = 0;
	}
	
	packet[1] = leds;
        picolcd_send(lcd, packet, 2);
}
