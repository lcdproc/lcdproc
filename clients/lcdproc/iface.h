/** \file clients/lcdproc/iface.h
 * Definitions and function prototypes for \c iface.c.
 * Imported from \c netlcdclient.h.
 */

/*-
 * Copyright (C) 2002 Luis Llorente Campo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */

#ifndef IFACE_H
#define IFACE_H

/** max number of interfaces in multi-interface mode */
#define MAX_INTERFACES 3

IfaceInfo iface[MAX_INTERFACES];	/* interface info */

/** Update screen content */
int iface_screen(int rep, int display, int *flags_ptr);
/** read interface stats from /proc/net/dev */
int get_iface_stats(IfaceInfo *interface);
/** send initial commands to server to add the speed screen */
void initialize_speed_screen(void);
/** send initial commands to server to add the transfer screen */
void initialize_transfer_screen(void);
/** format the time in ASCII */
void get_time_string(char *buff, time_t last_online);
/** format value, scaling value and adding proper suffixes */
void format_value(char *buff, double value, char *unit);
/** format value, scaling value and adding proper suffixes (for multi-interface
 * mode) */
void format_value_multi_interface(char *buff, double value, char *unit);
/** actualize widgets values in speed screen */
void actualize_speed_screen(IfaceInfo *iface, unsigned int interval, int index);
/** actualize widgets values in transfer screen */
void actualize_transfer_screen(IfaceInfo *iface, int index);

#endif
