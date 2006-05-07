/*
   netlcdclient - Client for LCDproc which shows networks statistics

   Copyright (C) 2002 Luis Llorente Campo <luisllorente@luisllorente.com>
   Multiinterface Extension by Stephan Skrodzki <skrodzki@stevekist.de>
   Adaptions to lcdproc by Andrew Foss with fixes by M. Dolze
   Cleanup, reorganization by Peter Marschall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared/sockets.h"
#include "shared/debug.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "main.h"
#include "util.h"
#include "iface.h"

#define UNSET_INT -1
#define UNSET_STR "\01"


int iface_count = 0;  /* number of interfaces */

char unit_label[10] = "B";  /* default unit label is Bytes */
int transfer_screen = 0;  /* by default, transfer screen is not shown */

/* command line parameters, used by getopt_long() */
static struct option const long_options[] =
{
	{"interface", required_argument, 0, 'i'},
	{"alias", required_argument, 0, 'a'},
	{"server", required_argument, 0, 's'},
	{"port", required_argument, 0, 'p'},
	{"unit", required_argument, 0, 'u'},
	{"transfer", no_argument, 0, 't'},
	{"daemon", no_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},
	{NULL, 0, NULL, 0}
};


/* reads and parses configuration file */
static int
iface_process_configfile()
{
	char *unit;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

	/* Read config settings */

	for (iface_count = 0; iface_count < MAX_INTERFACES; iface_count++) {
		char iface_label[12];
		
		sprintf(iface_label, "Interface%i", iface_count);
		debug(RPT_DEBUG, "Label %s count %i\n", iface_label, iface_count);
		iface[iface_count].name = strdup(config_get_string("Iface", iface_label, 0, ""));
		if (iface[iface_count].name == NULL) {
			report(RPT_CRIT, "malloc failure");
			return -1;
		}	
		if (*iface[iface_count].name == '\0')
			break;
		sprintf(iface_label, "Alias%i", iface_count);
		iface[iface_count].alias = strdup(config_get_string("Iface", iface_label, 0, iface[iface_count].name));
		if (iface[iface_count].alias == NULL)
			// make alia the same as the interface name in case strdup() failed
			iface[iface_count].alias = iface[iface_count].name;
		debug(RPT_DEBUG, "Interface %i: %s alias %s\n",
			iface_count, iface[iface_count].name, iface[iface_count].alias);
	}
	
	unit = config_get_string("Iface", "Unit", 0, "byte");
	if ((strcasecmp(unit, "byte") == 0) ||
	    (strcasecmp(unit, "bytes") == 0))
		strncpy(unit_label, "B", sizeof(unit_label));
	else if ((strcasecmp(unit, "bit") == 0) ||
		 (strcasecmp(unit, "bits") == 0))
		strncpy(unit_label, "b", sizeof(unit_label));
	else if ((strcasecmp(unit, "packet") == 0) ||
		 (strcasecmp(unit, "packets") == 0))
		strncpy(unit_label, "pkt", sizeof(unit_label));
	else {
		report(RPT_ERR, "illegal Unit value: %s", unit);
		return -1;
	}	
	unit_label[sizeof(unit_label)-1] = '\0';

	transfer_screen = config_get_bool("Iface", "Transfer", 0, 0);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// IFace screen shows info about percentage of the Network interface usage/throughput
//
// +--------------------+	+--------------------+
// |## Net Load: LAN ##@|	|### Net Load ######@|
// |UL:       123.456 Kb|	|LAN: U: 34kb D: 56Mb|
// |DL:       654.321 Kb|	+--------------------+
// |Total:    777.777 Kb|
// +--------------------+
//
int
iface_screen(int rep, int display, int *flags_ptr)
{
	//ALF TODO need to make this actual time since last run
	unsigned int interval = difftime(time(NULL), iface[0].last_online);  /* interval since last update */
	int iface_nmbr;
	
	if (!interval)
		return 0; /* need at least 1 second, no divide by 0 */

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;
		
		/* get configuration options */
		iface_process_configfile();

		/* set initial speed screen with widgets */
		initialize_speed_screen();

		/* set initial transfer screen if needed */
		if (transfer_screen) {
			initialize_transfer_screen();
		}

		/* initialize all interface structs */
		for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
		    	iface[iface_nmbr].last_online = 0;
		    	iface[iface_nmbr].status = down;
		}
		return 0;
	}

	/* for each interface do */
	for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
		/*read iface_parameter stats */
		if (!get_iface_stats(&iface[iface_nmbr])) {
			/* there was an error, so we exit the loop */
	    		break;
	    	}

		/* actualize speed values in display */
	    	actualize_speed_screen(&iface[iface_nmbr], interval, iface_nmbr);

		/* if needed, actualize transfer values in display */
		if (transfer_screen)
			actualize_transfer_screen(&iface[iface_nmbr], iface_nmbr);

		/* Actual values are the old ones in the next loop */
		iface[iface_nmbr].rc_byte_old = iface[iface_nmbr].rc_byte;
		iface[iface_nmbr].tr_byte_old = iface[iface_nmbr].tr_byte;
		iface[iface_nmbr].rc_pkt_old = iface[iface_nmbr].rc_pkt;
		iface[iface_nmbr].tr_pkt_old = iface[iface_nmbr].tr_pkt;
	}

	return 0;
}	  // End iface_screen()


