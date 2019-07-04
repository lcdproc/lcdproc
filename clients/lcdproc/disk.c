/** \file clients/lcdproc/disk.c
 * Implements the 'Disk' screen.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "disk.h"
#include "util.h"


/**
 * Gives disk stats.
 * Stays onscreen until it is done; rolls over all mounted file systems
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## DISKS: myhost ##@|	|## DISKS: myhost ##@|
 * |/       18.3G E--  F|	|-local  18.3G E--- F|
 * |-local  18.3G E--- F|	+--------------------+
 * |/boot  949.6M E-   F|
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
disk_screen(int rep, int display, int *flags_ptr)
{
	mounts_type mnt[256];
	int count = 0;

	/* Holds info to display (avoid recalculating it) */
	struct disp {
		char dev[16];
		char cap[8];
		int full;
	} table[256];
	int i;
	static int num_disks = 0;
	static int dev_wid = 6;
	static int gauge_wid = 6, gauge_scale, hbar_pos;

	u_int64_t size;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		dev_wid = (lcd_wid >= 20) ? (lcd_wid - 8) / 2 : (lcd_wid / 2) - 1;
		hbar_pos = (lcd_wid >= 20) ? dev_wid + 10 : dev_wid + 3;
		gauge_wid = lcd_wid - hbar_pos;
		gauge_scale = gauge_wid * lcd_cellwid;

		sock_send_string(sock, "screen_add D\n");
		sock_printf(sock, "screen_set D -name {Disk Use: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add D title title\n");
		sock_printf(sock, "widget_set D title {DISKS: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add D f frame\n");
		sock_printf(sock, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, lcd_hgt - 1);
		sock_send_string(sock, "widget_add D err1 string\n");
		sock_send_string(sock, "widget_add D err2 string\n");
		sock_send_string(sock, "widget_set D err1 5 2 {  Reading  }\n");
		sock_send_string(sock, "widget_set D err2 5 3 {Filesystems}\n");
	}

	/* Get rid of old, unmounted filesystems... */
	machine_get_fs(mnt, &count);
	if (!count) {
		sock_send_string(sock, "widget_set D err1 1 2 {Error Retrieving}\n");
		sock_send_string(sock, "widget_set D err2 1 3 {Filesystem Stats}\n");
		return 0;
	}

	/* Fill the display structure... */
	sock_send_string(sock, "widget_set D err1 0 0 .\n");
	sock_send_string(sock, "widget_set D err2 0 0 .\n");
	for (i = 0; i < count; i++) {
		if (strlen(mnt[i].mpoint) > dev_wid)
			sprintf(table[i].dev, "-%s", (mnt[i].mpoint) + (strlen(mnt[i].mpoint) - (dev_wid - 1)));
		else
			sprintf(table[i].dev, "%s", mnt[i].mpoint);

		table[i].full = !mnt[i].blocks ? gauge_scale :
			gauge_scale * (u_int64_t) (mnt[i].blocks - mnt[i].bfree)
			/ mnt[i].blocks;

		size = (u_int64_t) mnt[i].bsize * mnt[i].blocks;
		memset(table[i].cap, '\0', 8);

		sprintf_memory(table[i].cap, (double) size, 1);
	}

	/*
	 * Display stuff...  (show for two seconds, then scroll once per
	 * second, then hold at the end for two seconds)
	 */
	sock_printf(sock, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, count);
	for (i = 0; i < count; i++) {
		char tmp[lcd_wid + 1];	/* should be large enough */

		if (table[i].dev[0] == '\0')
			continue;

		if (i >= num_disks) {	/* Make sure we have enough lines... */
			sock_printf(sock, "widget_add D s%i string -in f\n", i);
			sock_printf(sock, "widget_add D h%i hbar -in f\n", i);
		}
		if (lcd_wid >= 20) {	/* 20+x columns */
			sprintf(tmp, "%-*s %6s E%*sF", dev_wid, table[i].dev, table[i].cap, gauge_wid, "");
		}
		else {		/* < 20 columns */
			sprintf(tmp, "%-*s E%*sF", dev_wid, table[i].dev, gauge_wid, "");
		}
		sock_printf(sock, "widget_set D s%i 1 %i {%s}\n", i, i + 1, tmp);
		sock_printf(sock, "widget_set D h%i %i %i %i\n",
					i, hbar_pos, i + 1, table[i].full);
	}

	/* Now remove extra widgets... */
	for (; i < num_disks; i++) {
		sock_printf(sock, "widget_del D s%i\n", i);
		sock_printf(sock, "widget_del D h%i\n", i);
	}

	num_disks = count;

	return 0;
}
