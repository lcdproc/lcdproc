/* This is the LCDproc driver for Longan Nano device
 * (https://longan.sipeed.com/en/)

     Copyright (C) 2020, Fabien Marteau <mail@fabienm.eu>

   based on GPL'ed code:

   * IOWarrior
    Copyright (C) 2004, Peter Marschall <peter@adpm.de>
    Copyright (c) 2004  Christian Vogelgsang <chris@lallafa.de>

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


#ifndef LONGANNANO_H
#define LONGANNANO_H

#define DEFAULT_SERIALNO    ""
#define DEFAULT_SIZE        "20x4"
#define DEFAULT_BACKLIGHT    1
#define DEFAULT_BRIGHTNESS    200

/* define some LN constants */
#define lnVendor        0x07c0   /* CodeMercs */
#define lnProd40        0x1500   /* LN40 */
#define lnProd24        0x1501   /* LN24 */
#define lnProd56        0x1503   /* LN56 */
#define lnProd24PVa     0x1511   /* LN24 PowerVampire */
#define lnProd24PVb     0x1512   /* LN24 PowerVampire */
#define lnTimeout        1000

#define USB_REQ_SET_REPORT    0x09
#define USB_REQ_GET_REPORT    0x01

#define LN_OK            0
#define LN_ERROR        -1

#define LNLCD_BUSY        0x80
#define LNLCD_ADDR_MASK    0x7F

/* LN56 uses a different USB request size */
#define LNLCD_SIZE        ((p->productID == 0x1503) ? 64 : 8)

/* LonganNanos drive HD44780 cmpatible displays that have these cells: */
#define CELLWIDTH    LCD_DEFAULT_CELLWIDTH
#define CELLHEIGHT    LCD_DEFAULT_CELLHEIGHT


/* Constants for userdefchar_mode */
#define NUM_CCs        8 /* max. number of custom characters */


typedef struct cgram_cache {
  unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
  int clean;
} CGram;


/** private data for the \c LonganNano driver */
typedef struct LonganNano_private_data {
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
MODULE_EXPORT char *symbol_prefix = "LonganNano_";

/* API: functions for the server core */
MODULE_EXPORT int  LonganNano_init(Driver *drvthis);
MODULE_EXPORT void LonganNano_close(Driver *drvthis);
MODULE_EXPORT int  LonganNano_width(Driver *drvthis);
MODULE_EXPORT int  LonganNano_height(Driver *drvthis);
MODULE_EXPORT int  LonganNano_cellheight(Driver *drvthis);
MODULE_EXPORT int  LonganNano_cellwidth(Driver *drvthis);
MODULE_EXPORT void LonganNano_clear(Driver *drvthis);
MODULE_EXPORT void LonganNano_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void LonganNano_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void LonganNano_flush(Driver *drvthis);
MODULE_EXPORT void LonganNano_backlight(Driver *drvthis, int on);
MODULE_EXPORT void LonganNano_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void LonganNano_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void LonganNano_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  LonganNano_get_free_chars(Driver *drvthis);
MODULE_EXPORT void LonganNano_set_char(Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int  LonganNano_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void LonganNano_output(Driver *drvthis, int state);
MODULE_EXPORT const char *LonganNano_get_info(Driver *drvthis);

#endif    /* LONGANNANO_H */