/*************************************************************************
 * Read interface statistics from system and  store in the struct
 * passed as a pointer. If there are no errors, it returns 1. If errors,
 * returns 0.
 *************************************************************************
 */

int
get_iface_stats (IfaceInfo *interface)
{
#ifdef linux
	FILE *file;   /* file handler */
	char buffer[1024];  /* buffer to work with the file */
	static int first_time = 1;  /* is it first time we call this function? */
	char *ch_pointer = NULL;  /* pointer to where interface values are in file */

	/* Open the file in read-only mode and parse */

	if ((file = fopen(DEVFILE, "r")) != NULL) {
		/* Skip first 2 header lines of file */
		fgets(buffer, sizeof(buffer), file);
		fgets(buffer, sizeof(buffer), file);
		
		/* By default, treat interface as down */
		interface->status = down;
		
		/* Search iface_name and scan values */
		while ((fgets(buffer, sizeof(buffer), file) != NULL)) {
			if (strstr(buffer, interface->name)) {
				/* interface exists */
				interface->status = up; /* is up */
				interface->last_online = time(NULL); /* save actual time */
				
				/* search ':' and skip over it */
				ch_pointer = strchr(buffer, ':');
				ch_pointer++;
				
				/* Now ch_pointer points to values of iface_name */
				/* Scan values from here */
				sscanf(ch_pointer, "%lf %lf %*s %*s %*s %*s %*s %*s %lf %lf",
					&interface->rc_byte,
					&interface->rc_pkt,
					&interface->tr_byte,
					&interface->tr_pkt);

				/* if is the first time we call this function,
				 * old values are the same as new so we don't
				 * get big speeds when calculating
				 */
				if (first_time) {
					interface->rc_byte_old = interface->rc_byte;
					interface->tr_byte_old = interface->tr_byte;
					interface->rc_pkt_old = interface->rc_pkt;
					interface->tr_pkt_old = interface->tr_pkt;
					first_time = 0;  /* now it isn't first time */
				}
			}
		} /* while */

		fclose(file);  /* close file */
		return 1;  /* everything went OK */

	}
	else {  /* error when opening the file */
		report(RPT_CRIT, "Error: Could not open %s\n", DEVFILE);
		return 0; /* something went wrong */
	}
#endif

#ifdef __FreeBSD__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <string.h>
#include <stdio.h>

	static int      first_time = 1;	/* is it first time we call this function? */
	int             rows;
	int             name[6] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_IFDATA, 0, IFDATA_GENERAL};
	size_t          len;
	struct ifmibdata ifmd; /* ifmibdata contains the network statistics */

	len = sizeof(rows);
	/* get number of interfaces */
	if (sysctlbyname("net.link.generic.system.ifcount", &rows, &len, NULL, 0) == 0) {
		interface->status = down; /* set status down by default */

		len = sizeof(ifmd);
		/* walk through all interfaces in the ifmib table from last to first */
		for ( ; rows > 0; rows--) {
			name[4] = rows; /* set the interface index */
			/* retrive the ifmibdata for the current index */
			if (sysctl(name, 6, &ifmd, &len, NULL, 0) == -1) {
				report(RPT_ERR, "Reading ifmibdata failed");
				break;
			}
			/* check if its interface name matches */
			if (strcmp(ifmd.ifmd_name, interface->name) == 0) {
				interface->last_online = time(NULL);	/* save actual time */

				if ((ifmd.ifmd_flags & IFF_UP) == IFF_UP)
					interface->status = up;	/* is up */

				interface->rc_byte = ifmd.ifmd_data.ifi_ibytes;
				interface->tr_byte = ifmd.ifmd_data.ifi_obytes;
				interface->rc_pkt = ifmd.ifmd_data.ifi_ipackets;
				interface->tr_pkt = ifmd.ifmd_data.ifi_opackets;

				if (first_time) {
					interface->rc_byte_old = interface->rc_byte;
					interface->tr_byte_old = interface->tr_byte;
					interface->rc_pkt_old = interface->rc_pkt;
					interface->tr_pkt_old = interface->tr_pkt;
					first_time = 0;	/* now it isn't first time */
				}
				return 1;
			}
		}
		/* if we are here there is no interface with the given name */
		report(RPT_CRIT, "There is no interface named %s", interface->name);
		return 0;
	} else {
		report(RPT_CRIT, "Reading interface count via sysctl failed");
		return 0;
	}

