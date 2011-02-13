/*
  This is the LCDproc driver for the "LCTerm" serial LCD terminal
  from Helmut Neumark Elektronik, www.neumark.de

  Copyright (C) 2002  Michael Schwingen <michael@schwingen.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

  This driver is mostly based on the HD44780 and the LCDM001 driver.
  (Hopefully I have NOT forgotten any file I have stolen code from.
  If so send me an e-mail or add your copyright here!)
*/

#ifndef LCTERM_H
#define LCTERM_H

MODULE_EXPORT int  lcterm_init (Driver *drvthis);
MODULE_EXPORT void lcterm_close (Driver *drvthis);
MODULE_EXPORT int  lcterm_width (Driver *drvthis);
MODULE_EXPORT int  lcterm_height (Driver *drvthis);
MODULE_EXPORT void lcterm_clear (Driver *drvthis);
MODULE_EXPORT void lcterm_flush (Driver *drvthis);
MODULE_EXPORT void lcterm_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void lcterm_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void lcterm_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void lcterm_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void lcterm_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  lcterm_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void lcterm_set_char (Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int  lcterm_get_free_chars(Driver *drvthis);

#define DEFAULT_DEVICE "/dev/lcd"
#define NUM_CCs 8

#endif

