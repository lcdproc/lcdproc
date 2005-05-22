/******************************************************************************
*
*  util.c - utility functions to print some numerical values in a nice fashion
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
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  ---
* 
*  $Source$
*  $Revision$
*  Checked in by: $Author$
*
*******************************************************************************/

#include "util.h"


/** print a memory value with the correct unit to a given string */
char *
sprintf_memory(char *dst, double value, double roundlimit)
{
	if (dst != NULL) {
		static char *units[] = { "", "k", "M", "G", "T", "P", "E", "Z", "Y", NULL };
		int offs = 0;
		char *format = "%.1f%s";

		if ((roundlimit <= 0.0) || (roundlimit > 1.0))
			roundlimit = 0.5;

		while (units[offs] != NULL > 0) {
			if (value <= 1024 * roundlimit)
				break;
			offs++;
			value /= 1024;
		}
		if (value < 100)
			format = "%.2f%s";
		if (value < 10)
			format = "%.3f%s";
		
		sprintf(dst, format, value, units[offs]);
	}	
	return dst;	
}


/** print a percentage value to a given string */
char *
sprintf_percent(char *dst, double percent)
{
	if (dst != NULL) {
		if (percent > 99.9)
			strcpy(dst, "100%");
		else
			sprintf(dst, "%.1f%%", (percent >= 0) ? percent : 0);
	}	
	return dst;
}

/* EOF */
