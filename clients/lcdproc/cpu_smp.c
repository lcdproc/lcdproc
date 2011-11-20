/** \file clients/lcdproc/cpu_smp.c
 * Display cpu info for multi-processor machines.
 *
 * Adapted from cpu.c.
 *
 * Really boring for the moment, it only shows a current usage percentage graph
 * for each CPU.
 *
 * It will handle up to 2xlcd_hgt or 8 CPUs, whichever is less.  If there are
 * more CPUs than lines on the LCD, it puts 2 CPUs per line, splitting the line
 * in half.  Otherwise, it uses one line per CPU.
 *
 * If the number of lines used to display the bar graphs for the CPUs is smaller
 * than the LCD's height, a title line is introduced, so that the screen looks
 * similar to other lcdproc screens.
 * In all other cases (i.e. \#CPUs == LCD height or \#CPUs >= 2 * LCD height),
 * the title is left out to display as many CPUs graphs as possible.
 */

/*-
 * Copyright (C) 2000       J Robert Ray
 * Copyright (C) 2006,2007  Peter Marschall
 *
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "cpu_smp.h"


/**
 * CPU screen shows info about percentage of the CPU being used
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
cpu_smp_screen (int rep, int display, int *flags_ptr)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	int z;
	static float cpu[MAX_CPUS][CPU_BUF_SIZE + 1];	/* last buffer is scratch */
	load_type load[MAX_CPUS];
	int num_cpus = MAX_CPUS;
	int bar_size;
	int lines_used;

	/* get SMP load - inform about max #CPUs allowed */
	machine_get_smpload(load, &num_cpus);

	/* restrict num_cpus to max. twice the display height */
	if (num_cpus > 2 * lcd_hgt)
		num_cpus = 2 * lcd_hgt;

	/* 2 CPUs per line if more CPUs than lines */
	bar_size = (num_cpus > lcd_hgt) ? (lcd_wid / 2 - 6) : (lcd_wid - 6);
	lines_used = (num_cpus > lcd_hgt) ? (num_cpus + 1) / 2 : num_cpus;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add P\n");

		/* print title if he have room for it */
		if (lines_used < lcd_hgt) {
			sock_send_string(sock, "widget_add P title title\n");
			sock_printf(sock, "widget_set P title {SMP CPU %s}\n", get_hostname());
		}
		else {
			sock_send_string(sock, "screen_set P -heartbeat off\n");
		}

		sock_printf(sock, "screen_set P -name {CPU Use: %s}\n", get_hostname());

		for (z = 0; z < num_cpus; z++) {
			int y_offs = (lines_used < lcd_hgt) ? 2 : 1;
			int x = (num_cpus > lcd_hgt) ? ((z % 2) * (lcd_wid/2) + 1) : 1;
			int y = (num_cpus > lcd_hgt) ? (z/2 + y_offs) : (z + y_offs);

			sock_printf(sock, "widget_add P cpu%d_title string\n", z);
			sock_printf(sock, "widget_set P cpu%d_title %d %d \"CPU%d[%*s]\"\n",
					z, x, y, z, bar_size, "");
			sock_printf(sock, "widget_add P cpu%d_bar hbar\n", z);
		}

		return 0;
	}

	for (z = 0; z < num_cpus; z++) {
		int y_offs = (lines_used < lcd_hgt) ? 2 : 1;
		int x = (num_cpus > lcd_hgt) ? ((z % 2) * (lcd_wid/2) + 6) : 6;
		int y = (num_cpus > lcd_hgt) ? (z/2 + y_offs) : (z + y_offs);
		float value = 0.0;
		int i, n;

		/* Shift values over by one */
		for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
			cpu[z][i] = cpu[z][i + 1];

		/* Read new data */
		cpu[z][CPU_BUF_SIZE-1] = (load[z].total > 0L)
				    ? (((float) load[z].user + (float) load[z].system + (float) load[z].nice) / (float) load[z].total) * 100.0
				    : 0.0;

		/* Average values for final result */
		for (i = 0; i < CPU_BUF_SIZE; i++) {
			value += cpu[z][i];
		}
		value /= CPU_BUF_SIZE;

		n = (int) ((value * lcd_cellwid * bar_size) / 100.0 + 0.5);
		sock_printf(sock, "widget_set P cpu%d_bar %d %d %d\n", z, x, y, n);
	}

	return 0;
}
