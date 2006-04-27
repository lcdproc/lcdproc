/* Copyright (c) 2003 Thomas Runge (coto@core.de)
 * Mach and Darwin specific code is Copyright (c) 2006 Eric Pooch (epooch@tenon.com)
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
#include <kvm.h>
#include <errno.h>

#include <mach/mach.h>

#include "main.h"
#include "machine.h"
#include "config.h"
#include "shared/LL.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
//AUTOFRAMEWORK(CoreFoundation)
//AUTOFRAMEWORK(IOKit)

static int pageshift;
#define pagetok(size) ((size) << pageshift)
static int swapmode(int *rettotal, int *retfree);

static mach_port_t lcdproc_port;

int machine_init()
{
	/* get the page size with "getpagesize" and calculate pageshift from it */
	int pagesize = 0;
	pageshift = 0;

	lcdproc_port = mach_host_self();
	host_page_size(lcdproc_port, &pagesize);

	while (pagesize > 1)
	{
		pageshift++;
		pagesize >>= 1;
	}

	/* we only need the amount of log(2)1024 for our conversion */
	pageshift -= 10;

	return(TRUE);
}

int machine_close()
{
	return(TRUE);
}

int machine_get_battstat(int *acstat, int *battflag, int *percent)
{
	CFTypeRef blob = IOPSCopyPowerSourcesInfo();
	CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

	int i;
	CFDictionaryRef pSource = NULL;
	const void *psValue;

	*acstat = LCDP_AC_ON;
	*battflag = LCDP_BATT_ABSENT;
	*percent  = 100;

	if (CFArrayGetCount(sources) == 0) return(FALSE);

	for (i = 0; i < CFArrayGetCount(sources); i++)
	{
		pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
		if (!pSource) break;

		psValue = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSNameKey));

		if (CFDictionaryGetValueIfPresent(pSource, CFSTR(kIOPSIsPresentKey), &psValue) && (CFBooleanGetValue(psValue) > 0))
		{
			psValue = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));

			if (CFStringCompare(psValue,CFSTR(kIOPSBatteryPowerValue),0)==kCFCompareEqualTo)
			{
				/* We are running on a battery power source. */
				*battflag = LCDP_BATT_UNKNOWN;
				*acstat = LCDP_AC_OFF;
			}
			else if (CFDictionaryGetValueIfPresent(pSource, CFSTR(kIOPSIsChargingKey), &psValue))
			{
				/* We are running on an AC power source, 
				but we also have a battery power source present. */

				if (CFBooleanGetValue(psValue) > 0)
					*battflag = LCDP_BATT_CHARGING;
				else
					*battflag = LCDP_BATT_UNKNOWN;
			}

			if (*battflag != LCDP_BATT_ABSENT)
			{
				int curCapacity = 0;
				int maxCapacity = 0;

				psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSCurrentCapacityKey));
				CFNumberGetValue(psValue, kCFNumberSInt32Type, &curCapacity);

				psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSMaxCapacityKey));
				CFNumberGetValue(psValue, kCFNumberSInt32Type, &maxCapacity);

				*percent = (int)((double)curCapacity/(double)maxCapacity * 100);

				/*	There is a way to check this through the IOKit, 
					but I am not sure what gets for kIOPSLowWarnLevelKey and kIOPSDeadWarnLevelKey, and this is easier.
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

	return(TRUE);
}

int machine_get_fs(mounts_type fs[], int *cnt)
{
	struct statfs *mntbuf;
	struct statfs *pp;
	int statcnt, fscnt, i; 

	fscnt = getmntinfo(&mntbuf, MNT_WAIT);
	if (fscnt == 0) 
	{ 
		perror("getmntinfo");
		return(FALSE);
	}
	for (statcnt = 0, pp = mntbuf, i = 0; i < fscnt; pp++, i++)
	{
		if (    strcmp(pp->f_fstypename, "procfs")
			&& strcmp(pp->f_fstypename, "kernfs")
			&& strcmp(pp->f_fstypename, "linprocfs")
#ifndef STAT_NFS
			&& strcmp(pp->f_fstypename, "nfs")
#endif
#ifndef STAT_SMBFS
			&& strcmp(pp->f_fstypename, "smbfs")
#endif
		)
		{
			snprintf(fs[statcnt].dev,    255, "%s", pp->f_mntfromname);
			snprintf(fs[statcnt].mpoint, 255, "%s", pp->f_mntonname);
			snprintf(fs[statcnt].type,   255, "%s", pp->f_fstypename);

			fs[statcnt].blocks = pp->f_blocks;
			if (fs[statcnt].blocks > 0)
			{
				fs[statcnt].bsize = pp->f_bsize;
				fs[statcnt].bfree = pp->f_bfree;
				fs[statcnt].files = pp->f_files;
				fs[statcnt].ffree = pp->f_ffree;
			}
			statcnt++;
		}
	}

	*cnt = statcnt;

	return(TRUE);
}

int machine_get_load(load_type *curr_load)
{
	static load_type last_load = { 0, 0, 0, 0, 0 };
	static load_type last_ret_load;
	load_type load;

	host_cpu_load_info_data_t	load_info;
	mach_msg_type_number_t		info_count;

	info_count = HOST_CPU_LOAD_INFO_COUNT;
	if (host_statistics(lcdproc_port, HOST_CPU_LOAD_INFO, (host_info_t)&load_info, &info_count))
	{
		perror("host_statistics");
		return(FALSE);
	}

	load.user   = (unsigned long) (load_info.cpu_ticks[CPU_STATE_USER]);
	load.nice   = (unsigned long) (load_info.cpu_ticks[CPU_STATE_NICE]);
	load.system = (unsigned long) (load_info.cpu_ticks[CPU_STATE_SYSTEM]);
	load.idle   = (unsigned long) (load_info.cpu_ticks[CPU_STATE_IDLE]);
	load.total  = load.user + load.nice + load.system + load.idle;

	if (load.total != last_load.total)
	{
		curr_load->user   = load.user   - last_load.user;
		curr_load->nice   = load.nice   - last_load.nice;
		curr_load->system = load.system - last_load.system;
		curr_load->idle   = load.idle   - last_load.idle;
		curr_load->total  = load.total  - last_load.total;
		last_ret_load = *curr_load;
		last_load = load;
	}
	else
	{
		*curr_load = last_ret_load;
	}

	return TRUE;
}


int machine_get_loadavg(double *load)
{
	double loadavg[1];

	if (getloadavg(loadavg, 1) == -1)
		return(FALSE);

	*load = loadavg[0];

	return(TRUE);
}

int machine_get_meminfo(meminfo_type *result)
{
	int total_pages, free_pages;

	vm_statistics_data_t	vm_info;
	mach_msg_type_number_t	info_count;

	info_count = HOST_VM_INFO_COUNT;
	if (host_statistics(lcdproc_port, HOST_VM_INFO, (host_info_t)&vm_info, &info_count))
	{
		perror("host_statistics");
		return(FALSE);
	}

	result[0].total		= pagetok(vm_info.active_count + vm_info.inactive_count +
								vm_info.free_count + vm_info.wire_count);
	result[0].free		= pagetok(vm_info.free_count);
	result[0].buffers	= 0;
	result[0].cache		= 0;
	result[0].shared	= 0;

	/* swap */
	result[1].total   = 0;
	result[1].free    = 0;

	if (swapmode(&total_pages, &free_pages) != -1)
	{
		result[1].total = total_pages;
		result[1].free  = free_pages;
	}

	return(TRUE);
}

