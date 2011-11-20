/** \file clients/lcdproc/load.c
 * Implements the 'Load' screen.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "shared/configfile.h"
#include "shared/sockets.h"
#include "main.h"
#include "mode.h"
#include "machine.h"
#include "load.h"


/**
 * Shows a display very similar to "xload"'s histogram.
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## LOAD 0.44: myh #@|	|myhost 0.24        1|
 * |                   1|	|        |||||||||||0|
 * |            ||||||  |	+--------------------+
 * |    ||||  |||||||| 0|
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  The backlight state
 */
int
xload_screen(int rep, int display, int *flags_ptr)
{
	static int gauge_hgt = 0;
	static double loads[LCD_MAX_WIDTH];
	static double lowLoad = LOAD_MIN;
	static double highLoad = LOAD_MAX;
	int loadtop, i;
	double loadmax = 0, factor;
	int status = BACKLIGHT_ON;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		/* get config values */
		lowLoad = config_get_float("Load", "LowLoad", 0, LOAD_MIN);
		highLoad = config_get_float("Load", "HighLoad", 0, LOAD_MAX);

		gauge_hgt = (lcd_hgt > 2) ? (lcd_hgt - 1) : lcd_hgt;
		memset(loads, '\0', sizeof(double) * LCD_MAX_WIDTH);

		sock_send_string(sock, "screen_add L\n");
		sock_printf(sock, "screen_set L -name {Load: %s}\n", get_hostname());
		/* Add the vbars... */
		for (i = 1; i < lcd_wid; i++) {
			sock_printf(sock, "widget_add L bar%i vbar\n", i);
			sock_printf(sock, "widget_set L bar%i %i %i 0\n", i, i, lcd_hgt);
		}
		/* And add a title... */
		if (lcd_hgt > 2) {
			sock_send_string(sock, "widget_add L title title\n");
			sock_send_string(sock, "widget_set L title {LOAD        }\n");
		} else {
			sock_send_string(sock, "widget_add L title string\n");
			sock_send_string(sock, "widget_set L title 1 1 {LOAD}\n");
			sock_send_string(sock, "screen_set L -heartbeat off\n");
		}
		sock_send_string(sock, "widget_add L zero string\n");
		sock_send_string(sock, "widget_add L top string\n");
		sock_printf(sock, "widget_set L zero %i %i 0\n", lcd_wid, lcd_hgt);
		sock_printf(sock, "widget_set L top %i %i 1\n", lcd_wid, (lcd_hgt + 1 - gauge_hgt));
	}

	/* shift load history */
	for (i = 0; i < (lcd_wid - 2); i++)
		loads[i] = loads[i + 1];

	/*
	 * get new load value, ignore failure: the shift above makes sure we
	 * still have the previous value
	 */
	machine_get_loadavg(&(loads[lcd_wid - 2]));

	/* determine max. load from history */
	for (i = 0; i < lcd_wid - 1; i++)
		loadmax = max(loadmax, loads[i]);

	/* poor man's ceil() */
	loadtop = (int) loadmax;
	if (loadtop < loadmax)
		loadtop++;
	loadtop = max(1, loadtop);

	factor = (double) (lcd_cellhgt * gauge_hgt) / (double) loadtop;

	/* display load */
	sock_printf(sock, "widget_set L top %i %i %i\n", lcd_wid, (lcd_hgt + 1 - gauge_hgt), loadtop);

	for (i = 0; i < lcd_wid - 1; i++) {
		double x = loads[i] * factor;

		sock_printf(sock, "widget_set L bar%i %i %i %i\n", i + 1, i + 1, lcd_hgt, (int) x);
	}

	/* And now the title... */
	if (lcd_hgt > 2)
		sock_printf(sock, "widget_set L title {LOAD %2.2f: %s}\n", loads[lcd_wid - 2], get_hostname());
	else
		sock_printf(sock, "widget_set L title 1 1 {%s %2.2f}\n", get_hostname(), loads[lcd_wid - 2]);

	/* set return status depending on max & current load */
	if (lowLoad < highLoad) {
		status = (loadmax > lowLoad) ? BACKLIGHT_ON : BACKLIGHT_OFF;
		if (loads[lcd_wid - 2] > highLoad)
			status = BLINK_ON;
	}

	return status;
}
