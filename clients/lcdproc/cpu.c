#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "../../shared/sockets.h"
#include "../../shared/debug.h"

#include "main.h"
#include "mode.h"
#include "cpu.h"

#ifdef SOLARIS
#include <sys/types.h>
#include <sys/param.h>
#include <sys/cpuvar.h>
#endif

#ifdef HAVE_LIBKSTAT
#include <kstat.h>
#endif

struct load {
	unsigned long total, user, system, nice, idle;
};

#ifdef HAVE_LIBKSTAT
kstat_ctl_t *kc;
#endif

int load_fd = 0;
static void get_load (struct load *result);

static void
get_load (struct load *result)
{
	static struct load last_load = { 0, 0, 0, 0, 0 };
	struct load curr_load;

#ifndef HAVE_LIBKSTAT
	reread (load_fd, "get_load:");
	sscanf (buffer, "%*s %lu %lu %lu %lu\n", &curr_load.user, &curr_load.nice, &curr_load.system, &curr_load.idle);
#else
	kstat_t *k_space;
	void	*val_ptr;
	k_space = kstat_lookup(kc, "cpu_stat", 0, "cpu_stat0");
	if (k_space == NULL) {
		printf("\nkstat lookup error");
		exit(1);
	} else if (kstat_read(kc, k_space, NULL) == -1) {
		printf("\nkstat read error");
		exit(1);
	}
	k_space=kstat_lookup(kc, "cpu_stat", -1, "cpu_stat0");
	{
		struct cpu_stat cinfo;
		kstat_read(kc,k_space,&cinfo);
		curr_load.idle=cinfo.cpu_sysinfo.cpu[CPU_IDLE];
		curr_load.user=cinfo.cpu_sysinfo.cpu[CPU_USER];
		curr_load.system=cinfo.cpu_sysinfo.cpu[CPU_KERNEL];
		curr_load.nice=cinfo.cpu_sysinfo.cpu[CPU_WAIT];
	}
	curr_load.nice=0;
#endif
	curr_load.total = curr_load.user + curr_load.nice + curr_load.system + curr_load.idle;
	result->total = curr_load.total - last_load.total;
	result->user = curr_load.user - last_load.user;
	result->nice = curr_load.nice - last_load.nice;
	result->system = curr_load.system - last_load.system;
	result->idle = curr_load.idle - last_load.idle;
	last_load.total = curr_load.total;
	last_load.user = curr_load.user;
	last_load.nice = curr_load.nice;
	last_load.system = curr_load.system;
	last_load.idle = curr_load.idle;
}

int
cpu_init ()
{
#ifndef HAVE_LIBKSTAT
	if (!load_fd) {
		load_fd = open ("/proc/stat", O_RDONLY);
	}
#else
	kc = kstat_open();
	if (kc == NULL) {
		exit(1);
	}
#endif


	return 0;
}

