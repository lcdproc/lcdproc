/*
 * irtrans driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 * Copyright (C) 1998-2007 <info@irtrans.de>
 *
 * Copyright (C) 2007 Phant0m <phantom@netkeke.com>
 * porting the LCDproc 0.4.3 code to LCDproc 0.5.1
 *
 * Inspired by:
 *  TextMode driver (LCDproc authors)
 *  irtrans driver (Irtrans)
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

#ifndef LCD_TEXT_H
#define LCD_TEXT_H

MODULE_EXPORT int irtrans_init(Driver *drvthis);
MODULE_EXPORT void irtrans_close(Driver *drvthis);
MODULE_EXPORT int irtrans_width(Driver *drvthis);
MODULE_EXPORT int irtrans_height(Driver *drvthis);
MODULE_EXPORT void irtrans_clear(Driver *drvthis);
MODULE_EXPORT void irtrans_flush(Driver *drvthis);
MODULE_EXPORT void irtrans_string(Driver *drvthis, int x, int y,
                                  char string[]);
MODULE_EXPORT void irtrans_chr(Driver * drvthis, int x, int y, char c);
MODULE_EXPORT void irtrans_set_contrast(Driver *drvthis, int promille);
MODULE_EXPORT void irtrans_backlight(Driver *drvthis, int on);

typedef int SOCKET;
typedef int WSAEVENT;

#define IRTRANS_DEFAULT_SIZE "16x2"
#define IRTRANS_DEFAULT_HOSTNAME "localhost"

#endif
