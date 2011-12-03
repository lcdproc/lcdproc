/** \file clients/lcdproc/machine_FreeBSD.c
 * Collects system information on FreeBSD.
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

#ifdef __FreeBSD__

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
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_mib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_MACHINE_APM_BIOS_H
# include <machine/apm_bios.h>
#endif

#ifdef HAVE_SYS_PCPU_H
# include <sys/pcpu.h>
#endif

#include "main.h"
#include "machine.h"
#include "shared/LL.h"

static int pageshift;
static kvm_t *kvmd;
#define pagetok(size) ((size) << pageshift)
static int swapmode(int *retavail, int *retfree);


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

	/* open kernel virtual memory */
	if ((kvmd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL) {
		perror("kvm_open failed");
		return (FALSE);
	}

	return (TRUE);
}

int
machine_close(void)
{
	if (kvmd != NULL) {
		kvm_close(kvmd);
		kvmd = NULL;
	}
	return (TRUE);
}

int
machine_get_battstat(int *acstat, int *battflag, int *percent)
{
	*acstat = LCDP_AC_ON;
	*battflag = LCDP_BATT_ABSENT;
	*percent = 100;

#ifdef HAVE_MACHINE_APM_BIOS_H
	int apmd;
	struct apm_info aip;

	if ((apmd = open("/dev/apm", O_RDONLY)) == -1) {
		perror("get_battstat_open");
		return (TRUE);
	}

	if (ioctl(apmd, APMIO_GETINFO, &aip) == -1) {
		perror("get_battstat_ioctl");
		return (FALSE);
	}

	switch (aip.ai_acline) {
	    case 0:
		*acstat = LCDP_AC_OFF;
		break;
	    case 1:
		*acstat = LCDP_AC_ON;
		break;
	    default:
		*acstat = LCDP_AC_UNKNOWN;
		break;
	}

	switch (aip.ai_batt_stat) {
	    case 0:
		*battflag = LCDP_BATT_HIGH;
		break;
	    case 1:
		*battflag = LCDP_BATT_LOW;
		break;
	    case 2:
		*battflag = LCDP_BATT_CRITICAL;
		break;
	    case 3:
		*battflag = LCDP_BATT_CHARGING;
		break;
	    default:
		*battflag = LCDP_BATT_UNKNOWN;
		break;
	}

	*percent = aip.ai_batt_life;
	if (*percent == 255)
		*percent = -1;

	close(apmd);
#endif

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
		    && strcmp(pp->f_fstypename, "devfs")
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

	size = sizeof(cp_time);
	if (sysctlbyname("kern.cp_time", cp_time, &size, NULL, 0) < 0) {
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
	int total_pages, free_pages;
	size_t size;

	size = sizeof(int);

	if (sysctlbyname("vm.stats.vm.v_page_count", &total_pages, &size, NULL, 0) < 0) {
		perror("sysctl vm.stats.vm.v_page_count");
		return (FALSE);
	}

	if (sysctlbyname("vm.stats.vm.v_free_count", &free_pages, &size, NULL, 0) < 0) {
		perror("sysctl vm.stats.vm.v_free_count");
		return (FALSE);
	}

	/* memory */
	result[0].total = pagetok(total_pages);
	result[0].free = pagetok(free_pages);

	/* unused anyway */
	result[0].shared = 0;
	result[0].buffers = 0;
	result[0].cache = 0;

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
	int nproc, i;
	procinfo_type *p;

	if (kvmd == NULL)
		return (FALSE);

	kprocs = kvm_getprocs(kvmd, KERN_PROC_PROC, 0, &nproc);
	if (kprocs == NULL) {
		perror("kvm_getprocs");
		return (FALSE);
	}

	for (i = 0; i < nproc; i++) {
		p = malloc(sizeof(procinfo_type));
		if (!p) {
			perror("mem_top_malloc");
			return (FALSE);
		}
#if (__FreeBSD_version > 500000)
		strncpy(p->name, kprocs->ki_comm, 15);
#else
		strncpy(p->name, kprocs->kp_proc.p_comm, 15);
#endif
		p->name[15] = '\0';
#if (__FreeBSD_version > 500000)
		p->totl = kprocs->ki_size / 1024;
		p->number = kprocs->ki_pid;
#else
		p->totl = kprocs->kp_eproc.e_vm.vm_map.size / 1024;
		p->number = kprocs->kp_proc.p_pid;
#endif
		LL_Push(procs, (void *)p);

		kprocs++;
	}

	return (TRUE);
}

int
machine_get_smpload(load_type * result, int *numcpus)
{
	int i, num;
	size_t size;
	load_type load;
#ifdef HAVE_SYS_PCPU_H
	static load_type last_load[MAX_CPUS];
	struct pcpu *pcpudata;

	if (kvmd == NULL)
		return (FALSE);
#endif

	if (numcpus == NULL)
		return (FALSE);

	size = sizeof(int);
	if (sysctlbyname("hw.ncpu", &num, &size, NULL, 0) < 0) {
		perror("sysctl hw.ncpu");
		return (FALSE);
	}

	/* restrict #CPUs to max. *numcpus */
	num = (*numcpus >= num) ? num : *numcpus;
	*numcpus = num;

#ifndef HAVE_SYS_PCPU_H
	if (machine_get_load(&load) == FALSE)
		return (FALSE);
#endif

	for (i = 0; i < num; i++) {
#ifdef HAVE_SYS_PCPU_H
		pcpudata = kvm_getpcpu(kvmd, i);

		if (pcpudata == NULL || pcpudata == (void *)-1)
			return (FALSE);

		/* extract the data for single CPU */
		load.user = (unsigned long)(pcpudata->pc_cp_time[CP_USER]);
		load.nice = (unsigned long)(pcpudata->pc_cp_time[CP_NICE]);
		load.system = (unsigned long)(pcpudata->pc_cp_time[CP_SYS] +
					      pcpudata->pc_cp_time[CP_INTR]);
		load.idle = (unsigned long)(pcpudata->pc_cp_time[CP_IDLE]);
		load.total = load.user + load.nice + load.system + load.idle;

		/* store difference in result */
		result[i].user = load.user - last_load[i].user;
		result[i].nice = load.nice - last_load[i].nice;
		result[i].system = load.system - last_load[i].system;
		result[i].idle = load.idle - last_load[i].idle;
		result[i].total = load.total - last_load[i].total;

		/* store current value for next round */
		last_load[i].user = load.user;
		last_load[i].nice = load.nice;
		last_load[i].system = load.system;
		last_load[i].idle = load.idle;
		last_load[i].total = load.total;

		/* free pcpu buffer */
		free(pcpudata);
#else
		result[i] = load;
#endif
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

	size = sizeof(boottime);
	time(&now);
	if (sysctlbyname("kern.boottime", &boottime, &size, NULL, 0) < 0) {
		perror("sysctl kern.cp_time failed");
		return (FALSE);
	}

	*up = (double)(now - boottime.tv_sec);

	if (machine_get_load(&curr_load) == FALSE)
		*idle = 100.;
	else
		*idle = 100. * curr_load.idle / curr_load.total;

	return (TRUE);
}

/**
 * Reads info about swap space from system and returns it in parameters passed.
 * \param retavail  Total available swap space
 * \param retfree   Free swap space
 * \return  -1 on error, otherwise number of swap areas (typically 0 for total)
 */
static int
swapmode(int *retavail, int *retfree)
{
	int n;
	struct kvm_swap swapary[1];

	*retavail = 0;
	*retfree = 0;

	if (kvmd == NULL)
		return -1;

	n = kvm_getswapinfo(kvmd, swapary, 1, 0);
	if (n < 0 || swapary[0].ksw_total == 0) {
		/* strange */
	}
	else {
		*retavail = pagetok(swapary[0].ksw_total);
		*retfree = pagetok(swapary[0].ksw_total - swapary[0].ksw_used);
	}

	return (n);
}

int
machine_get_iface_stats(IfaceInfo * interface)
{
	int rows;
	int name[6] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_IFDATA, 0, IFDATA_GENERAL};
	size_t len;
	struct ifmibdata ifmd;	/* ifmibdata contains the network statistics */

	len = sizeof(rows);
	/* get number of interfaces */
	if (sysctlbyname("net.link.generic.system.ifcount", &rows, &len, NULL, 0) == 0) {
		interface->status = down;	/* set status down by default */

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
		perror("read sysctlbyname");
		return (FALSE);
	}
}

#endif				/* __FreeBSD__ */
