/*-
 * Base driver module for Toshiba T6963 based LCD displays.
 *
 * Copyright (c) 2001 Manuel Stahl <mythos@xmythos.de>
 *               2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifndef T6963_H
#define T6963_H

/* API functions */
MODULE_EXPORT int  t6963_init (Driver *drvthis);
MODULE_EXPORT void t6963_close (Driver *drvthis);
MODULE_EXPORT int  t6963_width (Driver *drvthis);
MODULE_EXPORT int  t6963_height (Driver *drvthis);
MODULE_EXPORT int  t6963_cellwidth(Driver *drvthis);
MODULE_EXPORT int  t6963_cellheight(Driver *drvthis);
MODULE_EXPORT void t6963_clear (Driver *drvthis);
MODULE_EXPORT void t6963_flush (Driver *drvthis);
MODULE_EXPORT void t6963_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void t6963_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void t6963_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  t6963_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void t6963_set_char (Driver *drvthis, int n, unsigned char *dat);

/* Internal functions */
static void t6963_graphic_clear(Driver *drvthis);
static void t6963_set_nchar(Driver *drvthis, int n, int num);

#endif