int
cpu_close ()
{
#ifndef HAVE_LIBKSTAT
	if (load_fd)
		close (load_fd);
#else
	kstat_close(kc);
#endif

	load_fd = 0;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CPU screen shows info about percentage of the CPU being used
//
int
cpu_screen (int rep, int display)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	int i, j, n;
	static int first = 1;
	float value;
	static float cpu[CPU_BUF_SIZE + 1][5];	// last buffer is scratch
	struct load load;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add C\n");
		sprintf (buffer, "screen_set C -name {CPU Use: %s}\n", host);
		sock_send_string (sock, buffer);
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add C title title\n");
			sock_send_string (sock, "widget_set C title {CPU LOAD}\n");
			sock_send_string (sock, "widget_add C one string\n");
			sock_send_string (sock, "widget_set C one 1 2 {0        1 Sys  0.0%}\n");
			sock_send_string (sock, "widget_add C two string\n");
			sock_send_string (sock, "widget_add C three string\n");
			sock_send_string (sock, "widget_set C one 1 2 {Usr  0.0% Nice  0.0%}\n");
			sock_send_string (sock, "widget_set C two 1 3 {Sys  0.0% Idle  0.0%}\n");
			sock_send_string (sock, "widget_set C three 1 4 {0%              100%}\n");
			sock_send_string (sock, "widget_add C usr string\n");
			sock_send_string (sock, "widget_add C nice string\n");
			sock_send_string (sock, "widget_add C idle string\n");
			sock_send_string (sock, "widget_add C sys string\n");
			sock_send_string (sock, "widget_add C bar hbar\n");
			sock_send_string (sock, "widget_set C bar 3 4 0\n");
		} else {
			sock_send_string (sock, "widget_add C cpu0 string\n");
			if (lcd_wid >= 20)
				sock_send_string (sock, "widget_set C cpu0 1 1 {CPU0[         ]}\n");
			else
				sock_send_string (sock, "widget_set C cpu0 1 1 {CPU0[     ]}\n");
			sock_send_string (sock, "widget_add C cpu0% string\n");
			sprintf (buffer, "widget_set C cpu0%% 1 %i { 0.0%%}\n", lcd_wid - 4);
			sock_send_string (sock, buffer);
			sock_send_string (sock, "widget_add C usni string\n");
			sock_send_string (sock, "widget_set C usni 1 2 {    U    S    N    I}\n");
			sock_send_string (sock, "widget_add C usr hbar\n");
			sock_send_string (sock, "widget_add C sys hbar\n");
			sock_send_string (sock, "widget_add C nice hbar\n");
			sock_send_string (sock, "widget_add C idle hbar\n");
			sock_send_string (sock, "widget_add C total hbar\n");
			sock_send_string (sock, "widget_set C total 6 1 0\n");
		}

		return 0;
	}
	//else return 0;

	get_load (&load);

	// Shift values over by one
	for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
		for (j = 0; j < 5; j++)
			cpu[i][j] = cpu[i + 1][j];

	// Read new data
	cpu[CPU_BUF_SIZE - 1][0] = ((float) load.user / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][1] = ((float) load.system / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][2] = ((float) load.nice / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][3] = ((float) load.idle / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][4] = (((float) load.user + (float) load.system + (float) load.nice) / (float) load.total) * 100.0;

	// Only clear on first display...
	if (!rep) {
		/*
		   // Make all the same, if this is the first time...
		   for(i=0; i<CPU_BUF_SIZE-1; i++)
		   for(j=0; j<5; j++)
		   cpu[i][j] = cpu[CPU_BUF_SIZE-1][j];
		 */
	}
	// Average values for final result
	for (i = 0; i < 5; i++) {
		value = 0;
		for (j = 0; j < CPU_BUF_SIZE; j++) {
			value += cpu[j][i];
		}
		value /= CPU_BUF_SIZE;
		cpu[CPU_BUF_SIZE][i] = value;
	}

	if (lcd_hgt >= 4) {
		value = cpu[CPU_BUF_SIZE][4];
		n = (int) (value * 70.0);
		if (value >= 99.9) {
			sprintf (tmp, "100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C title {CPU %s: %s}\n", tmp, host);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][0];
		if (value >= 99.9) {
			sprintf (tmp, " 100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C usr 5 2 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][1];
		if (value >= 99.9) {
			sprintf (tmp, " 100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C sys 5 3 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][2];
		if (value >= 99.9) {
			sprintf (tmp, " 100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C nice 16 2 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][3];
		if (value >= 99.9) {
			sprintf (tmp, " 100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C idle 16 3 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][4];
		n = (int) (value * (lcd_cellwid * 14) / 100.0);
		sprintf (buffer, "widget_set C bar 3 4 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);
	}									  // end if(lcd_hgt >= 4)
	else								  // 20x2 version
	{
		value = cpu[CPU_BUF_SIZE][4];
		if (value >= 99.9) {
			sprintf (tmp, "100%%");
		} else {
			sprintf (tmp, "%4.1f%%", value);
		}
		sprintf (buffer, "widget_set C cpu0%% %i 1 {%s}\n", lcd_wid - 4, tmp);
		if (display)
			sock_send_string (sock, buffer);

		n = (float) (value * lcd_cellwid * (float) (lcd_wid - 11)) / 100.0;
		sprintf (buffer, "widget_set C total 6 1 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][0];
		n = (float) (value * lcd_cellwid * 4.0) / 100.0;
		sprintf (buffer, "widget_set C usr 1 2 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][1];
		n = (float) (value * lcd_cellwid * 3.0) / 100.0;
		sprintf (buffer, "widget_set C sys 7 2 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][2];
		n = (float) (value * lcd_cellwid * 3.0) / 100.0;
		sprintf (buffer, "widget_set C nice 12 2 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);

		value = cpu[CPU_BUF_SIZE][3];
		n = (float) (value * lcd_cellwid * 3.0) / 100.0;
		sprintf (buffer, "widget_set C idle 17 2 %i\n", n);
		if (display)
			sock_send_string (sock, buffer);

	}

	return 0;
}										  // End cpu_screen()

//////////////////////////////////////////////////////////////////////////
// Cpu Graph Screen shows a quick-moving histogram of CPU use.
//
int
cpu_graph_screen (int rep, int display)
{
	int i, j, n = 0;
	static int first = 1;
	float value, maxload;
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 2
	static float cpu[CPU_BUF_SIZE + 1];	// last buffer is scratch
	static int cpu_past[LCD_MAX_WIDTH];
	struct load load;
	int status = 0;

	if (first) {
		first = 0;
		sock_send_string (sock, "screen_add G\n");
		sprintf (buffer, "screen_set G -name {CPU Graph: %s}\n", host);
		sock_send_string (sock, buffer);
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add G title title\n");
			sprintf (buffer, "widget_set G title {CPU: %s}\n", host);
		} else {
			sock_send_string (sock, "widget_add G title string\n");
			sprintf (buffer, "widget_set G title 1 1 {CPU: %s}\n", host);
		}
		sock_send_string (sock, buffer);

		for (i = 1; i <= lcd_wid; i++) {
			sprintf (tmp, "widget_add G %i vbar\n", i);
			sock_send_string (sock, tmp);
			sprintf (tmp, "widget_set G %i %i %i 0\n", i, i, lcd_hgt);
			sock_send_string (sock, tmp);
		}
	}

	get_load (&load);

	// Shift values over by one
	for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
		cpu[i] = cpu[i + 1];

	// Read new data
	cpu[CPU_BUF_SIZE - 1] = ((float) load.user + (float) load.system + (float) load.nice) / (float) load.total;

	// Only clear on first display...
	if (!rep) {
		/*
		   // Make all the same, if this is the first time...
		   for(i=0; i<CPU_BUF_SIZE-1; i++)
		   cpu[i] = cpu[CPU_BUF_SIZE-1];
		 */
	}
	// Average values for final result
	value = 0;
	for (j = 0; j < CPU_BUF_SIZE; j++) {
		value += cpu[j];
	}
	value /= (float) CPU_BUF_SIZE;
	cpu[CPU_BUF_SIZE] = value;

	maxload = 0;
	for (i = 0; i < lcd_wid - 1; i++) {
		cpu_past[i] = cpu_past[i + 1];

		j = cpu_past[i];
		sprintf (tmp, "widget_set G %i %i %i %i\n", i + 1, i + 1, lcd_hgt, j);
		if (display)
			sock_send_string (sock, tmp);

		if (cpu_past[i] > maxload)
			maxload = cpu_past[i];
	}

	value = cpu[CPU_BUF_SIZE];
	if (lcd_hgt > 2)
		n = (int) (value * (float) (lcd_cellhgt) * (float) (lcd_hgt - 1));
	else
		n = (int) (value * (float) (lcd_cellhgt) * (float) (lcd_hgt));

	cpu_past[lcd_wid - 1] = n;
	sprintf (tmp, "widget_set G %i %i %i %i\n", lcd_wid, lcd_wid, lcd_hgt, n);
	if (display)
		sock_send_string (sock, tmp);

	if (n > maxload)
		maxload = n;

	if (cpu_past[lcd_wid - 1] > 0)
		status = BACKLIGHT_ON;
	if (maxload < 1)
		status = BACKLIGHT_OFF;

//  return status;
	return 0;
}										  // End cpu_graph_screen()
