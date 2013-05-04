/** \file clients/lcdproc/chrono.c
 * Implements the 'OldTime', 'TimeDate', 'Uptime', 'MiniClock', and 'BigClock'
 * screens.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

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

#include "shared/configfile.h"
#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "chrono.h"


static char *tickTime(char *time, int heartbeat);


/**
 * TimeDate Screen displays current time and date, uptime, OS ver...
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## Linux 2.6.11 ###@|	|### TIME: myhost ##@|
 * |Up xxx days hh:mm:ss|	|17.05.2005 11:32:57a|
 * |  Wed May 17, 1998  |	+--------------------+
 * |11:32:57a  100% idle|
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
time_screen(int rep, int display, int *flags_ptr)
{
	char now[40];
	char today[40];
	int xoffs;
	int days, hour, min, sec;
	static int heartbeat = 0;
	static const char *timeFormat = NULL;
	static const char *dateFormat = NULL;
	time_t thetime;
	struct tm *rtime;
	double uptime, idle;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		/* get config values */
		timeFormat = config_get_string("TimeDate", "TimeFormat", 0, "%H:%M:%S");
		dateFormat = config_get_string("TimeDate", "DateFormat", 0, "%b %d %Y");

		sock_send_string(sock, "screen_add T\n");
		sock_printf(sock, "screen_set T -name {Time Screen: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add T title title\n");
		sock_send_string(sock, "widget_add T one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add T two string\n");
			sock_send_string(sock, "widget_add T three string\n");

			/* write title bar: OS name, OS version, hostname */
			sock_printf(sock, "widget_set T title {%s %s: %s}\n",
				get_sysname(), get_sysrelease(), get_hostname());
		}
		else {
			/* write title bar: hostname */
			sock_printf(sock, "widget_set T title {TIME: %s}\n", get_hostname());
		}
	}

	/* toggle colon display */
	heartbeat ^= 1;

	time(&thetime);
	rtime = localtime(&thetime);

	if (strftime(today, sizeof(today), dateFormat, rtime) == 0)
		*today = '\0';
	if (strftime(now, sizeof(now), timeFormat, rtime) == 0)
		*now = '\0';
	tickTime(now, heartbeat);

	if (lcd_hgt >= 4) {
		char tmp[40];	/* should be large enough */

		machine_get_uptime(&uptime, &idle);

		/* display the uptime... */
		days = (int) uptime / 86400;
		hour = ((int) uptime % 86400) / 3600;
		min  = ((int) uptime % 3600) / 60;
		sec  = ((int) uptime % 60);

		if (lcd_wid >= 20)
			sprintf(tmp, "Up %3d day%s %02d:%02d:%02d",
				days, ((days != 1) ? "s" : ""), hour, min, sec);
		else
			sprintf(tmp, "Up %dd %02d:%02d:%02d", days, hour, min, sec);

		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set T one %i 2 {%s}\n", xoffs, tmp);

		/* display the date */
		xoffs = (lcd_wid > strlen(today)) ? ((lcd_wid - strlen(today)) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set T two %i 3 {%s}\n", xoffs, today);

		/* display the time & idle time... */
		sprintf(tmp, "%s %3i%% idle", now, (int) idle);
		xoffs = (lcd_wid > strlen(tmp)) ? ((lcd_wid - strlen(tmp)) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set T three %i 4 {%s}\n", xoffs, tmp);
	}
	else {			/* 2 line version of the screen */
		xoffs = (lcd_wid > (strlen(today) + strlen(now) + 1))
			? ((lcd_wid - ((strlen(today) + strlen(now) + 1))) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set T one %i 2 {%s %s}\n", xoffs, today, now);
	}

	return 0;
}				/* End time_screen() */


/**
 * OldTime Screen displays current time and date...
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## DATE & TIME ####@|	|### TIME: myhost ##@|
 * |       myhost       |	|2005-05-17 11:32:57a|
 * |11:32:75a Wednesday,|	+--------------------+
 * |       May 17, 2005 |
 * +--------------------+
 *
 * Alternate 2-line version without the title bar:
 *
 * +----------------+
 * |   2012-12-27  @|
 * |    15:07:01    |
 * +----------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
clock_screen(int rep, int display, int *flags_ptr)
{
	char now[40];
	char today[40];
	int xoffs;
	static int heartbeat = 0;
	static int showTitle = 1;
	static const char *timeFormat = NULL;
	static const char *dateFormat = NULL;
	time_t thetime;
	struct tm *rtime;

	if ((*flags_ptr & INITIALIZED) == 0) {
		char tmp[257];	/* should be large enough for host name */

		*flags_ptr |= INITIALIZED;

		/* get config values */
		timeFormat = config_get_string("OldTime", "TimeFormat", 0, "%H:%M:%S");
		dateFormat = config_get_string("OldTime", "DateFormat", 0, "%b %d %Y");
		showTitle = config_get_bool("OldTime", "ShowTitle", 0, 1);

		sock_send_string(sock, "screen_add O\n");
		sock_printf(sock, "screen_set O -name {Old Clock Screen: %s}\n", get_hostname());
		if (!showTitle)
			sock_send_string(sock, "screen_set O -heartbeat off\n");
		sock_send_string(sock, "widget_add O one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add O title title\n");
			sock_send_string(sock, "widget_add O two string\n");
			sock_send_string(sock, "widget_add O three string\n");

			sock_printf(sock, "widget_set O title {DATE & TIME}\n");

			sprintf(tmp, "%s", get_hostname());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sock_printf(sock, "widget_set O one %i 2 {%s}\n", xoffs, tmp);
		}
		else {
			if (showTitle) {
				sock_send_string(sock, "widget_add O title title\n");
				sock_printf(sock, "widget_set O title {TIME: %s}\n", get_hostname());
			}
			else {
				sock_send_string(sock, "widget_add O two string\n");
			}
		}
	}

	/* toggle colon display */
	heartbeat ^= 1;

	time(&thetime);
	rtime = localtime(&thetime);

	if (strftime(today, sizeof(today), dateFormat, rtime) == 0)
		*today = '\0';
	if (strftime(now, sizeof(now), timeFormat, rtime) == 0)
		*now = '\0';
	tickTime(now, heartbeat);

	if (lcd_hgt >= 4) {	/* 4-line version of the screen */
		xoffs = (lcd_wid > strlen(today)) ? ((lcd_wid - strlen(today)) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set O two %i 3 {%s}\n", xoffs, today);

		xoffs = (lcd_wid > strlen(now)) ? ((lcd_wid - strlen(now)) / 2) + 1 : 1;
		if (display)
			sock_printf(sock, "widget_set O three %i 4 {%s}\n", xoffs, now);
	}
	else {			/* 2-line version of the screen */
		if (showTitle) {
			xoffs = (lcd_wid > (strlen(today) + strlen(now) + 1))
				? ((lcd_wid - ((strlen(today) + strlen(now) + 1))) / 2) + 1 : 1;
			if (display)
				sock_printf(sock, "widget_set O one %i 2 {%s %s}\n", xoffs, today, now);
		}
		else {
			xoffs = (lcd_wid > strlen(today)) ? ((lcd_wid - strlen(today)) / 2) + 1 : 1;
			if (display)
				sock_printf(sock, "widget_set O one %i 1 {%s}\n", xoffs, today);
			xoffs = (lcd_wid > strlen(now)) ? ((lcd_wid - strlen(now)) / 2) + 1 : 1;
			if (display)
				sock_printf(sock, "widget_set O two %i 2 {%s}\n", xoffs, now);
		}
	}

	return 0;
}				/* End clock_screen() */