int machine_get_procs(LinkedList *procs)
{
	#warning machine_get_procs not implemented
	return(FALSE);
	/* FIX ME */
	/* This needs to be finished. */
	procinfo_type	*p;

	processor_set_t *psets, pset;
	task_t          *tasks;

	unsigned int nproc, i;

	if (host_processor_sets(lcdproc_port, &psets, &nproc))
	{
		perror("host_processor_sets");
		return(FALSE);
	}

	for (i = 0; i < nproc; i++)
	{
		if (host_processor_set_priv(lcdproc_port, psets[i], &pset))
		{
			perror("host_processor_set_priv");
			return(FALSE);
		}

		p = malloc(sizeof(procinfo_type));
		if (!p)
		{
			perror("mem_top_malloc");
			return(FALSE);
		}

	}
	return(TRUE);
}

int machine_get_smpload(load_type *result, int *numcpus)
{
	int i, num;
	size_t size;
	load_type curr_load;

	size = sizeof(int);
	if (sysctlbyname("hw.ncpu", &num, &size, NULL, 0) < 0)
	{
		perror("sysctl hw.ncpu");
		return(FALSE);
	}

	if (machine_get_load(&curr_load) == FALSE)
		return(FALSE);

	*numcpus = num;
	num = num > 8 ? 8 : num;
	/* Don't know how to get per-cpu-load values */
	for (i = 0; i < num; i++)
	{
		result[i] = curr_load;
	}

	return(TRUE);
}	

int machine_get_uptime(double *up, double *idle)
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
		*idle = 100.*curr_load.idle/curr_load.total;

	return(TRUE); 
}

static int swapmode(int *rettotal, int *retfree)
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

	if (sysctl(mib, 2, &xsu, &size, NULL, 0) != 0)
	{
		perror("sysctl");
		return(FALSE);
	}

	*rettotal = (xsu.xsu_total/1024);
	*retfree  = ((xsu.xsu_total-xsu.xsu_used)/1024);

	return(TRUE);	
	#endif

	*rettotal = 0;
	*retfree = 0;

	return(FALSE);
}

#endif /* __APPLE__ */
