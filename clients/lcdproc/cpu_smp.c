/******************************************************************************
*
*  cpu_smp.c - dipslay cpu info for multi-processor machines
*  Copyright (C) 2000  J Robert Ray
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
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  ---
* 
*  $Source$
*  $Revision$ $Date$
*  Checked in by: $Author$
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
#include "cpu_smp.h"

#ifdef SOLARIS
#include <sys/types.h>
#include <sys/param.h>
#include <sys/cpuvar.h>
#endif
#ifdef HAVE_LIBKSTAT
#include <kstat.h>
#endif
#ifdef __NetBSD__
#include <sys/param.h>  
#include <sys/sysctl.h>
#include <sys/sched.h>
#include <errno.h>
#endif

#define MAX_CPUS 8

struct load {
	unsigned long total, user, system, nice, idle;
};

#ifdef HAVE_LIBKSTAT
kstat_ctl_t *kc;
#endif

static int load_fd = 0;

static char *numnames[MAX_CPUS] = { "one", "two", "three", "four", "five", "six", "seven", "eight" };

#ifdef __NetBSD__
static void
get_netbsd_load (struct load *result, int *numcpus)
{
	int mib[2], i, num;
	size_t size;
	struct load curr_load;
	u_int64_t cp_time[CPUSTATES];

	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	size = sizeof(int);

	if(sysctl(mib, 2, &num, &size, NULL, 0) < 0)
	{
		fprintf(stderr, "sysctl kern.boottime failed: %s\n", strerror(errno));
	}

	mib[0] = CTL_KERN;
	mib[1] = KERN_CP_TIME; 
	size = sizeof(cp_time);

	if(sysctl(mib, 2, cp_time, &size, NULL, 0) < 0)
	{
		fprintf(stderr, "sysctl kern.cp_time failed: %s\n", strerror(errno));
		curr_load.user = 1;
		curr_load.nice = 1;  
		curr_load.system = 1;
		curr_load.idle = 1;
	}
	else
	{
		curr_load.user = cp_time[CP_USER];
		curr_load.nice = cp_time[CP_NICE];
		curr_load.system = cp_time[CP_SYS] + cp_time[CP_INTR];
		curr_load.idle = cp_time[CP_IDLE];
	}
	curr_load.total = curr_load.user + curr_load.nice + curr_load.system + curr_load.idle;

	*numcpus = num;
	num = num > 8 ? 8 : num;
	/* we can not get per-cpu-load values */
	for(i = 0; i < num; i++)
	{
		result[i] = curr_load;
	}
}
#endif /* __NetBSD__ */

static void
get_load (struct load *result, int *numcpus)
{
#ifdef __NetBSD__
	get_netbsd_load(result, numcpus);
#else
	char *token;
	static struct load last_load[MAX_CPUS];
	struct load curr_load[MAX_CPUS];

#ifndef HAVE_LIBKSTAT
	*numcpus = 0;

	reread (load_fd, "get_load");

	// Look for lines starting with "cpu0", "cpu1", etc.

	token = strtok (buffer, "\n");
	while (token) {
		if (token[0] == 'c') {
			if (isdigit (token[3])) {
				sscanf (token, "%*s %lu %lu %lu %lu", &curr_load[*numcpus].user, &curr_load[*numcpus].nice, &curr_load[*numcpus].system, &curr_load[*numcpus].idle);

				curr_load[*numcpus].total = curr_load[*numcpus].user + curr_load[*numcpus].nice + curr_load[*numcpus].system + curr_load[*numcpus].idle;
				result[*numcpus].total = curr_load[*numcpus].total - last_load[*numcpus].total;
				result[*numcpus].user = curr_load[*numcpus].user - last_load[*numcpus].user;
				result[*numcpus].nice = curr_load[*numcpus].nice - last_load[*numcpus].nice;
				result[*numcpus].system = curr_load[*numcpus].system - last_load[*numcpus].system;
				result[*numcpus].idle = curr_load[*numcpus].idle - last_load[*numcpus].idle;
				last_load[*numcpus].total = curr_load[*numcpus].total;
				last_load[*numcpus].user = curr_load[*numcpus].user;
				last_load[*numcpus].nice = curr_load[*numcpus].nice;
				last_load[*numcpus].system = curr_load[*numcpus].system;
				last_load[*numcpus].idle = curr_load[*numcpus].idle;

				(*numcpus)++;
			}
		} else {
			break;
		}
		token = strtok (NULL, "\n");
	}
#else
	kstat_t	*k_space;
	void    *val_ptr;
	int		ncpu=0,count;
	char	buffer[10];

	*numcpus=sysconf(_SC_NPROCESSORS_CONF);
	for (count = 0; count < MAX_CPUS;count++) {
		sprintf (buffer, "cpu_stat%d",count);
		k_space = kstat_lookup(kc, "cpu_stat", count, buffer);
		if (k_space == NULL) {
		} else if (kstat_read(kc, k_space, NULL) == -1) {
		} else  {
			struct cpu_stat cinfo;
			k_space=kstat_lookup(kc, "cpu_stat", -1, buffer);
			kstat_read(kc,k_space,&cinfo);
			curr_load[ncpu].idle=cinfo.cpu_sysinfo.cpu[CPU_IDLE];
			curr_load[ncpu].user=cinfo.cpu_sysinfo.cpu[CPU_USER];
			curr_load[ncpu].system=cinfo.cpu_sysinfo.cpu[CPU_KERNEL];
			curr_load[ncpu].nice=cinfo.cpu_sysinfo.cpu[CPU_WAIT];
			curr_load[ncpu].nice=0;
			curr_load[ncpu].total = curr_load[ncpu].user + curr_load[ncpu].nice + curr_load[ncpu].system + curr_load[ncpu].idle;
			result[ncpu].total = curr_load[ncpu].total - last_load[ncpu].total;
			result[ncpu].user = curr_load[ncpu].user - last_load[ncpu].user;
			result[ncpu].nice = curr_load[ncpu].nice - last_load[ncpu].nice;
			result[ncpu].system = curr_load[ncpu].system - last_load[ncpu].system;
			result[ncpu].idle = curr_load[ncpu].idle - last_load[ncpu].idle;
			last_load[ncpu].total = curr_load[ncpu].total;
			last_load[ncpu].user = curr_load[ncpu].user;
			last_load[ncpu].nice = curr_load[ncpu].nice;
			last_load[ncpu].system = curr_load[ncpu].system;
			last_load[ncpu].idle = curr_load[ncpu].idle;
			ncpu=ncpu++;
		}
	}
#endif
#endif /* __NetBSD__ */
}

