#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef IRIX
#include <strings.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "shared/sockets.h"
#include "shared/LL.h"

#include "main.h"
#include "mode.h"
#include "mem.h"

#ifdef SOLARIS
#include <strings.h>
#include <sys/stat.h>
#include <sys/swap.h>
#ifdef HAVE_PROCFS_H
# include <procfs.h>
#endif
#ifdef HAVE_SYS_PROCFS_H
# include <sys/procfs.h>
#endif
#endif


struct meminfo {
	int total, cache, buffers, free, shared;
};

int meminfo_fd = 0;

static void get_mem_info (struct meminfo *result);

static void
get_mem_info (struct meminfo *result)
{

#ifndef SOLARIS
	reread (meminfo_fd, "get_meminfo:");
	result[0].total = getentry ("MemTotal:", buffer);
	result[0].free = getentry ("MemFree:", buffer);
	result[0].shared = getentry ("MemShared:", buffer);
	result[0].buffers = getentry ("Buffers:", buffer);
	result[0].cache = getentry ("Cached:", buffer);
	result[1].total = getentry ("SwapTotal:", buffer);
	result[1].free = getentry ("SwapFree:", buffer);
#else
	#define MAXSTRSIZE 80
	swaptbl_t	*s=NULL;
	int            i, n, num;
	char           *strtab;    /* string table for path names */

	result[0].total = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) / 1024;
	result[0].free = sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGESIZE) / 1024;
	result[0].shared = 0;
	result[0].buffers = 0;
	result[0].cache = 0;
again:
	if ((num = swapctl(SC_GETNSWP, 0)) == -1) {
		perror("swapctl: GETNSWP");
		exit(1);
	}
	if (num == 0) {
		fprintf(stderr, "No Swap Devices Configured\n");
		exit(2);
	}
	/* allocate swaptable for num+1 entries */
	if ((s = (swaptbl_t *)
		malloc(num * sizeof(swapent_t) +
			sizeof(struct swaptable))) ==
		(void *) 0) {
		fprintf(stderr, "Malloc Failed\n");
		exit(3);
	}
	/* allocate num+1 string holders */
	if ((strtab = (char *)
		malloc((num + 1) * MAXSTRSIZE)) == (void *) 0) {
		fprintf(stderr, "Malloc Failed\n");
		exit(3);
	}
	/* initialize string pointers */
	for (i = 0; i < (num + 1); i++) {
		s->swt_ent[i].ste_path = strtab + (i * MAXSTRSIZE);
	}


	s->swt_n = num + 1;
	if ((n = swapctl(SC_LIST, s)) < 0) {
		perror("swapctl");
		exit(1);
	}
	if (n > num) {        /* more were added */
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
#endif
}

int
mem_init ()
{
#ifndef SOLARIS
	if (!meminfo_fd) {
		meminfo_fd = open ("/proc/meminfo", O_RDONLY);
	}

#endif
	return 0;
}

int
mem_close ()
{
#ifndef SOLARIS
	if (meminfo_fd)
		meminfo_fd = open ("/proc/meminfo", O_RDONLY);

	meminfo_fd = 0;

#endif
	return 0;
}

