/*
 * client.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef CLIENTS_H
#define CLIENTS_H

#include "client.h"
#include "shared/LL.h"

/* extern LinkedList *clientlist;   Not needed outside ? */

/* Initialize and kill client list...*/
int clients_init ();
int clients_shutdown ();

/* Add/remove clients (return -1 for error) */
int clients_add_client (Client *c);
int clients_remove_client (Client *c);

/* List functions */
Client * clients_getfirst ();
Client * clients_getnext ();
int clients_client_count ();

/* Search for a client with a particular filedescriptor...*/
Client * clients_find_client_by_sock (int sock);

#endif
