/** \file clients/lcdproc/iface.c
 * Shows networks statistics. Imported from netlcdclient.
 */

/*-
 * Copyright (C) 2002 Luis Llorente Campo <luisllorente@luisllorente.com>
 *
 * Multi-interface extension by Stephan Skrodzki <skrodzki@stevekist.de>
 * Adaptions to lcdproc by Andrew Foss with fixes by M. Dolze
 * Cleanup, reorganization by Peter Marschall
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "shared/sockets.h"
#include "shared/report.h"
#include "main.h"
#include "machine.h"
#include "util.h"
#include "iface.h"

#include "elektragen.h"

#define UNSET_INT -1
#define UNSET_STR "\01"

static int iface_count = 0;	/* number of interfaces */
static IfaceUnit unit = IFACE_UNIT_BYTE; /* default unit label is Bytes */
static int transfer_screen = 0;	/* by default, transfer screen is not shown */

static inline char *
unit_label(IfaceUnit unit)
{
	switch (unit)
	{
	case IFACE_UNIT_BYTE:
		return "B";
	case IFACE_UNIT_BIT:
		return "b";
	case IFACE_UNIT_PACKET:
		return "pkt";
	default:
		return NULL;
	}
}


/** Reads and parses configuration file.
 * \return  0 on success, -1 on error
 */
static int
iface_process_config(Elektra * elektra)
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	/* Read config settings */
	for (iface_count = 0; iface_count < MAX_INTERFACES; iface_count++) {
		char iface_label[12];

		sprintf(iface_label, "Interface%i", iface_count);
		debug(RPT_DEBUG, "Label %s count %i", iface_label, iface_count);
		iface[iface_count].name = strdup(elektraGetV(elektra, CONF_IFACE_INTERFACE_NAME, iface_count));

		sprintf(iface_label, "Alias%i", iface_count);
		iface[iface_count].alias = strdup(elektraGetV(elektra, CONF_IFACE_INTERFACE_ALIAS, iface_count));
		if (strlen(iface[iface_count].alias) == 0)
			iface[iface_count].alias = iface[iface_count].name;
		
		debug(RPT_DEBUG, "Interface %i: %s alias %s",
		      iface_count, iface[iface_count].name, iface[iface_count].alias);
	}

	unit = elektraGet(elektra, CONF_IFACE_UNIT);
	transfer_screen = elektraGet(elektra, CONF_IFACE_TRANSFER);

	return 0;
}


/**
 * IFace screen shows info about percentage of the Network interface usage /
 * throughput.
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## Net Load: LAN ##@|	|### Net Load ######@|
 * |UL:       123.456 Kb|	|LAN: U: 34kb D: 56Mb|
 * |DL:       654.321 Kb|	+--------------------+
 * |Total:    777.777 Kb|
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \param elektra    Elektra instance holding the configuration
 * \return  Always 0
 */
int
iface_screen(int rep, int display, int *flags_ptr, Elektra * elektra)
{
	/* interval since last update */
	unsigned int interval = difftime(time(NULL), iface[0].last_online);
	int iface_nmbr;

	if (!interval)
		return 0;	/* need at least 1 second, no divide by 0 */

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		/* get configuration options */
		iface_process_config(elektra);

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
		/* read iface_parameter stats */
		if (!machine_get_iface_stats(&iface[iface_nmbr])) {
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
}


/**
 * Send commands to server to add speed screen with all required widgets.
 */
void
initialize_speed_screen(void)
{
	int iface_nmbr;	/* interface number */

	sock_send_string(sock, "screen_add I\n");
	sock_send_string(sock, "screen_set I name {Load}\n");
	sock_send_string(sock, "widget_add I title title\n");

	/* Single interface mode */
	if ((iface_count == 1) && (lcd_hgt >= 4 )) {
		sock_printf(sock, "widget_set I title {Net Load: %s}\n", iface[0].alias);
		sock_send_string(sock, "widget_add I dl string\n");
		sock_send_string(sock, "widget_set I dl 1 2 {DL:}\n");
		sock_send_string(sock, "widget_add I ul string\n");
		sock_send_string(sock, "widget_set I ul 1 3 {UL:}\n");
		sock_send_string(sock, "widget_add I total string\n");
		sock_send_string(sock, "widget_set I total 1 4 {Total:}\n");
	}
	/* multi-interfaces mode: one line per interface */
	else {
		/* Set title */
		switch (unit)
		{
		case IFACE_UNIT_BIT:
			sock_printf(sock, "widget_set I title {Net Load (bits)}\n");
			break;
		case IFACE_UNIT_BYTE:
			sock_printf(sock, "widget_set I title {Net Load (bytes)}\n");
			break;
		case IFACE_UNIT_PACKET:
			sock_printf(sock, "widget_set I title {Net Load (packets)}\n");
			break;
		default:
			break;
		}

		/* frame from (2, left) to (width, height) that is iface_count lines high */
		sock_send_string (sock, "widget_add I f frame\n");
		sock_printf(sock, "widget_set I f 1 2 %d %d %d %d v 16\n",
			    lcd_wid, lcd_hgt, lcd_wid, iface_count,
			    /* scroll rate: 1 line every X ticks (=1/8 sec) */
			    ((lcd_hgt >= 4) ? 8 : 16));

		/* Add interfaces to frame */
		for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
			sock_printf(sock, "widget_add I i%1d string -in f\n", iface_nmbr);
			sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s NA (never)}\n",
				    iface_nmbr, iface_nmbr+1, iface[iface_nmbr].alias);
		}
	}
}


