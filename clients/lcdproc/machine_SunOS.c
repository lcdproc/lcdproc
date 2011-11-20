/** \file clients/lcdproc/machine_SunOS.c
 * Collects system information on Solaris.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#ifdef sun

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <dirent.h>
#include <utmpx.h>
#include <procfs.h>
#include <kstat.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/loadavg.h>
#include <sys/mount.h>
#include <sys/cpuvar.h>
#include <sys/swap.h>
#include <sys/statvfs.h>

#include "machine.h"
#include "main.h"
#include "config.h"
#include "shared/LL.h"


static kstat_ctl_t *kc;

int 
machine_init(void)
{
	kc = NULL;

	kc = kstat_open();
	if (kc == NULL) {
		perror("kstat_open");
		return (FALSE);
	}

	return (TRUE);
}

int 
machine_close(void)
{
	if (kc != NULL) {
		kstat_close(kc);
		kc = NULL;
	}

	return (TRUE);
}

int 
machine_get_battstat(int *acstat, int *battflag, int *percent)
{
	*acstat = LCDP_AC_ON;
	*battflag = LCDP_BATT_ABSENT;
	*percent = 100;

	return (TRUE);
}

int 
machine_get_fs(mounts_type fs[], int *cnt)
{
	FILE *mtab_fd;
	char line[256];
	int x = 0, y;

#ifdef STAT_STATVFS
	struct statvfs fsinfo;
#else
	struct statfs fsinfo;
#endif

#ifdef MTAB_FILE
	mtab_fd = fopen(MTAB_FILE, "r");
#else
#error "Can't find your mounted filesystem table file."
#endif

	/* Get rid of old, unmounted filesystems... */
	memset(fs, 0, sizeof(mounts_type) * 256);

	while (x < 256) {
		if (fgets(line, 256, mtab_fd) == NULL) {
			fclose(mtab_fd);
			*cnt = x;
			break;
		}

		sscanf(line, "%s %s %s", fs[x].dev, fs[x].mpoint, fs[x].type);

		if (strcmp(fs[x].type, "proc")
#ifndef STAT_NFS
		    && strcmp(fs[x].type, "nfs")
#endif
#ifndef STAT_SMBFS
		    && strcmp(fs[x].type, "smbfs")
#endif
			) {
#ifdef STAT_STATVFS
			y = statvfs(fs[x].mpoint, &fsinfo);
#elif STAT_STATFS2_BSIZE
			y = statfs(fs[x].mpoint, &fsinfo);
#elif STAT_STATFS4
			y = statfs(fs[x].mpoint, &fsinfo, sizeof(fsinfo), 0);
#else
#error "statfs for this system noy yet supported"
#endif

			fs[x].blocks = fsinfo.f_blocks;
			if (fs[x].blocks > 0) {
				fs[x].bsize = fsinfo.f_bsize;
				fs[x].bfree = fsinfo.f_bfree;
				fs[x].files = fsinfo.f_files;
				fs[x].ffree = fsinfo.f_ffree;
				x++;
			}
		}
	}

	fclose(mtab_fd);
	*cnt = x;

	return (TRUE);
}

int 
machine_get_load(load_type * curr_load)
{
	static load_type last_load = {0, 0, 0, 0, 0};
	load_type load;
	kstat_t *k_space;
	struct cpu_stat cinfo;

	k_space = kstat_lookup(kc, "cpu_stat", 0, "cpu_stat0");
	if (k_space == NULL) {
		fprintf(stderr, "kstat lookup error\n");
		return (FALSE);
	}

	if (kstat_read(kc, k_space, NULL) == -1) {
		fprintf(stderr, "kstat read error\n");
		return (FALSE);
	}

	k_space = kstat_lookup(kc, "cpu_stat", -1, "cpu_stat0");
	if (k_space == NULL) {
		fprintf(stderr, "kstat lookup error\n");
		return (FALSE);
	}
	if (kstat_read(kc, k_space, &cinfo) == -1) {
		fprintf(stderr, "kstat read error\n");
		return (FALSE);
	}
	load.idle = cinfo.cpu_sysinfo.cpu[CPU_IDLE];
	load.user = cinfo.cpu_sysinfo.cpu[CPU_USER];
	load.system = cinfo.cpu_sysinfo.cpu[CPU_KERNEL];
	load.nice = cinfo.cpu_sysinfo.cpu[CPU_WAIT];
	load.total = load.user + load.nice + load.system + load.idle;

	curr_load->user = load.user - last_load.user;
	curr_load->nice = load.nice - last_load.nice;
	curr_load->system = load.system - last_load.system;
	curr_load->idle = load.idle - last_load.idle;
	curr_load->total = load.total - last_load.total;

	/* struct assingment is legal in C89 */
	last_load = load;

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
#define MAXSTRSIZE 80
	swaptbl_t *s;
	int i, n, num;
	char *strtab;		/* string table for path names */

	s = NULL;

	result[0].total = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) / 1024;
	result[0].free = sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGESIZE) / 1024;
	result[0].shared = 0;
	result[0].buffers = 0;
	result[0].cache = 0;

