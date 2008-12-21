
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
mode_init(void)
{
	machine_init();

	return(0);
}

void
mode_close(void)
{
	machine_close();
}

int
update_screen(ScreenMode *m, int display)
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
	const char *contributors[] = {
		"William Ferrell",
		"Scott Scriven",
		"Gareth Watts",
		"Lorand Bruhacs",
		"Benjamin Tse",
		"Matthias Prinke",
		"Richard Rognlie",
		"Tom Wheeley",
		"Bjoern Andersson",
		"Andrew McMeikan",
		"David Glaude",
		"Todd Porter",
		"Bjoern Andersson",
		"Jason Dale Woodward",
		"Ethan Dicks",
		"Michael Reinelt",
		"Simon Harrison",
		"Charles Steinkuehler",
		"Harald Klein",
		"Philip Pokorny",
		"Glen Gray",
		"David Douthitt",
		"Eddie Sheldrake",
		"Rene Wagner",
		"Andre Breiler",
		"Joris Robijn",
		"Guillaume Filion",
		"Chris Debenham",
		"Mark Haemmerling",
		"Robin Adams",
		"Manuel Stahl",
		"Mike Patnode",
		"Peter Marschall",
		"Markus Dolze",
		"Volker Boerchers",
		"Lucian Muresan",
		"Matteo Pillon",
		"Laurent Arnal",
		"Simon Funke",
		"Matthias Goebl",
		"Stefan Herdler",
		"Bernhard Walle",
		"Andrew Foss",
		"Anthony J. Mirabella",
		"Cedric Tessier",
		"John Sanders",
		NULL
	};
	int contr_num = 0;
	int i;

        if ((*flags_ptr & INITIALIZED) == 0) {
                *flags_ptr |= INITIALIZED;

		/* get number of contributors */
		for (contr_num = 0; contributors[contr_num] != NULL; contr_num++)
			;
		sock_send_string(sock, "screen_add A\n");
		sock_send_string(sock, "screen_set A -name {Credits for LCDproc}\n");
		sock_send_string(sock, "widget_add A title title\n");
		sock_printf(sock, "widget_set A title {LCDPROC %s}\n", version);
		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add A text scroller\n");
			sock_printf(sock, "widget_set A text 1 2 %d 2 h 8 {%s}\n",
					lcd_wid, "LCDproc was brought to you by:");
		}
		
		// frame from (2nd/3rd line, left) to (last line, right)
		sock_send_string(sock, "widget_add A f frame\n");
		sock_printf(sock, "widget_set A f 1 %i %i %i %i %i v %i\n",
				((lcd_hgt >= 4) ? 3 : 2), lcd_wid, lcd_hgt, lcd_wid, contr_num,
				// scroll rate: 1 line every X ticks (= 1/8 sec)
				((lcd_hgt >= 4) ? 8 : 12));

		// frame contents
		for (i = 1; i < contr_num; i++) {
			sock_printf(sock, "widget_add A c%i string -in f\n", i);
			sock_printf(sock, "widget_set A c%i 1 %i {%s}\n", i, i, contributors[i]);
		}	
	}

	return(0);
}  // End credit_screen()


