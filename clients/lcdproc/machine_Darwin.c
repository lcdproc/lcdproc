/** \file clients/lcdproc/machine_Darwin.c
 * Collects system information on MacOS / Darwin.
 */

/*-
 * Copyright (c) 2003 Thomas Runge (coto@core.de)
 *
 * Mach and Darwin specific code is:
 * Copyright (c) 2006 Eric Pooch (epooch@tenon.com)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __APPLE__

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/dkstat.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/user.h>

#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <mach/mach.h>

#include "main.h"
#include "machine.h"
#include "config.h"
#include "shared/LL.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

static int pageshift;
#define pagetok(size) ((size) << pageshift)
static int swapmode(int *rettotal, int *retfree);

static mach_port_t lcdproc_port;


int
machine_init(void)
{
	/*
	 * get the page size with "getpagesize" and calculate pageshift from
	 * it
	 */
	unsigned int pagesize = 0;
	pageshift = 0;

	lcdproc_port = mach_host_self();
	host_page_size(lcdproc_port, &pagesize);

	while (pagesize > 1) {
		pageshift++;
		pagesize >>= 1;
	}

	/* we only need the amount of log(2)1024 for our conversion */
	pageshift -= 10;

	return (TRUE);
}

int
machine_close(void)
{
	return (TRUE);
}

int
machine_get_battstat(int *acstat, int *battflag, int *percent)
{
	CFTypeRef blob = IOPSCopyPowerSourcesInfo();
	CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

	int i;
	CFDictionaryRef pSource = NULL;
	const void *psValue;

	*acstat = LCDP_AC_ON;
	*battflag = LCDP_BATT_ABSENT;
	*percent = 100;

	if (CFArrayGetCount(sources) == 0)
		return (FALSE);

	for (i = 0; i < CFArrayGetCount(sources); i++) {
		pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
		if (!pSource)
			break;

		psValue = (CFStringRef) CFDictionaryGetValue(pSource, CFSTR(kIOPSNameKey));

		if (CFDictionaryGetValueIfPresent(pSource, CFSTR(kIOPSIsPresentKey), &psValue) && (CFBooleanGetValue(psValue) > 0)) {
			psValue = (CFStringRef) CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));

			if (CFStringCompare(psValue, CFSTR(kIOPSBatteryPowerValue), 0) == kCFCompareEqualTo) {
				/* We are running on a battery power source. */
				*battflag = LCDP_BATT_UNKNOWN;
				*acstat = LCDP_AC_OFF;
			}
			else if (CFDictionaryGetValueIfPresent(pSource, CFSTR(kIOPSIsChargingKey), &psValue)) {
				/*
				 * We are running on an AC power source, but
				 * we also have a battery power source
				 * present.
				 */

				if (CFBooleanGetValue(psValue) > 0)
					*battflag = LCDP_BATT_CHARGING;
				else
					*battflag = LCDP_BATT_UNKNOWN;
			}

			if (*battflag != LCDP_BATT_ABSENT) {
				int curCapacity = 0;
				int maxCapacity = 0;

				psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSCurrentCapacityKey));
				CFNumberGetValue(psValue, kCFNumberSInt32Type, &curCapacity);

				psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSMaxCapacityKey));
				CFNumberGetValue(psValue, kCFNumberSInt32Type, &maxCapacity);

				*percent = (int)((double)curCapacity / (double)maxCapacity * 100);

				/*
				 * There is a way to check this through the
				 * IOKit, but I am not sure what gets
				 * returned for kIOPSLowWarnLevelKey and
				 * kIOPSDeadWarnLevelKey, and this is easier.
				 */
				if (*battflag == LCDP_BATT_UNKNOWN) {
					if (*percent > 50)
						*battflag = LCDP_BATT_HIGH;
					else if (*percent > 2)
						*battflag = LCDP_BATT_LOW;
					else
						*battflag = LCDP_BATT_CRITICAL;
				}
				/* printf ("powerSource %d of %d: percent: %d/%d %d\n", i, CFArrayGetCount(sources), curCapacity, maxCapacity, *percent);*/
			}
		}
	}

	CFRelease(blob);
	CFRelease(sources);

	return (TRUE);
}

int
machine_get_fs(mounts_type fs[], int *cnt)
{
	struct statfs *mntbuf;
	struct statfs *pp;
	int statcnt, fscnt, i;

	fscnt = getmntinfo(&mntbuf, MNT_WAIT);
	if (fscnt == 0) {
		perror("getmntinfo");
		return (FALSE);
	}
	for (statcnt = 0, pp = mntbuf, i = 0; i < fscnt; pp++, i++) {
		if (strcmp(pp->f_fstypename, "procfs")
		    && strcmp(pp->f_fstypename, "kernfs")
		    && strcmp(pp->f_fstypename, "linprocfs")
#ifndef STAT_NFS
		    && strcmp(pp->f_fstypename, "nfs")
#endif
#ifndef STAT_SMBFS
		    && strcmp(pp->f_fstypename, "smbfs")
#endif
			) {
			snprintf(fs[statcnt].dev, 255, "%s", pp->f_mntfromname);
			snprintf(fs[statcnt].mpoint, 255, "%s", pp->f_mntonname);
			snprintf(fs[statcnt].type, 63, "%s", pp->f_fstypename);

			fs[statcnt].blocks = pp->f_blocks;
			if (fs[statcnt].blocks > 0) {
				fs[statcnt].bsize = pp->f_bsize;
				fs[statcnt].bfree = pp->f_bfree;
				fs[statcnt].files = pp->f_files;
				fs[statcnt].ffree = pp->f_ffree;
			}
			statcnt++;
		}
	}

	*cnt = statcnt;

	return (TRUE);
}

