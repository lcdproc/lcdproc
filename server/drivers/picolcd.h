/*-
 * picoLCD driver for LCDproc
 *
 * Copyright (c) 2007 NitroSecurity, Inc.
 * 		 2007-2008 Peter Marschall
 * 		 2007-2008 Mini-Box.com, Nicu Pavel <npavel@ituner.com>
 * 		 2008 Jack Cleaver
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 */

#ifndef PICOLCD_H
#define PCIOLCD_H

#ifdef HAVE_LIBUSB_1_0
# include <libusb.h>
# define USB_DEVICE_HANDLE libusb_device_handle
#else
# include <usb.h>
# define USB_DEVICE_HANDLE usb_dev_handle
#endif

/* 12 keys plus a 0 placeholder */
#define KEYPAD_MAX		13
#define KEYPAD_LIGHTS		8
#define KEYPAD_LABEL_MAX	25

#define IN_REPORT_KEY_STATE	0x11
#define IN_REPORT_IR_DATA	0x21

#define OUT_REPORT_CMD		0x94
#define OUT_REPORT_DATA		0x95

#define PICOLCD_MAX_DATA_LEN	24

#define DEFAULT_LIRCPORT	8765
#define DEFAULT_LIRC_TIME_us	0	/* false */
#define DEFAULT_FLUSH_THRESHOLD 8000 /* microseconds */
#define DEFAULT_CONTRAST	1000 /* Full */
#define DEFAULT_BRIGHTNESS	1000 /* Full */
#define DEFAULT_OFFBRIGHTNESS	0    /* Off */
#define DEFAULT_BACKLIGHT	1    /* On */
#define DEFAULT_KEYLIGHTS	1    /* On */
#define DEFAULT_LINKLIGHTS	1    /* On */
#define DEFAULT_TIMEOUT		500  /* Half second */
#define DEFAULT_REPEAT_DELAY 300	/* milliseconds */
#define DEFAULT_REPEAT_INTERVAL 200	/* milliseconds */


typedef struct _lcd_packet {
	unsigned char data[255];
	unsigned int type;
} lcd_packet;

typedef struct _picolcd_device {
	char *device_name;          /* Device name */
	char *description;          /* Device description */
	unsigned char initseq[PICOLCD_MAX_DATA_LEN];     /* init sequence */
	char *keymap[KEYPAD_MAX];   /* key pad button names */
	unsigned int vendor_id;     /* vendor id for detection */
	unsigned int device_id;     /* device id for detection */
	int bklight_max;            /* maximum value for 'backlight on' */
	int bklight_min;            /* maximum value for 'backlight off' */
	int contrast_max;           /* contrast maximum value */
	int contrast_min;           /* minimum contrast value */
	int width;                  /* width of lcd screen */
	int height;                 /* height of lcd screen */
	/* Pointer to function that writes data to the LCD format */
	void (*write) (USB_DEVICE_HANDLE *lcd, const int row, const int col, const unsigned char *data);
	/* Pointer to function that defines a custom character */
	void (*cchar) (Driver *drvthis, int n, unsigned char *dat);
} picolcd_device;


MODULE_EXPORT int  picoLCD_init(Driver *drvthis);
MODULE_EXPORT void picoLCD_close(Driver *drvthis);
MODULE_EXPORT int  picoLCD_width(Driver *drvthis);
MODULE_EXPORT int  picoLCD_height(Driver *drvthis);
MODULE_EXPORT void picoLCD_clear(Driver *drvthis);
MODULE_EXPORT void picoLCD_flush(Driver *drvthis);
MODULE_EXPORT void picoLCD_string(Driver *drvthis, int x, int y, unsigned char string[]);
MODULE_EXPORT void picoLCD_chr(Driver *drvthis, int x, int y, unsigned char c);
MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis);

MODULE_EXPORT int picoLCD_get_free_chars (Driver *drvthis);
MODULE_EXPORT void picoLCD_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT void picoLCD_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  picoLCD_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void picoLCD_cursor(Driver *drvthis, int x, int y, int type);

MODULE_EXPORT int  picoLCD_get_contrast(Driver *drvthis);
MODULE_EXPORT void picoLCD_set_contrast(Driver *drvthis, int promille);
MODULE_EXPORT int  picoLCD_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void picoLCD_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int state);
MODULE_EXPORT void picoLCD_output(Driver *drvthis, int state);

MODULE_EXPORT char *picoLCD_get_info(Driver *drvthis);

#endif
