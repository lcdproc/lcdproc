/** \file clients/lcdproc/machine_OpenBSD.c
 * Collects system information on OpenBSD.
 */

/*-
 * Copyright (c) 2003 Thomas Runge (coto@core.de)
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

#ifdef __OpenBSD__

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/dkstat.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/user.h>
#include <machine/apmvar.h>
#include <kvm.h>
#include <errno.h>

#include "main.h"
#include "machine.h"
#include "config.h"
#include "shared/LL.h"

static int pageshift;
#define pagetok(size) ((size) << pageshift)
#if OpenBSD >= 201111
#define PROCSIZE(pp) ((pp)->p_vm_tsize + (pp)->p_vm_dsize + (pp)->p_vm_ssize)
#else
#define PROCSIZE(pp) ((pp).vm_tsize + (pp).vm_dsize + (pp).vm_ssize)
#endif


int
machine_init(void)
{
	/*
	 * get the page size with "getpagesize" and calculate pageshift from
	 * it
	 */
	int pagesize = getpagesize();
	pageshift = 0;
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
	int apmd;
	struct apm_power_info api;

	*acstat = LCDP_AC_ON;
	*battflag = LCDP_BATT_ABSENT;
	*percent = 100;

	if ((apmd = open("/dev/apm", O_RDONLY)) == -1) {
		perror("get_battstat_open");
		return (TRUE);
	}

	if (ioctl(apmd, APM_IOC_GETPOWER, &api) == -1) {
		perror("get_battstat_ioctl");
		return (FALSE);
	}

	switch (api.ac_state) {
	    case APM_AC_OFF:
		*acstat = LCDP_AC_OFF;
		break;
	    case APM_AC_ON:
		*acstat = LCDP_AC_ON;
		break;
	    default:
		*acstat = LCDP_AC_UNKNOWN;
		break;
	}

	switch (api.battery_state) {
	    case APM_BATT_HIGH:
		*battflag = LCDP_BATT_HIGH;
		break;
	    case APM_BATT_LOW:
		*battflag = LCDP_BATT_LOW;
		break;
	    case APM_BATT_CRITICAL:
		*battflag = LCDP_BATT_CRITICAL;
		break;
	    case APM_BATT_CHARGING:
		*battflag = LCDP_BATT_CHARGING;
		break;
	    case APM_BATTERY_ABSENT:
		*battflag = LCDP_BATT_ABSENT;
		break;
	    default:
		*battflag = LCDP_BATT_UNKNOWN;
		break;
	}

	*percent = api.battery_life;

	close(apmd);

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
	long cp_time[CPUSTATES];
	size_t size;
	int mib[2];

	mib[0] = CTL_KERN;
	mib[1] = KERN_CPTIME;
	size = sizeof(cp_time);
	if (sysctl(mib, 2, cp_time, &size, NULL, 0) < 0) {
		perror("sysctl kern.cp_time failed");
		return (FALSE);
	}

	load.user = (unsigned long)(cp_time[CP_USER]);
	load.nice = (unsigned long)(cp_time[CP_NICE]);
	load.system = (unsigned long)(cp_time[CP_SYS] + cp_time[CP_INTR]);
	load.idle = (unsigned long)(cp_time[CP_IDLE]);
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

	return (TRUE);
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
	size_t size;
	int mib[2];
	struct uvmexp suvm;

	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;
	size = sizeof(suvm);

	if (sysctl(mib, 2, &suvm, &size, NULL, 0) < 0) {
		perror("sysctl vm.uvmexp failed");
		return (FALSE);
	}

	/* memory */
	result[0].total = pagetok(suvm.npages);
	result[0].free = pagetok(suvm.free);

	/* not used anyway */
	result[0].shared = 0;
	result[0].buffers = 0;
	result[0].cache = 0;

	/* swap */
	result[1].total = suvm.pagesize * suvm.swpages;
	result[1].free = suvm.pagesize * suvm.swpginuse;
	result[1].free = result[1].total - result[1].free;

	return (TRUE);
}

int
machine_get_procs(LinkedList * procs)
{
	struct kinfo_proc *kprocs;
	int nproc, i;
	procinfo_type *p;
	kvm_t *kvmd;

	if ((kvmd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL) {
		perror("kvm_open");
		return (FALSE);
	}

#if OpenBSD >= 201111
	kprocs = kvm_getprocs(kvmd, KERN_PROC_ALL, 0, sizeof(struct kinfo_proc), &nproc);
#else
	kprocs = kvm_getprocs(kvmd, KERN_PROC_ALL, 0, &nproc);
#endif
	if (kprocs == NULL) {
		perror("kvm_getprocs");
		kvm_close(kvmd);
		return (FALSE);
	}

	for (i = 0; i < nproc; i++) {
		p = malloc(sizeof(procinfo_type));
		if (!p) {
			perror("mem_top_malloc");
			kvm_close(kvmd);
			return (FALSE);
		}
#if OpenBSD >= 201111
		strncpy(p->name, kprocs->p_comm, 15);
		p->name[15] = '\0';
		p->totl = pagetok(PROCSIZE(kprocs));
		p->number = kprocs->p_pid;
#else
		strncpy(p->name, kprocs->kp_proc.p_comm, 15);
		p->name[15] = '\0';
		p->totl = pagetok(PROCSIZE(kprocs->kp_eproc.e_vm));
		p->number = kprocs->kp_proc.p_pid;
#endif
		LL_Push(procs, (void *)p);

		kprocs++;
	}
	kvm_close(kvmd);

	return (TRUE);
}

int
machine_get_smpload(load_type * result, int *numcpus)
{
	int mib[2], i, num;
	size_t size;
	load_type curr_load;

	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	size = sizeof(int);

	if (sysctl(mib, 2, &num, &size, NULL, 0) < 0) {
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
	int mib[2];
	load_type curr_load;

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	time(&now);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) < 0) {
		perror("sysctl kern.boottime failed");
		return (FALSE);
	}

	*up = (double)(now - boottime.tv_sec);

	if (machine_get_load(&curr_load) == FALSE)
		*idle = 100.;
	else
		*idle = 100. * curr_load.idle / curr_load.total;

	return (TRUE);
}

/* Get network statistics */
int
machine_get_iface_stats(IfaceInfo * interface)
{
	/* Implementation missing */
	return 0;
}

#endif				/* __OpenBSD__ */
