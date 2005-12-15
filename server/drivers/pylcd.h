/* pylcd.h */

/*
 This is the header file of the LCDproc driver for
 the "pylcd" device from Pyramid.

 Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>

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

MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "pylcd_";

typedef struct
{
    int FD;
    char devicename[255];
    int width;
    int height;
    fd_set rdfs;
    struct timeval timeout;
    char framebuffer[WIDTH*HEIGHT+2];
    int FB_modified;
    int LEDtoggle;
    char last_key_pressed[6];
    unsigned long long last_key_time;
    unsigned long long last_buf_time;
    int C_x;
    int C_y;
    int C_state;
} pylcd_private_data;

#endif

