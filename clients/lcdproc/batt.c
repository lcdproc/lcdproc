#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>

#include "../../shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "batt.h"

int batt_fd = 0;

static int get_batt_stat (int *acstat, int *battstat, int *battflag, int *percent);

static int
get_batt_stat (int *acstat, int *battstat, int *battflag, int *percent)
{
	char str[64];

	if (!batt_fd)
		batt_fd = open ("/proc/apm", O_RDONLY);
	if (batt_fd <= 0)
		return -1;

	if (lseek (batt_fd, 0, 0) != 0)
		return -1;

	if (read (batt_fd, str, sizeof (str) - 1) < 0)
		return -1;

	if (3 > sscanf (str + 13, "0x%x 0x%x 0x%x %d", acstat, battstat, battflag, percent))
		return -1;

	return 0;

}

int
batt_init ()
{
	static int first = 1;

	if (first) {
		first = 0;
		sock_send_string (sock, "screen_add B\n");
	}

	return 0;
}

int
batt_close ()
{
	if (batt_fd)
		close (batt_fd);
	batt_fd = 0;

	return 0;
}

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
	static int first = 1;

	int acstat = 0, battstat = 0, battflag = 0, percent = 0;

	if (first) {
		first = 0;

		sprintf (buffer, "screen_set B -name {APM stats: %s}\n", host);
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add B title title\n");
		sprintf (tmp, "widget_set B title {LCDPROC %s}\n", version);
		sock_send_string (sock, tmp);
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

	get_batt_stat (&acstat, &battstat, &battflag, &percent);

	if (percent >= 0)
		sprintf (buffer, "%i%%", percent);
	else
		sprintf (buffer, "??%%");
	sprintf (tmp, "widget_set B title {Batt: %s: %s}\n", buffer, host);

	if (display)
		sock_send_string (sock, tmp);

	if (lcd_hgt >= 4)				  // 4-line version of the screen
	{
		sprintf (tmp, "widget_set B one 1 2 {");
		switch (acstat) {
		case 0:
			sprintf (tmp + strlen (tmp), "AC: Off}\n");
			break;
		case 1:
			sprintf (tmp + strlen (tmp), "AC: On}\n");
			break;
		case 2:
			sprintf (tmp + strlen (tmp), "AC: Backup}\n");
			break;
		default:
			sprintf (tmp + strlen (tmp), "AC: Unknown}\n");
			break;
		}

		if (display)
			sock_send_string (sock, tmp);

		sprintf (tmp, "widget_set B two 1 3 {");
		if (battflag == 0xff) {
			sprintf (tmp + strlen (tmp), "Battery Stat Unknown");
		} else {
			sprintf (tmp + strlen (tmp), "Batt:");
			if (battflag & 1)
				sprintf (tmp + strlen (tmp), " High");
			if (battflag & 2)
				sprintf (tmp + strlen (tmp), " Low");
			if (battflag & 4)
				sprintf (tmp + strlen (tmp), " Critical");
			if (battflag & 8 || battstat == 3)
				sprintf (tmp + strlen (tmp), " Charging");
			if (battflag & 128)
				sprintf (tmp + strlen (tmp), " (NONE)");
		}
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);

		if (percent > 0) {
			sprintf (tmp, "widget_set B gauge 2 4 %i\n", (percent * ((lcd_wid - 2) * lcd_cellwid) / 100));
			if (display)
				sock_send_string (sock, tmp);
		}
	}									  // end if(lcd_hgt >= 4)
	else								  // two-line version of the screen
	{
		sprintf (tmp, "widget_set B one 1 2 {");
		if (acstat == 1)
			sprintf (tmp + strlen (tmp), "AC, ");

		if (battflag == 0xff) {
			sprintf (tmp + strlen (tmp), "No battery???");
		} else {
			sprintf (tmp + strlen (tmp), "Batt:");
			if (battflag & 1)
				sprintf (tmp + strlen (tmp), " High");
			if (battflag & 2)
				sprintf (tmp + strlen (tmp), " Low");
			if (battflag & 4)
				sprintf (tmp + strlen (tmp), " Critical");
			if (battflag & 8 || battstat == 3)
				sprintf (tmp + strlen (tmp), " Charging");
			if (battflag & 128)
				sprintf (tmp + strlen (tmp), " (NONE)");
		}
		sprintf (tmp + strlen (tmp), "}\n");
		if (display)
			sock_send_string (sock, tmp);

	}

	return 0;
}
