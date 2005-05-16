#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "disk.h"

///////////////////////////////////////////////////////////////////////////
// Gives disk stats. 
//
// Stays onscreen until it is done.
//

// TODO: Disk screen!  Requires virtual pages in the server, though...

int
disk_screen (int rep, int display)
{
	static mounts_type mnt[256];
	static int count = 0;

	// Holds info to display (avoid recalculating it)
	struct disp {
		char dev[16];
		char cap[8];
		int full;
	} table[256];
	int i;
	static int num_disks = 0;
	static int first = 1;		  // First line to display, sort of.
	static int dev_wid = 6;
	static int gauge_wid = 6;

#define huge long long int
	huge size;

	if (first) {
		first = 0;
		dev_wid = (lcd_wid >= 20) ? (lcd_wid - 8) / 2 : (lcd_wid / 2) - 1;
		gauge_wid = (lcd_wid >= 20) ? (lcd_wid - dev_wid - 10) : (lcd_wid - dev_wid - 3);

		sock_send_string (sock, "screen_add D\n");
		sprintf (buffer, "screen_set D -name {Disk Use: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add D title title\n");
		sprintf (buffer, "widget_set D title {DISKS: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add D f frame\n");
		//sock_send_string(sock, "widget_set D f 1 2 20 4 20 3 v 8\n");
		sprintf (buffer, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, lcd_hgt - 1);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add D err1 string\n");
		sock_send_string (sock, "widget_add D err2 string\n");
		sock_send_string (sock, "widget_set D err1 5 2 {  Reading  }\n");
		sock_send_string (sock, "widget_set D err2 5 3 {Filesystems}\n");
	}
	// Grab disk stats on first display, and fill "table".
	// Get rid of old, unmounted filesystems...
	memset (table, 0, sizeof (struct disp) * 256);

	machine_get_fs(mnt, &count);

	// Fill the display structure...
	if (count) {
		sock_send_string (sock, "widget_set D err1 30 5 .\n");
		sock_send_string (sock, "widget_set D err2 30 5 .\n");
		for (i = 0; i < count; i++) {
			if (strlen(mnt[i].mpoint) > dev_wid)
				sprintf (table[i].dev, "-%s", (mnt[i].mpoint) + (strlen(mnt[i].mpoint) - (dev_wid - 1)));
			else
				sprintf (table[i].dev, "%s", mnt[i].mpoint);

			//table[i].full = (lcd_cellwid * 4);
			table[i].full = (huge) (lcd_cellwid * gauge_wid)
				 * (huge) (mnt[i].blocks - mnt[i].bfree)
				 / (huge) mnt[i].blocks;

			size = (huge) mnt[i].bsize * (huge) mnt[i].blocks;
			memset (table[i].cap, 0, 8);

			// Kilobytes
			if ((size >= 0) && (size < (huge) 1000 * (huge) 1000))
				sprintf (table[i].cap, "%3.1fk", (double) (size) / 1024.0);
			// Megabytes
			else if ((size >= (huge) 1000 * (huge) 1000) && (size < (huge) 1000 * (huge) 1000 * (huge) 1000))
				sprintf (table[i].cap, "%3.1fM", (float) (size / (huge) 1024) / 1024.0);
			// Gigabytes
			else if ((size >= (huge) 1000 * (huge) 1000 * (huge) 1000) &&
				 (size < (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000))
				sprintf (table[i].cap, "%3.1fG", (float) (size / ((huge) 1024 * (huge) 1024)) / 1024.0);
			// Terabytes
			else if ((size >= (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000) &&
				 (size < (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000))
				sprintf (table[i].cap, "%3.1fT", (float) (size / ((huge) 1024 * (huge) 1024 * (huge) 1024)) / 1024.0);
			// PectaBytes -- Yeah!  I want some!
			else if ((size >= (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000) &&
				 (size < (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000 * (huge) 1000))
				sprintf (table[i].cap, "%3.1fP", (float) (size / ((huge) 1024 * (huge) 1024 * (huge) 1024 * (huge) 1024)) / 1024.0);

		}
	}

	if (!count) {
		sock_send_string (sock, "widget_set D err1 1 2 {Error Retrieving}\n");
		sock_send_string (sock, "widget_set D err2 1 3 {Filesystem Stats}\n");
		return 0;
	}
	// Display stuff...  (show for two seconds, then scroll once per
	//  second, then hold at the end for two seconds)
	sprintf (buffer, "widget_set D f 1 2 %i %i %i %i v 12\n", lcd_wid, lcd_hgt, lcd_wid, count);
	sock_send_string (sock, buffer);
	//sprintf(tmp, "widget_set D f 1 2 20 4 20 %i v 8\n", count);
	//sock_send_string(sock, tmp);
	for (i = 0; i < count; i++) {
		if (table[i].dev[0] == '\0')
			continue;
		if (i >= num_disks)		  // Make sure we have enough lines...
		{
			sprintf (tmp, "widget_add D s%i string -in f\n", i);
			sock_send_string (sock, tmp);
			sprintf (tmp, "widget_add D h%i hbar -in f\n", i);
			sock_send_string (sock, tmp);
		}
		if (lcd_wid >= 20) {			// 20+x columns
			sprintf (tmp, "%-*s %6s E%*sF", dev_wid, table[i].dev, table[i].cap, gauge_wid, "");
			sprintf (buffer, "widget_set D s%i 1 %i {%s}\n", i, i + 1, tmp);
			sock_send_string (sock, buffer);
			sprintf (buffer, "widget_set D h%i %i %i %i\n", i, 10 + dev_wid, i + 1, table[i].full);
			sock_send_string (sock, buffer);
		}
		else {					// < 20 columns
			sprintf (tmp, "%-*s E%*sF", dev_wid, table[i].dev, gauge_wid, "");
			sprintf (buffer, "widget_set D s%i 1 %i {%s}\n", i, i + 1, tmp);
			sock_send_string (sock, buffer);
			sprintf (buffer, "widget_set D h%i %i %i %i\n", i, 3 + dev_wid, i + 1, table[i].full);
			sock_send_string (sock, buffer);
		}
	}

	// Now remove extra widgets...
	for (; i < num_disks; i++) {
		sprintf (tmp, "widget_del D s%i\n", i);
		sock_send_string (sock, tmp);
		sprintf (tmp, "widget_del D h%i\n", i);
		sock_send_string (sock, tmp);
	}

	num_disks = count;

#undef huge

/*    
// ** FILESYSTEMS *****
// /      543.2M E----F
// /dos/c   2.1G E----F
// /stuff   4.3G E----F
*/
	return 0;
}
