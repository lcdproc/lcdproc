/*  This is the LCDproc driver for Cwlinux devices (http://www.cwlinux.com)

    Copyright (C) 2002, Andrew Ip
                  2002, David Glaude
                  2003, David Glaude

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

#ifndef CWLNX_H
#define CWLNX_H

#include "lcd.h"

// #define DEFAULT_CONTRAST 560
// #define DEFAULT_BRIGHTNESS 60
// #define DEFAULT_OFFBRIGHTNESS 0
// #define DEFAULT_EXITBRIGHTNESS 0

#define DEFAULT_CELLWIDTH 6
#define DEFAULT_CELLHEIGHT 8
#define DEFAULT_DEVICE "/dev/lcd"
#define DEFAULT_SPEED B19200
#define DEFAULT_SIZE "20x4"
#define DEFAULT_BACKLIGHT 1
#define DEFAULT_BRIGHTNESS 200

/* These are the keys for a (possibly) broken LK202-25...*/
/* NOTE: You should configure these settings in the configfile
 *       These defines are just used to get *some* defaults
 */
#define CWLNX_KEY_UP    'A'
#define CWLNX_KEY_DOWN  'B'
#define CWLNX_KEY_LEFT  'C'
#define CWLNX_KEY_RIGHT 'D'
#define CWLNX_KEY_YES   'E'
#define CWLNX_KEY_NO    'F'

#define DEFAULT_PAUSE_KEY	CWLNX_KEY_UP
#define DEFAULT_BACK_KEY	CWLNX_KEY_LEFT
#define DEFAULT_FORWARD_KEY	CWLNX_KEY_RIGHT
#define DEFAULT_MAIN_MENU_KEY	CWLNX_KEY_DOWN

MODULE_EXPORT int  CwLnx_init(Driver * drvthis, char *device);
MODULE_EXPORT void CwLnx_close(Driver * drvthis);
/* */ MODULE_EXPORT int  CwLnx_width (Driver * drvthis);
/* */ MODULE_EXPORT int  CwLnx_height (Driver * drvthis);
MODULE_EXPORT void CwLnx_clear (Driver * drvthis);
MODULE_EXPORT void CwLnx_flush(Driver * drvthis);
MODULE_EXPORT void CwLnx_string(Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void CwLnx_chr(Driver * drvthis, int x, int y, char c);
MODULE_EXPORT void CwLnx_vbar(Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CwLnx_hbar(Driver * drvthis, int x, int y, int len, int promille, int options);
 MODULE_EXPORT void CwLnx_num(Driver * drvthis, int x, int num);
MODULE_EXPORT int  CwLnx_icon(Driver * drvthis, int x, int y, int icon);
MODULE_EXPORT void CwLnx_set_char(Driver * drvthis, int n, char *dat);
MODULE_EXPORT int  CwLnx_get_contrast(Driver * drvthis);
MODULE_EXPORT void CwLnx_set_contrast(Driver * drvthis, int contrast);
MODULE_EXPORT void CwLnx_clear(Driver * drvthis);
MODULE_EXPORT void CwLnx_backlight(Driver * drvthis, int on);

// MODULE_EXPORT void CwLnx_flush_box(Driver * drvthis, int lft, int top, int rgt, int bot);
// MODULE_EXPORT void CwLnx_init_vbar( Driver * drvthis);
// MODULE_EXPORT void CwLnx_init_hbar( Driver * drvthis);
// MODULE_EXPORT void CwLnx_init_num( Driver * drvthis);
// MODULE_EXPORT void CwLnx_draw_frame(Driver * drvthis, char *dat);

#endif

