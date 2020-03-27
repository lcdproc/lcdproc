/** \file clients/lcdproc/cpu.c
 * Implements the 'CPU' and 'CPUGraph' screens.
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
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "cpu.h"
#include "util.h"


/**
 * CPU screen shows info about percentage of the CPU being used
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## CPU 51.9%: myh #@|	|CPU [----    ]48.1%@|
 * |Usr 46.0% Nice  0.0%|	|U--  S-   N    I--- |
 * |Sys  5.9% Idle 48.1%|	+--------------------+
 * |0%--------      100%|
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
cpu_screen(int rep, int display, int *flags_ptr)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	static double cpu[CPU_BUF_SIZE + 1][5];	/* last buffer is scratch */
	static int gauge_wid = 0;
	static int usni_wid = 0;
	static int us_wid = 0;
	static int ni_wid = 0;

	int i, j;
	double value;
	load_type load;
	char tmp[25];		/* should be large enough */

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add C\n");
		sock_printf(sock, "screen_set C -name {CPU Use: %s}\n", get_hostname());
		if (lcd_hgt >= 4) {
			us_wid = ((lcd_wid + 1) / 2) - 7; /* Usr/Sys label width -7 for " xx.x% " */
			ni_wid = lcd_wid / 2 - 6;       /* Nice/Idle label width -6 for " xx.x%" */

			sock_send_string(sock, "widget_add C title title\n");
			sock_send_string(sock, "widget_set C title {CPU LOAD}\n");
			sock_send_string(sock, "widget_add C one string\n");
			sock_send_string(sock, "widget_add C two string\n");
			sock_printf(sock, "widget_set C one 1 2 {%-*.*s       %-*.*s}\n",
					us_wid, us_wid, "Usr", ni_wid, ni_wid, "Nice");
			sock_printf(sock, "widget_set C two 1 3 {%-*.*s       %-*.*s}\n",
					us_wid, us_wid, "Sys", ni_wid, ni_wid, "Idle");
			sock_send_string(sock, "widget_add C usr string\n");
			sock_send_string(sock, "widget_add C nice string\n");
			sock_send_string(sock, "widget_add C idle string\n");
			sock_send_string(sock, "widget_add C sys string\n");
			pbar_widget_add("C", "bar");
		}
		else {
			usni_wid = lcd_wid / 4;	  /* 4 gauges */
			gauge_wid = lcd_wid - 10; /* room between "CPU " and "99.9%@" */

			sock_send_string(sock, "widget_add C cpu string\n");
			sock_printf(sock, "widget_set C cpu 1 1 {CPU }\n");
			sock_send_string(sock, "widget_add C cpu% string\n");
			sock_printf(sock, "widget_set C cpu%% 1 %d { 0.0%%}\n", lcd_wid - 5);
			pbar_widget_add("C", "usr");
			pbar_widget_add("C", "sys");
			pbar_widget_add("C", "nice");
			pbar_widget_add("C", "idle");
			pbar_widget_add("C", "total");
		}

		return (0);
	}

	machine_get_load(&load);

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

	if (!display)
		return (0);

	if (lcd_hgt >= 4) {	/* 4-line display */
		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][4]);
		sock_printf(sock, "widget_set C title {CPU %5s: %s}\n", tmp, get_hostname());

		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][0]);
		sock_printf(sock, "widget_set C usr %i 2 {%5s}\n", ((lcd_wid + 1) / 2) - 5, tmp);

		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][1]);
		sock_printf(sock, "widget_set C sys %i 3 {%5s}\n", ((lcd_wid + 1) / 2) - 5, tmp);

		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][2]);
		sock_printf(sock, "widget_set C nice %i 2 {%5s}\n", lcd_wid - 4, tmp);

		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][3]);
		sock_printf(sock, "widget_set C idle %i 3 {%5s}\n", lcd_wid - 4, tmp);

		pbar_widget_set("C", "bar", 1, 4, lcd_wid, cpu[CPU_BUF_SIZE][4] * 10, "0%", "100%");
	}
	else {			/* 2-line display */
		sprintf_percent(tmp, cpu[CPU_BUF_SIZE][4]);
		sock_printf(sock, "widget_set C cpu%% %d 1 {%5s}\n", lcd_wid - 5, tmp);

		pbar_widget_set("C", "total", 5, 1, gauge_wid, cpu[CPU_BUF_SIZE][4] * 10, NULL, NULL);
		pbar_widget_set("C", "usr",  1 + 0 * usni_wid, 2, usni_wid, cpu[CPU_BUF_SIZE][0] * 10, "U", NULL);
		pbar_widget_set("C", "sys",  1 + 1 * usni_wid, 2, usni_wid, cpu[CPU_BUF_SIZE][1] * 10, "S", NULL);
		pbar_widget_set("C", "nice", 1 + 2 * usni_wid, 2, usni_wid, cpu[CPU_BUF_SIZE][2] * 10, "N", NULL);
		pbar_widget_set("C", "idle", 1 + 3 * usni_wid, 2, usni_wid, cpu[CPU_BUF_SIZE][3] * 10, "I", NULL);
	}

	return (0);
}				/* End cpu_screen() */



