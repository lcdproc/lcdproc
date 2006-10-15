
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
#include "machine.h"
#ifdef LCDPROC_EYEBOXONE
# include "eyebox.h"
#endif  

// TODO: Clean this up...  Support multiple display sizes..


int
mode_init()
{
	machine_init();

	return(0);
}

void
mode_close()
{
	machine_close();
}

int
update_screen(mode *m, int display)
{
	static int status = -1;
	int old_status = status;

	if (m && m->func) {
#ifdef LCDPROC_EYEBOXONE
		int init_flag = (m->flags & INITIALIZED);
#endif  
		status = m->func(m->timer, display, &(m->flags));
#ifdef LCDPROC_EYEBOXONE
		/* Eyebox Init */
		if (init_flag == 0)
			eyebox_screen(m->which,0);
		/* Eyebox Flush */
		eyebox_screen(m->which,1);
#endif  
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
		sock_printf(sock, "widget_set A title {LCDPROC %s}\n", version);
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
			sock_printf(sock, "widget_set A text 1 2 %d 2 h 8 { for Linux & *BSD by William Ferrell, Scott Scriven}\n", lcd_wid);
		}
	}

	return(0);
}  // End credit_screen()

