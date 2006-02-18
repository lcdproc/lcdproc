
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined( IRIX ) || defined( SOLARIS )
#include <strings.h>
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
#include "machine.h"

// TODO: Clean this up...  Support multiple display sizes..

char *tmp    = NULL;
char *buffer = NULL;

int
mode_init()
{
	if ((buffer = malloc(1024)) == NULL) {
		perror("malloc buffer");
		return(0);
	}

	if ((tmp = malloc(1024)) == NULL) {
		perror("malloc tmp");
		return(0);
	}

	machine_init();

	return(0);
}

void
mode_close()
{
	machine_close();

	if (tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
}

int
update_screen(mode *m, int display)
{
	static int status = -1;
	int old_status = status;

	if (m)
	{
		switch(m->which)
		{
			case 'g':
			case 'G':
				status = cpu_graph_screen(m->timer, display, &(m->flags));
				break;
			case 'c':
			case 'C':
				status = cpu_screen(m->timer, display, &(m->flags));
				break;
			case 'o':
			case 'O':
				status = clock_screen(m->timer, display, &(m->flags));
				break;
			case 'k':
			case 'K':
				status = big_clock_screen(m->timer, display, &(m->flags));
				break;
			case 'm':
			case 'M':
				status = mem_screen(m->timer, display, &(m->flags));
				break;
			case 's':
			case 'S':
				status = mem_top_screen(m->timer, display, &(m->flags));
				break;
			case 'u':
			case 'U':
				status = uptime_screen(m->timer, display, &(m->flags));
				break;
			case 't':
			case 'T':
				status = time_screen(m->timer, display, &(m->flags));
				break;
			case 'd':
			case 'D':
				status = disk_screen(m->timer, display, &(m->flags));
				break;
			case 'l':
			case 'L':
				status = xload_screen(m->timer, display, &(m->flags));
				break;
			case 'b':
			case 'B':
				status = battery_screen(m->timer, display, &(m->flags));
				break;
			case 'a':
			case 'A':
				status = credit_screen(m->timer, display, &(m->flags));
				break;
			case 'p':
			case 'P':
				status = cpu_smp_screen(m->timer, display, &(m->flags));
				break;
			case 'n':
			case 'N':
				status = mini_clock_screen(m->timer, display, &(m->flags));
				break;
			default:
				break;
		}
	}

	if (status != old_status)
	{
		if (status == BACKLIGHT_OFF)
			sock_send_string(sock, "backlight off\n");
		if (status == BACKLIGHT_ON)
			sock_send_string(sock, "backlight on\n");
		if (status == BLINK_ON)
			sock_send_string(sock, "backlight blink\n");
	}

	return(status);
}


///////////////////////////////////////////////////////////////////////////
//////////////////////// Let the Modes Begin! /////////////////////////////
///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Credit Screen shows who wrote this...
//
int
credit_screen(int rep, int display, int *flags_ptr)
{
        if ((*flags_ptr & INITIALIZED) == 0) {
                *flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add A\n");
		sock_send_string(sock, "screen_set A -name {Credits for LCDproc}\n");
		sock_send_string(sock, "widget_add A title title\n");
		sprintf(buffer, "widget_set A title {LCDPROC %s}\n", version);
		sock_send_string(sock, buffer);
		if (lcd_hgt >= 4)
		{
			sock_send_string(sock, "widget_add A one string\n");
			sock_send_string(sock, "widget_add A two string\n");
			sock_send_string(sock, "widget_add A three string\n");
			sock_send_string(sock, "widget_set A one 1 2 { for Linux & *BSD}\n");
			sock_send_string(sock, "widget_set A two 1 3 { by William Ferrell}\n");
			sock_send_string(sock, "widget_set A three 1 4 { and Scott Scriven}\n");
		}
		else
		{
			sock_send_string(sock, "widget_add A text scroller\n");
			sprintf(buffer, "widget_set A text 1 2 %d 2 h 8 { for Linux & *BSD by William Ferrell, Scott Scriven}\n", lcd_wid);
			sock_send_string(sock, buffer);
		}
	}

	return(0);
}  // End credit_screen()