int
machine_get_load(load_type * curr_load)
{
	static load_type last_load = {0, 0, 0, 0, 0};
	static load_type last_ret_load;
	load_type load;

	host_cpu_load_info_data_t load_info;
	mach_msg_type_number_t info_count;

	info_count = HOST_CPU_LOAD_INFO_COUNT;
	if (host_statistics(lcdproc_port, HOST_CPU_LOAD_INFO, (host_info_t) & load_info, &info_count)) {
		perror("host_statistics");
		return (FALSE);
	}

	load.user = (unsigned long)(load_info.cpu_ticks[CPU_STATE_USER]);
	load.nice = (unsigned long)(load_info.cpu_ticks[CPU_STATE_NICE]);
	load.system = (unsigned long)(load_info.cpu_ticks[CPU_STATE_SYSTEM]);
	load.idle = (unsigned long)(load_info.cpu_ticks[CPU_STATE_IDLE]);
	load.total = load.user + load.nice + load.system + load.idle;

	if (load.total != last_load.total) {
		curr_load->user = load.user - last_load.user;
		curr_load->nice = load.nice - last_load.nice;
		curr_load->system = load.system - last_load.system;
		curr_load->idle = load.idle - last_load.idle;
		curr_load->total = load.total - last_load.total;
		last_ret_load = *curr_load;
		last_load = load;
	}
	else {
		*curr_load = last_ret_load;
	}

	return TRUE;
}


int
machine_get_loadavg(double *load)
{
	double loadavg[LOADAVG_NSTATS];

	if (getloadavg(loadavg, LOADAVG_NSTATS) <= LOADAVG_1MIN)
		return (FALSE);

	*load = loadavg[LOADAVG_1MIN];

	return (TRUE);
}

int
machine_get_meminfo(meminfo_type * result)
{
	int total_pages, free_pages;

	vm_statistics_data_t vm_info;
	mach_msg_type_number_t info_count;

	info_count = HOST_VM_INFO_COUNT;
	if (host_statistics(lcdproc_port, HOST_VM_INFO, (host_info_t) & vm_info, &info_count)) {
		perror("host_statistics");
		return (FALSE);
	}

	result[0].total = pagetok(vm_info.active_count + vm_info.inactive_count +
				  vm_info.free_count + vm_info.wire_count);
	result[0].free = pagetok(vm_info.free_count);
	result[0].buffers = 0;
	result[0].cache = 0;
	result[0].shared = 0;

	/* swap */
	result[1].total = 0;
	result[1].free = 0;

	if (swapmode(&total_pages, &free_pages) != -1) {
		result[1].total = total_pages;
		result[1].free = free_pages;
	}

	return (TRUE);
}

int
machine_get_procs(LinkedList * procs)
{
	struct kinfo_proc *kprocs;

	procinfo_type *p;
	int nproc, i;
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};

	size_t size = 0;

	/* Call sysctl with a NULL buffer as a dry run. */
	if (sysctl(mib, 4, NULL, &size, NULL, 0) < 0) {
		perror("Failure calling sysctl");
		return FALSE;
	}
	/* Allocate a buffer based on previous results of sysctl. */
	kprocs = (struct kinfo_proc *)alloca(size);
	if (kprocs == NULL) {
		perror("mem_alloca");
		return FALSE;
	}
	/* Call sysctl again with the new buffer. */
	if (sysctl(mib, 4, kprocs, &size, NULL, 0) < 0) {
		perror("Failure calling sysctl");
		return FALSE;
	}

	nproc = size / sizeof(struct kinfo_proc);

	for (i = 0; i < nproc; i++, kprocs++) {
		mach_port_t task;
		unsigned int status = kprocs->kp_proc.p_stat;

		if (status == SIDL || status == SZOMB)
			continue;

		p = malloc(sizeof(procinfo_type));
		if (!p) {
			perror("mem_malloc");
			continue;
		}
		strncpy(p->name, kprocs->kp_proc.p_comm, 15);
		p->name[15] = '\0';

		p->number = kprocs->kp_proc.p_pid;

		LL_Push(procs, (void *)p);

		/* Normal user can't get tasks for processes owned by root. */
		if (kprocs->kp_eproc.e_pcred.p_ruid == 0)
			continue;

		/* Get the memory data for each pid from Mach. */
		if (task_for_pid(mach_task_self(), kprocs->kp_proc.p_pid, &task) == KERN_SUCCESS) {
			task_basic_info_data_t info;
			mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;

			if (task_info(task, TASK_BASIC_INFO, (task_info_t) & info, &count) == KERN_SUCCESS) {
				p->totl = (unsigned long)( /* info.virtual_size */ info.resident_size / 1024);
			}
		}
		else {
			/*
			 * This error pops up very often because of Mac OS X
			 * security fixes. It might pop up all of the time on
			 * an intel Mac. Basically, we cannot get many tasks
			 * unless we are root.
			 */
			/* perror("task_for_pid"); */
			p->totl = 0;
		}
	}

	kprocs -= i;

	return (TRUE);
}