/**
 * Uptime Screen shows info about system uptime and OS version
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## SYSTEM UPTIME ##@|	|# Linux 2.6.11: my#@|
 * |       myhost       |	| xxx days hh:mm:ss  |
 * | xxx days hh:mm:ss  |	+--------------------+
 * |   Linux 2.6.11     |
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
uptime_screen(int rep, int display, int *flags_ptr)
{
	int xoffs;
	int days, hour, min, sec;
	double uptime, idle;
	static int heartbeat = 0;
	char tmp[257];	/* should be large enough for host name */

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add U\n");
		sock_printf(sock, "screen_set U -name {Uptime Screen: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add U title title\n");
		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add U one string\n");
			sock_send_string(sock, "widget_add U two string\n");
			sock_send_string(sock, "widget_add U three string\n");

			sock_send_string(sock, "widget_set U title {SYSTEM UPTIME}\n");

			sprintf(tmp, "%s", get_hostname());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sock_printf(sock, "widget_set U one %i 2 {%s}\n", xoffs, tmp);

			sprintf(tmp, "%s %s", get_sysname(), get_sysrelease());
			xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
			sock_printf(sock, "widget_set U three %i 4 {%s}\n", xoffs, tmp);
		}
		else {
			sock_send_string(sock, "widget_add U one string\n");

			sock_printf(sock, "widget_set U title {%s %s: %s}\n",
					get_sysname(), get_sysrelease(), get_hostname());
		}
	}

	/* toggle colon display */
	heartbeat ^= 1;

	machine_get_uptime(&uptime, &idle);
	days = (int) uptime / 86400;
	hour = ((int) uptime % 86400) / 3600;
	min =  ((int) uptime % 3600) / 60;
	sec =  ((int) uptime % 60);
	if (lcd_wid >= 20)
		sprintf(tmp, "%d day%s %02d:%02d:%02d",
			days, ((days != 1) ? "s" : ""), hour, min, sec);
	else
		sprintf(tmp, "%dd %02d:%02d:%02d", days, hour, min, sec);

	if (display) {
		xoffs = (lcd_wid > strlen(tmp)) ? (((lcd_wid - strlen(tmp)) / 2) + 1) : 1;
		if (lcd_hgt >= 4)
			sock_printf(sock, "widget_set U two %d 3 {%s}\n", xoffs, tmp);
		else
			sock_printf(sock, "widget_set U one %d 2 {%s}\n", xoffs, tmp);
	}

	return 0;
}				/* End uptime_screen() */


