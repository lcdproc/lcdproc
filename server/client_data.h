/*
 * client_data.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
 */

#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "shared/LL.h"

#define CLIENT_NAME_SIZE 256

typedef struct client_data {
	int ack;
	char *name;
	// and other stuff...  doesn't matter yet
	LinkedList *screenlist;
	// list of requested keys...
	char *client_keys ;
	LinkedList *menulist;

} client_data;

// sets up an existing (empty) client_data struct
int client_data_init (client_data * d);
// destroys members of a client's data
int client_data_destroy (client_data * d);

#endif
