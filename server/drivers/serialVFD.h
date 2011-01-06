/*-
 * Copyright (C) 2006 Stefan Herdler
 *
 * This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
 * driver. It may contain parts of other drivers of this package too.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * 2006-05-16 Version 0.3: everything should work (not all hardware tested!)
 */

#ifndef SERIALVFD_H
#define SERIALVFD_H

#define DEFAULT_CELL_WIDTH	5
#define DEFAULT_CELL_HEIGHT	7
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		9600
#define DEFAULT_BRIGHTNESS	140
#define DEFAULT_SIZE		"20x2"
#define DEFAULT_DISPLAYTYPE 	0
#define DEFAULT_PARA_WAIT	2
#define DEFAULT_OFF_BRIGHTNESS	300
#define DEFAULT_ON_BRIGHTNESS	1000
#define DEFAULT_LPTPORT		0x378


MODULE_EXPORT int  serialVFD_init (Driver *drvthis);
MODULE_EXPORT void serialVFD_close (Driver *drvthis);
MODULE_EXPORT int  serialVFD_width (Driver *drvthis);
MODULE_EXPORT int  serialVFD_height (Driver *drvthis);
MODULE_EXPORT int  serialVFD_cellwidth (Driver *drvthis);
MODULE_EXPORT int  serialVFD_cellheight (Driver *drvthis);
MODULE_EXPORT void serialVFD_clear (Driver *drvthis);
MODULE_EXPORT void serialVFD_flush (Driver *drvthis);
MODULE_EXPORT void serialVFD_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void serialVFD_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void serialVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void serialVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void serialVFD_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  serialVFD_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void serialVFD_set_char (Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int serialVFD_get_free_chars (Driver *drvthis);

MODULE_EXPORT int  serialVFD_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void serialVFD_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void serialVFD_backlight (Driver *drvthis, int on);

MODULE_EXPORT const char * serialVFD_get_info( Driver *drvthis );

/** private data for the \c serialVFD driver */
typedef struct serialVFD_private_data {
	int use_parallel;	/**< use parallel port? */
	unsigned short port;	/**< port in parallel mode */
	char device[200];	/**> device in serial mode */
	int fd;			/**< file descriptor in serial mode */
	int speed;		/**< Speed in serial mode */
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;
	CGmode ccmode;
	int on_brightness;
	int off_brightness;
	int hw_brightness;
	int customchars;
	int predefined_hbar;	/**< display has predefined hbar-characters */
	int predefined_vbar;	/**< display has predefined vbar-characters */
	int ISO_8859_1;
	unsigned int refresh_timer;
	unsigned int para_wait;
	unsigned char charmap[129];
	int display_type;	/**< display type */
	int last_custom;	/**< last custom character written */
	unsigned char custom_char[31][7]; 	/**< stored custom characters */
	unsigned char custom_char_store[31][7]; 	/**< custom characters backingstore */
	unsigned char hw_cmd[11][10];	/**< table of hardwarespecific commands */
	int usr_chr_dot_assignment[57];	/**< how to setup usercharacters */
	unsigned int usr_chr_mapping[31];	/**< where to place the usercharacters (0..30) in the asciicode */
	unsigned int usr_chr_load_mapping[31];	/**< needed for displays with different read and write mapping */
	int hbar_cc_offset;	/**< character offset of the bars */
	int vbar_cc_offset;	/**< character offset of the bars */
	char info[255];
} PrivateData;

#endif
