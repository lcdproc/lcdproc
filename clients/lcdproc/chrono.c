#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <limits.h>
#include <sys/param.h>
#include <errno.h>


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
#include "machine.h"
#include "chrono.h"


static int TwentyFourHour = 1;

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


//////////////////////////////////////////////////////////////////////
// Time Screen displays current time and date, uptime, OS ver...
//
//+--------------------+	+--------------------+
//|## Linux 2.6.11 ###@|	|### TIME: myhost ##@|
//|Up xxx days hh:mm:ss|	|17.05.2005 11:32:57a|
//|  Wed May 17, 1998  |	+--------------------+
//|11:32:57a  100% idle|
//+--------------------+
//
int
time_screen (int rep, int display, int *flags_ptr)
{
	char now[20];
	int xoffs;
	int days, hour, min, sec;
	static int heartbeat = 0;
	static char colon[] = {':', ' '};
	time_t thetime;
	struct tm *rtime;
	double uptime, idle;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string (sock, "screen_add T\n");
		sprintf (buffer, "screen_set T -name {Time Screen: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add T title title\n");
		sock_send_string (sock, "widget_add T one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add T two string\n");
			sock_send_string (sock, "widget_add T three string\n");

			// write title bar: OS name, OS version, hostname
			sprintf (buffer, "widget_set T title {%s %s: %s}\n",
				get_sysname(), get_sysrelease(), get_hostname());
			sock_send_string (sock, buffer);
		} else {
			// write title bar: hostname
			sprintf (buffer, "widget_set T title {TIME: %s}\n", get_hostname());
			sock_send_string (sock, buffer);
		}
	}

	// toggle colon display
	heartbeat ^= 1;

	time (&thetime);
	rtime = localtime (&thetime);

	if (TwentyFourHour) {
		sprintf (now, "%02d%c%02d%c%02d",
			rtime->tm_hour, colon[heartbeat], rtime->tm_min, colon[heartbeat], rtime->tm_sec);
	} else {
		sprintf (now, "%02d%c%02d%c%02d%*s",
			((rtime->tm_hour + 11) % 12) + 1, colon[heartbeat], rtime->tm_min, colon[heartbeat], rtime->tm_sec,
			(lcd_wid > 20) ? 2 : 1, (rtime->tm_hour >= 12) ? "pm" : "am");
	}

	if (lcd_hgt >= 4) {
		char *day = shortdays[rtime->tm_wday];
		char *month = shortmonths[rtime->tm_mon];

		machine_get_uptime(&uptime, &idle);

		// display the uptime...
		days = (int) uptime / 86400;
		hour = ((int) uptime % 86400) / 60 / 60;
		min = (((int) uptime % 86400) % 3600) / 60;
		sec = ((int) uptime % 60);

		if (lcd_wid >= 20)
			sprintf (tmp, "Up %3d day%s %02d%c%02d%c%02d",
				days, ((days != 1) ? "s" : ""), hour, colon[heartbeat], min, colon[heartbeat], sec);
		else
			sprintf (tmp, "Up %dd %02d%c%02d%c%02d",
				days, hour, colon[heartbeat], min, colon[heartbeat], sec);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf (buffer, "widget_set T one 1 2 {%s}\n", tmp);
		if (display)
			sock_send_string (sock, buffer);

		// display the date
		sprintf (tmp, "%s %s %d, %d",
			day, month, rtime->tm_mday, rtime->tm_year + 1900);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf (buffer, "widget_set T two %i 3 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);

		// display the time & idle time...
		sprintf (tmp, "%s %3i%% idle", now, (int) idle);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf (buffer, "widget_set T three %i 4 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);
	}
	else {							// 2 line version of the screen
		if (lcd_wid >= 20)				// 20+x columns
			sprintf(tmp, "%02d.%02d.%04d %s",
				rtime->tm_mday, rtime->tm_mon + 1, rtime->tm_year + 1900, now);
		else						// <20 columns
			sprintf(tmp, "%02d.%02d. %s",
				rtime->tm_mday, rtime->tm_mon + 1, now);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf(buffer, "widget_set T one %i 2 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);
	}

	return 0;
}										  // End time_screen()

//////////////////////////////////////////////////////////////////////
// Old Clock Screen displays current time and date...
//
//+--------------------+	+--------------------+
//|## DATE & TIME ####@|	|### TIME: myhost ##@|
//|       myhost       |	|2005-05-17 11:32:57a|
//|11:32:75a Wednesday,|	+--------------------+
//|       May 17, 2005 |
//+--------------------+
//
int
clock_screen (int rep, int display, int *flags_ptr)
{
	char now[20];
	int xoffs;
	static int heartbeat = 0;
	static char colon[] = {':', ' '};
	time_t thetime;
	struct tm *rtime;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string (sock, "screen_add O\n");
		sprintf (buffer, "screen_set O -name {Old Clock Screen: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add O title title\n");
		sock_send_string (sock, "widget_add O one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add O two string\n");
			sock_send_string (sock, "widget_add O three string\n");

			sprintf (buffer, "widget_set O title {DATE & TIME}\n");
			sock_send_string (sock, buffer);

			sprintf (tmp, "%s", get_hostname());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sprintf (buffer, "widget_set O one %i 2 {%s}\n", xoffs, tmp);
			sock_send_string (sock, buffer);
		} else {
			sprintf (buffer, "widget_set O title {TIME: %s}\n", get_hostname());
			sock_send_string (sock, buffer);
		}
	}

	// toggle colon display
	heartbeat ^= 1;

	time (&thetime);
	rtime = localtime (&thetime);

	if (TwentyFourHour) {
		sprintf (now, "%02d%c%02d%c%02d",
			rtime->tm_hour, colon[heartbeat], rtime->tm_min, colon[heartbeat], rtime->tm_sec);
	} else {
		sprintf (now, "%02d%c%02d%c%02d%*s",
			((rtime->tm_hour + 11) % 12) + 1, colon[heartbeat], rtime->tm_min, colon[heartbeat], rtime->tm_sec,
			(lcd_wid > 20) ? 2 : 1, (rtime->tm_hour >= 12) ? "pm" : "am");
	}

	if (lcd_hgt >= 4) {				// 4-line version of the screen
		char *day = days[rtime->tm_wday];
		char *month = months[rtime->tm_mon];

		sprintf (tmp, "%s %s", now, day);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf (buffer, "widget_set O two %i 3 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);

		sprintf (tmp, "%s %d, %d", month, rtime->tm_mday, rtime->tm_year + 1900);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf (buffer, "widget_set O three %i 4 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);
	}
	else {						// 2-line version of the screen
		if (lcd_wid >= 20)			// 20+x columns
			sprintf (tmp, "%d-%02d-%02d %s",
				rtime->tm_year + 1900, rtime->tm_mon + 1, rtime->tm_mday, now);
		else					// <20 columns
			sprintf (tmp, "%02d/%02d %s",
				rtime->tm_mon + 1, rtime->tm_mday, now);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		sprintf(buffer, "widget_set O one %i 2 {%s}\n", xoffs, tmp);
		if (display)
			sock_send_string (sock, buffer);
	}

	return 0;
}										  // End clock_screen()

////////////////////////////////////////////////////////////////////
// Uptime Screen shows info about system uptime and OS version
//
//+--------------------+	+--------------------+
//|## SYSTEM UPTIME ##@|	|# Linux 2.6.11: my#@|
//|       myhost       |	| xxx days hh:mm:ss  |
//| xxx days hh:mm:ss  |	+--------------------+
//|   Linux 2.6.11     |
//+--------------------+
//
int
uptime_screen (int rep, int display, int *flags_ptr)
{
	int xoffs;
	int days, hour, min, sec;
	double uptime, idle;
	static int heartbeat = 0;
	static char colon[] = {':', ' '};

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string (sock, "screen_add U\n");
		sprintf (buffer, "screen_set U -name {Uptime Screen: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add U title title\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add U one string\n");
			sock_send_string (sock, "widget_add U two string\n");
			sock_send_string (sock, "widget_add U three string\n");

			sock_send_string (sock, "widget_set U title {SYSTEM UPTIME}\n");

			sprintf (tmp, "%s", get_hostname());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sprintf (buffer, "widget_set U one %i 2 {%s}\n", xoffs, tmp);
			sock_send_string (sock, buffer);

			sprintf (tmp, "%s %s", get_sysname(), get_sysrelease());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sprintf (buffer, "widget_set U three %i 4 {%s}\n", xoffs, tmp);
			sock_send_string (sock, buffer);
		} else {
			sock_send_string (sock, "widget_add U one string\n");

			sprintf (tmp, "widget_set U title {%s %s: %s}\n", get_sysname(), get_sysrelease(), get_hostname());
			sock_send_string (sock, tmp);
		}
	}

	// toggle colon display
	heartbeat ^= 1;

	machine_get_uptime(&uptime, &idle);
	days = (int) uptime / 86400;
	hour = ((int) uptime % 86400) / 60 / 60;
	min = (((int) uptime % 86400) % 3600) / 60;
	sec = ((int) uptime % 60);
	if (lcd_wid >= 20)
		sprintf (tmp, "%d day%s %02d%c%02d%c%02d",
			days, ((days != 1) ? "s" : ""), hour, colon[heartbeat], min, colon[heartbeat], sec);
	else
		sprintf (tmp, "%dd %02d%c%02d%c%02d",
			days, hour, colon[heartbeat], min, colon[heartbeat], sec);

	xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
	if (lcd_hgt >= 4)
		sprintf (buffer, "widget_set U two %d 3 {%s}\n", xoffs, tmp);
	else
		sprintf (buffer, "widget_set U one %d 2 {%s}\n", xoffs, tmp);
	if (display)
		sock_send_string (sock, buffer);

	return 0;
}										  // End uptime_screen()

