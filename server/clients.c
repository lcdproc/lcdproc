/*
 * clients.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * Inits/shuts down client system,
 * and searches for clients in the list.
 * On short: manages the list of clients that are connected.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sock.h"
#include "client.h"
#include "clients.h"
#include "shared/report.h"
#include "render.h"

LinkedList *clientlist;

/* Initialize and kill client list...*/
int
clients_init ()
{
	debug(RPT_INFO, "client_init()");

	clientlist = LL_new ();
	if (!clientlist) {
		report( RPT_ERR, "client_init:  Unable to create client list");
		return -1;
	}

	return 0;
}

int
clients_shutdown ()
{
	Client *c;

	debug (RPT_INFO, "clients_shutdown()");

	/* Free all client structures... */
	for (c=LL_GetFirst (clientlist); c; c=LL_GetNext (clientlist) ) {
		debug (RPT_DEBUG, "clients_shutdown: ...");
		if (c) {
			debug (RPT_DEBUG, "clients_shutdown: ... %i ...", c->sock);
			if (client_destroy (c) != 0) {
				report (RPT_ERR, "clients_shutdown: Error freeing client");
			} else {
				debug (RPT_DEBUG, "clients_shutdown: Freed client...");
			}
		} else {
			debug (RPT_DEBUG, "clients_shutdown: No client!");
		}
	}

	/* Then, free the list...*/
	LL_Destroy (clientlist);

	debug (RPT_DEBUG, "clients_shutdown: done");

	return 0;
}

int
clients_add_client (Client *c)
{
	/* Add the client to the clients list... */
	return LL_Push (clientlist, c);
}

int
clients_remove_client (Client *c)
{
	/* Remove the client from the clients list... */
	return (LL_Remove (clientlist, c) == NULL)?-1:0;
}

Client *
clients_getfirst ()
{
	return (Client *) LL_GetFirst(clientlist);
}

Client *
clients_getnext ()
{
	return (Client *) LL_GetNext(clientlist);
}

int
clients_client_count ()
{
	return LL_Length(clientlist);
}


/* A client is identified by the file descriptor
 * associated with it. Find one.
 */

Client *
clients_find_client_by_sock (int sock)
{
	Client *c;

	debug(RPT_INFO, "clients_find_client_by_sock(%i)", sock);

	for( c=LL_GetFirst(clientlist); c; c=LL_GetNext(clientlist) ) {
		if (c->sock == sock) {
			return c;
		}
	}

	debug (RPT_ERR, "client_find_sock: failed");

	return NULL;
}
