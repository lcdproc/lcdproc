#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "chrono.h"

#if HAVE_UTMPX_H
#include <utmpx.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_KVM_H
#include <kvm.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_SYS_SCHED_H
#include <sys/sched.h>
#endif

#if HAVE_SYS_DKSTAT_H
#include <sys/dkstat.h>
#endif

#if FREEBSD
/* definitions for indices in the nlist array */
/* from /usr/src/src.bin/top/machine.c */
static struct nlist nlst[] = {
#define X_CCPU      0
    { "_ccpu" },
#define X_CP_TIME   1   
    { "_cp_time" },
    { 0 }
};
#elif OPENBSD
#define X_CP_TIME   0
static struct nlist nlst[] = {
	{ "_cp_time" },     /* 0 */
	{ 0 }
};
#endif

int TwentyFourHour = 1;
int uptime_fd = 0;

#ifdef SYS_NMLN
 static char kver[SYS_NMLN];
 static char sysname[SYS_NMLN];
#else
# ifdef _SYS_NAMELEN
  static char kver[_SYS_NAMELEN];
  static char sysname[_SYS_NAMELEN];
# else
  // Last hope! May segfault!
  static char *kver;
  static char *sysname;
# endif
#endif

#if FREEBSD
static double freebsd_get_uptime(double *up, double *idle);
#define get_uptime freebsd_get_uptime
#elif (NETBSD || OPENBSD)
static double opennetbsd_get_uptime(double *up, double *idle);
#define get_uptime opennetbsd_get_uptime
#elif LINUX
static double linux_get_uptime(double *up, double *idle);
#define get_uptime linux_get_uptime
#elif SOLARIS
static double solaris_get_uptime(double *up, double *idle);
#define get_uptime solaris_get_uptime
#else
#error "Does not know how to get the uptime."
#endif

#if FREEBSD
static double freebsd_get_uptime(double *up, double *idle) {
	double uptime = 0;
	struct timeval boottime;
	time_t now, uuptime;
	size_t size;
	long cp_time[CPUSTATES];
	long iidle = 0;
	int mib[2];
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kvmd = NULL;

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);

	time(&now);
	if(sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
				boottime.tv_sec != 0)
	{
		uuptime = now - boottime.tv_sec;
		uuptime += 30;

		uptime = (double) uuptime;
	}

	/* open kernel virtual memory */
	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
	if(kvmd==NULL)
	{
		fprintf(stderr, "kvm_openfiles: %s\n", errbuf);
		exit(EXIT_FAILURE);
	}
	else
	{
		/* ask for specified values */
		if(kvm_nlist(kvmd, nlst) >= 0)
		{
			if(nlst[X_CCPU].n_type != 0)
			{
				/* read values for cpu time (user, nice, sys, intr, idle) */
				if(kvm_read(kvmd, nlst[X_CP_TIME].n_value, (char *)&cp_time,
					sizeof(cp_time))==sizeof(cp_time))
				{
					//ZZZ oder besser alle anderen zusammenaddieren und durch
					// idle dividieren?
					iidle = cp_time[CP_IDLE];
				}
			}
		}
		kvm_close(kvmd);

		*idle = (double) (iidle / 100.);
	}
	*up = uptime;
  
	return uptime; 
}
#endif

#if OPENBSD
static long get_openbsd_idle_time() {
	kvm_t *kvmd;
	long cp_time[CPUSTATES];
	char errbuf[_POSIX2_LINE_MAX];

	cp_time[CP_IDLE] = 0;
	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
	if(kvmd==NULL) {
		fprintf(stderr, "kvm_openfiles: %s\n", errbuf);
		return(0);
	} else{
		if(kvm_nlist(kvmd, nlst) >= 0) {
			if(kvm_read(kvmd, nlst[X_CP_TIME].n_value, (char *)cp_time, sizeof(cp_time)) != sizeof(cp_time)) {
				fprintf(stderr, "kvm_read: %s\n", errbuf);
			}
		}
		kvm_close(kvmd);
	}
	return(cp_time[CP_IDLE]);
}
#endif

