/******************************************************************************
*
*  util.h - header file for util.c
*  Copyright (C) 2005  Peter Marschall
*
*  ---
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*  ---
* 
*  $Source$
*  $Revision$
*  Checked in by: $Author$
*
*******************************************************************************/


#ifndef LCDPROC_UTIL_H
# define LCDPROC_UTIL_H

#include <stdio.h>
#include <string.h>

/** print a memory value with the correct unit to a given string */
char *sprintf_memory(char *dst, double value, double roundlimit);

/** print a percentage value to a given string */
char *sprintf_percent(char *dst, double percent);

/** converts value into power-of-x representation */
char *convert_double(double *value, int base, double roundlimit);

#endif /* LCDPROC_UTIL_H */

/* EOF */