/////////////////////////////////////////////////////////////////////////
// Mem Screen displays info about memory and swap usage...
//
int
mem_screen (int rep, int display)
{
	int n;
	struct meminfo mem[2];
	static int first = 1;
	static int which_title = 0;
	float value;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add M\n");
		sprintf (buffer, "screen_set M -name {Memory & Swap: %s}\n", host);
		sock_send_string (sock, buffer);

		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add M title title\n");
			sock_send_string (sock, "widget_set M title { MEM -==- SWAP}\n");
			sock_send_string (sock, "widget_add M totl string\n");
			sock_send_string (sock, "widget_add M used string\n");
			sock_send_string (sock, "widget_set M totl 9 2 Totl\n");
			sock_send_string (sock, "widget_set M used 9 3 Free\n");
			sock_send_string (sock, "widget_add M EF string\n");
			sock_send_string (sock, "widget_set M EF 1 4 {E       F  E       F}\n");
			sock_send_string (sock, "widget_add M memused string\n");
			sock_send_string (sock, "widget_add M swapused string\n");
			//sock_send_string(sock, "widget_set M memgauge 2 4 0\n");
			//sock_send_string(sock, "widget_set M swapgauge 13 4 0\n");
		} else {
			sock_send_string (sock, "widget_add M m string\n");
			sock_send_string (sock, "widget_add M s string\n");
			if (lcd_wid >= 20) {
				sock_send_string (sock, "widget_set M m 1 1 {M     [       ]}\n");
				sock_send_string (sock, "widget_set M s 1 2 {S     [       ]}\n");
			} else {
				sock_send_string (sock, "widget_set M m 1 1 {M     [   ]}\n");
				sock_send_string (sock, "widget_set M s 1 2 {S     [   ]}\n");
			}
			sock_send_string (sock, "widget_add M mem% string\n");
			sock_send_string (sock, "widget_add M swap% string\n");
			sock_send_string (sock, "widget_set M mem% 16 1 { 0.0%}\n");
			sock_send_string (sock, "widget_set M swap% 16 2 { 0.0%}\n");

			//sock_send_string(sock, "widget_set M memgauge 8 1 0\n");
			//sock_send_string(sock, "widget_set M swapgauge 8 2 0\n");
		}

		sock_send_string (sock, "widget_add M memtotl string\n");
		sock_send_string (sock, "widget_add M swaptotl string\n");

		sock_send_string (sock, "widget_add M memgauge hbar\n");
		sock_send_string (sock, "widget_add M swapgauge hbar\n");

		//sock_send_string(sock, "\n");
	}

	get_mem_info (mem);

	// flip the title back and forth...
	if (lcd_hgt >= 4) {
		if (which_title & 4) {
			sprintf (buffer, "widget_set M title {%s}\n", host);
			sock_send_string (sock, buffer);
		} else
			sock_send_string (sock, "widget_set M title { MEM -==- SWAP}\n");
		which_title = (which_title + 1) & 7;

		// Total memory
		sprintf (tmp, "widget_set M memtotl 1 2 {%6dk}\n", mem[0].total);
		if (display)
			sock_send_string (sock, tmp);

		// Free memory (plus buffers and cache)
		sprintf (tmp, "widget_set M memused 1 3 {%6dk}\n", mem[0].free + mem[0].buffers + mem[0].cache);
		if (display)
			sock_send_string (sock, tmp);

		// Total swap
		sprintf (tmp, "widget_set M swaptotl 14 2 {%6dk}\n", mem[1].total);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap
		sprintf (tmp, "widget_set M swapused 14 3 {%6dk}\n", mem[1].free);
		if (display)
			sock_send_string (sock, tmp);

		// Free memory graph
		n = (int) (35.0 - ((float) mem[0].free + (float) mem[0].buffers + (float) mem[0].cache)
					  / (float) mem[0].total * 35.0);
		sprintf (tmp, "widget_set M memgauge 2 4 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap graph
		n = (int) (35.0 - (float) mem[1].free / (float) mem[1].total * 35.0);
		sprintf (tmp, "widget_set M swapgauge 13 4 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);
	} else {
		// Total memory
		sprintf (tmp, "widget_set M memtotl 2 1 {%4dM}\n", (int) (mem[0].total / 1024.0));
		if (display)
			sock_send_string (sock, tmp);

		// Total swap
		sprintf (tmp, "widget_set M swaptotl 2 2 {%4dM}\n", (int) (mem[1].total / 1024.0));
		if (display)
			sock_send_string (sock, tmp);

		// Free memory graph
		value = ((float) mem[0].free + (float) mem[0].buffers + (float) mem[0].cache)
			 / (float) mem[0].total;
		value = 1.0 - value;
		n = (int) ((lcd_cellwid * (float) (lcd_wid - 13)) * (value));
		sprintf (tmp, "widget_set M memgauge 8 1 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);

		value *= 100.0;
		if (value >= 99.9) {
			sprintf (buffer, "100%%");
		} else {
			sprintf (buffer, "%4.1f%%", value);
		}
		sprintf (tmp, "widget_set M mem%% %i 1 {%s}\n", lcd_wid - 4, buffer);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap graph
		value = ((float) mem[1].free / (float) mem[1].total);
		value = 1.0 - value;
		n = (int) ((lcd_cellwid * (float) (lcd_wid - 13)) * (value));
		sprintf (tmp, "widget_set M swapgauge 8 2 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);

		value *= 100.0;
		if (value >= 99.9) {
			sprintf (buffer, "100%%");
		} else {
			sprintf (buffer, "%4.1f%%", value);
		}
		sprintf (tmp, "widget_set M swap%% %i 2 {%s}\n", lcd_wid - 4, buffer);
		if (display)
			sock_send_string (sock, tmp);

	}

	return 0;
}										  // End mem_screen()

typedef struct proc_mem_info {
	char name[16];					  // Is this really long enough?
	// Size isn't used any more...
	// Totl stores the "size" of the program now...
	int size, totl;
	int number;
} proc_mem_info;

static int
sort_procs (void *a, void *b)
{
	proc_mem_info *one, *two;

	if (!a)
		return 0;
	if (!b)
		return 0;

	one = (proc_mem_info *) a;
	two = (proc_mem_info *) b;

	return (two->totl > one->totl);
}

int
mem_top_screen (int rep, int display)
{
	// Much of this code was ripped from "gmemusage"
	char buf[128];

	DIR *proc;
	FILE *StatusFile;
	struct dirent *procdir;

	char procName[16];
	int
	 procSize, procRSS, procData, procStk, procExe;
	const char
	*NameLine = "Name:", *VmSizeLine = "VmSize:", *VmRSSLine = "VmRSS", *VmDataLine = "VmData", *VmStkLine = "VmStk", *VmExeLine = "VmExe";
	const int
	 NameLineLen = strlen (NameLine), VmSizeLineLen = strlen (VmSizeLine), VmDataLineLen = strlen (VmDataLine), VmStkLineLen = strlen (VmStkLine), VmExeLineLen = strlen (VmExeLine), VmRSSLineLen = strlen (VmRSSLine);

	int threshold = 400, unique;
	int i;
	proc_mem_info *p;
	LL *procs;
	static int first = 1;

	if (first) {
		first = 0;
		sock_send_string (sock, "screen_add S\n");
		sprintf (buffer, "screen_set S -name {Top Memory Use: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add S title title\n");
		sprintf (buffer, "widget_set S title {TOP MEM: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add S f frame\n");
		if (lcd_hgt >= 4)
			sock_send_string (sock, "widget_set S f 1 2 20 4 20 5 v 8\n");
		else
			sock_send_string (sock, "widget_set S f 1 2 20 2 20 5 v 16\n");
		for (i = 1; i <= 5; i++) {
			sprintf (buffer, "widget_add S %i string -in f\n", i);
			sock_send_string (sock, buffer);
		}
		sock_send_string (sock, "widget_set S 1 1 1 Checking...\n");
	}

	procs = LL_new ();
	if (!procs) {
		fprintf (stderr, "mem_top_screen: Error allocating list\n");
		return -1;
	}

	if ((proc = opendir ("/proc")) == NULL) {
		fprintf (stderr, "mem_top_screen: unable to open /proc");
		perror ("");
		return -1;
	}

	while ((procdir = readdir (proc))) {
		if (!index ("1234567890", procdir->d_name[0])) {
			continue;
		}
	#ifndef SOLARIS
		sprintf (buf, "/proc/%s/status", procdir->d_name);
		if ((StatusFile = fopen (buf, "r")) == NULL) {
			// Not a serious error; process has finished before we could
			// examine it:
			//fprintf ( stderr , "mem_top_screen: cannot open %s for reading" ,
			//          buf ) ;
			//perror ( "" ) ;
			continue;
		}
		procRSS = procSize = procData = procStk = procExe = 0;
		while (fgets (buf, sizeof (buf), StatusFile)) {
			if (!strncmp (buf, NameLine, NameLineLen)) {
				/* Name: procName */
				sscanf (buf, "%*s %s", procName);
			} else if (!strncmp (buf, VmSizeLine, VmSizeLineLen)) {
				/* VmSize: procSize kB */
				sscanf (buf, "%*s %d", &procSize);
			} else if (!strncmp (buf, VmRSSLine, VmRSSLineLen)) {
				/* VmRSS: procRSS kB */
				sscanf (buf, "%*s %d", &procRSS);
			} else if (!strncmp (buf, VmDataLine, VmDataLineLen)) {
				/* VmData: procData kB */
				sscanf (buf, "%*s %d", &procData);
			} else if (!strncmp (buf, VmStkLine, VmStkLineLen)) {
				/* VmStk: procStk kB */
				sscanf (buf, "%*s %d", &procStk);
			} else if (!strncmp (buf, VmExeLine, VmExeLineLen)) {
				/* VmExe: procExe kB */
				sscanf (buf, "%*s %d", &procExe);
			}
		}
	#else
		sprintf (buf, "/proc/%s/psinfo", procdir->d_name);
		if ((StatusFile = fopen (buf, "r")) == NULL) {
			// Not a serious error; process has finished before we could
			// examine it:
			//fprintf ( stderr , "mem_top_screen: cannot open %s for reading" ,
			//          buf ) ;
			//perror ( "" ) ;
			continue;
		}
		{
			psinfo_t psinfo;
			fread(&psinfo,sizeof(psinfo),1,StatusFile);
			procRSS = procSize = procData = procStk = procExe = 0;
			strcpy(procName,psinfo.pr_fname);
			procSize=psinfo.pr_size;
			procRSS=psinfo.pr_rssize;
			// Following values not accurate, not sure what needs to be set to
			procData=psinfo.pr_size;
			procStk=0;
			procExe=0;
		}
	#endif
		fclose (StatusFile);
		if (procSize > threshold) {
			// Figure out if it's sharing any memory...
			unique = 1;
			LL_Rewind (procs);
			do {
				p = LL_Get (procs);
				if (p) {
					if (0 == strcmp (p->name, procName)) {
						unique = 0;
						p->number++;
						p->totl += procData + procStk + procExe;
					}
				}
			} while (LL_Next (procs) == 0);

			// If this is the first one by this name...
			if (unique) {
				p = malloc (sizeof (proc_mem_info));
				if (!p) {
					fprintf (stderr, "mem_top_screen: Error allocating process entry\n");
					goto end;		  // Ack!  I hate goto's!
				}
				strcpy (p->name, procName);
				p->size = procSize;
				p->totl = procData + procStk + procExe;
				p->number = 1;
				// TODO:  Check for errors here?
				LL_Push (procs, (void *) p);
			}
		}

	}
	closedir (proc);
	// Now, print some info...
	LL_Rewind (procs);
	LL_Sort (procs, sort_procs);
	LL_Rewind (procs);
	for (i = 1; i <= 5; i++) {
		p = LL_Get (procs);
		if (p) {
			//printf("Mem hog: %s: %ik\n", p->name, p->size);
			if (p->number > 1)
				sprintf (buffer, "widget_set S %i 1 %i {%i%6ik %s(%i)}\n", i, i, i, p->totl, p->name, p->number);
			else
				sprintf (buffer, "widget_set S %i 1 %i {%i%6ik %s}\n", i, i, i, p->totl, p->name);
			if (display)
				sock_send_string (sock, buffer);
		} else {
			//printf("Mem hog: none?\n");
			//sprintf (buffer, "widget_set S %i 1 %i {}\n", i, i);
			sprintf (buffer, "widget_set S %i 1 %i \" \"\n", i, i);
			if (display)
				sock_send_string (sock, buffer);
		}

		LL_Next (procs);
	}

 end:								  // Ack!  I hate using labels!  
	// Now clean it all up...
	LL_Rewind (procs);
	do {
		p = (proc_mem_info *) LL_Get (procs);
		if (p) {
			//printf("Proc: %6ik %s\n", p->size, p->name);
			free (p);
		}
	} while (LL_Next (procs) == 0);
	LL_Destroy (procs);

	return 0;

}										  // End mem_top_screen()