#if (NETBSD || OPENBSD)
static double opennetbsd_get_uptime(double *up, double *idle) {
	double uptime = 0;
	struct timeval boottime;
	u_int64_t iidle = 0;
	time_t now, uuptime;
	size_t size;
	int mib[2];
#if NETBSD
	u_int64_t cp_time[CPUSTATES];
#endif

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);

	time(&now);
	if(sysctl(mib, 2, &boottime, &size, NULL, 0) < 0) {
		fprintf(stderr, "sysctl kern.boottime failed: %s\n",
				strerror(errno));
	}
	else
		if(boottime.tv_sec != 0) {
			uuptime = now - boottime.tv_sec;
			uuptime += 30;

			uptime = (double) uuptime;
		}

#if OPENBSD
	iidle = get_openbsd_idle_time();
#else
	mib[0] = CTL_KERN;
	mib[1] = KERN_CP_TIME;
	size = sizeof(cp_time);

	if(sysctl(mib, 2, cp_time, &size, NULL, 0) < 0)
	{
		fprintf(stderr, "sysctl kern.cp_time failed: %s\n",
				strerror(errno));
	}
	else
		iidle = cp_time[CP_IDLE];
#endif // __OpenBSD__

	*up = uptime;
	*idle = (double) (iidle / 100.);
	return uptime; 
}
#endif // NetBSD or OpenBSD

#if LINUX
static double linux_get_uptime (double *up, double *idle) {
	double uptime;

	reread (uptime_fd, "get_uptime:");
	sscanf (buffer, "%lf %lf", &uptime, idle);

	*up = uptime;

	return uptime;
}
#endif

#if SOLARIS
static double solaris_get_uptime (double *up, double *idle) {
	double uptime;
	struct utmpx                *u, id;

    id.ut_type = BOOT_TIME;
    u = getutxid(&id);

    uptime=time(0) - u->ut_xtime;
   
	*up = uptime;

	return uptime;
}
#endif