//////////////////////////////////////////////////////////////////////
// Big Clock Screen displays current time...
//
// +--------------------+
// |    _   _      _  _ |
// |  ||_ . _||_|. _|  ||
// |  ||_|. _|  |.|_   ||
// |                    |
// +--------------------+
//
int
big_clock_screen (int rep, int display, int *flags_ptr)
{
	time_t thetime;
	struct tm *rtime;
	int pos[] = { 1, 4, 8, 11, 15, 18 };
	char  cmdbuf[64] ;
	char fulltxt[16], old_fulltxt[16];
	static int heartbeat = 0;
	int j = 0;

	// toggle colon display
	heartbeat ^= 1;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string (sock, "screen_add K\n");
		sock_send_string (sock, "screen_set K -name {Big Clock Screen} -heartbeat off\n");
		sock_send_string (sock, "widget_add K d0 num\n");
		sock_send_string (sock, "widget_add K d1 num\n");
		sock_send_string (sock, "widget_add K d2 num\n");
		sock_send_string (sock, "widget_add K d3 num\n");
		sock_send_string (sock, "widget_add K d4 num\n");
		sock_send_string (sock, "widget_add K d5 num\n");
		sock_send_string (sock, "widget_add K c0 num\n");
		sock_send_string (sock, "widget_add K c1 num\n");

		sock_send_string (sock, "widget_set K d0 1 0\n");
		sock_send_string (sock, "widget_set K d1 4 0\n");
		sock_send_string (sock, "widget_set K d2 8 0\n");
		sock_send_string (sock, "widget_set K d3 11 0\n");
		sock_send_string (sock, "widget_set K d4 15 0\n");
		sock_send_string (sock, "widget_set K d5 18 0\n");

		strcpy(old_fulltxt, "000000");
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

	if (heartbeat) {	// 10 means: colon
		sock_send_string (sock, "widget_set K c0 7 10\n");
		sock_send_string (sock, "widget_set K c1 14 10\n");
	}
	else {			// kludge: use illegal number to clear colon display
		sock_send_string (sock, "widget_set K c0 7 11\n");
		sock_send_string (sock, "widget_set K c1 14 11\n");
	}	

	return 0;
}										  // End big_clock_screen()


//////////////////////////////////////////////////////////////////////
// Mini Clock Screen displays the current time with hours & minutes only
//
//+--------------------+	+--------------------+
//|                    |	|       11:32        |
//|       11:32        |	|                    |
//|                    |	+--------------------+
//|                    |
//+--------------------+
//
int
mini_clock_screen (int rep, int display, int *flags_ptr)
{
	time_t thetime;
	struct tm *rtime;
	static char colon[] = {':', ' '};
	static int heartbeat = 0;

	// toggle colon display
	heartbeat ^= 1;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string (sock, "screen_add N\n");
		sock_send_string (sock, "screen_set N -name {Essential Clock Screen} -heartbeat off\n");
		sock_send_string (sock, "widget_add N one string\n");
	}

	time (&thetime);
	rtime = localtime (&thetime);

	sprintf(tmp, "%02d%c%02d", rtime->tm_hour, colon[heartbeat & 0x01], rtime->tm_min);
	sprintf(buffer, "widget_set N one %d %d {%s}\n",
			((lcd_wid - 5) / 2) + 1, (lcd_hgt / 2), tmp);
	sock_send_string (sock, buffer);

	return 0;
}										  // End mini_clock_screen()
