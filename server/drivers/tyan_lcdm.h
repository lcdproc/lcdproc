/*  This is the LCDproc driver header for tyan lcd module (tyan Barebone GS series)

    Author: yhlu@tyan.com

    Copyright (C) 2004 Tyan Corp

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
#ifndef TYAN_LCDM_H
#define TYAN_LCDM_H

#define DEFAULT_CELL_WIDTH	5
#define DEFAULT_CELL_HEIGHT	8
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		9600
#define DEFAULT_SIZE		"16x2"

#define TYAN_LCDM_KEY_ENTER	0xF2
#define TYAN_LCDM_KEY_ESCAPE	0xF3
#define TYAN_LCDM_KEY_RIGHT	0xF5
#define TYAN_LCDM_KEY_LEFT	0xF6
#define TYAN_LCDM_KEY_UP	0xF7
#define TYAN_LCDM_KEY_DOWN	0xF8

#define TYAN_LCDM_CMD_BEGIN	0xF1
#define TYAN_LCDM_CMD_END	0xF2


/* Constants for userdefchar_mode */
#define NUM_CCs	8	 /* max. number of custom characters */


typedef struct cgram_cache {
	unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
	int clean;
} CGram;


/** private data for the \c tyan driver */
typedef struct tyan_lcdm_private_data {
	char device[200];
	int speed;
	int fd;

	unsigned char *framebuf;
	unsigned char *backingstore;

	int width;
	int height;
	int cellwidth;
	int cellheight;

	/* definable characters */
	CGram cc[NUM_CCs];
	CGmode ccmode;
} PrivateData;


/* API: variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "tyan_lcdm_";

/* API: functions for the server core */
MODULE_EXPORT int  tyan_lcdm_init (Driver *drvthis, char *device);
MODULE_EXPORT void tyan_lcdm_close (Driver *drvthis);
MODULE_EXPORT int  tyan_lcdm_width (Driver *drvthis);
MODULE_EXPORT int  tyan_lcdm_height (Driver *drvthis);
MODULE_EXPORT int  tyan_lcdm_cellwidth (Driver *drvthis);
MODULE_EXPORT int  tyan_lcdm_cellheight (Driver *drvthis);
MODULE_EXPORT void tyan_lcdm_clear (Driver *drvthis);
MODULE_EXPORT void tyan_lcdm_flush (Driver *drvthis);
MODULE_EXPORT void tyan_lcdm_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void tyan_lcdm_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void tyan_lcdm_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void tyan_lcdm_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void tyan_lcdm_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  tyan_lcdm_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT int  tyan_lcdm_get_free_chars (Driver *drvthis);
MODULE_EXPORT void tyan_lcdm_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT void tyan_lcdm_backlight (Driver *drvthis, int on);

#endif
