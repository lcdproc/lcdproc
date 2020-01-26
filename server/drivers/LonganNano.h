/* This is the LCDproc driver for Longan Nano device
 * (https://longan.sipeed.com/en/)

     Copyright (C) 2020, Fabien Marteau <mail@fabienm.eu>

   based on GPL'ed code:

   * IOWarrior
    Copyright (C) 2004, Peter Marschall <peter@adpm.de>
    Copyright (c) 2004  Christian Vogelgsang <chris@lallafa.de>

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


#ifndef LONGANNANO_H
#define LONGANNANO_H


#define DEFAULT_DEVICE		"/dev/cuaU0"
#define DISPLAY_DEFAULT_SIZE 	"40x4"

/* Default 9600 baud, 1 hz update rate, time resolution of 1 ms */
#define DEFAULT_SPEED		B115200
#define DEFAULT_UPDATE_RATE	1.0
#define SECOND_GRANULARITY	1000

MODULE_EXPORT int rawserial_init(Driver *drvthis);
MODULE_EXPORT void rawserial_close(Driver *drvthis);
MODULE_EXPORT int rawserial_width(Driver *drvthis);
MODULE_EXPORT int rawserial_height(Driver *drvthis);
MODULE_EXPORT void rawserial_clear(Driver *drvthis);
MODULE_EXPORT void rawserial_flush(Driver *drvthis);
MODULE_EXPORT void rawserial_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void rawserial_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *rawserial_get_info(Driver *drvthis);

#endif    /* LONGANNANO_H */
