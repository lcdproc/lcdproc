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

#include "client.h"
#include "clients.h"
#include "shared/report.h"
#include "render.h"

LinkedList *clientlist = NULL;

/* Initialize and kill client list...*/
int
clients_init ()
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	clientlist = LL_new ();
	if (!clientlist) {
		report( RPT_ERR, "%s: Unable to create client list", __FUNCTION__);
		return -1;
	}

	return 0;
}

int
clients_shutdown ()
{
	Client *c;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	if( !clientlist ) {
		/* Program shutdown before completed startup */
		return -1;
	}

	/* Free all client structures... */
	for (c=LL_GetFirst (clientlist); c; c=LL_GetNext (clientlist) ) {
		debug (RPT_DEBUG, "%s: ...", __FUNCTION__);
		if (c) {
			debug (RPT_DEBUG, "%s: ... %i ...", __FUNCTION__, c->sock);
			if (client_destroy (c) != 0) {
				report (RPT_ERR, "%s: Error freeing client", __FUNCTION__);
			} else {
				debug (RPT_DEBUG, "%s: Freed client...", __FUNCTION__);
			}
		} else {
			debug (RPT_DEBUG, "%s: No client!", __FUNCTION__);
		}
	}

	/* Then, free the list...*/
	LL_Destroy (clientlist);

	debug (RPT_DEBUG, "%s: done", __FUNCTION__);

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

	debug(RPT_DEBUG, "%s( sock=%i )", __FUNCTION__, sock);

	for( c=LL_GetFirst(clientlist); c; c=LL_GetNext(clientlist) ) {
		if (c->sock == sock) {
			return c;
		}
	}

	debug (RPT_ERR, "%s: failed", __FUNCTION__);

	return NULL;
}
