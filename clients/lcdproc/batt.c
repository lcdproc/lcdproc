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

static int first = TRUE;

////////////////////////////////////////////////////////////////////////
// Battery Screen shows apm battery status...
//

//####################
//## Battery: 100% ###
//AC: Unknown
//Batt: Low (Charging)
//E------------------F

int
battery_screen (int rep, int display)
{
	int acstat = 0, battflag = 0, percent = 0;

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
			sock_send_string (sock, "widget_set B three 1 4 {E                  F}\n");
			sock_send_string (sock, "widget_set B gauge 2 4 0\n");
		}
	}
	// Only run once every 16 frames...
	//if(rep&0x0f) return 0;

	machine_get_battstat(&acstat, &battflag, &percent);

	if(percent >= 0)
		sprintf(tmp, "%d%%", percent);
	else
		sprintf(tmp, "??%%");
	sprintf(buffer, "widget_set B title {Batt: %s: %s}\n", tmp, get_hostname());
	if(display)
		sock_send_string (sock, buffer);

	if(lcd_hgt >= 4)				  // 4-line version of the screen
	{
		strcpy(buffer, "widget_set B one 1 2 {AC: ");
		switch(acstat)
		{
			case LCDP_AC_OFF:
				strcat(buffer, "Off");
				break;
			case LCDP_AC_ON:
				strcat(buffer, "On");
				break;
			case LCDP_AC_BACKUP:
				strcat(buffer, "Backup");
				break;
			case LCDP_AC_UNKNOWN:
			default:
				strcat(buffer, "Unknown");
				break;
		}
		strcat(buffer, "}\n");
		if(display)
			sock_send_string (sock, buffer);

		strcpy(buffer, "widget_set B two 1 3 {Batt: ");
		switch(battflag)
		{
			case LCDP_BATT_HIGH:
				strcat(buffer, "High");
				break;
			case LCDP_BATT_LOW:
				strcat(buffer, "Low");
				break;
			case LCDP_BATT_CRITICAL:
				strcat(buffer, "Critical");
				break;
			case LCDP_BATT_CHARGING:
				strcat(buffer, "Charging");
				break;
			case LCDP_BATT_ABSENT:
				strcat(buffer, "Absent");
				break;
			case LCDP_BATT_UNKNOWN:
			default:
				strcat(buffer, "Unknown");
				break;
		}
		strcat(buffer, "}\n");
		if(display)
			sock_send_string(sock, buffer);

		if(percent > 0)
		{
			sprintf(buffer, "widget_set B gauge 2 4 %d\n", (percent * ((lcd_wid - 2) * lcd_cellwid) / 100));
			if(display)
				sock_send_string(sock, buffer);
		}
	}									  // end if(lcd_hgt >= 4)
	else								  // two-line version of the screen
	{
		strcpy(buffer, "widget_set B one 1 2 {");
		if(acstat == LCDP_AC_ON)
			strcat(buffer, "AC, ");
		strcat(buffer, "Batt: ");

		switch(battflag)
		{
			case LCDP_BATT_HIGH:
				strcat(buffer, "High");
				break;
			case LCDP_BATT_LOW:
				strcat(buffer, "Low");
				break;
			case LCDP_BATT_CRITICAL:
				strcat(buffer, "Critical");
				break;
			case LCDP_BATT_CHARGING:
				strcat(buffer, "Charging");
				break;
			case LCDP_BATT_ABSENT:
				strcat(buffer, "Absent");
				break;
			case LCDP_BATT_UNKNOWN:
			default:
				strcat(buffer, "Unknown");
				break;
		}
		strcat(buffer, "}\n");
		if(display)
			sock_send_string(sock, buffer);
	}

	return 0;
}
