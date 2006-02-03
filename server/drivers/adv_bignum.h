/* 	This is the bignumber-library used by the serialVFD driver.

	Copyright (C) 2006 Stefan Herdler

	Based on drivers.c and lcd-lib.c.
	It may contain parts of other drivers of this package too.

	2006-01-26 Version 0.1: everything should work (not all hardware tested!)

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

#ifndef ADV_BIGNUM_H
#define ADV_BIGNUM_H

#include "lcd.h"

void lib_adv_bignum (Driver *drvthis, int x, int num, int height , int do_init, int customchars);



#endif
