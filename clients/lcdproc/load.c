#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "load.h"

int loadavg_fd = 0;

static double get_loadavg (void);

static double
get_loadavg (void)
{
	double load;

	reread (loadavg_fd, "get_load:");
	sscanf (buffer, "%lf", &load);
	return load;
}

int
load_init ()
{
#ifndef SOLARIS
	if (!loadavg_fd)
		loadavg_fd = open ("/proc/loadavg", O_RDONLY);

#endif
	return 0;
}

int
load_close ()
{
#ifndef SOLARIS
	if (loadavg_fd)
		close (loadavg_fd);

	loadavg_fd = 0;

#endif
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Shows a display very similar to "xload"'s histogram.
//

int
xload_screen (int rep, int display)
{
	static int first = 1;
	static float loads[LCD_MAX_WIDTH];
	int n, i;
	float loadmax = 0, factor, x;
	int status = 0;

	if (first)						  // Only the first time this is ever called...
	{
		first = 0;
		memset (loads, 0, sizeof (float) * LCD_MAX_WIDTH);

		sock_send_string (sock, "screen_add X\n");
		sprintf (buffer, "screen_set X -name {X-Load: %s}\n", host);
		sock_send_string (sock, buffer);
		// Add the vbars...
		for (i = 1; i < lcd_wid; i++) {
			sprintf (tmp, "widget_add X %i vbar\n", i);
			sock_send_string (sock, tmp);
			sprintf (tmp, "widget_set X %i %i %i 0\n", i, i, lcd_hgt);
			sock_send_string (sock, tmp);
		}
		// And add a title...
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add X title title\n");
			sock_send_string (sock, "widget_set X title {LOAD        }\n");
		} else {
			sock_send_string (sock, "widget_add X title string\n");
			sock_send_string (sock, "widget_set X title 1 1 {LOAD}\n");
			sock_send_string (sock, "widget_del X heartbeat\n");
		}
		sock_send_string (sock, "widget_add X zero string\n");
		sock_send_string (sock, "widget_add X top string\n");
		sprintf (tmp, "widget_set X zero %i %i 0\n", lcd_wid, lcd_hgt);
		sock_send_string (sock, tmp);
		sprintf (tmp, "widget_set X top %i %i 1\n", lcd_wid, (lcd_hgt == 2) ? 1 : 2);
		sock_send_string (sock, tmp);
	}

	for (n = 0; n < (lcd_wid - 2); n++)
		loads[n] = loads[n + 1];
	loads[lcd_wid - 2] = get_loadavg ();

	for (n = 0; n < lcd_wid - 1; n++)
		if (loads[n] > loadmax)
			loadmax = loads[n];

	n = (int) loadmax;
	if ((float) n < loadmax) {
		n++;
	}
	sprintf (tmp, "widget_set X top %i %i %i\n", lcd_wid, (lcd_hgt == 2) ? 1 : 2, n);
	//if(display) sock_send_string(sock, tmp);
	sock_send_string (sock, tmp);

	if (loadmax < 1.0)
		factor = (float) (lcd_cellhgt) * ((lcd_hgt == 2) ? 2.0 : 3.0);
	else
		factor = (float) (lcd_cellhgt) * ((lcd_hgt == 2) ? 2.0 : 3.0) / (float) n;

	for (n = 0; n < lcd_wid - 1; n++) {
		x = (loads[n] * factor);

		sprintf (tmp, "widget_set X %i %i %i %i\n", n + 1, n + 1, lcd_hgt, (int) x);
		//if(display) sock_send_string(sock, tmp);
		sock_send_string (sock, tmp);
	}

	if (loadmax < LOAD_MIN) {
		status = BACKLIGHT_OFF;
	}
	if (loadmax > LOAD_MIN) {
		status = BACKLIGHT_ON;
	}
	if (loads[lcd_wid - 2] > LOAD_MAX) {
		status = BLINK_ON;
	}
	// And now the title...
	if (lcd_hgt >= 4)
		sprintf (tmp, "widget_set X title {LOAD %2.2f: %s}\n", loads[lcd_wid - 2], host);
	else
		sprintf (tmp, "widget_set X title 1 1 {%s %2.2f}\n", host, loads[lcd_wid - 2]);
	//if(display) sock_send_string(sock, tmp);
	sock_send_string (sock, tmp);

	return status;
}										  // End xload_screen()
