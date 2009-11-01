/*
 * picoLCD driver for lcdPROC
 *
 * (c) 2007 NitroSecurity, Inc.
 * Written by Gatewood Green <woody@nitrosecurity.com> or <woody@linif.org>
 * (c) 2007-2008 Peter Marschall - adapted coding style and reporting to LCDproc
 * (c) 2007 Mini-Box.com, Nicu Pavel <npavel@ituner.com> 
 *     - removed libusblcd and hid dependency
 *     - added vbar, hbar, custom char, bignum support
 * (c) 2008 Jack Cleaver - add LIRC connection
 * (c) 2008 Mini-Box.com Nicu Pavel <npavel@mini-box.com>
 *      - Added support for 4x20 picoLCD
 * License: GPL (same as usblcd and lcdPROC)
 *
 * picoLCD: http://www.mini-box.com/picoLCD-20x2-OEM  
 * Can be purchased separately or preinstalled in units such as the 
 * M300 http://www.mini-box.com/Mini-Box-M300-LCD
 * picoLCD 4x20: http://www.mini-box.com/PicoLCD-4X20-Sideshow
 *
 * The picoLCD is usb connected and is driven (currently) via userspace 
 * using libusb library.
 *
 *   libusb: http://libusb.sf.net
 * 
 */

#ifndef PICOLCD_H
#define PCIOLCD_H

#include "lcd.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#include <usb.h>

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
#define DEFAULT_FLUSH_THRESHOLD_JIFFY 100 /* 6.1 millisec */


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
	int bklight_max;            /* maximum backlight value */
	int bklight_min;            /* minimum backlight value */
	int contrast_max;           /* contrast maximum value */
	int contrast_min;           /* minimum contrast value */
	int width;                  /* width of lcd screen */
	int height;                 /* height of lcd screen */
	/* Pointer to function that writes data to the LCD format */
	void (*write)(usb_dev_handle *lcd, const int row, const int col, const unsigned char *data); 
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
MODULE_EXPORT void picoLCD_set_char (Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis);

MODULE_EXPORT void picoLCD_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  picoLCD_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void picoLCD_cursor(Driver *drvthis, int x, int y, int type);

MODULE_EXPORT int  picoLCD_get_contrast(Driver *drvthis);
MODULE_EXPORT void picoLCD_set_contrast(Driver *drvthis, int promille);
//MODULE_EXPORT int  picoLCD_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void picoLCD_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int state);
//MODULE_EXPORT void picoLCD_output(Driver *drvthis, int state);

MODULE_EXPORT char *picoLCD_get_info(Driver *drvthis);

#endif
