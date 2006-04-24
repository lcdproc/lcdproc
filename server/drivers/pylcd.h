/* pylcd.h */

/*
 This is the header file of the LCDproc driver for
 the "pyramid" LCD device from Pyramid.

 Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>
 Copyright (C) 2006 coresystems GmbH <info@coresystems.de>

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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307

 */

#ifndef PYLCD_H
#define PYLCD_H

#include "lcd.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define MAXCOUNT 10
#define WIDTH 16
#define HEIGHT 2

//Pyramid DS says 16, HD44780U DS says 8
//#define CUSTOMCHARS 16
#define CUSTOMCHARS 8

#define CELLHEIGHT  8
#define CELLWIDTH   5


typedef enum {
    normal = 0,
    hbar = 1,
    vbar = 2,
    beat = 4,
    icon = 8,
    custom1 = 16
} custom_type;

typedef struct
{
    /* device io */
    int FD;
    char device[255];
    fd_set rdfs;
    struct timeval timeout;

    /* device description */
    int width;
    int height;
    int customchars;
    int cellwidth;
    int cellheight;

    /* output handling */
    char framebuffer[WIDTH*HEIGHT+2];
    int FB_modified;
    custom_type custom;

    /* button handling */
    char last_key_pressed[6];
    unsigned long long last_key_time;
    unsigned long long last_buf_time;

    /* cursor handling */
    int C_x;
    int C_y;
    int C_state;

    /* led handling */
    char led[7];

} PrivateData;

#endif

