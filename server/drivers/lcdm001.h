/**
 * This is the header file of the LCDproc driver for the "LCDM001" device from
 * kernelconcepts.de
 */

/*-
 *  Copyright (C) 2001  Rene Wagner <reenoo@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifndef LCDM001_H
#define LCDM001_H

MODULE_EXPORT int  lcdm001_init (Driver *drvthis);
MODULE_EXPORT void lcdm001_close (Driver *drvthis);
MODULE_EXPORT int  lcdm001_width (Driver *drvthis);
MODULE_EXPORT int  lcdm001_height (Driver *drvthis);
MODULE_EXPORT void lcdm001_clear (Driver *drvthis);
MODULE_EXPORT void lcdm001_flush (Driver *drvthis);
MODULE_EXPORT void lcdm001_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void lcdm001_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void lcdm001_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void lcdm001_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void lcdm001_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  lcdm001_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void lcdm001_output (Driver *drvthis, int on);
MODULE_EXPORT char lcdm001_getkey (Driver *drvthis);

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_CURSORBLINK	0	/* cursor blinking off */

/*
 * Use our own chars for the heartbeat instead of core's default. This
 * combination is at least visible.
 */
#define LCDM001_OPEN_HEART	' '
#define LCDM001_FILLED_HEART	'*'

#define LEFT_KEY '1'
#define RIGHT_KEY '2'
#define UP_KEY '3'
#define DOWN_KEY '4'

#endif
