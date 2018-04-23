/**
 * \file server/drivers/serialPOS_common.h
 *
 * Common header file for serialPOS driver
 */

#ifndef SERVER_DRIVERS_SERIALPOS_COMMON_H_
#define SERVER_DRIVERS_SERIALPOS_COMMON_H_

/*-
 * Common declarations and shared data structures for serialPOS
 *
 * Copyright (C) 2006, 2007 Eric Pooch
 * Copyright (C) 2018, Shenghao Yang
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

#include "lcd.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_DEVICE      "/dev/ttyS0"
#define DEFAULT_SPEED       9800
#define DEFAULT_LINEWRAP    0
#define DEFAULT_AUTOSCROLL  0
#define DEFAULT_CURSORBLINK 0
#define DEFAULT_SIZE        "16x2"
#define DEFAULT_TYPE        "AEDEX"
#define MAX_CELLWID 5
#define MAX_CELLHGT 8

#define AEDEXDefaultPrefix "!#"
#define AEDEXPrefix "~`"
/*
 * Max width is limited because the following expression can not evaluate
 * to zero - used in hbar()
 *
 * 1000 / (cellwid * width) [Integer division]
 */
#define MAX_WIDTH (1000 / MAX_CELLWID)
/*
 * Max height limited in the same way as maximum width, expression here is:
 * (used in vbar())
 *
 * 1000 / (cellhgt * height) [Integer division]
 */
#define MAX_HEIGHT (1000 / MAX_CELLHGT)

/**
 * All POS protocols supported by serialPOS
 */
typedef enum {
        /*
         * Implemented, no custom character support, no cursor support, no
         * backlight support.
         */
        POS_AEDEX,
        /*
         * Implemented, no custom character support, no cursor support, no
         * backlight support.
         */
        POS_Epson, /* http://www.barcode-manufacturer.com/pdf/vfd_manual.pdf */
        /*
         * Implemented as POS_AEDEX
         */
        POS_Emax,
        /*
         * Implemented, no custom character support, no cursor support, no
         * backlight support.
         */
        POS_LogicControls,
        /*
         * Implemented as POS_LogicControls
         */
        POS_Ultimate,
} POS_EmulationType;

typedef enum {
        AEDEXLine1Display = 1, /*       upper line display      */
        AEDEXLine2Display, /*   bottom line display     */
        AEDEXLine3Display, /*   not sure what this code really is       */
        AEDEXContinuousScroll, /*       upper line message scroll continuously  */
        AEDEXDisplayTime, /*    "hh':'mm" h,m='0'-'9' display time      */
        AEDEXSingleScroll, /*   upper line message scroll one pass      */
        AEDEXAllScroll, /*      not sure what this code really is       */
        AEDEXChangeCode, /*     change attention code   */
        AEDEXAllLineDisplay /*  two line display        */
} AEDEXCommands;

/**
 * Private data for the \c serialPOS driver
 */
typedef struct serialPOS_private_data {
        int fd; /**< LCD file descriptor */

        /* dimensions */
        int width, height;
        int cellwidth, cellheight;

        /* framebuffer and buffer for old LCD contents */
        uint8_t *framebuf; /**< Framebuffer mutated by server calls */
        uint8_t *backingstore; /**< real LCD display buffer */

        POS_EmulationType emulation_mode;

        char info[255]; /**< static data from serialPOS_get_info */
} PrivateData;

#endif /* SERVER_DRIVERS_SERIALPOS_COMMON_H_ */
