/* 	This file is part the LCDproc driver for various serial VFD Devices.

	It contains the hardwaredependent commands ach characterset.

	If you want to add a new device to the driver add a new section
	to the displaytype-switch-case in the init-function, add a new load_...
	function below and fill it with the corrrect commands for the display.
	(Try wich displaytype works best with your display, copy and modify
	it's section that is the easiest way I guess.)

	Copyright (C) 2006 Stefan Herdler

	2006-05-15 Version 0.3: everything should work (not all hardware tested!)

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


#include "port.h"
#include "serialVFD_io.h"
#include "serialVFD.h"
#include "lcd.h"
#define WR_on  0x10
#define WR_off 0x11
#define Busy   0x80
#define LPTPORT 0x378

#define MAXBUSY 300

void
serialVFD_write_serial (Driver *drvthis, char *dat, size_t length)
{
	PrivateData *p = drvthis->private_data;
	write (p->fd,dat,length);
}

void
serialVFD_write_parallel (Driver *drvthis, char *dat, size_t length)
{
	PrivateData *p = drvthis->private_data;
	int i_para, j_para;

	for(i_para = 0; i_para < length; i_para++) {
		port_out(p->port, dat[i_para]);
		port_in(p->port+1);
		port_out(p->port+2, WR_on);
		port_in(p->port+1);
		port_out(p->port+2, WR_off);

		for(j_para=0; j_para < MAXBUSY; j_para++) {
			if((port_in(p->port+1)) & Busy)
				break;
		}
	}
}

int
serialVFD_init_serial (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	struct termios portset;

	/* Set up io port correctly, and open it...*/
	debug( RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open (p->device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (p->fd == -1) {
		report (RPT_ERR, "%s: open() of %s failed (%s)\n", __FUNCTION__, p->device, strerror (errno));
		return -1;
	}

	tcgetattr (p->fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw( &portset );
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	// Set port speed
	cfsetospeed (&portset, p->speed);
	cfsetispeed (&portset, B0);

	// Do it...
	tcsetattr (p->fd, TCSANOW, &portset);
	return 0;
}

int
serialVFD_init_parallel (Driver *drvthis)
{
	int ret=0;
	PrivateData *p = drvthis->private_data;
	debug( RPT_DEBUG, "%s: Opening parallelport at: 0x%X", __FUNCTION__, p->port);
//	if(port_access_multiple(p->port,3)) return -1;
	if(port_access(p->port) != 0) ret=-1;
	if(port_access(p->port+1) != 0) ret=-1;
	if(port_access(p->port+2) != 0) ret=-1;
	if(ret == -1) {
		report (RPT_ERR, "%s: port_access() of 0x%X failed (%s)\n", __FUNCTION__, p->port, strerror (errno));
		return -1;
	}
	return 0;
}


