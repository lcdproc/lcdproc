/** \file server/drivers/sdeclcd.h
 * This is the LCDproc driver for SDEC LCD Devices.
 * They are found in the Watchguard FireBox firewall appliances.
 * They are interfaced through the parallel port.
 *
 * The code is based on the spec file LMC-S2D20-01.pdf,
 * a technical hardware and programming guide for this LCD.
 */

/*-
 *  Copyright(C) 2011, Francois Mertz <fireboxled AT gmail.com>
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

#ifndef SDECLCD_H
#define SDECLCD_H

MODULE_EXPORT int sdeclcd_init(Driver *);
MODULE_EXPORT void sdeclcd_close(Driver *);
MODULE_EXPORT int sdeclcd_width(Driver *);
MODULE_EXPORT int sdeclcd_height(Driver *);
MODULE_EXPORT void sdeclcd_flush(Driver *);
MODULE_EXPORT void sdeclcd_string(Driver *, int, int, char *);
MODULE_EXPORT void sdeclcd_chr(Driver *, int, int, char);
MODULE_EXPORT void sdeclcd_clear(Driver *);
MODULE_EXPORT int sdeclcd_cellwidth(Driver *);
MODULE_EXPORT int sdeclcd_cellheight(Driver *);
MODULE_EXPORT int sdeclcd_get_free_char(Driver *);
MODULE_EXPORT int sdeclcd_icon(Driver *, int, int, int);
MODULE_EXPORT void sdeclcd_heartbeat(Driver *, int);
MODULE_EXPORT void sdeclcd_vbar(Driver *, int, int, int, int, int);
MODULE_EXPORT void sdeclcd_hbar(Driver *, int, int, int, int, int);
MODULE_EXPORT void sdeclcd_num(Driver *, int, int);
MODULE_EXPORT const char *sdeclcd_get_key(Driver *);
MODULE_EXPORT void sdeclcd_backlight(Driver *, int);
MODULE_EXPORT const char *sdeclcd_get_info(Driver *);

#endif
