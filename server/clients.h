/** \file server/clients.h
 * Manage the list of clients that are connected.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 */

#ifndef CLIENTS_H
#define CLIENTS_H

#include "client.h"

/* Initialize and kill client list...*/
int clients_init(void);
int clients_shutdown(void);

/* Add/remove clients (return NULL for error) */
Client *clients_add_client(Client *c);
Client *clients_remove_client(Client *c, Direction whereto);

/* List functions */
Client *clients_getfirst(void);
Client *clients_getnext(void);
int clients_client_count(void);

/* Search for a client with a particular filedescriptor...*/
Client * clients_find_client_by_sock(int sock);

#endif