#endif				/* __FreeBSD__ */

} /* get_iface_stats() */

/*************************************************************************
 * Send commands to server to add speed screen with all required widgets
 *************************************************************************
 */
void
initialize_speed_screen(void)
{
	int iface_nmbr; /* interface number */

	/* Add screen */
	sock_send_string(sock, "screen_add I\n");

	/* Set screen name */
	sock_send_string(sock, "screen_set I name {Load}\n");

	/* Add title to screen */
	sock_send_string(sock, "widget_add I title title\n");

	if ((iface_count == 1) && (lcd_hgt >= 4 )) { /* Single interface mode */
		/* Set title */
		sock_printf(sock, "widget_set I title {Net Load: %s}\n", iface[0].alias);

		/* Add and set download, upload and total string widgets */
		sock_send_string(sock, "widget_add I dl string\n");
		sock_send_string(sock, "widget_set I dl 1 2 {DL:}\n");
		sock_send_string(sock, "widget_add I ul string\n");
		sock_send_string(sock, "widget_set I ul 1 3 {UL:}\n");
		sock_send_string(sock, "widget_add I total string\n");
		sock_send_string(sock, "widget_set I total 1 4 {Total:}\n");
	}
	else { /* multi-interfaces mode: one line per interface */
		/* Set title */
		if (strstr(unit_label, "B")) {
			sock_printf(sock, "widget_set I title {Net Load (bytes)}\n");
		}
		else {
			if (strstr(unit_label, "b")) {
				sock_printf(sock, "widget_set I title {Net Load (bits)}\n");
			}
			else {
				sock_printf(sock, "widget_set I title {Net Load (packets)}\n");
			}
		}
		
		sock_send_string (sock, "widget_add I f frame\n");

		// frame from (2, left) to (width, height) that is iface_count lines high
		sock_printf(sock, "widget_set I f 1 2 %d %d %d %d v 16\n",
				lcd_wid, lcd_hgt, lcd_wid, iface_count,
				// scroll rate: 1 line every X ticks (=1/8 sec)
				((lcd_hgt >= 4) ? 8 : 16));

		for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
			sock_printf(sock, "widget_add I i%1d string -in f\n", iface_nmbr);
			sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s NA (never)}\n",\
						iface_nmbr, iface_nmbr+1, iface[iface_nmbr].alias);
		}
	}

} /* initialize_speed_screen() */

/*************************************************************************
 * Format the value (in bytes) passed as parameter according to its scale,
 * adding the proper suffix. Store the formatted value string in the
 * variable 'buff' passed as pointer.
 *************************************************************************
 */
void
format_value (char *buff, double value, char *unit)
{
	/* Convert bytes to bits, if necessary */
	if (strstr(unit, "b"))
		value *= 8;

	/* If units are bytes, then divide by 2^10, otherwise by 10^3 */
	char *mag = convert_double(&value, (strstr(unit, "B")) ? 1024 : 1000, 1.0f);

	/* Formatting rules:
	 * - if original value was < 1000, output decimal value only
	 * - otherwise format with 3 precision
	 */
	if (mag[0] == 0)
		sprintf(buff, "%8ld %s", (long) value, unit);
	else
		sprintf(buff, "%7.3f %s%s", value, mag, unit);

} /* format_value() */

/*************************************************************************
 * Format the value (in bytes) passed as parameter according to its scale,
 * adding the proper suffix. Store the formatted value string in the
 * variable 'buff' passed as pointer. Version for multi-interfaces mode
 *************************************************************************
 */
