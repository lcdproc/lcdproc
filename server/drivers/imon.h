/**
 * Driver for Soundgraph/Ahanix/Silverstone/Uneed/Accent iMON IR/VFD Module
 *
 * In order to be able to use it, you have to get and install one of
 * the following kernel modules:
 *  - standalone iMON VFD driver from http://venky.ws/projects/imon/
 *  - the iMON module included with LIRC ver. 0.7.1 or newer
 *    from http://www.lirc.org/
 *
 * Copyright (c) 2004, Venky Raju <dev@venky.ws>, original author of
 * the LCDproc 0.4.5 iMON driver, the standalone and the LIRC kernel
 * modules for the iMON IR/VFD at http://venky.ws/projects/imon/
 * Inspired by:
 * 	TextMode driver (LCDproc authors?)
 *	Sasem driver	(Oliver Stabel)
 *
 * Copyright (c)  2005 Lucian Muresan <lucianm AT users.sourceforge.net>,
 *                     porting the LCDproc 0.4.5 code to LCDproc 0.5
 * Copyright (c)  2006 John Saunders, use graphics characters
 *
 * This source code is being released under the GPL.
 * Please see the file COPYING in this package for details.
 *
 */

#ifndef IMON_H
#define IMON_H

MODULE_EXPORT int  imon_init(Driver *drvthis);
MODULE_EXPORT void imon_close(Driver *drvthis);
MODULE_EXPORT int  imon_width(Driver *drvthis);
MODULE_EXPORT int  imon_height(Driver *drvthis);
MODULE_EXPORT int  imon_cellwidth(Driver *drvthis);
MODULE_EXPORT int  imon_cellheight(Driver *drvthis);
MODULE_EXPORT void imon_clear(Driver *drvthis);
MODULE_EXPORT void imon_flush(Driver *drvthis);
MODULE_EXPORT void imon_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void imon_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT int  imon_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT const char *imon_get_info(Driver *drvthis);
MODULE_EXPORT void imon_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void imon_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  imon_get_free_chars(Driver *drvthis);
MODULE_EXPORT void imon_num(Driver *drvthis, int x, int num);

/**
 * these are not (yet?) supported by the iMON IR/VFD kernel module
 */
//MODULE_EXPORT void imon_set_char(Driver *drvthis, int n, char *dat);
//MODULE_EXPORT int  imon_get_brightness(Driver *drvthis, int state);
//MODULE_EXPORT void imon_set_brightness(Driver *drvthis, int state, int promille);
//MODULE_EXPORT void imon_backlight(Driver *drvthis, int on);

#endif
