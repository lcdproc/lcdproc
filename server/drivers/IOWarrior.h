/* This is the LCDproc driver for IO-Warrior devices (http://www.codemercs.de)

     Copyright (C) 2004, Peter Marschall <peter@adpm.de>

   based on GPL'ed code:

   * IOWarrior LCD routines
       Copyright (c) 2004  Christian Vogelgsang <chris@lallafa.de>

   * misc. files from LCDproc source tree

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */


#ifndef IOWARRIOR_H
#define IOWARRIOR_H

#define DEFAULT_SERIALNO	""
#define DEFAULT_SIZE		"20x4"
#define DEFAULT_BACKLIGHT	1
#define DEFAULT_BRIGHTNESS	200

/* define some IOW constants */
#define iowVendor		0x07c0   /* CodeMercs */
#define iowProd40		0x1500   /* IOW40 */
#define iowProd24		0x1501   /* IOW24 */
#define iowProd56		0x1503   /* IOW56 */
#define iowProd24PVa		0x1511   /* IOW24 PowerVampire */
#define iowProd24PVb		0x1512   /* IOW24 PowerVampire */
#define iowTimeout		1000

#define USB_REQ_SET_REPORT	0x09
#define USB_REQ_GET_REPORT	0x01

#define IOW_OK			0
#define IOW_ERROR		-1

#define IOWLCD_BUSY		0x80
#define IOWLCD_ADDR_MASK	0x7F

/* IOW56 uses a different USB request size */
#define IOWLCD_SIZE		((p->productID == 0x1503) ? 64 : 8)

/* IOWarriors drive HD44780 cmpatible displays that have these cells: */
#define CELLWIDTH	LCD_DEFAULT_CELLWIDTH
#define CELLHEIGHT	LCD_DEFAULT_CELLHEIGHT


/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */


typedef struct cgram_cache {
  unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
  int clean;
} CGram;


/** private data for the \c IOWarrior driver */
typedef struct IOWarrior_private_data {
  char manufacturer[LCD_MAX_WIDTH+1];
  char product[LCD_MAX_WIDTH+1];
  char serial[LCD_MAX_WIDTH+1];
  int productID;

  usb_dev_handle *udh;

  int width, height;
  int cellwidth, cellheight;

  /* The framebuffer */
  unsigned char *framebuf;

  /* last LCD contents (for incremental updates) */
  unsigned char *backingstore;

  /* definable characters */
  CGram cc[NUM_CCs];
  CGmode ccmode;

  /* output LED(s) state */
  unsigned int output_mask;
  int output_state;

  /* lastline controls the use of the last line, if pixel addressable (true, default) or */
  /* underline effect (false). To avoid the underline effect in the latter case, the last */
  /* line is always zeroed for whatever redefined character */
  char lastline;

  /* extended mode for some half-compatible modules */
  int ext_mode;

  int brightness;
  int backlight;

  char info[255];
} PrivateData;


/* API: variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0; /* For testing only */
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "IOWarrior_";

/* API: functions for the server core */
MODULE_EXPORT int  IOWarrior_init(Driver *drvthis);
MODULE_EXPORT void IOWarrior_close(Driver *drvthis);
MODULE_EXPORT int  IOWarrior_width(Driver *drvthis);
MODULE_EXPORT int  IOWarrior_height(Driver *drvthis);
MODULE_EXPORT int  IOWarrior_cellheight(Driver *drvthis);
MODULE_EXPORT int  IOWarrior_cellwidth(Driver *drvthis);
MODULE_EXPORT void IOWarrior_clear(Driver *drvthis);
MODULE_EXPORT void IOWarrior_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void IOWarrior_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void IOWarrior_flush(Driver *drvthis);
MODULE_EXPORT void IOWarrior_backlight(Driver *drvthis, int on);
MODULE_EXPORT void IOWarrior_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void IOWarrior_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void IOWarrior_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  IOWarrior_get_free_chars(Driver *drvthis);
MODULE_EXPORT void IOWarrior_set_char(Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int  IOWarrior_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void IOWarrior_output(Driver *drvthis, int state);
MODULE_EXPORT const char *IOWarrior_get_info(Driver *drvthis);

#endif	/* IOWARRIOR_H */

