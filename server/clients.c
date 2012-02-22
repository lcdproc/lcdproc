/** \file server/clients.c
 * This file contains code allowing LCDd to handle client connections and
 * data structures. It contains functions to initialize the internal list
 * of clients, terminate client connections, add new clients to the list,
 * and locating a client's socket.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 *  Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "shared/report.h"
#include "shared/LL.h"
#include "client.h"
#include "clients.h"
#include "render.h"

LinkedList *clientlist = NULL;

/* Initialize and kill client list...*/
int
clients_init(void)
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	clientlist = LL_new();
	if (!clientlist) {
		report(RPT_ERR, "%s: Unable to create client list", __FUNCTION__);
		return -1;
	}

	return 0;
}

int
clients_shutdown(void)
{
	Client *c;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	if (!clientlist) {
		/* Program shutdown before completed startup */
		return -1;
	}

	/* Free all client structures... */
	for (c = LL_GetFirst(clientlist); c; c = LL_GetNext(clientlist)) {
		debug(RPT_DEBUG, "%s: ...", __FUNCTION__);
		if (c) {
			debug(RPT_DEBUG, "%s: ... %i ...", __FUNCTION__, c->sock);
			if (client_destroy(c) != 0) {
				report(RPT_ERR, "%s: Error freeing client", __FUNCTION__);
			} else {
				debug(RPT_DEBUG, "%s: Freed client...", __FUNCTION__);
			}
		} else {
			debug(RPT_DEBUG, "%s: No client!", __FUNCTION__);
		}
	}

	/* Then, free the list...*/
	LL_Destroy(clientlist);

	debug(RPT_DEBUG, "%s: done", __FUNCTION__);

	return 0;
}

/* Add the client to the clients list... */
Client *
clients_add_client(Client *c)
{
	if (LL_Push(clientlist, c) == 0)
		return c;

	return NULL;
}

/* Remove the client from the clients list... */
Client *
clients_remove_client(Client *c, Direction whereto)
{
	Client *client = LL_Remove(clientlist, c, whereto);

	return client;
}

Client *
clients_getfirst(void)
{
	return (Client *) LL_GetFirst(clientlist);
}

Client *
clients_getnext(void)
{
	return (Client *) LL_GetNext(clientlist);
}

int
clients_client_count(void)
{
	return LL_Length(clientlist);
}


/* A client is identified by the file descriptor
 * associated with it. Find one.
 */

Client *
clients_find_client_by_sock(int sock)
{
	Client *c;

	debug(RPT_DEBUG, "%s(sock=%i)", __FUNCTION__, sock);

	for (c = LL_GetFirst(clientlist); c; c = LL_GetNext(clientlist)) {
		if (c->sock == sock) {
			return c;
		}
	}

	debug(RPT_ERR, "%s: failed", __FUNCTION__);

	return NULL;
}
