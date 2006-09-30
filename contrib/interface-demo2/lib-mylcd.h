/* lib-mylcd.h

 used by interfacedemo for "pyramid" LCD device.
 */

/*
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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

 */

#ifndef LIB_MYLCD_H
#define LIB_MYLCD_H

#define MAXREAD 1024
/* Keys */
#define UNKNOWNKEY -1
#define NOKEY 0
#define DOWNKEY 1
#define ENTERKEY 2
#define RIGHTKEY 3
#define ESCAPEKEY 4

struct
{
    int width;
    int height;
} screen;


/* Request information from LCDd
 */
int get_integer(int SD, char *request);

/* Used to parse the reply from the hello-msg.
 */
void parse_hello(char *input);

int parse_reply(char *input);

char *basic_send(int SD, char *request);

char *basic_read(int SD);

#endif

