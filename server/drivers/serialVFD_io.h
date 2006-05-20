/* 	This is the LCDproc driver for various serial VFD Devices

	Copyright (C) 2006 Stefan Herdler

	2006-05-16 Version 0.3: everything should work (not all hardware tested!)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

#ifndef SERIALVFD_IO_H
#define SERIALVFD_IO_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "serialVFD_displays.h"
#include "port.h"
#include "lpt-port.h"
#include "lcd.h"
#include "serialVFD.h"
#include "report.h"


int serialVFD_init_serial (Driver *drvthis);
int serialVFD_init_parallel (Driver *drvthis);
void serialVFD_write_serial (Driver *drvthis, char *dat, size_t length);
void serialVFD_write_parallel (Driver *drvthis, char *dat, size_t length);

typedef struct Port_fkt {
	void (*write_fkt) (Driver *drvthis, char *dat, size_t length);
	int (*init_fkt) (Driver *drvthis);
} Port_fkt;


static const Port_fkt Port_Function[] = {
	// initialisation function
	// write function

	{serialVFD_write_serial, serialVFD_init_serial},
	{serialVFD_write_parallel, serialVFD_init_parallel}
};


#endif
