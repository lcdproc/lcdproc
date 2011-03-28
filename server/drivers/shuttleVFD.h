/*
 * ShuttleVFD driver
 *
 * Copyright (C) 2007 Thien Vu
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
 *
 * Based on work from:
 *   LCDproc text driver
 *   setvfd from http://jeremy.infogami.com/SetVFD
 *   vfd from http://www.linuxowl.com/software/
 */

#ifndef SHUTTLE_VFD_H
#define SHUTTLE_VFD_H

// mandatory functions (necessary for all drivers)
MODULE_EXPORT int shuttleVFD_init(Driver *drvthis);
MODULE_EXPORT void shuttleVFD_close(Driver *drvthis);

// essential output functions (necessary for output drivers)
MODULE_EXPORT int shuttleVFD_width(Driver *drvthis);
MODULE_EXPORT int shuttleVFD_height(Driver *drvthis);
MODULE_EXPORT void shuttleVFD_clear(Driver *drvthis);
MODULE_EXPORT void shuttleVFD_flush(Driver *drvthis);
MODULE_EXPORT void shuttleVFD_string(
    Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void shuttleVFD_chr(Driver *drvthis, int x, int y, char c);

// extended output functions
MODULE_EXPORT int shuttleVFD_icon(Driver *drvthis, int x, int y, int icon);

// user-defined character functions
MODULE_EXPORT int shuttleVFD_cellwidth(Driver *drvthis);
MODULE_EXPORT int shuttleVFD_cellheight(Driver *drvthis);


// VFD physical dimensions
#define SHUTTLE_VFD_WIDTH         20
#define SHUTTLE_VFD_HEIGHT        1
#define SHUTTLE_VFD_CELLWIDTH     5
#define SHUTTLE_VFD_CELLHEIGHT    8

// VFD USB properties
#define SHUTTLE_VFD_VENDOR_ID1      0x1308
#define SHUTTLE_VFD_VENDOR_ID2      0x051c
#define SHUTTLE_VFD_PRODUCT_ID1     0x0003
// IR-receiver included in this model
#define SHUTTLE_VFD_PRODUCT_ID2     0x0005
#define SHUTTLE_VFD_INTERFACE_NUM   1

// VFD USB control message
#define SHUTTLE_VFD_PACKET_SIZE          8
#define SHUTTLE_VFD_WRITE_ATTEMPTS       3
#define SHUTTLE_VFD_SUCCESS_SLEEP_USEC   25600
#define SHUTTLE_VFD_RETRY_SLEEP_USEC     25600

// VFD Icons
#define SHUTTLE_VFD_ICON_CLOCK          (1 << 4)
#define SHUTTLE_VFD_ICON_RADIO          (1 << 3)
#define SHUTTLE_VFD_ICON_MUSIC          (1 << 2)
#define SHUTTLE_VFD_ICON_CD_DVD         (1 << 1)
#define SHUTTLE_VFD_ICON_TELEVISION     (1 << 0)
#define SHUTTLE_VFD_ICON_CAMERA         (1 << 9)
#define SHUTTLE_VFD_ICON_REWIND         (1 << 8)
#define SHUTTLE_VFD_ICON_RECORD         (1 << 7)
#define SHUTTLE_VFD_ICON_PLAY           (1 << 6)
#define SHUTTLE_VFD_ICON_PAUSE          (1 << 5)
#define SHUTTLE_VFD_ICON_STOP           (1 << 14)
#define SHUTTLE_VFD_ICON_FASTFORWARD    (1 << 13)
#define SHUTTLE_VFD_ICON_REVERSE        (1 << 12)
#define SHUTTLE_VFD_ICON_REPEAT         (1 << 11)
#define SHUTTLE_VFD_ICON_MUTE           (1 << 10)
#define SHUTTLE_VFD_ICON_VOL_01         (1 << 15)
#define SHUTTLE_VFD_ICON_VOL_02         (2 << 15)
#define SHUTTLE_VFD_ICON_VOL_03         (3 << 15)
#define SHUTTLE_VFD_ICON_VOL_04         (4 << 15)
#define SHUTTLE_VFD_ICON_VOL_05         (5 << 15)
#define SHUTTLE_VFD_ICON_VOL_06         (6 << 15)
#define SHUTTLE_VFD_ICON_VOL_07         (7 << 15)
#define SHUTTLE_VFD_ICON_VOL_08         (8 << 15)
#define SHUTTLE_VFD_ICON_VOL_09         (9 << 15)
#define SHUTTLE_VFD_ICON_VOL_10         (10 << 15)
#define SHUTTLE_VFD_ICON_VOL_11         (11 << 15)
#define SHUTTLE_VFD_ICON_VOL_12         (12 << 15)

#endif