/**
 * Format the value (in bytes) passed as parameter according to its scale,
 * adding the proper suffix. Store the formatted value string in the
 * variable 'buff' passed as pointer.
 * \param buff   Pointer to target buffer
 * \param value  Number of bytes
 * \param unit   String describing the unit. If this contains 'b', \c value is
 *               converted to bits. If this contains 'B', value is converted
 *               to binary multiples (1024 bytes = 1 KiB).
 */
void
format_value (char *buff, double value, IfaceUnit unit)
{
	char *mag;

	/* Convert bytes to bits, if necessary */
	if (unit == IFACE_UNIT_BIT)
		value *= 8;

	/* If units are bytes, then divide by 2^10, otherwise by 10^3 */
	mag = convert_double(&value, (unit == IFACE_UNIT_BYTE) ? 1024 : 1000, 1.0f);

	/*-
	 * Formatting rules:
	 * - if original value was < 1000, output decimal value only
	 * - otherwise format with 3 precision
	 */
	if (mag[0] == 0)
		sprintf(buff, "%8ld %s", (long) value, unit_label(unit));
	else
		sprintf(buff, "%7.3f %s%s", value, mag, unit_label(unit));
}


/**
 * Format the value (in bytes) passed as parameter according to its scale,
 * adding the proper suffix. Store the formatted value string in the
 * variable 'buff' passed as pointer. Version for multi-interfaces mode.
 * \param buff   Pointer to target buffer
 * \param value  Number of bytes
 * \param unit   String describing the unit. If this contains 'b', \c value is
 *               converted to bits. If this contains 'B', value is converted
 *               to binary multiples (1024 bytes = 1 KiB).
 */
void
format_value_multi_interface(char *buff, double value, IfaceUnit unit)
{
	char *mag;

	if (unit == IFACE_UNIT_BIT)
		value *= 8;

	mag = convert_double(&value, (unit == IFACE_UNIT_BYTE) ? 1024 : 1000, 1.0f);

	/*-
	 * Formatting rules:
	 * - if original value was < 1000, output decimal value only
	 * - with 1 precision if <10
	 * - decimal value with magnitude otherwise
	 */
	if (mag[0] == 0)
		sprintf(buff, "%4ld", (long) value);
	else if (value < 10)
		sprintf(buff, "%3.1f%s", value, mag);
	else
		sprintf(buff, "%3.0f%s", value, mag);
}


/**
 * Format the time in ASCII, depending on the elapsed time.
 * \param buff         Pointer to buffer for storing result
 * \param last_online  Time value
 */
void
get_time_string(char *buff, time_t last_online)
{

	time_t act_time;	/* actual time */
	char timebuff[30];

	act_time = time(NULL);

	if (last_online == 0) {	/* never was online */
		sprintf(buff, "never");
		return;
	}

	/* Transform Unix time format to UTC time format */
	strcpy(timebuff, ctime(&last_online));

	/* 86400 = 24 * 60 * 60. Is it more than 24 hours? */
	if ((act_time - last_online) > 86400 ) {
		timebuff[10] = '\0';
		sprintf(buff, "%s", &timebuff[4]);
		return;
	}
	else {
		timebuff[19] = '\0';
		sprintf(buff, "%s", &timebuff[11]);
		return;
	}
}


