/** \file clients/lcdproc/eyebox.c
 * This is the LCDproc client module for EyeboxOne devices
 *
 * This allows to use leds, one as a free CPU meter, and one as a free RAM
 * meter.
 */

/*-
 * Copyright (C) 2006 Cedric TESSIER (aka NeZetiC) http://www.nezetic.info
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "eyebox.h"
#include "util.h"

int
eyebox_screen(char display, int init)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	int i, j;
	double value;
	static double cpu[CPU_BUF_SIZE + 1][5];	/* last buffer is scratch */
	meminfo_type mem[2];
	load_type load;

	if (init == 0) {
		sock_printf(sock, "widget_add %c eyebo_cpu string\n", display);
		sock_printf(sock, "widget_add %c eyebo_mem string\n", display);

		return 0;
	}

	machine_get_load(&load);
	machine_get_meminfo(mem);

	/* Shift values over by one */
	for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
		for (j = 0; j < 5; j++)
			cpu[i][j] = cpu[i + 1][j];

	/* Read new data */
	if (load.total > 0L) {
		cpu[CPU_BUF_SIZE - 1][0] = 100.0 * ((double) load.user / (double) load.total);
		cpu[CPU_BUF_SIZE - 1][1] = 100.0 * ((double) load.system / (double) load.total);
		cpu[CPU_BUF_SIZE - 1][2] = 100.0 * ((double) load.nice / (double) load.total);
		cpu[CPU_BUF_SIZE - 1][3] = 100.0 * ((double) load.idle / (double) load.total);
		cpu[CPU_BUF_SIZE - 1][4] = 100.0 * (((double) load.user + (double) load.system + (double) load.nice) / (double) load.total);
	}
	else {
		cpu[CPU_BUF_SIZE - 1][0] = 0.0;
		cpu[CPU_BUF_SIZE - 1][1] = 0.0;
		cpu[CPU_BUF_SIZE - 1][2] = 0.0;
		cpu[CPU_BUF_SIZE - 1][3] = 0.0;
		cpu[CPU_BUF_SIZE - 1][4] = 0.0;
	}

	/* Average values for final result */
	for (i = 0; i < 5; i++) {
		value = 0.0;
		for (j = 0; j < CPU_BUF_SIZE; j++)
			value += cpu[j][i];
		value /= CPU_BUF_SIZE;
		cpu[CPU_BUF_SIZE][i] = value;
	}

	/*-
	 * /xBab = Use Bar
	 * a = Bar ID
	 * b = Level
	 */
	sock_printf(sock, "widget_set %c eyebo_cpu 1 2 {/xB%d%d}\n",
			display, 2,(int)(cpu[CPU_BUF_SIZE][4]/10));

	/*-
	 * /xBab = Use Bas
	 * a = Bar ID
	 * b = Level
	 */
	value = 1.0 - (double) (mem[0].free + mem[0].buffers + mem[0].cache)
		/ (double) mem[0].total;
	sock_printf(sock, "widget_set %c eyebo_mem 1 3 {/xB%d%d}\n", display, 1, (int) (value * 10));

	return 0;
}

void
eyebox_clear(void)
{
	/* Clear LEDs before exit */
	sock_send_string(sock, "screen_add OFF\n");
	sock_send_string(sock, "screen_set OFF -priority alert -name {EyeBO}\n");
	sock_send_string(sock, "widget_add OFF title title\n");
	sock_send_string(sock, "widget_set OFF title {EYEBOX ONE}\n");
	sock_send_string(sock, "widget_add OFF text string\n");
	sock_send_string(sock, "widget_add OFF about string\n");
	sock_send_string(sock, "widget_add OFF cpu string\n");
	sock_send_string(sock, "widget_add OFF mem string\n");

	sock_send_string(sock, "widget_set OFF text 1 2 {Reseting Leds...}\n");
	sock_send_string(sock, "widget_set OFF about 5 4 {EyeBO by NeZetiC}\n");
	sock_printf(sock, "widget_set OFF cpu 1 2 {/xB%d%d}\n", 2, 0);
	sock_printf(sock, "widget_set OFF mem 1 3 {/xB%d%d}\n", 1, 0);
	usleep(2000000);	/* Wait last order execution */
}