again:
	if ((num = swapctl(SC_GETNSWP, 0)) == -1) {
		perror("swapctl: GETNSWP");
		return (FALSE);
	}

	if (num == 0) {
		fprintf(stderr, "No Swap Devices Configured\n");
		return (FALSE);
	}

	/* allocate swaptable for num+1 entries */
	if ((s = (swaptbl_t *) malloc(num * sizeof(swapent_t) + sizeof(struct swaptable))) == NULL) {
		perror("malloc swap");
		return (FALSE);
	}

	/* allocate num+1 string holders */
	if ((strtab = (char *)malloc((num + 1) * MAXSTRSIZE)) == NULL) {
		perror("malloc string holder");
		return (FALSE);
	}

	/* initialize string pointers */
	for (i = 0; i < (num + 1); i++) {
		s->swt_ent[i].ste_path = strtab + (i * MAXSTRSIZE);
	}

	s->swt_n = num + 1;
	if ((n = swapctl(SC_LIST, s)) < 0) {
		perror("swapctl");
		return (FALSE);
	}

	/* more were added */
	if (n > num) {
		free(s);
		free(strtab);
		goto again;
	}

	result[1].total = 0;
	result[1].free = 0;

	for (i = 0; i < n; i++) {
		result[1].total = result[1].total + s->swt_ent[i].ste_pages * sysconf(_SC_PAGESIZE) / 1024;
		result[1].free = result[1].free + s->swt_ent[i].ste_free * sysconf(_SC_PAGESIZE) / 1024;
	}

	return (TRUE);
}

int 
machine_get_procs(LinkedList * procs)
{
	char buf[128];
	DIR *proc;
	FILE *StatusFile;
	struct dirent *procdir;
	procinfo_type *p;

	char procName[16];
	int procSize, procRSS, procData, procStk, procExe;
	int threshold = 400, unique;

	if ((proc = opendir("/proc")) == NULL) {
		perror("open /proc");
		return (FALSE);
	}

	while ((procdir = readdir(proc))) {
		psinfo_t psinfo;

		if (!strchr("1234567890", procdir->d_name[0]))
			continue;

		sprintf(buf, "/proc/%s/psinfo", procdir->d_name);
		if ((StatusFile = fopen(buf, "r")) == NULL) {
			/*
			 * Not a serious error; process has finished before
			 * we could examine it:
			 */
			continue;
		}

		procRSS = procSize = procData = procStk = procExe = 0;
		fread(&psinfo, sizeof(psinfo), 1, StatusFile);
		strcpy(procName, psinfo.pr_fname);
		procSize = psinfo.pr_size;
		procRSS = psinfo.pr_rssize;

		/*
		 * Following values not accurate, not sure what needs to be
		 * set to
		 */
		procData = psinfo.pr_size;
		procStk = 0;
		procExe = 0;

		fclose(StatusFile);

		if (procSize > threshold) {
			/* Figure out if it's sharing any memory... */
			unique = 1;
			LL_Rewind(procs);
			do {
				p = LL_Get(procs);
				if (p) {
					if (0 == strcmp(p->name, procName)) {
						unique = 0;
						p->number++;
						p->totl += procData + procStk + procExe;
					}
				}
			} while (LL_Next(procs) == 0);

			/* If this is the first one by this name... */
			if (unique) {
				p = malloc(sizeof(procinfo_type));
				if (p == NULL) {
					perror("allocating process entry");
					goto end;
				}
				strcpy(p->name, procName);
				p->totl = procData + procStk + procExe;
				p->number = 1;
				/* TODO:  Check for errors here? */
				LL_Push(procs, (void *)p);
			}
		}
	}
end:
	closedir(proc);

	return (TRUE);
}

int 
machine_get_smpload(load_type * result, int *numcpus)
{
	static load_type last_load[MAX_CPUS];
	load_type curr_load[MAX_CPUS];
	int ncpu = 0;
	int max_cpu, count;

	max_cpu = sysconf(_SC_NPROCESSORS_CONF);
	for (count = 0; count < max_cpu; count++) {
		kstat_t *k_space;
		char buffer[16];

		sprintf(buffer, "cpu_stat%d", count);

		k_space = kstat_lookup(kc, "cpu_stat", count, buffer);

		if ((k_space != NULL) && (kstat_read(kc, k_space, NULL) != -1)) {
			struct cpu_stat cinfo;

			k_space = kstat_lookup(kc, "cpu_stat", -1, buffer);
			kstat_read(kc, k_space, &cinfo);

			curr_load[ncpu].idle = cinfo.cpu_sysinfo.cpu[CPU_IDLE];
			curr_load[ncpu].user = cinfo.cpu_sysinfo.cpu[CPU_USER];
			curr_load[ncpu].system = cinfo.cpu_sysinfo.cpu[CPU_KERNEL];
			curr_load[ncpu].nice = cinfo.cpu_sysinfo.cpu[CPU_WAIT];
			curr_load[ncpu].total = curr_load[ncpu].user + curr_load[ncpu].nice +
				curr_load[ncpu].system + curr_load[ncpu].idle;

			result[ncpu].total = curr_load[ncpu].total - last_load[ncpu].total;
			result[ncpu].user = curr_load[ncpu].user - last_load[ncpu].user;
			result[ncpu].nice = curr_load[ncpu].nice - last_load[ncpu].nice;
			result[ncpu].system = curr_load[ncpu].system - last_load[ncpu].system;
			result[ncpu].idle = curr_load[ncpu].idle - last_load[ncpu].idle;

			/* struct assignment is legal in C89 */
			last_load[ncpu] = curr_load[ncpu];

			/* restrict # CPUs to min(*numcpus, MAX_CPUS) */
			ncpu++;
			if ((ncpu >= *numcpus) || (ncpu >= MAX_CPUS))
				break;
		}
	}
	*numcpus = ncpu;

	return (TRUE);
}

/* TODO get idle time! */
int 
machine_get_uptime(double *up, double *idle)
{
	struct utmpx *u, id;
	load_type curr_load;

	*up = 0;
	*idle = 0;

	id.ut_type = BOOT_TIME;

	u = getutxid(&id);
	if (u == NULL)
		return (FALSE);

	*up = time(0) - u->ut_xtime;

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

#endif				/* sun */
