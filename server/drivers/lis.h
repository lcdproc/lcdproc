/*-
 * Copyright (c) 2007, Daryl Fonseca-Holt <wyatt@prairieturtle.ca>
 *
 * Based on:
 * ula200 driver, Copyright (C) 2006, Bernhard Walle
 * IOWarrior driver, Copyright(C) 2004-2006 Peter Marschall <peter@adpm.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */
#ifndef LIS_H
#define LIS_H

/*----------------------------- Constants ---------------------------------*/

#define DISPLAY_VENDOR_ID	0x0403
#define DISPLAY_PRODUCT_ID	0x6001

#define CELLWIDTH  5
#define CELLHEIGHT 8

#define DEFAULT_BRIGHTNESS 1000

#define NUM_CCs 8


/*------------------------ Private data types -----------------------------*/

typedef struct cgram_cache {
  unsigned char cache[CELLHEIGHT];
  int clean;
} CGram;


/** private data for the \c lis driver */
typedef struct lis_private_data {
	/* the handle for the USB FTDI library */
	struct ftdi_context ftdic;

	/* the width and the height (in number of characters) of the display */
	int width, height;

	/* pixel width and height of each character */
	int cellwidth, cellheight;

	/* The framebuffer and the framebuffer for the last contents (incr. update) */
	unsigned char *framebuf;

	/* dirty line flags */
	unsigned int *line_flags;

	/* child thread flag */
	int child_flag;

	/* parent thread flag */
	int parent_flag;

	/* display brightness 0-1000 */
	int brightness;

	/* custom characters */
	CGram cc[NUM_CCs];
	CGmode ccmode;

	/* USB Vendor ID */
	int VendorID;

	/* USB Product ID */
	int ProductID;

	/*
	 * flag if the last row of each character can be set when defining
	 * custom characters.
	 */
	char lastline;
} PrivateData;


MODULE_EXPORT int  lis_init(Driver *drvthis);
MODULE_EXPORT void lis_close(Driver *drvthis);
MODULE_EXPORT int  lis_width(Driver *drvthis);
MODULE_EXPORT int  lis_height(Driver *drvthis);
MODULE_EXPORT void lis_clear(Driver * drvthis);
MODULE_EXPORT void lis_string(Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void lis_chr(Driver * drvthis, int x, int y, unsigned char c);
MODULE_EXPORT void lis_flush(Driver * drvthis);
MODULE_EXPORT void lis_vbar(Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void lis_hbar(Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  lis_icon(Driver * drvthis, int x, int y, int icon);
MODULE_EXPORT int  lis_cellheight(Driver *drvthis);
MODULE_EXPORT int  lis_cellwidth(Driver *drvthis);
MODULE_EXPORT void lis_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  lis_get_free_chars(Driver *drvthis);
MODULE_EXPORT void lis_set_char(Driver *drvthis, int n, unsigned char *dat);

#endif /* LIS_H */