void
format_value_multi_interface (char *buff, double value, char *unit)
{
	if (strstr(unit, "b"))
		value *= 8;

	char *mag = convert_double(&value, (strstr(unit, "B")) ? 1024 : 1000, 1.0f);

	/* Formatting rules:
	 * - if original value was < 1000, output decimal value only
	 * - with 1 precision if <10
	 * - decimal value with magnitude otherwise
	 */
	if (mag[0] == 0)
		sprintf(buff, "%4ld", (long) value);
	else if (value < 10)
		sprintf(buff, "%3.1f%s", value, mag);
	else
		sprintf(buff, "%3.f%s", value, mag);

} /* format_value_multi_interface() */

/*************************************************************************
 * Format the time in ASCII, depending on the elapsed time
 *************************************************************************
 */
void
get_time_string (char *buff, time_t last_online)
{

	time_t act_time;  /* actual time */
	char timebuff[30];

	act_time = time(NULL);

	if (last_online == 0) { /* never was online */
		sprintf(buff, "never");
		return;
	}
	
	/* Transform Unix time format to UTC time format */
	strcpy(timebuff, ctime(&last_online));

	if ((act_time - last_online) > 86400 ) { /* 86400 = 24 * 60 * 60. Is it more than 24 hours? */
		timebuff[10] = '\0';
		sprintf(buff, "%s", &timebuff[4]);
		return;
	}
	else {
		timebuff[19] = '\0';
		sprintf(buff, "%s", &timebuff[11]);
		return;
	}
} /* get_time_string() */

/*************************************************************************
 * Actualize values in display, calculating speeds in the defined interval
 * of time and sending proper commands to server. If measure unit is
 * 'pkt', we don't format this speed. Is always XXXX pkt/s
 *************************************************************************
 */
void
actualize_speed_screen(IfaceInfo *iface, unsigned int interval, int index)
{
	char speed[20];      /* buffer to store the formated speed string */
	double rc_speed;
	double tr_speed;

	if ((iface_count == 1) && ( lcd_hgt >= 4)) { /* single interface mode */	
		if (iface->status == up) {
			/* Calculate Download speed */
			if (strstr(unit_label, "pkt")) { /* don't format this value */
				rc_speed = (iface->rc_pkt - iface->rc_pkt_old) / interval;
				sprintf(speed, "%8ld %s", (long) rc_speed, unit_label);
			}
			else {
				rc_speed = (iface->rc_byte - iface->rc_byte_old) / interval;
				format_value(speed, rc_speed, unit_label);
			}

			/* Actualize Download speed */
			sock_printf(sock, "widget_set I dl 1 2 {DL: %*s/s}\n", lcd_wid - 6, speed);

			/* Calculate Upload speed */
			if (strstr(unit_label, "pkt")) { /* don't format this value */
				tr_speed = (iface->tr_pkt - iface->tr_pkt_old) / interval;
				sprintf(speed, "%8ld %s", (long)tr_speed, unit_label);
			}
			else {
				tr_speed = (iface->tr_byte - iface->tr_byte_old) / interval;
				format_value(speed, tr_speed, unit_label);
			}

			/* Actualize Upload speed */
			sock_printf(sock, "widget_set I ul 1 3 {UL: %*s/s}\n", lcd_wid - 6, speed);

			/* Calculate Total speed */
			if (strstr(unit_label, "pkt")) { /* don't format this value */
				sprintf(speed, "%7ld %s", (long)(rc_speed + tr_speed), unit_label);
			}
			else {
				format_value(speed, rc_speed + tr_speed, unit_label);
			}

			/* Actualize Total speed */
			sock_printf(sock, "widget_set I total 1 4 {Total: %*s/s}\n", lcd_wid - 9, speed);
		}
		else { /* Interface is down */
			get_time_string(speed, iface->last_online);
			sock_printf(sock, "widget_set I dl 1 2 {NA (%s)}\n", speed);
			sock_send_string(sock, "widget_set I ul 1 3 {}\n");
			sock_send_string(sock, "widget_set I total 1 4 {}\n");
		}
	}
	else { /* multi-interfaces mode: 1 line per interface */
		char speed1[20];      /* buffer to store the formated speed string */

		if (iface->status == up) {
			if (strstr(unit_label, "pkt")) {
				rc_speed = (iface->rc_pkt - iface->rc_pkt_old) / interval;
				tr_speed = (iface->tr_pkt - iface->tr_pkt_old) / interval;
			}
			else {
				rc_speed = (iface->rc_byte - iface->rc_byte_old) / interval;
				tr_speed = (iface->tr_byte - iface->tr_byte_old) / interval;
			}
			format_value_multi_interface(speed, rc_speed, unit_label);
			format_value_multi_interface(speed1, tr_speed, unit_label);
			sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s U:%.4s D:%.4s}\n",
					index, index+1, iface[index].alias, speed1, speed);
		}
		else { /* Interface is down */
			get_time_string(speed, iface->last_online);
			sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s NA (%s)}\n",
					index, index+1, iface[index].alias, speed);
		}
	}
} /* actualize_speed_screen() */

