/** \file clients/lcdproc/machine_NetBSD.c
 * Collects system information on NetBSD.
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

#ifdef __NetBSD__

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <kvm.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/dkstat.h>
#include <sched.h>
#include <uvm/uvm_extern.h>
#include <machine/apmvar.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>

#if (__NetBSD_Version__ >= 300000000)
# include <sys/statvfs.h>
#endif

#include "machine.h"
#include "main.h"
#include "config.h"
#include "shared/LL.h"

static int pageshift;
#define pagetok(size) ((size) << pageshift)
#define PROCSIZE(pp) ((pp)->p_vm_tsize + (pp)->p_vm_dsize + (pp)->p_vm_ssize)


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
	struct apm_power_info apmi;

	if ((apmd = open("/dev/apm", O_RDONLY)) == -1) {
		*acstat = LCDP_AC_ON;
		*battflag = LCDP_BATT_ABSENT;
		*percent = 100;
		return (TRUE);
	}

	memset(&apmi, 0, sizeof(apmi));
	if (ioctl(apmd, APM_IOC_GETPOWER, &apmi) == -1) {
		perror("APM_IOC_GETPOWER failed in get_batt_stat()");
		return (FALSE);
	}

	switch (apmi.ac_state) {
	    case APM_AC_OFF:
		*acstat = LCDP_AC_OFF;
		break;
	    case APM_AC_ON:
		*acstat = LCDP_AC_ON;
		break;
	    case APM_AC_BACKUP:
		*acstat = LCDP_AC_BACKUP;
		break;
	    default:
		*acstat = LCDP_AC_UNKNOWN;
		break;
	}

	*battflag = apmi.battery_state;
	*percent = apmi.battery_life;

	close(apmd);

	return (TRUE);
}

int
machine_get_fs(mounts_type fs[], int *cnt)
{
#if (__NetBSD_Version__ >= 300000000)
	struct statvfs *mntbuf;
	struct statvfs *pp;
#else
	struct statfs *mntbuf;
	struct statfs *pp;
#endif
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
	u_int64_t cp_time[CPUSTATES];
	size_t size;
	int mib[2];

	mib[0] = CTL_KERN;
	mib[1] = KERN_CP_TIME;
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
	struct uvmexp_sysctl uvmexp;

	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP2;
	size = sizeof(uvmexp);
	if (sysctl(mib, 2, &uvmexp, &size, NULL, 0) < 0) {
		perror("sysctl vm.uvmexp2 failed");
		return (FALSE);
	}

	/* memory */
	result[0].total = pagetok(uvmexp.npages);
	result[0].free = pagetok(uvmexp.free);

	/* not really */
	result[0].shared = pagetok(uvmexp.wired);
	result[0].buffers = pagetok(uvmexp.execpages);
	result[0].cache = pagetok(uvmexp.filepages);

	/* swap */
	result[1].total = pagetok(uvmexp.swpages);
	result[1].free = pagetok(uvmexp.swpginuse);
	result[1].free = result[1].total - result[1].free;

	return (TRUE);
}

int
machine_get_procs(LinkedList * procs)
{
	int nproc, i;
	kvm_t *kvmd;
	struct kinfo_proc2 *kprocs;
	procinfo_type *p;

	kprocs = NULL;
	if ((kvmd = kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "kvm_open")) == NULL) {
		perror("kvm_openfiles");
		return (FALSE);
	}

	kprocs = kvm_getproc2(kvmd, KERN_PROC_ALL, 0, sizeof(struct kinfo_proc2), &nproc);
	if (kprocs == NULL) {
		perror("kvm_getproc2");
		return (FALSE);
	}

	for (i = 0; i < nproc; i++) {
		p = malloc(sizeof(procinfo_type));
		if (!p) {
			perror("mem_top_malloc");
			kvm_close(kvmd);
			return (FALSE);
		}
		strncpy(p->name, kprocs->p_comm, 15);
		p->name[15] = '\0';
		p->totl = pagetok(PROCSIZE(kprocs));
		p->number = kprocs->p_pid;
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
	load_type curr_load;
	int mib[2];

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
	struct ifaddrs *ifa, *ifa_ptr;
	struct if_data *ifd;

	/* get first interface */
	if (getifaddrs(&ifa) == -1)
		perror("getifaddr failed");

	/* loop through all interfaces */
	for (ifa_ptr = ifa; ifa_ptr != NULL; ifa_ptr = ifa_ptr->ifa_next) {
		interface->status = down;	/* set status down by default */

		/* check if we got the right interface and if it is Link type */
		if ((strcmp(ifa_ptr->ifa_name, interface->name) == 0) &&
		    (ifa_ptr->ifa_addr->sa_family == AF_LINK)) {

			ifd = (struct if_data *)ifa_ptr->ifa_data;

			interface->rc_byte = ifd->ifi_ibytes;
			interface->tr_byte = ifd->ifi_obytes;
			interface->rc_pkt = ifd->ifi_ipackets;
			interface->tr_pkt = ifd->ifi_opackets;

			if (interface->last_online == 0) {
				interface->rc_byte_old = interface->rc_byte;
				interface->tr_byte_old = interface->tr_byte;
				interface->rc_pkt_old = interface->rc_pkt;
				interface->tr_pkt_old = interface->tr_pkt;
			}

			if ((ifa_ptr->ifa_flags & IFF_UP) == IFF_UP) {
				interface->status = up;	/* is up */
				interface->last_online = time(NULL);	/* save actual time */
			}

			return (TRUE);
		}
	}
	freeifaddrs(ifa);

	/* if we are here there is no interface with the given name */
	return (TRUE);
}

#endif				/* __NetBSD__ */
