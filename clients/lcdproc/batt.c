/** \file clients/lcdproc/batt.c
 * Implements the 'battery' screen showing the APM battery status.
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
#include <sys/utsname.h>

#include "shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "batt.h"
#include "machine.h"

/** Map status code > status text */
typedef struct {
	int status;
	const char *name;
} NameTable;

/**
 * Converts a numeric AC power status into a status text.
 *
 * \param status  Numeric status as returned by machine_get_battstat
 * \return  Name of the status
 */
static const char *
ac_status(int status)
{
	const NameTable ac_table[] = {
		{ LCDP_AC_OFF,     "Off"     },
		{ LCDP_AC_ON,      "On"      },
		{ LCDP_AC_BACKUP,  "Backup"  },
		{ LCDP_AC_UNKNOWN, "Unknown" },
		{ 0, NULL }
	};
	int i;

	for (i = 0; ac_table[i].name != NULL; i++)
		if (status == ac_table[i].status)
			return ac_table[i].name;

	return ac_table[LCDP_AC_UNKNOWN].name;
}

/**
 * Converts a numeric battery status into a status text.
 *
 * \param status  Numeric status as returned by machine_get_battstat
 * \return  Name of the status
 */
static const char *
battery_status(int status)
{
	const NameTable batt_table[] = {
		{ LCDP_BATT_HIGH,     "High"     },
		{ LCDP_BATT_LOW,      "Low"      },
		{ LCDP_BATT_CRITICAL, "Critical" },
		{ LCDP_BATT_CHARGING, "Charging" },
		{ LCDP_BATT_ABSENT,   "Absent"   },
		{ LCDP_BATT_UNKNOWN,  "Unknown"  },
		{ 0, NULL }
	};
	int i;

	for (i = 0; batt_table[i].name != NULL; i++)
		if (status == batt_table[i].status)
			return batt_table[i].name;

	return batt_table[LCDP_AC_UNKNOWN].name;
}

/**
 * Battery Screen shows apm battery status...
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## AC: 100%: myho #@|	|## AC: 100%: myho #@|
 * |AC: On              |	|AC, Batt: Absent    |
 * |Batt: Absent        |	+--------------------+
 * |E------------------F|
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
battery_screen(int rep, int display, int *flags_ptr)
{
	int acstat = 0, battstat = 0, percent = 0;
	int gauge_wid = lcd_wid - 2;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add B\n");
		sock_printf(sock, "screen_set B -name {APM stats: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add B title title\n");
		sock_printf(sock, "widget_set B title {LCDPROC %s}\n", version);
		sock_send_string(sock, "widget_add B one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string(sock, "widget_add B two string\n");
			sock_send_string(sock, "widget_add B three string\n");
			sock_send_string(sock, "widget_add B gauge hbar\n");

			sock_send_string(sock, "widget_set B one 1 2 {AC: Unknown}\n");
			sock_send_string(sock, "widget_set B two 1 3 {Batt: Unknown}\n");
			sock_printf(sock, "widget_set B three 1 4 {E%*sF}\n", gauge_wid, "");
			sock_send_string(sock, "widget_set B gauge 2 4 0\n");
		}
	}

	machine_get_battstat(&acstat, &battstat, &percent);

	if (display) {
		char tmp[20];

		if (percent >= 0)
			sprintf(tmp, "%d%%", percent);
		else
			sprintf(tmp, "??%%");
		sock_printf(sock, "widget_set B title {%s: %s: %s}\n",
				(acstat == LCDP_AC_ON && battstat == LCDP_BATT_ABSENT) ? "AC" : "Batt",
				tmp, get_hostname());

		if (lcd_hgt >= 4) {		/* 4-line version of the screen */
			sock_printf(sock, "widget_set B one 1 2 {AC: %s}\n", ac_status(acstat));
			sock_printf(sock, "widget_set B two 1 3 {Batt: %s}\n", battery_status(battstat));
			if (percent > 0)
				sock_printf(sock, "widget_set B gauge 2 4 %d\n",
						(percent * gauge_wid * lcd_cellwid) / 100);
		}
		else {				/* two-line version of the screen */
			sock_printf(sock, "widget_set B one 1 2 {%sBatt: %s}\n",
					(acstat == LCDP_AC_ON) ? "AC, " : "",
					battery_status(battstat));
		}
	}

	return 0;
}

