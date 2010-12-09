/*-
 * Copyright (C) 2006 Stefan Herdler
 *
 * This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
 * driver. It may contain parts of other drivers of this package too.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * 2006-05-16 Version 0.3: everything should work (not all hardware tested!)
 */

#ifndef SERIALVFD_IO_H
#define SERIALVFD_IO_H

int serialVFD_init_serial (Driver *drvthis);
int serialVFD_init_parallel (Driver *drvthis);
void serialVFD_write_serial (Driver *drvthis, unsigned char *dat, size_t length);
void serialVFD_write_parallel (Driver *drvthis, unsigned char *dat, size_t length);
void serialVFD_close_serial (Driver *drvthis);
void serialVFD_close_parallel (Driver *drvthis);

/** Function list for low-level I/O routines */
typedef struct Port_fkt {
	void (*write_fkt) (Driver *drvthis, unsigned char *dat, size_t length);
	int (*init_fkt) (Driver *drvthis);
	void (*close_fkt) (Driver *drvthis);
} Port_fkt;

/**
 * Array with function pointers to low-level I/O routines.
 * Port_Function[0] stores routines for serial ports, Port_Function[1] those
 * for parallel ports.
 */
static const Port_fkt Port_Function[] = {
	{serialVFD_write_serial, serialVFD_init_serial, serialVFD_close_serial},
	{serialVFD_write_parallel, serialVFD_init_parallel, serialVFD_close_parallel}
};

#endif
