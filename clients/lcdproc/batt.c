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


typedef struct {
	int status;
	const char *name;
} NameTable;	


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


////////////////////////////////////////////////////////////////////////
// Battery Screen shows apm battery status...
//
// +--------------------+	+--------------------+
// |## Batt: 100%: an ##|	|## Batt: 100%: an ##|
// |AC: On              |	|AC, Batt: Absent    |
// |Batt: Absent        |	+--------------------+
// |E------------------F|
// +--------------------+
//
int
battery_screen (int rep, int display)
{
	static int first = TRUE;
	int acstat = 0, battstat = 0, percent = 0;
	int gauge_wid = lcd_wid - 2;

	if (first == TRUE) {
		first = FALSE;

		sock_send_string (sock, "screen_add B\n");
		sprintf (buffer, "screen_set B -name {APM stats: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add B title title\n");
		sprintf (buffer, "widget_set B title {LCDPROC %s}\n", version);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add B one string\n");
		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add B two string\n");
			sock_send_string (sock, "widget_add B three string\n");
			sock_send_string (sock, "widget_add B gauge hbar\n");
			sock_send_string (sock, "widget_set B one 1 2 {AC: Unknown}\n");
			sock_send_string (sock, "widget_set B two 1 3 {Batt: Unknown}\n");
			sprintf(buffer, "widget_set B three 1 4 {E%*sF}\n", gauge_wid, "");
			sock_send_string (sock, buffer);
			sock_send_string (sock, "widget_set B gauge 2 4 0\n");
		}
	}
	// Only run once every 16 frames...
	//if (rep & 0x0F) return 0;

	machine_get_battstat(&acstat, &battstat, &percent);

	if (percent >= 0)
		sprintf(tmp, "%d%%", percent);
	else
		sprintf(tmp, "??%%");
	sprintf(buffer, "widget_set B title {%s: %s}\n",
			(acstat == LCDP_AC_ON && battstat == LCDP_BATT_ABSENT) ? "AC" : "Batt:",
			tmp, get_hostname());
	if(display)
		sock_send_string (sock, buffer);

	if(lcd_hgt >= 4) {				  // 4-line version of the screen
		sprintf(buffer, "widget_set B one 1 2 {AC: %s}\n", ac_status(acstat));
		if (display)
			sock_send_string (sock, buffer);

		sprintf(buffer, "widget_set B two 1 3 {Batt: %s}\n", battery_status(battstat));
		if (display)
			sock_send_string(sock, buffer);

		if (percent > 0) {
			sprintf(buffer, "widget_set B gauge 2 4 %d\n", (percent * gauge_wid * lcd_cellwid) / 100);
			if(display)
				sock_send_string(sock, buffer);
		}
	}	
	else {						  // two-line version of the screen
		if (acstat == LCDP_AC_ON)
			sprintf(buffer, "widget_set B one 1 2 {AC, Batt: %s}\n", battery_status(battstat));
		else
			sprintf(buffer, "widget_set B one 1 2 {Batt: %s}\n", battery_status(battstat));
		if (display)
			sock_send_string(sock, buffer);
	}

	return 0;
}

