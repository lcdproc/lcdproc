/** \file server/drivers/pylcd.h
 * This is the header file of the LCDproc driver for the "pyramid" LCD device
 * from Pyramid.
 */

/*-
 * Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>
 *               2006 coresystems GmbH <info@coresystems.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifndef PYLCD_H
#define PYLCD_H

#define MAXCOUNT 10		/* Size of read buffer including NUL */

/* Display properties */
#define WIDTH 16
#define HEIGHT 2
#define SCREEN_SIZE (WIDTH * HEIGHT)
#define CUSTOMCHARS 8
#define CELLHEIGHT  8
#define CELLWIDTH   5

/** private data for the \c pyramid driver */
typedef struct pyramid_private_data {
    /* device io */
    int FD;
    char device[255];
    fd_set rdfs;
    struct timeval timeout;

    /* device description */
    int width;
    int height;
    int customchars;
    int cellwidth;
    int cellheight;

    /* output handling */
    char framebuffer[SCREEN_SIZE];
    char backingstore[SCREEN_SIZE];
    CGmode ccmode;
    unsigned char cc_cache[CUSTOMCHARS][CELLHEIGHT];

    /* button handling */
    char last_key_pressed[6];
    unsigned long long last_key_time;

    /* cursor handling */
    int C_x;
    int C_y;
    int C_state;

    /* led handling */
    char led[8];

} PrivateData;

MODULE_EXPORT int  pyramid_init(Driver *drvthis);
MODULE_EXPORT void pyramid_close(Driver *drvthis);
MODULE_EXPORT int  pyramid_width(Driver *drvthis);
MODULE_EXPORT int  pyramid_height(Driver *drvthis);
MODULE_EXPORT int  pyramid_cellwidth(Driver *drvthis);
MODULE_EXPORT int  pyramid_cellheight(Driver *drvthis);
MODULE_EXPORT void pyramid_clear(Driver *drvthis);
MODULE_EXPORT void pyramid_flush(Driver *drvthis);
MODULE_EXPORT void pyramid_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void pyramid_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void pyramid_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void pyramid_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  pyramid_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void pyramid_cursor(Driver *drvthis, int x, int y, int state);
MODULE_EXPORT int  pyramid_get_free_chars(Driver *drvthis);
MODULE_EXPORT void pyramid_set_char(Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT void pyramid_output(Driver *drvthis, int state);
MODULE_EXPORT const char *pyramid_get_key(Driver *drvthis);
MODULE_EXPORT const char *pyramid_get_info(Driver *drvthis);

#endif