// A couple of tables for later use...
static char *days[] = {
	"Sunday,   ",
	"Monday,   ",
	"Tuesday,  ",
	"Wednesday,",
	"Thursday, ",
	"Friday,   ",
	"Saturday, ",
};
static char *shortdays[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

static char *months[] = {
	"  January",
	" February",
	"    March",
	"    April",
	"      May",
	"     June",
	"     July",
	"   August",
	"September",
	"  October",
	" November",
	" December",
};
static char *shortmonths[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

int
chrono_init ()
{
	struct utsname *unamebuf;

	if (!uptime_fd) {
		unamebuf = (struct utsname *) malloc (sizeof (struct utsname));
		uptime_fd = open ("/proc/uptime", O_RDONLY);

#if 0
		kversion_fd = open ("/proc/sys/kernel/osrelease", O_RDONLY);

		reread (kversion_fd, "main:");
		sscanf (buffer, "%s", kver);

		close (kversion_fd);
#endif

		/* Get OS name and version from uname() */
		/* Changed to check if eq -1 instead of non-zero */
		/* since uname may return any non-negative value */
		if (uname (unamebuf) == -1) {
			perror ("Error calling uname:");
		}
		strcpy (kver, unamebuf->release);
		strcpy (sysname, unamebuf->sysname);

		free (unamebuf);

	}

	return 0;
}

int
chrono_close ()
{
	if (uptime_fd)
		close (uptime_fd);

	uptime_fd = 0;

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Time Screen displays current time and date, uptime, OS ver...
//
//+--------------------+
//|## Linux 2.0.33 ###@|
//|Up xxx days hh:mm:ss|
//|  Wed May 17, 1998  |
//|11:32:57a  100% idle|
//+--------------------+
//

int
time_screen (int rep, int display)
{
	char hr[8], min[8], sec[8], ampm[8];
	char day[16], month[16];
	static int first = 1;
	static int colons = 0;
	time_t thetime;
	struct tm *rtime;
	double uptime, idle;
	int i = 0;
	char colon;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add T\n");
		sprintf (buffer, "screen_set T -name {Time Screen: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add T title title\n");
		sock_send_string (sock, "widget_set T title {Time Screen}\n");
		sock_send_string (sock, "widget_add T one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add T two string\n");
			sock_send_string (sock, "widget_add T three string\n");
		} else {
			sprintf (buffer, "widget_set T title {TIME: %s}\n", host);
			sock_send_string (sock, buffer);
		}
	}

	if (colons)
		colon = ':';
	else
		colon = ' ';

	time (&thetime);
	rtime = localtime (&thetime);

	if (TwentyFourHour) {
		sprintf (hr, "%02d", rtime->tm_hour);
	} else {
		if (rtime->tm_hour > 12) {
			i = 1;
			sprintf (hr, "%02d", (rtime->tm_hour - 12));
		} else if (rtime->tm_hour == 0) {
			i = 0;
			sprintf (hr, "12");
		} else {
			i = 0;
			sprintf (hr, "%02d", rtime->tm_hour);
		}
	}
	if (rtime->tm_hour >= 12)
		sprintf (ampm, "%s", "P");
	else
		sprintf (ampm, "%s", "A");

	sprintf (min, "%02d", rtime->tm_min);
	sprintf (sec, "%02d", rtime->tm_sec);

	strcpy (day, shortdays[rtime->tm_wday]);
	strcpy (month, shortmonths[rtime->tm_mon]);

	///////////////////// Write the title bar (os name and version)

	if (lcd_hgt >= 4) {
		sprintf (tmp, "widget_set T title {");
		sprintf (tmp + strlen (tmp), "%s %s: %s}\n", sysname, kver, host);
		if (display)
			sock_send_string (sock, tmp);
	}
	/////////////////////// Display the time...
	if (lcd_hgt >= 4) {
		get_uptime (&uptime, &idle);
		idle = (idle * 100) / uptime;

		if (TwentyFourHour)
			sprintf (tmp, "%s%c%s%c%s   %2i%% idle", hr, colon, min, colon, sec, (int) idle);
		else
			sprintf (tmp, "%s%c%s%c%s%s  %2i%% idle", hr, colon, min, colon, sec, ampm, (int) idle);
		// Center the output line...
		i = ((lcd_wid - strlen (tmp)) / 2) + 1;
		sprintf (buffer, "widget_set T three %i 4 {%s}\n", i, tmp);
		if (display)
			sock_send_string (sock, buffer);

		sprintf (tmp, "%s %s %d, %d", day, month, rtime->tm_mday, (rtime->tm_year + 1900));
		sprintf (buffer, "widget_set T two 3 3 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		/////////////////////// Display the uptime...
		i = (int) uptime / 86400;
		sprintf (day, "Up %d day%s,", i, (i != 1 ? "s" : ""));
		i = ((int) uptime % 86400) / 60 / 60;
		sprintf (hr, "%02i", i);
		i = (((int) uptime % 86400) % 3600) / 60;
		sprintf (min, "%02i", i);
		i = ((int) uptime % 60);
		sprintf (sec, "%02i", i);
		if (colons)
			sprintf (tmp, "%s %s:%s:%s", day, hr, min, sec);
		else
			sprintf (tmp, "%s %s %s %s", day, hr, min, sec);
		//// Center this line automatically...
		//i = ((lcd_wid - strlen(tmp)) / 2) + 1;
		sprintf (buffer, "widget_set T one 1 2 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

	} else							  // 20x2 version of the screen
	{
		sprintf (tmp, "widget_set T one 1 2 {");
		if (lcd_wid >= 20)
			sprintf (tmp + strlen (tmp), "%d-%02d-%02d ", rtime->tm_year + 1900, rtime->tm_mon + 1, rtime->tm_mday);
		else							  // 16x2 version...
			sprintf (tmp + strlen (tmp), "%02d/%02d ", rtime->tm_mon + 1, rtime->tm_mday);
		sprintf (tmp + strlen (tmp), "%s%c%s%c%s", hr, colon, min, colon, sec);
		if (!TwentyFourHour)
			sprintf (tmp + strlen (tmp), "%s", ampm);
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);
	}

	colons = colons ^ 1;

	return 0;
}										  // End time_screen()

//////////////////////////////////////////////////////////////////////
// Clock Screen displays current time and date...
//
int
clock_screen (int rep, int display)
{
	char hr[8], min[8], sec[8], ampm[8];
	char day[16], month[16];
	static int first = 1;
	static int colons = 0;
	time_t thetime;
	struct tm *rtime;
	int i = 0;
	char colon;

	if (lcd_hgt < 4)
		return 0;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add O\n");
		sprintf (buffer, "screen_set O -name {Clock Screen: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add O title title\n");
		sock_send_string (sock, "widget_add O one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_set O title {DATE & TIME}\n");
			sock_send_string (sock, "widget_add O two string\n");
			sock_send_string (sock, "widget_add O three string\n");
			sprintf (buffer, "widget_set O one 3 2 {%s}\n", host);
			sock_send_string (sock, buffer);
		} else {
			sprintf (buffer, "widget_set O title {TIME: %s}\n", host);
			sock_send_string (sock, buffer);
		}
	}

	if (colons)
		colon = ':';
	else
		colon = ' ';

	time (&thetime);
	rtime = localtime (&thetime);

	strcpy (day, days[rtime->tm_wday]);

	if (TwentyFourHour) {
		sprintf (hr, "%02d", rtime->tm_hour);
	} else {
		if (rtime->tm_hour > 12) {
			i = 1;
			sprintf (hr, "%02d", (rtime->tm_hour - 12));
		} else {
			i = 0;
			sprintf (hr, "%02d", rtime->tm_hour);
		}
	}
	if (rtime->tm_hour == 12)
		i = 1;
	sprintf (min, "%02d", rtime->tm_min);
	sprintf (sec, "%02d", rtime->tm_sec);
	if (i == 1) {
		sprintf (ampm, "%s", "P");
	} else {
		sprintf (ampm, "%s", "A");
	}
	strcpy (month, months[rtime->tm_mon]);

	if (lcd_hgt >= 4)				  // 4-line version of the screen
	{
		sprintf (tmp, "widget_set O two 1 3 {");
		if (TwentyFourHour)
			sprintf (tmp + strlen (tmp), "%s%c%s%c%s  %s", hr, colon, min, colon, sec, day);
		else
			sprintf (tmp + strlen (tmp), "%s%c%s%c%s%s %s", hr, colon, min, colon, sec, ampm, day);
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);

		sprintf (tmp, "widget_set O three 2 4 {");
		sprintf (tmp + strlen (tmp), "%s %d, %d", month, rtime->tm_mday, (rtime->tm_year + 1900));
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);
	}									  // end if(lcd_hgt >= 4)
	else								  // 20x2 version of the screen
	{
		sprintf (tmp, "widget_set O one 1 2 {");
		sprintf (tmp + strlen (tmp), "%d-%02d-%02d ", rtime->tm_year + 1900, rtime->tm_mon + 1, rtime->tm_mday);
		sprintf (tmp + strlen (tmp), "%s%c%s%c%s", hr, colon, min, colon, sec);
		if (!TwentyFourHour)
			sprintf (tmp + strlen (tmp), "%s", ampm);
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);
	}

	colons = colons ^ 1;

	return 0;
}										  // End clock_screen()