/*************************************************************************
 * Send commands to server to add transfer screen with all required widgets
 *************************************************************************
 */
void
initialize_transfer_screen(void)
{
	int iface_nmbr;  /* interface number */	

	/* Add screen */
	sock_send_string(sock, "screen_add NT\n");

	/* Set screen name */
	sock_send_string(sock, "screen_set NT name {Transfer}\n");

	/* Add title to screen */
	sock_send_string(sock, "widget_add NT title title\n");

	if ((iface_count == 1) && (lcd_hgt >= 4)) { /* single interface mode */
    		/* Set title */
		sock_printf(sock, "widget_set NT title {Transfer: %s}\n", iface[0].alias);

		/* Add and set download, upload and total string widgets */
		sock_send_string(sock, "widget_add NT dl string\n");
		sock_send_string(sock, "widget_set NT dl 1 2 {DL:}\n");
		sock_send_string(sock, "widget_add NT ul string\n");
		sock_send_string(sock, "widget_set NT ul 1 3 {UL:}\n");
		sock_send_string(sock, "widget_add NT total string\n");
		sock_send_string(sock, "widget_set NT total 1 4 {Total:}\n");
	}
	else { /* multi-interfaces mode: one line per interface */
		/* Set title (transfer screen is always in "bytes") */
		sock_send_string(sock, "widget_set NT title {Net Transfer (bytes)}\n");
		sock_send_string(sock, "widget_add NT f frame\n");

		// frame from (2, left) to (width, height) that is iface_count lines high
		sock_printf(sock, "widget_set NT f 1 2 %d %d %d %d v 16\n",
				lcd_wid, lcd_hgt, lcd_wid, iface_count,
				// scroll rate: 1 line every X ticks (=1/8 sec)
				((lcd_hgt >= 4) ? 8 : 16));

		for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
			sock_printf(sock, "widget_add NT i%1d string -in f\n", iface_nmbr);
			sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s NA (never)}\n",
					iface_nmbr, iface_nmbr+1, iface[iface_nmbr].alias);
		}
	}
} /* initialize_transfer_screen() */

/*************************************************************************
 * Actualize values in display, formatting transfer measures and sending
 * proper commands to server. Traffic is shown in "bytes" unit
 *************************************************************************
 */
void
actualize_transfer_screen(IfaceInfo *iface, int index)
{
	char transfer[20];   /* buffer to store the formated traffic string */

	if ((iface_count == 1) && (lcd_hgt >= 4)) { /* single interface mode */
		if (iface->status == up) {
			/* download traffic */
			format_value(transfer, iface->rc_byte, "B");
			sock_printf(sock, "widget_set NT dl 1 2 {DL: %*s}\n", lcd_wid - 4, transfer);

			/* upload traffic */
			format_value(transfer, iface->tr_byte, "B");
			sock_printf(sock, "widget_set NT ul 1 3 {UL: %*s}\n", lcd_wid - 4, transfer);

			/* total traffic */
			format_value(transfer, iface->rc_byte + iface->tr_byte, "B");
			sock_printf(sock, "widget_set NT total 1 4 {Total: %*s}\n", lcd_wid - 7, transfer);
		}
		else { /* Interface is down */
			get_time_string(transfer, iface->last_online);
			sock_printf(sock, "widget_set NT dl 1 2 {NA (%s)}\n", transfer);
			sock_send_string(sock, "widget_set NT ul 1 3 {}\n");
			sock_send_string(sock, "widget_set NT total 1 4 {}\n");
		}
	}
	else { /* multi-interfaces mode: one line per interface */
		char transfer1[20];   /* buffer to store the formated traffic string */

		if (iface->status == up) {
			format_value_multi_interface(transfer, iface->rc_byte, "B");
			format_value_multi_interface(transfer1, iface->tr_byte, "B");
			sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s U:%.4s D:%.4s}\n",
					index, index+1, iface[index].alias, transfer1, transfer);
		}
		else { /* Interface is down */
			get_time_string(transfer, iface->last_online);
			sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s NA (%s)}\n",
					index, index+1, iface[index].alias, transfer);
		}
	}
} /* actualize_transfer_screen() */
