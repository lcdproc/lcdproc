#ifndef IFACE_H
#define IFACE_H
/* netlcdclient.h - definitions and function prototipes
 * 
 * Copyright (C) 2002 Luis Llorente Campo
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */
		  

#include <time.h>

#define DEVFILE "/proc/net/dev"  /* file to read statistics from */

/* status definitions */

typedef enum {
	down = 0,
	up = 1,
} IfaceStatus;

#define MAX_INTERFACES 3  /* max number of interfaces in multi-interface mode */

/* Struct for interface values (transmision, reception, etc..) */

typedef struct iface_info
{
	/* interface name and alias (=display name) */
	char *name;
	char *alias;

	IfaceStatus status;

	time_t last_online;

	/* received bytes */
	double rc_byte;
	double rc_byte_old;

	/* transmited bytes */
	double tr_byte;
	double tr_byte_old;

	/* received packets */
	double rc_pkt;
	double rc_pkt_old;

	/* transmited packets */
	double tr_pkt;
	double tr_pkt_old;
} IfaceInfo;


int iface_screen (int rep, int display, int *flags_ptr);
IfaceInfo iface[MAX_INTERFACES];  /* interface info */

/************************/
/* Functions prototipes */
/************************/

/* show usage options */
static void usage (int status);

/* parse command line parameters */
static int decode_switches (int argc, char **argv);

/* read interface stats from /proc/net/dev */
int get_iface_stats (IfaceInfo *interface);

/* send initial commands to server to add the speed screen */
void initialize_speed_screen (void);

/* send initial commands to server to add the transfer screen */
void initialize_transfer_screen(void);

/* format the time in ASCII */
void get_time_string (char *buff, time_t last_online);

/* format value, scaling value and adding proper suffixes */
void format_value (char *buff, double value, char *unit);

/* format value, scaling value and adding proper suffixes (for multi-interface 
 * mode) */
void format_value_multi_interface(char *buff, double value, char *unit);

/* actualize widgets values in speed screen */
void actualize_speed_screen (IfaceInfo *iface, unsigned int interval, int index);

/* actualize widgets values in transfer screen */
void actualize_transfer_screen (IfaceInfo *iface, int index);

#ifdef NETLCDCLIENT
/* exit the program cleanly */
void exit_program (int val);
#endif //NETLCDCLIENT

#endif