////////////////////////////////////////////////////////////////////
// Uptime Screen shows info about system uptime and OS version
//
int
uptime_screen (int rep, int display)
{
	int i;
	char date[16], hour[8], min[8], sec[8];
	double uptime, idle;
	static int first = 1;
	static int colons = 0;
	char colon;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add U\n");
		sprintf (buffer, "screen_set U -name {Uptime Screen: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add U title title\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_set U title {SYSTEM UPTIME}\n");
			sock_send_string (sock, "widget_add U one string\n");
			sock_send_string (sock, "widget_add U two string\n");
			sock_send_string (sock, "widget_add U three string\n");

			sprintf (buffer, "widget_set U one 3 2 {%s}\n", host);
			sock_send_string (sock, buffer);
			sprintf (tmp, "widget_set U three 5 4 {%s %s}\n", sysname, kver);
			sock_send_string (sock, tmp);
		} else {
			sprintf (tmp, "widget_set U title {%s %s: %s}\n", sysname, kver, host);
			sock_send_string (sock, tmp);
			sock_send_string (sock, "widget_add U one string\n");
		}
	}

	if (colons)
		colon = ':';
	else
		colon = ' ';

	get_uptime (&uptime, &idle);
	i = (int) uptime / 86400;
	sprintf (date, "%d day%s,", i, (i != 1 ? "s" : ""));
	i = ((int) uptime % 86400) / 60 / 60;
	sprintf (hour, "%02i", i);
	i = (((int) uptime % 86400) % 3600) / 60;
	sprintf (min, "%02i", i);
	i = ((int) uptime % 60);
	sprintf (sec, "%02i", i);
	sprintf (tmp, "%s %s%c%s%c%s", date, hour, colon, min, colon, sec);
	i = ((20 - strlen (tmp)) / 2) + 1;

	if (lcd_hgt >= 4)
		sprintf (buffer, "widget_set U two %i 3 {%s}\n", i, tmp);
	else
		sprintf (buffer, "widget_set U one %i 2 {%s}\n", i, tmp);
	if (display)
		sock_send_string (sock, buffer);

	colons = colons ^ 1;

	return 0;
}										  // End uptime_screen()

