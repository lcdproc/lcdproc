#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "shared/sockets.h"
#include "main.h"
#include "mode.h"
#include "machine.h"
#include "load.h"


///////////////////////////////////////////////////////////////////////////
// Shows a display very similar to "xload"'s histogram.
//
// +--------------------+	+--------------------+
// |## LOAD 0.44: myh #@|	|myhost 0.24        1|
// |                   1|	|        |||||||||||0|
// |            ||||||  |	+--------------------+
// |    ||||  |||||||| 0|
// +--------------------+
//
int
xload_screen (int rep, int display, int *flags_ptr)
{
	static int gauge_hgt = 0;
	static double loads[LCD_MAX_WIDTH];
	int loadtop, i;
	double loadmax = 0, factor;
	int status = 0;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		gauge_hgt = (lcd_hgt > 2) ? (lcd_hgt - 1) : lcd_hgt;
		memset (loads, '\0', sizeof (double) * LCD_MAX_WIDTH);

		sock_send_string (sock, "screen_add X\n");
		sprintf (buffer, "screen_set X -name {X-Load: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		// Add the vbars...
		for (i = 1; i < lcd_wid; i++) {
			sprintf (tmp, "widget_add X bar%i vbar\n", i);
			sock_send_string (sock, tmp);
			sprintf (tmp, "widget_set X bar%i %i %i 0\n", i, i, lcd_hgt);
			sock_send_string (sock, tmp);
		}
		// And add a title...
		if (lcd_hgt > 2) {
			sock_send_string (sock, "widget_add X title title\n");
			sock_send_string (sock, "widget_set X title {LOAD        }\n");
		} else {
			sock_send_string (sock, "widget_add X title string\n");
			sock_send_string (sock, "widget_set X title 1 1 {LOAD}\n");
			sock_send_string (sock, "screen_set X -heartbeat off\n");
		}
		sock_send_string (sock, "widget_add X zero string\n");
		sock_send_string (sock, "widget_add X top string\n");
		sprintf (tmp, "widget_set X zero %i %i 0\n", lcd_wid, lcd_hgt);
		sock_send_string (sock, tmp);
		sprintf (tmp, "widget_set X top %i %i 1\n", lcd_wid, (lcd_hgt + 1 - gauge_hgt));
		sock_send_string (sock, tmp);
	}

	// shift load history
	for (i = 0; i < (lcd_wid - 2); i++)
		loads[i] = loads[i + 1];

	// get new load value
	machine_get_loadavg(&(loads[lcd_wid - 2]));

	// determine max. load from history
	for (i = 0; i < lcd_wid - 1; i++)
		loadmax = max(loadmax, loads[i]);

	// poor man's ceil()
	loadtop = (int) loadmax;
	if (loadtop < loadmax)
		loadtop++;
	loadtop = max(1, loadtop);

	factor = (double) (lcd_cellhgt * gauge_hgt) / (double) loadtop;

	// display load
	sprintf (tmp, "widget_set X top %i %i %i\n", lcd_wid, (lcd_hgt + 1 - gauge_hgt), loadtop);
	sock_send_string (sock, tmp);

	for (i = 0; i < lcd_wid - 1; i++) {
		double x = loads[i] * factor;

		sprintf (tmp, "widget_set X bar%i %i %i %i\n", i + 1, i + 1, lcd_hgt, (int) x);
		sock_send_string (sock, tmp);
	}

	// And now the title...
	if (lcd_hgt > 2)
		sprintf (tmp, "widget_set X title {LOAD %2.2f: %s}\n", loads[lcd_wid - 2], get_hostname());
	else
		sprintf (tmp, "widget_set X title 1 1 {%s %2.2f}\n", get_hostname(), loads[lcd_wid - 2]);
	sock_send_string (sock, tmp);

	// set return status depending on max & current load
	status = (loadmax >  LOAD_MIN) ? BACKLIGHT_ON : BACKLIGHT_OFF;
	if (loads[lcd_wid - 2] > LOAD_MAX)
		status = BLINK_ON;

	return status;
}										  // End xload_screen()