int
machine_get_smpload(load_type * result, int *numcpus)
{
	int i, num;
	size_t size;
	load_type curr_load;

	size = sizeof(int);
	if (sysctlbyname("hw.ncpu", &num, &size, NULL, 0) < 0) {
		perror("sysctl hw.ncpu");
		return (FALSE);
	}

	if (machine_get_load(&curr_load) == FALSE)
		return (FALSE);

	if (numcpus == NULL)
		return (FALSE);

	/* restrict #CPUs to max. *numcpus */
	num = (*numcpus >= num) ? num : *numcpus;
	*numcpus = num;

	/* Don't know how to get per-cpu-load values */
	for (i = 0; i < num; i++) {
		result[i] = curr_load;
	}

	return (TRUE);
}

int
machine_get_uptime(double *up, double *idle)
{
	size_t size;
	time_t now;
	struct timeval boottime;
	load_type curr_load;

	int mib[2];
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);

	time(&now);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
	    boottime.tv_sec != 0)
		*up = (double)(now - boottime.tv_sec);

	if (machine_get_load(&curr_load) == FALSE)
		*idle = 100.;
	else
		*idle = 100. * curr_load.idle / curr_load.total;

	return (TRUE);
}

static int
swapmode(int *rettotal, int *retfree)
{
#ifdef VM_SWAPUSAGE
	size_t size;
	struct xsw_usage xsu;

	int mib[2];
	mib[0] = CTL_VM;
	mib[1] = VM_SWAPUSAGE;
	size = sizeof(xsu);

	*rettotal = 0;
	*retfree = 0;

	if (sysctl(mib, 2, &xsu, &size, NULL, 0) != 0) {
		perror("sysctl");
		return (FALSE);
	}

	*rettotal = (xsu.xsu_total / 1024);
	*retfree = ((xsu.xsu_total - xsu.xsu_used) / 1024);

	return (TRUE);
#endif

	*rettotal = 0;
	*retfree = 0;

	return (FALSE);
}

/* Get network statistics */
int
machine_get_iface_stats(IfaceInfo * interface)
{
	int rows;
	int name[6] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_SYSTEM, IFMIB_IFCOUNT};
	size_t len;
	struct ifmibdata ifmd;	/* ifmibdata contains the network statistics */

	len = sizeof(rows);
	/* get number of interfaces */
	if (sysctl(name, 5, &rows, &len, 0, 0) == 0) {
		interface->status = down;	/* set status down by default */

		name[3] = IFMIB_IFDATA;
		name[4] = 0;
		name[5] = IFDATA_GENERAL;

		len = sizeof(ifmd);
		/*
		 * walk through all interfaces in the ifmib table from last
		 * to first
		 */
		for (; rows > 0; rows--) {
			name[4] = rows;	/* set the interface index */
			/* retrive the ifmibdata for the current index */
			if (sysctl(name, 6, &ifmd, &len, NULL, 0) == -1) {
				perror("read sysctl");
				break;
			}
			/* check if its interface name matches */
			if (strcmp(ifmd.ifmd_name, interface->name) == 0) {

				interface->rc_byte = ifmd.ifmd_data.ifi_ibytes;
				interface->tr_byte = ifmd.ifmd_data.ifi_obytes;
				interface->rc_pkt = ifmd.ifmd_data.ifi_ipackets;
				interface->tr_pkt = ifmd.ifmd_data.ifi_opackets;

				if (interface->last_online == 0) {
					interface->rc_byte_old = interface->rc_byte;
					interface->tr_byte_old = interface->tr_byte;
					interface->rc_pkt_old = interface->rc_pkt;
					interface->tr_pkt_old = interface->tr_pkt;
				}

				if ((ifmd.ifmd_flags & IFF_UP) == IFF_UP) {
					interface->status = up;	/* is up */
					interface->last_online = time(NULL);	/* save actual time */
				}

				return (TRUE);
			}
		}
		/* if we are here there is no interface with the given name */
		return (TRUE);
	}
	else {
		perror("read sysctl IFMIB_IFCOUNT");
		return (FALSE);
	}
}


#endif				/* __APPLE__ */