//////////////////////////////////////////////////////////////////////
// Big Clock Screen displays current time...
//
//Curses display is ugly, but should look nice on Matrix Orbital.
//
//+--------------------+
//|111555 444222 111333|
//|111555 444222 111333|
//|111555 444222 111333|
//|111555 444222 111333|
//+--------------------+
//
int
big_clock_screen (int rep, int display)
{
	static int first = 1;
	time_t thetime;
	struct tm *rtime;
	int pos[] = { 1, 4, 8, 11, 15, 18 };
	char  cmdbuf[64] ;
	//  int i=0;

	char fulltxt[16], old_fulltxt[16];
	int j = 0;

	if (lcd_hgt < 4)
		return 0;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add K\n");
		sock_send_string (sock, "screen_set K -name {Big Clock Screen} -heartbeat off\n");
		sock_send_string (sock, "widget_add K d0 num\n");
		sock_send_string (sock, "widget_add K d1 num\n");
		sock_send_string (sock, "widget_add K d2 num\n");
		sock_send_string (sock, "widget_add K d3 num\n");
		sock_send_string (sock, "widget_add K d4 num\n");
		sock_send_string (sock, "widget_add K d5 num\n");
//		sock_send_string (sock, "widget_add K c0 num\n");
//		sock_send_string (sock, "widget_add K c1 num\n");
//      sock_send_string(sock, "widget_add K one string\n");
		sock_send_string (sock, "widget_set K d0 1 0\n");
		sock_send_string (sock, "widget_set K d1 4 0\n");
		sock_send_string (sock, "widget_set K d2 8 0\n");
		sock_send_string (sock, "widget_set K d3 11 0\n");
		sock_send_string (sock, "widget_set K d4 15 0\n");
		sock_send_string (sock, "widget_set K d5 18 0\n");
//		sock_send_string (sock, "widget_set K c0 7 10\n");
//		sock_send_string (sock, "widget_set K c1 14 10\n");
		old_fulltxt[0] = '0';
		old_fulltxt[1] = '0';
		old_fulltxt[2] = '0';
		old_fulltxt[3] = '0';
		old_fulltxt[4] = '0';
		old_fulltxt[5] = '0';
		old_fulltxt[6] = 0;
//      sock_send_string(sock, "widget_set K one 1 4 {000000}\n");
	}

	time (&thetime);
	rtime = localtime (&thetime);

	sprintf (fulltxt, "%02d%02d%02d", rtime->tm_hour, rtime->tm_min, rtime->tm_sec);
	for (j = 0; j < 6; j++) {
		if (fulltxt[j] != old_fulltxt[j]) {
			sprintf (cmdbuf, "widget_set K d%d %d %c\n", j, pos[j], fulltxt[j]);
			sock_send_string (sock, cmdbuf);
			old_fulltxt[j] = fulltxt[j];
		}
	}

	return 0;
}										  // End big_clock_screen()
