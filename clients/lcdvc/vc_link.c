/** \file clients/lcdvc/vc_link.c
 * Functions to handle a virtual console.
 */

/*-
 * This file is part of lcdvc, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *               2006-2008, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "lcdvc.h"
#include "vc_link.h"
#include "shared/report.h"
#include "shared/sockets.h"

int vcs0, vcsa;
unsigned short vc_width = 0, vc_height = 0;
unsigned short vc_cursor_x = 0, vc_cursor_y = 0;
char *vc_buf = NULL;


int open_vcs(void)
{
	/* Open the /dev/vcsX and /dev/vcsaX devices */
	vcs0 = open(vcs_device, O_RDONLY);
	if (vcs0 < 0) {
		report(RPT_ERR, "Could not open %s: %s", vcs_device, strerror(errno));
		return -1;
	}
	vcsa = open(vcsa_device, O_RDONLY);
	if (vcsa < 0) {
		report(RPT_ERR, "Could not open %s: %s", vcsa_device, strerror(errno));
		return -1;
	}
	return 0;
}


int read_vcdata(void)
{
	unsigned short new_vc_height;
	unsigned short new_vc_width;

	int bytes_read;
	unsigned char buf[20];

	/* Read size and cursor position from /dev/vcsa */
	lseek(vcsa, 0, SEEK_SET);
	bytes_read = read(vcsa, buf, 4);
	if (bytes_read != 4) {
		report(RPT_ERR, "Could not read from %s", vcsa_device);
		return -1;
	}
	new_vc_height = buf[0];
	new_vc_width = buf[1];
	vc_cursor_x = buf[2];
	vc_cursor_y = buf[3];

	/* Screen resize or initial buffer allocation ? */
	if ((new_vc_width != vc_width) || (new_vc_height != vc_height)) {
		vc_width = new_vc_width;
		vc_height = new_vc_height;

		if (vc_width * vc_height > 0) {
			vc_buf = realloc(vc_buf, vc_width * vc_height);

			if (vc_buf == NULL) {
				report(RPT_ERR, "malloc failure: %s", strerror(errno));
				return -1;
			}
			memset(vc_buf, ' ', vc_width * vc_height);
		}
	}

	/* Read characters from /dev/cvs0 */
	lseek(vcs0, 0, SEEK_SET);
	bytes_read = read(vcs0, vc_buf, vc_width * vc_height);
	if (bytes_read != vc_width * vc_height) {
		report(RPT_ERR, "Could not read from %s", vcs_device);
		return -1;
	}
	return 0;
}