/**
 * Cpu Graph Screen shows a quick-moving histogram of CPU use.
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## CPU: myhost ####@|	|CPU: myhos||       @|
 * |             ||     |	|         ||||       |
 * |            |||     |	+--------------------+
 * |            ||||    |
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
cpu_graph_screen(int rep, int display, int *flags_ptr)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 2
	static double cpu[CPU_BUF_SIZE];
	static int cpu_past[LCD_MAX_WIDTH];
	static int gauge_hgt = 0;

	int i, n = 0;
	double value;
	load_type load;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		gauge_hgt = (lcd_hgt > 2) ? (lcd_hgt - 1) : lcd_hgt;

		sock_send_string(sock, "screen_add G\n");
		sock_printf(sock, "screen_set G -name {CPU Graph: %s}\n", get_hostname());

		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add G title title\n");
			sock_printf(sock, "widget_set G title {CPU: %s}\n", get_hostname());
		}
		else {
			sock_send_string(sock, "widget_add G title string\n");
			sock_printf(sock, "widget_set G title 1 1 {CPU: %s}\n", get_hostname());
		}

		for (i = 1; i <= lcd_wid; i++) {
			sock_printf(sock, "widget_add G bar%d vbar\n", i);
			sock_printf(sock, "widget_set G bar%d %d %d 0\n", i, i, lcd_hgt);
			cpu_past[i - 1] = 0;
		};

		/* Clear out CPU averaging array */
		for (i = 0; i < CPU_BUF_SIZE; i++)
			cpu[i] = 0.;
	}

	/* Shift values over by one */
	for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
		cpu[i] = cpu[i + 1];

	/* Read and save new data */
	machine_get_load(&load);
	cpu[CPU_BUF_SIZE-1] = (load.total > 0L)
			      ? ((double) load.user + (double) load.system + (double) load.nice) / (double) load.total
			      : 0;

	/* Average values for final result */
	value = 0.0;
	for (i = 0; i < CPU_BUF_SIZE; i++)
		value += cpu[i];
	value /= CPU_BUF_SIZE;

	/* Scale result to available height (leave 1st line free when height > 2) */
	n = (int) (value * lcd_cellhgt * gauge_hgt);

	/* Shift and update display the graph */
	for (i = 0; i < lcd_wid - 1; i++) {
		cpu_past[i] = cpu_past[i + 1];

		if (display) {
			sock_printf(sock, "widget_set G bar%d %d %d %d\n",
			              i + 1, i + 1, lcd_hgt, cpu_past[i]);
		}
	}

	/* Save the newest entry and display it */
	cpu_past[lcd_wid - 1] = n;
	if (display) {
		sock_printf(sock, "widget_set G bar%d %d %d %d\n", lcd_wid, lcd_wid, lcd_hgt, n);
	}

	return (0);
}				/* End cpu_graph_screen() */