/**
 * Actualize values in display, calculating speeds in the defined interval
 * of time and sending proper commands to server. If measure unit is
 * 'pkt', we don't format this speed. Is always XXXX pkt/s.
 * \param iface     Pointer to interface data
 * \param interval  Time of last update
 * \param index     Interface index
 */
void
actualize_speed_screen(IfaceInfo *iface, unsigned int interval, int index)
{
	char speed[20];		/* buffer to store the formated speed string */
	double rc_speed;
	double tr_speed;

	/* single interface mode */
	if ((iface_count == 1) && ( lcd_hgt >= 4)) {
		if (iface->status == up) {
			/* Calculate and actualize download speed */
			if (unit == IFACE_UNIT_PACKET) {
				rc_speed = (iface->rc_pkt - iface->rc_pkt_old) / interval;
				sprintf(speed, "%8ld %s", (long) rc_speed, unit_label(unit));
			}
			else {
				rc_speed = (iface->rc_byte - iface->rc_byte_old) / interval;
				format_value(speed, rc_speed, unit);
			}
			sock_printf(sock, "widget_set I dl 1 2 {DL: %*s/s}\n", lcd_wid - 6, speed);

			/* Calculate and actualize upload speed */
			if (unit == IFACE_UNIT_PACKET) {
				tr_speed = (iface->tr_pkt - iface->tr_pkt_old) / interval;
				sprintf(speed, "%8ld %s", (long)tr_speed, unit_label(unit));
			}
			else {
				tr_speed = (iface->tr_byte - iface->tr_byte_old) / interval;
				format_value(speed, tr_speed, unit);
			}
			sock_printf(sock, "widget_set I ul 1 3 {UL: %*s/s}\n", lcd_wid - 6, speed);

			/* Calculate and actualize total speed */
			if (unit == IFACE_UNIT_PACKET) {
				sprintf(speed, "%7ld %s", (long)(rc_speed + tr_speed), unit_label(unit));
			}
			else {
				format_value(speed, rc_speed + tr_speed, unit);
			}
			sock_printf(sock, "widget_set I total 1 4 {Total: %*s/s}\n", lcd_wid - 9, speed);
		}
		else {
			get_time_string(speed, iface->last_online);
			sock_printf(sock, "widget_set I dl 1 2 {NA (%s)}\n", speed);
			sock_send_string(sock, "widget_set I ul 1 3 {}\n");
			sock_send_string(sock, "widget_set I total 1 4 {}\n");
		}
	}
	/* multi-interfaces mode: 1 line per interface */
	else {
		char speed1[20];	/* buffer to store the formated speed string */

		if (iface->status == up) {
			if (unit == IFACE_UNIT_PACKET) {
				rc_speed = (iface->rc_pkt - iface->rc_pkt_old) / interval;
				tr_speed = (iface->tr_pkt - iface->tr_pkt_old) / interval;
			}
			else {
				rc_speed = (iface->rc_byte - iface->rc_byte_old) / interval;
				tr_speed = (iface->tr_byte - iface->tr_byte_old) / interval;
			}
			format_value_multi_interface(speed, rc_speed, unit);
			format_value_multi_interface(speed1, tr_speed, unit);
			if (lcd_wid > 16)
				sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s U:%.4s D:%.4s}\n",
					    index, index+1, iface->alias, speed1, speed);
			else
				sock_printf(sock, "widget_set I i%1d 1 %1d {%4.4s ^%.4s v%.4s}\n",
					    index, index+1, iface->alias, speed1, speed);
		}
		else {
			get_time_string(speed, iface->last_online);
			sock_printf(sock, "widget_set I i%1d 1 %1d {%5.5s NA (%s)}\n",
					index, index+1, iface->alias, speed);
		}
	}
}


/**
 * Send commands to server to add transfer screen with all required widgets.
 */
