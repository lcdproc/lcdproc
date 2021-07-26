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

#include "shared/configfile.h"
#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "disk.h"
#include "util.h"

#define DISK_IGNORE_MAX 10

/**
 * Checks if a mountpoint is on the "ignore" list. 
 * 
 * \param mount			The mountpoint being evaluated
 * \param ignore_list	List of ignore entries from the configuration.
 * \return true if mount is present in ignore_list[], false otherwise.
 */
static bool 
disk_is_ignored(const char *mount, char *ignore_list[DISK_IGNORE_MAX])
{
	for (int i = 0; i < DISK_IGNORE_MAX; i++) {
		if (ignore_list[i] == NULL) { // End-of-list if < DISK_IGNORE_MAX
			return false;
		}
		if (strcmp(mount, ignore_list[i]) == 0) {
			return true;
		}
	}
	return false;
}

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

	int i;
	static int num_disks = 0;
	static int dev_wid = 6;
	static int gauge_wid = 6, gauge_scale, hbar_pos;
	int i_widget = 0; // Index into the widget array due to "ignore" list = 0;
	static char* disk_ignore[DISK_IGNORE_MAX] = {NULL};

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		const char* cfg_val = NULL;
		while ( (count < DISK_IGNORE_MAX) && 
			(cfg_val = config_get_string("Disk", "Ignore", count, NULL))) {
			disk_ignore[count] = strdup(cfg_val);
			count++;
		};

		dev_wid = (lcd_wid >= 20) ? (lcd_wid - 8) / 2 : (lcd_wid / 2) - 1;
		hbar_pos = (lcd_wid >= 20) ? dev_wid + 10 : dev_wid + 3;
		gauge_wid = lcd_wid - hbar_pos;
		gauge_scale = gauge_wid * lcd_cellwid;

		sock_send_string(sock, "screen_add D\n");
		sock_printf(sock, "screen_set D -name {Disk Use: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add D title title\n");
		sock_printf(sock, "widget_set D title {DISKS:%s}\n", get_hostname());
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

	/*
	 * Display stuff...  (show for two seconds, then scroll once per
	 * second, then hold at the end for two seconds)
	 */
	sock_printf(sock, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, count);
	for (i = 0; i < count; i++) {
		char tmp[lcd_wid + 1];	/* should be large enough */
		char cap[8] = {'\0'};
		char dev[dev_wid+1];
		u_int64_t size = 0;
		int full = 0;

		if (mnt[i].mpoint[0] == '\0' || disk_is_ignored(mnt[i].mpoint, disk_ignore))
			continue;

		// Parse/format the incoming FS data:
		// Naturally this could be condensed with the line formatting below
		// but I'm inclined to leave it like this to keep the printf formatting more legible.
		if (strlen(mnt[i].mpoint) > dev_wid)
			sprintf(dev, "-%s", (mnt[i].mpoint) + (strlen(mnt[i].mpoint) - (dev_wid - 1)));
		else
			sprintf(dev, "%s", mnt[i].mpoint);

		full = !mnt[i].blocks ? gauge_scale :
			gauge_scale * (u_int64_t) (mnt[i].blocks - mnt[i].bfree)
			/ mnt[i].blocks;

		size = (u_int64_t) mnt[i].bsize * mnt[i].blocks;
		sprintf_memory(cap, (double) size, 1);

		// Actual display/server output
		if (i_widget >= num_disks) {	/* Make sure we have enough lines... */
			sock_printf(sock, "widget_add D s%i string -in f\n", i_widget);
			sock_printf(sock, "widget_add D h%i hbar -in f\n", i_widget);
		}
		if (lcd_wid >= 20) {	/* 20+x columns */
			sprintf(tmp, "%-*s %6s E%*sF", dev_wid, dev, cap, gauge_wid, "");
		} else {		/* < 20 columns */
			sprintf(tmp, "%-*s E%*sF", dev_wid, dev, gauge_wid, "");
		}
		sock_printf(sock, "widget_set D s%i 1 %i {%s}\n", i_widget, i_widget + 1, tmp);
		sock_printf(sock, "widget_set D h%i %i %i %i\n",
					i_widget, hbar_pos, i_widget + 1, full);
		// Only increment current widget index if we "consumed" a display row.
		i_widget++; 
	}

	/* Now remove extra widgets... */
	for (i=i_widget; i < num_disks; i++) {
		sock_printf(sock, "widget_del D s%i\n", i);
		sock_printf(sock, "widget_del D h%i\n", i);
	}
	num_disks = i_widget;

	// And update the count so there aren't blank spaces due to ignored entries.
	sock_printf(sock, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, num_disks);


	return 0;
}
