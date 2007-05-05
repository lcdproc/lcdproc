/*
 * Driver for picoLCD graphical displays
 * Header file
 *
 * (c) 2007 NitroSecurity, Inc.
 * Written by Gatewood Green <woody@nitrosecurity.com> or <woody@linif.org>
 * (c) 2007 Mini-Box.com.
 * <npavel@ituner.com> removed libusblcd dependency
 *
 * License: GPL (same as usblcd and lcdPROC)
 *
 * picoLCD: http://www.mini-box.com/picoLCD-20x2-OEM  
 * Can be purchased separately or preinstalled in units such as the 
 * M300 http://www.mini-box.com/Mini-Box-M300-LCD
 *
 * See picolcd.c for full details
 *
 */

#ifndef PICOLCD_H
#define PCIOLCD_H

#include "lcd.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#define picoLCD_VENDOR  0x04d8
#define picoLCD_DEVICE  0x0002

#define picoLCD_MAX_LEDS 6

#define IN_REPORT_KEY_STATE	0x11
#define IN_REPORT_IR_DATA	0x21

#define PICOLCD_MAX_DATA_LEN	24


typedef struct _lcd_packet lcd_packet;
struct _lcd_packet {
	unsigned char data[255];
	unsigned int type;
};

MODULE_EXPORT int  picoLCD_init(Driver *drvthis);
MODULE_EXPORT void picoLCD_close(Driver *drvthis);
MODULE_EXPORT int  picoLCD_width(Driver *drvthis);
MODULE_EXPORT int  picoLCD_height(Driver *drvthis);
MODULE_EXPORT void picoLCD_clear(Driver *drvthis);
MODULE_EXPORT void picoLCD_flush(Driver *drvthis);
MODULE_EXPORT void picoLCD_string(Driver *drvthis, int x, int y, unsigned char *str);
MODULE_EXPORT void picoLCD_chr(Driver *drvthis, int x, int y, unsigned char c);
MODULE_EXPORT void picoLCD_set_char (Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT char *picoLCD_get_key(Driver *drvthis);

MODULE_EXPORT void picoLCD_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void picoLCD_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int picoLCD_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void picoLCD_cursor(Driver *drvthis, int x, int y, int type);

//MODULE_EXPORT int  picoLCD_get_contrast(Driver *drvthis);
MODULE_EXPORT int  picoLCD_set_contrast(Driver *drvthis, int promille);
//MODULE_EXPORT int  picoLCD_get_brightness (Driver *drvthis, int state);
//MODULE_EXPORT int  picoLCD_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void picoLCD_backlight(Driver *drvthis, int promille);
//MODULE_EXPORT void picoLCD_output(Driver *drvthis, int state);

MODULE_EXPORT char *picoLCD_get_info(Driver *drvthis);

#endif
