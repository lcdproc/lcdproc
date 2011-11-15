/** \file server/drivers/imonlcd.h
 * Driver for SoundGraph iMON OEM LCD Modules
 */

/*-
 * Copyright (c) 2004, Venky Raju <dev (at) venky (dot) ws>
 *               2007, Dean Harding <dean (at) codeka dotcom>
 *               2007, Christian Leuschen <christian (dot) leuschen (at) gmx (dot) de>
 *               2009, Jonathan Kyler <jkyler (at) users (dot) sourceforge (dot) net>
 *               2009, Eric Pooch < epooch (at) cox (dot) net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifndef IMONLCD_H
#define IMONLCD_H

/* mandatory functions (necessary for all drivers) */
MODULE_EXPORT int  imonlcd_init (Driver *drvthis);
MODULE_EXPORT void imonlcd_close (Driver *drvthis);

/* essential output functions (necessary for output drivers) */
MODULE_EXPORT int  imonlcd_width (Driver *drvthis);
MODULE_EXPORT int  imonlcd_height (Driver *drvthis);
MODULE_EXPORT void imonlcd_clear (Driver *drvthis);
MODULE_EXPORT void imonlcd_flush (Driver *drvthis);
MODULE_EXPORT void imonlcd_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void imonlcd_chr (Driver *drvthis, int x, int y, char c);

/* essential input functions (necessary for all input drivers) */
/* char *imonlcd_get_key NOT IMPLEMENTED */

/* extended output functions (optional; core provides alternatives) */
MODULE_EXPORT void imonlcd_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void imonlcd_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void imonlcd_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  imonlcd_icon (Driver *drvthis, int x, int y, int icon);
/* MODULE_EXPORT void imonlcd_heartbeat (Driver *drvthis, int state); NOT IMPLEMENTED */
/* MODULE_EXPORT void imonlcd_cursor (Driver *drvthis, int x, int y, int type); NOT IMPLEMENTED */

/* user-defined character functions */
/* MODULE_EXPORT void imonlcd_set_char (Driver *drvthis, int n, char *dat); NOT IMPLEMENTED */
/* MODULE_EXPORT int  imonlcd_get_free_chars (Driver *drvthis); NOT IMPLEMENTED */
MODULE_EXPORT int  imonlcd_cellwidth (Driver *drvthis);
MODULE_EXPORT int  imonlcd_cellheight (Driver *drvthis);

/* Hardware functions */
MODULE_EXPORT int  imonlcd_get_contrast (Driver *drvthis);
MODULE_EXPORT void imonlcd_set_contrast (Driver *drvthis, int promille);
/* MODULE_EXPORT int  imonlcd_get_brightness (Driver *drvthis, int state); NOT IMPLEMENTED */
/* MODULE_EXPORT void imonlcd_set_brightness (Driver *drvthis, int state, int promille); NOT IMPLEMENTED */
MODULE_EXPORT void imonlcd_backlight(Driver *drvthis, int on);
MODULE_EXPORT void imonlcd_output (Driver *drvthis, int state);

/* informational functions */
MODULE_EXPORT const char *imonlcd_get_info (Driver *drvthis);

#endif