void
initialize_transfer_screen(void)
{
	int iface_nmbr;		/* interface number */

	sock_send_string(sock, "screen_add NT\n");
	sock_send_string(sock, "screen_set NT name {Transfer}\n");
	sock_send_string(sock, "widget_add NT title title\n");

	/* single interface mode */
	if ((iface_count == 1) && (lcd_hgt >= 4)) {
		sock_printf(sock, "widget_set NT title {Transfer: %s}\n", iface[0].alias);
		sock_send_string(sock, "widget_add NT dl string\n");
		sock_send_string(sock, "widget_set NT dl 1 2 {DL:}\n");
		sock_send_string(sock, "widget_add NT ul string\n");
		sock_send_string(sock, "widget_set NT ul 1 3 {UL:}\n");
		sock_send_string(sock, "widget_add NT total string\n");
		sock_send_string(sock, "widget_set NT total 1 4 {Total:}\n");
	}
	/* multi-interfaces mode: one line per interface */
	else {
		/* Set title (transfer screen is always in "bytes") */
		sock_send_string(sock, "widget_set NT title {Net Transfer (bytes)}\n");

		/* frame from (2, left) to (width, height) that is iface_count lines high */
		sock_send_string(sock, "widget_add NT f frame\n");
		sock_printf(sock, "widget_set NT f 1 2 %d %d %d %d v 16\n",
			    lcd_wid, lcd_hgt, lcd_wid, iface_count,
			    /* scroll rate: 1 line every X ticks (=1/8 sec) */
			    ((lcd_hgt >= 4) ? 8 : 16));

		/* Add interfaces */
		for (iface_nmbr = 0; iface_nmbr < iface_count; iface_nmbr++) {
			sock_printf(sock, "widget_add NT i%1d string -in f\n", iface_nmbr);
			sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s NA (never)}\n",
				    iface_nmbr, iface_nmbr+1, iface[iface_nmbr].alias);
		}
	}
}


/**
 * Actualize values in display, formatting transfer measures and sending
 * proper commands to server. Traffic is shown in "bytes" unit.
 * \param iface  Pointer to interface data
 * \param index  Interface index
 */
void
actualize_transfer_screen(IfaceInfo *iface, int index)
{
	char transfer[20];	/* buffer to store the formated traffic string */

	/* single interface mode */
	if ((iface_count == 1) && (lcd_hgt >= 4)) {
		if (iface->status == up) {
			/* download traffic */
			format_value(transfer, iface->rc_byte, IFACE_UNIT_BYTE);
			sock_printf(sock, "widget_set NT dl 1 2 {DL: %*s}\n", lcd_wid - 4, transfer);

			/* upload traffic */
			format_value(transfer, iface->tr_byte, IFACE_UNIT_BYTE);
			sock_printf(sock, "widget_set NT ul 1 3 {UL: %*s}\n", lcd_wid - 4, transfer);

			/* total traffic */
			format_value(transfer, iface->rc_byte + iface->tr_byte, IFACE_UNIT_BYTE);
			sock_printf(sock, "widget_set NT total 1 4 {Total: %*s}\n", lcd_wid - 7, transfer);
		}
		else {
			get_time_string(transfer, iface->last_online);
			sock_printf(sock, "widget_set NT dl 1 2 {NA (%s)}\n", transfer);
			sock_send_string(sock, "widget_set NT ul 1 3 {}\n");
			sock_send_string(sock, "widget_set NT total 1 4 {}\n");
		}
	}
	/* multi-interfaces mode: one line per interface */
	else {
		char transfer1[20];	/* buffer to store the formated traffic string */

		if (iface->status == up) {
			format_value_multi_interface(transfer, iface->rc_byte, IFACE_UNIT_BYTE);
			format_value_multi_interface(transfer1, iface->tr_byte, IFACE_UNIT_BYTE);
			if (lcd_wid > 16)
				sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s U:%.4s D:%.4s}\n",
					    index, index+1, iface->alias, transfer1, transfer);
			else
				sock_printf(sock, "widget_set NT i%1d 1 %1d {%4.4s ^%.4s v%.4s}\n",
					    index, index+1, iface->alias, transfer1, transfer);
		}
		else {
			get_time_string(transfer, iface->last_online);
			sock_printf(sock, "widget_set NT i%1d 1 %1d {%5.5s NA (%s)}\n",
					index, index+1, iface->alias, transfer);
		}
	}
}
