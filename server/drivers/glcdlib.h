/*-
 * This so-called "meta-driver" extends LCDproc's supported drivers by
 * all the drivers supported by graphlcd-base, which you can get from
 * http://projects.vdr-developer.org/projects/graphlcd/ .
 * In order to be able to use it, you have to get and install the glcdprocdriver
 * from http://lucianm.github.com/GLCDprocDriver/ .
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2005 Lucian Muresan <lucianm AT users.sourceforge.net>
 */


#ifndef GLCDLIBLCDPROCDRIVER_H
#define GLCDLIBLCDPROCDRIVER_H

#include "lcd.h"



typedef unsigned short u16;
typedef unsigned char u8;

// ****************************************************************************************
// *                  F U N C T I O N S                                                   *
// ****************************************************************************************


MODULE_EXPORT int  glcdlib_init (Driver *drvthis);
MODULE_EXPORT void glcdlib_close (Driver *drvthis);
MODULE_EXPORT int  glcdlib_width (Driver *drvthis);
MODULE_EXPORT int  glcdlib_height (Driver *drvthis);
MODULE_EXPORT void glcdlib_clear (Driver *drvthis);
MODULE_EXPORT void glcdlib_flush (Driver *drvthis);
MODULE_EXPORT void glcdlib_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void glcdlib_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT int  glcdlib_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT int  glcdlib_cellwidth (Driver *drvthis);
MODULE_EXPORT int  glcdlib_cellheight (Driver *drvthis);
MODULE_EXPORT int  glcdlib_get_free_chars (Driver *drvthis);
MODULE_EXPORT const char * glcdlib_get_info (Driver *drvthis);
MODULE_EXPORT int  glcdlib_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void glcdlib_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void glcdlib_set_char (Driver *drvthis, int n, char *dat);
MODULE_EXPORT void glcdlib_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void glcdlib_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void glcdlib_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
//MODULE_EXPORT void glcdlib_backlight(Driver *drvthis, int on);

#endif
