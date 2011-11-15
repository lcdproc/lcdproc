/*  This is the LCDproc driver header for the Logitech MX5000 keyboard

    Author: Christian Jodar (tian@gcstar.org)

    Copyright (C) 2008 Christian Jodar

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

#ifndef LCD_MX5000_H
#define LCD_MX5000_H

#include "lcd.h"

#define SCREEN_WIDTH        17
#define SCREEN_HEIGHT       4
#define CELL_WIDTH          6
#define CELL_HEIGHT         10
#define DEFAULT_DEVICE      "/dev/hiddev0"
#define DEFAULT_WAIT        1000

/** private data for the \c mx5000 driver */
typedef struct mx5000_private_data {
    char device[200];
    int wait;
    int fd;
    struct MX5000ScreenContent *sc;
    char info[255];
    char changed;
} PrivateData;


/* API: variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "mx5000_";

/* API: functions for the server core */
MODULE_EXPORT int  mx5000_init (Driver *drvthis, char *device);
MODULE_EXPORT void mx5000_close (Driver *drvthis);
MODULE_EXPORT int  mx5000_width (Driver *drvthis);
MODULE_EXPORT int  mx5000_height (Driver *drvthis);
MODULE_EXPORT int  mx5000_cellwidth (Driver *drvthis);
MODULE_EXPORT int  mx5000_cellheight (Driver *drvthis);
MODULE_EXPORT void mx5000_clear (Driver *drvthis);
MODULE_EXPORT void mx5000_flush (Driver *drvthis);
MODULE_EXPORT void mx5000_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void mx5000_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void mx5000_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void mx5000_heartbeat (Driver *drvthis, int state);
MODULE_EXPORT void mx5000_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  mx5000_icon(Driver *drvthis, int x, int y, int icon);

/*
MODULE_EXPORT void mx5000_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
*/

MODULE_EXPORT int  mx5000_get_free_chars (Driver *drvthis);
MODULE_EXPORT void mx5000_set_char (Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT void mx5000_backlight (Driver *drvthis, int on);

#endif
