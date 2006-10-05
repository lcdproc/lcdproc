/******************************************************************************
*
*  cpu_smp.c - dipslay cpu info for multi-processor machines
*  Copyright (C) 2000  J Robert Ray
*  Copyright (C) 2006  Peter Marschall
*
*  Adapted from cpu.c.
*
*  Really boring for the moment, it only shows a current usage percentage graph
*  for each CPU.
*
*  It will handle up to 2xlcd_hgt or 8 CPUs, whichever is less.  If there are
*  more CPUs than lines on the LCD, it puts 2 CPUs per line, splitting the line
*  in half.  Otherwise, it uses one line per CPU.
*
*  If you have a four-line display and a dual-processor machine, two lines will
*  go unused.  This would be a good spot for a histogram or other info.  Also,
*  it could always display two CPUs per line to provide extra space for other
*  info, when there otherwise wouldn't be extra space (e.g., on a two proc-
*  machine, with a two-line LCD [this is my setup]).
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
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "shared/sockets.h"
#include "shared/debug.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "cpu_smp.h"

#define MAX_CPUS 16

//////////////////////////////////////////////////////////////////////////
// CPU screen shows info about percentage of the CPU being used
//
int
cpu_smp_screen (int rep, int display, int *flags_ptr)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	int z;
	static float cpu[MAX_CPUS][CPU_BUF_SIZE + 1];	// last buffer is scratch
	load_type load[MAX_CPUS];
	int num_cpus;
	int bar_size;

	machine_get_smpload(load, &num_cpus);

	bar_size = (num_cpus > lcd_hgt) ? (lcd_wid / 2 - 6) : (lcd_wid - 6);

	// restrict num_cpus to max. twic display height
	if (num_cpus > 2 * lcd_hgt)
		num_cpus = 2 * lcd_hgt;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add P\n");
		sock_send_string(sock, "screen_set P -heartbeat off\n");

		sock_printf(sock, "screen_set P -name {CPU Use: %s}\n", get_hostname());

		for (z = 0; z < num_cpus; z++) {
			int x = (num_cpus > lcd_hgt) ? ((z % 2) * (lcd_wid/2) + 1) : 1;
			int y = (num_cpus > lcd_hgt) ? (z/2 + 1) : (z+1);

			sock_printf(sock, "widget_add P cpu%d_title string\n", z);
			sock_printf(sock, "widget_set P cpu%d_title %d %d \"CPU%d[%*s]\"\n",
					z, x, y, z, bar_size, "");
			sock_printf(sock, "widget_add P cpu%d_bar hbar\n", z);
		}

		return 0;
	}

	for (z = 0; z < num_cpus; z++) {
		int x = (num_cpus > lcd_hgt) ? ((z % 2) * (lcd_wid/2) + 6) : 6;
		int y = (num_cpus > lcd_hgt) ? (z/2 + 1) : (z+1);
		float value = 0.0;
		int i, n;

		// Shift values over by one
		for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
			cpu[z][i] = cpu[z][i + 1];

		// Read new data
		cpu[z][CPU_BUF_SIZE-1] = (load[z].total > 0L)
				    ? (((float) load[z].user + (float) load[z].system + (float) load[z].nice) / (float) load[z].total) * 100.0
				    : 0.0;

		// Average values for final result
		for (i = 0; i < CPU_BUF_SIZE; i++) {
			value += cpu[z][i];
		}
		value /= CPU_BUF_SIZE;

		n = (int) ((value * lcd_cellwid * bar_size) / 100.0 + 0.5);
		sock_printf(sock, "widget_set P cpu%d_bar %d %d %d\n", z, x, y, n);
	}

	return 0;
}										  // End cpu_screen()