/**
 * Big Clock Screen displays current time...
 *
 *\verbatim
 *
 * +--------------------+
 * |    _   _      _  _ |
 * |  ||_ . _||_|. _|  ||
 * |  ||_|. _|  |.|_   ||
 * |                    |
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
big_clock_screen(int rep, int display, int *flags_ptr)
{
	time_t thetime;
	struct tm *rtime;
	int pos[] = {1, 4, 8, 11, 15, 18};
	char fulltxt[16];
	static char old_fulltxt[16];
	static int heartbeat = 0;
	static int TwentyFourHour = 1;
	int j = 0;
	int digits = (lcd_wid >= 20) ? 6 : 4;
	int xoffs = (lcd_wid + 1 - (pos[digits - 1] + 2)) / 2;

	/* toggle colon display */
	heartbeat ^= 1;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add K\n");
		sock_send_string(sock, "screen_set K -name {Big Clock Screen} -heartbeat off\n");
		sock_send_string(sock, "widget_add K d0 num\n");
		sock_send_string(sock, "widget_add K d1 num\n");
		sock_send_string(sock, "widget_add K d2 num\n");
		sock_send_string(sock, "widget_add K d3 num\n");
		sock_send_string(sock, "widget_add K c0 num\n");

		if (digits > 4) {
			sock_send_string(sock, "widget_add K d4 num\n");
			sock_send_string(sock, "widget_add K d5 num\n");
			sock_send_string(sock, "widget_add K c1 num\n");
		}

		strcpy(old_fulltxt, "      ");
	}

	time(&thetime);
	rtime = localtime(&thetime);

	sprintf(fulltxt, "%02d%02d%02d",
			((TwentyFourHour) ? rtime->tm_hour : (((rtime->tm_hour + 11) % 12) + 1)),
			rtime->tm_min, rtime->tm_sec);

	for (j = 0; j < digits; j++) {
		if (fulltxt[j] != old_fulltxt[j]) {
			sock_printf(sock, "widget_set K d%d %d %c\n", j, xoffs+pos[j], fulltxt[j]);
			old_fulltxt[j] = fulltxt[j];
		}
	}

	if (heartbeat) {	/* 10 means: colon */
		sock_printf(sock, "widget_set K c0 %d 10\n", xoffs + 7);
		if (digits > 4)
			sock_printf(sock, "widget_set K c1 %d 10\n", xoffs + 14);
	}
	else {			/* kludge: use illegal number to clear colon display */
		sock_printf(sock, "widget_set K c0 %d 11\n", xoffs + 7);
		if (digits > 4)
			sock_printf(sock, "widget_set K c1 %d 11\n", xoffs + 14);
	}

	return 0;
}				/* End big_clock_screen() */


/**
 * MiniClock Screen displays the current time with hours & minutes only
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |                    |	|       11:32        |
 * |       11:32        |	|                    |
 * |                    |	+--------------------+
 * |                    |
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
mini_clock_screen(int rep, int display, int *flags_ptr)
{
	char now[40];
	time_t thetime;
	struct tm *rtime;
	static const char *timeFormat = NULL;
	static int heartbeat = 0;
	int xoffs;

	/* toggle colon display */
	heartbeat ^= 1;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		/* get config values */
		timeFormat = config_get_string("MiniClock", "TimeFormat", 0, "%H:%M");

		sock_send_string(sock, "screen_add N\n");
		sock_send_string(sock, "screen_set N -name {Mini Clock Screen} -heartbeat off\n");
		sock_send_string(sock, "widget_add N one string\n");
	}

	time(&thetime);
	rtime = localtime(&thetime);

	if (strftime(now, sizeof(now), timeFormat, rtime) == 0)
		*now = '\0';
	tickTime(now, heartbeat);

	xoffs = (lcd_wid > strlen(now)) ? (((lcd_wid - strlen(now)) / 2) + 1) : 1;
	sock_printf(sock, "widget_set N one %d %d {%s}\n", xoffs, (lcd_hgt / 2), now);

	return 0;
}				/* End mini_clock_screen() */


/** Helper function: toggle between ':' and ' ' in time strings.
 * \note The time string passed is modified directly!
 * \param time       String containing a formatted time value.
 * \param heartbeat  Even numbers to display ':', odd to display ' '.
 * \return  Pointer to the modfied time string.
 */
static char *
tickTime(char *time, int heartbeat)
{
	if (time != NULL) {
		static char colon[] = {':', ' '};
		char *ptr = time;

		for (heartbeat %= 2; *ptr != '\0'; ptr++) {
			if (*ptr == colon[0])
				*ptr = colon[heartbeat];
		}
	}
	return (time);
}

