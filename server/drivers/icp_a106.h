/*
  This is the LCDproc driver for the ICP A106 alarm/LCD board,
  used in 19" rack cases by ICP

  Both LCD and alarm functions are accessed via one serial port, using
  separate commands. Unfortunately, the device runs at slow 1200bps and the
  LCD does not allow user-defined characters, so the bargraphs do not look
  very nice.

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

#ifndef ICP_A106_H
#define ICP_A106_H

MODULE_EXPORT int  icp_a106_init (Driver *drvthis);
MODULE_EXPORT void icp_a106_close (Driver *drvthis);
MODULE_EXPORT int  icp_a106_width (Driver *drvthis);
MODULE_EXPORT int  icp_a106_height (Driver *drvthis);
MODULE_EXPORT void icp_a106_clear (Driver *drvthis);
MODULE_EXPORT void icp_a106_flush (Driver *drvthis);
MODULE_EXPORT void icp_a106_string (Driver *drvthis, int x, int y, char *s);
MODULE_EXPORT void icp_a106_chr (Driver *drvthis, int x, int y, char ch);

MODULE_EXPORT void icp_a106_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void icp_a106_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void icp_a106_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  icp_a106_icon (Driver *drvthis, int x, int y, int icon);

#define DEFAULT_DEVICE "/dev/lcd"

#endif

