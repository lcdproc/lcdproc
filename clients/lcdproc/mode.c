/*
 * This file contains status-gathering code *and* modescreen functions.
 * It's long, and messy.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/utsname.h>

#include <string.h>
#if defined( IRIX ) || defined( SOLARIS )
# include <strings.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/sockets.h"

#include "mode.h"
#include "main.h"

#include "batt.h"
#include "chrono.h"
#include "cpu.h"
#include "cpu_smp.h"
#include "disk.h"
#include "load.h"
#include "mem.h"

int ELLIPSIS = '-';

// TODO: Clean this up...  Support multiple display sizes..

char buffer[1024];
char host[16];

void reread (int f, char *errmsg);
int getentry (const char *tag, const char *bufptr);

int
mode_init (mode * sequence)
{
	int i;
	struct utsname Uname;

	// Grab the host name
	if (0 > uname (&Uname)) {
		perror ("Can't get hostname");
		strcpy (host, "host");
	} else {
		strncpy (host, Uname.nodename, 15);
		host[16] = 0;
		if (index (host, '.'))	  // truncate at the first dot
			*(index (host, '.')) = '\0';
		for (i = 0; i < 16; i++)
			if (host[i] == '\n')
				host[i] = 0;
	}

	// Init all the modescreens we need
	for (i = 0; sequence[i].which > 1; i++) {
		switch (sequence[i].which) {
		case 'g':
		case 'G':					  // cpu_graph_screen
			cpu_init ();
			break;
		case 'c':
		case 'C':					  // cpu_screen
			cpu_init ();
			break;
		case 'o':
		case 'O':					  // clock_screen
			chrono_init ();
			break;
		case 'k':
		case 'K':					  // big_clock_screen
			chrono_init ();
			break;
		case 'm':
		case 'M':					  // mem_screen
			mem_init ();
			break;
		case 's':
		case 'S':					  // mem_info_screen
			mem_init ();
			break;
		case 'u':
		case 'U':					  // uptime_screen
			chrono_init ();
			break;
		case 't':
		case 'T':					  // time_screen
			chrono_init ();
			break;
		case 'd':
		case 'D':					  // disk_screen
			disk_init ();
			break;
		case 'x':
		case 'X':					  // xload_screen
			load_init ();
			break;
		case 'b':
		case 'B':					  // battery_screen
			batt_init ();
			break;
		case 'a':
		case 'A':					  // credit_screen
			break;
		case 'p':
		case 'P':					  // cpu_smp_screen
			cpu_smp_init ();
			break;
		default:
			break;
		}
	}

	return 0;
}

void
mode_close ()
{
	batt_close ();
	chrono_close ();
	cpu_close ();
	disk_close ();
	load_close ();
	mem_close ();
	cpu_smp_close ();
}

int
update_screen (mode * m, int display)
{
	static int status = -1;
	int old_status = status;

	if (m) {
		switch (m->which) {
		case 'g':
		case 'G':
			status = cpu_graph_screen (m->timer, display);
			break;
		case 'c':
		case 'C':
			status = cpu_screen (m->timer, display);
			break;
		case 'o':
		case 'O':
			status = clock_screen (m->timer, display);
			break;
		case 'k':
		case 'K':
			status = big_clock_screen (m->timer, display);
			break;
		case 'm':
		case 'M':
			status = mem_screen (m->timer, display);
			break;
		case 's':
		case 'S':
			status = mem_top_screen (m->timer, display);
			break;
		case 'u':
		case 'U':
			status = uptime_screen (m->timer, display);
			break;
		case 't':
		case 'T':
			status = time_screen (m->timer, display);
			break;
		case 'd':
		case 'D':
			status = disk_screen (m->timer, display);
			break;
		case 'x':
		case 'X':
			status = xload_screen (m->timer, display);
			break;
		case 'b':
		case 'B':
			status = battery_screen (m->timer, display);
			break;
		case 'a':
		case 'A':
			status = credit_screen (m->timer, display);
			break;
		case 'p':
		case 'P':
			status = cpu_smp_screen (m->timer, display);
			break;
		default:
			break;
		}
		/* Debugging tool...
		   if(display)
		   {
		   printf("Displayed Mode %c\n", m->which);
		   }
		 */
	}

	if (status != old_status) {
		if (status == BACKLIGHT_OFF) {
			sock_send_string (sock, "backlight off\n");
		}
		if (status == BACKLIGHT_ON) {
			sock_send_string (sock, "backlight on\n");
		}
		if (status == BLINK_ON) {
			sock_send_string (sock, "backlight blink\n");
		}
	}
	return status;

}

void
reread (int f, char *errmsg)
{
	if (lseek (f, 0L, 0) == 0 && read (f, buffer, sizeof (buffer) - 1) > 0)
		return;
	perror (errmsg);
	exit (1);
}

int
getentry (const char *tag, const char *bufptr)
{
	char *tail;
	int retval, len = strlen (tag);

	while (bufptr) {
		if (*bufptr == '\n')
			bufptr++;
		if (!strncmp (tag, bufptr, len)) {
			retval = strtol (bufptr + len, &tail, 10);
			if (tail == bufptr + len)
				return -1;
			else
				return retval;
		}
		bufptr = strchr (bufptr, '\n');
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////
//////////////////////// Let the Modes Begin! /////////////////////////////
///////////////////////////////////////////////////////////////////////////

// Er, these have been moved..  :)

char tmp[1024];

////////////////////////////////////////////////////////////////////////
// Credit Screen shows who wrote this...
//
int
credit_screen (int rep, int display)
{
	static int first = 1;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add A\n");
		sock_send_string (sock, "screen_set A -name {Credits for LCDproc}\n");
		sock_send_string (sock, "widget_add A title title\n");
		sprintf (tmp, "widget_set A title {LCDPROC %s}\n", version);
		sock_send_string (sock, tmp);
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add A one string\n");
			sock_send_string (sock, "widget_add A two string\n");
			sock_send_string (sock, "widget_add A three string\n");
			sock_send_string (sock, "widget_set A one 1 2 {     for Linux      }\n");
			sock_send_string (sock, "widget_set A two 1 3 { by William Ferrell}\n");
			sock_send_string (sock, "widget_set A three 1 4 { and Scott Scriven}\n");
		} else {
			sock_send_string (sock, "widget_add A text scroller\n");
			sock_send_string (sock, "widget_set A text 1 2 20 2 v 8 {     for Linux       by William Ferrell and Scott Scriven}\n");
		}
	}

	return 0;
}										  // End credit_screen()