int
cpu_smp_init ()
{
#ifndef __NetBSD__
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
#endif /* __NetBSD__ */

	return 0;
}

int
cpu_smp_close ()
{
#ifndef __NetBSD__
#ifndef HAVE_LIBKSTAT
	if (load_fd)
		close (load_fd);
#else
	kstat_close(kc);
#endif

	load_fd = 0;

#endif /* __NetBSD__ */
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CPU screen shows info about percentage of the CPU being used
//
int
cpu_smp_screen (int rep, int display)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	int i, j, n, z;
	static int first = 1;
	float value;
	static float cpu[MAX_CPUS][CPU_BUF_SIZE + 1][5];	// last buffer is scratch
	struct load load[MAX_CPUS];
	int numprocs;
	char buf[256];
	char *graphsize;

	if (first) {
		first = 0;

		get_load (load, &numprocs);

		sock_send_string (sock, "screen_add P\n");
		sock_send_string (sock, "widget_del P heartbeat\n");

		sprintf (buffer, "screen_set P -name {CPU Use: %s}\n", host);
		sock_send_string (sock, buffer);

		graphsize = calloc (sizeof (char), lcd_wid);
		if (numprocs > lcd_hgt) {
			for (i = 0; i < lcd_wid / 2 - 6; i++) {
				graphsize[i] = ' ';
			}
		} else {
			for (i = 0; i < lcd_wid - 6; i++) {
				graphsize[i] = ' ';
			}
		}

		for (i = 0; i < numprocs; i++) {
			if (i + 1 > lcd_hgt * 2)
				break;

			sprintf (buf, "widget_add P %s_title string\n", numnames[i]);
			sock_send_string (sock, buf);

			if (numprocs > lcd_hgt) {
				sprintf (buf, "widget_set P %s_title %d %d \"CPU%d[%s]\"\n", numnames[i], i % 2 * lcd_wid / 2 + 1, i / 2 + 1, i, graphsize);
			} else {
				sprintf (buf, "widget_set P %s_title 1 %d \"CPU%d[%s]\"\n", numnames[i], i + 1, i, graphsize);
			}
			sock_send_string (sock, buf);

			sprintf (buf, "widget_add P %s_bar hbar\n", numnames[i]);
			sock_send_string (sock, buf);
		}

		free (graphsize);

		return 0;
	}

	get_load (load, &numprocs);

	for (z = 0; z < numprocs; z++) {

		// Shift values over by one
		for (i = 0; i < (CPU_BUF_SIZE - 1); i++)
			for (j = 4; j < 5; j++)
				cpu[z][i][j] = cpu[z][i + 1][j];

		if (load[z].total == 0) {
			continue;
		}
		// Read new data
#if 0									  // we're only using 4 right now
		cpu[z][CPU_BUF_SIZE - 1][0] = ((float) load[z].user / (float) load[z].total) * 100.0;
		cpu[z][CPU_BUF_SIZE - 1][1] = ((float) load[z].system / (float) load[z].total) * 100.0;
		cpu[z][CPU_BUF_SIZE - 1][2] = ((float) load[z].nice / (float) load[z].total) * 100.0;
		cpu[z][CPU_BUF_SIZE - 1][3] = ((float) load[z].idle / (float) load[z].total) * 100.0;
#endif
		cpu[z][CPU_BUF_SIZE - 1][4] = (((float) load[z].user + (float) load[z].system + (float) load[z].nice) / (float) load[z].total) * 100.0;

		// Average values for final result
		for (i = 4; i < 5; i++) {
			value = 0;
			for (j = 0; j < CPU_BUF_SIZE; j++) {
				value += cpu[z][j][i];
			}
			value /= CPU_BUF_SIZE;
			cpu[z][CPU_BUF_SIZE][i] = value;
		}

		value = cpu[z][CPU_BUF_SIZE][4];

		if (numprocs > lcd_hgt) {
			n = (float) (value * lcd_cellwid * ((float) lcd_wid / 2.f - 6.f)) / 100.f + .5f;
			sprintf (buf, "widget_set P %s_bar %d %d %d\n", numnames[z], z % 2 * lcd_wid / 2 + 6, z / 2 + 1, n);
		} else {
			n = (float) (value * lcd_cellwid * ((float) lcd_wid - 6.f)) / 100.f + .5f;
			sprintf (buf, "widget_set P %s_bar 6 %d %d\n", numnames[z], z + 1, n);
		}
		sock_send_string (sock, buf);

		if (z + 1 > lcd_hgt * 2)
			break;
	}

	return 0;
}										  // End cpu_screen()
