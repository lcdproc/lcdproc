/*
 * clients.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Inits/shuts down client system,
 * creates/destroys individual clients,
 * enqueues/dequeues messages from clients,
 * and searches for clients in the list.
 *
 * :)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sock.h"
#include "clients.h"
#include "client_data.h"
#include "shared/report.h"
#include "render.h"

LinkedList *clients;

/* Initialize and kill client list...*/
int
client_init ()
{
	debug(RPT_INFO, "client_init()");

	clients = LL_new ();
	if (!clients) {
		report( RPT_ERR, "client_init:  Unable to create client list");
		return -1;
	}

	return 0;
}

int
client_shutdown ()
{
	client *c;

	debug (RPT_INFO, "client_shutdown()");

	/* Free all client structures...
	 * Note that the regular list loop doesn't work here, because
	 * client_destroy() calls LL_Remove()
	 */
	for (c = LL_Pop (clients); c; c = LL_Pop (clients)) {
		debug (RPT_DEBUG, "client_shutdown: ...");
		if (c) {
			debug (RPT_DEBUG, "client_shutdown: ... %i ...", c->sock);
			if (client_destroy (c) != 0) {
				report (RPT_ERR, "client_shutdown: Error freeing client");
			} else {
				debug (RPT_DEBUG, "client_shutdown: Freed client...");
			}
		} else {
			debug (RPT_DEBUG, "client_shutdown: No client!");
		}
	}

	/* Then, free the list...*/
	LL_Destroy (clients);

	debug (RPT_DEBUG, "client_shutdown: done");

	return 0;
}

/* A client is identified by the file descriptor
 + associated with it.
 *
 * Create and destroy clients....
 */
client *
client_create (int sock)
{
	client *c;

	debug (RPT_DEBUG, "client_create(%i)", sock);

	/* Allocate new client...*/
	c = malloc (sizeof (client));
	if (!c) {
		report (RPT_ERR, "client_create: error allocating new client");
		return NULL;
	}
	/* Init struct members*/
	c->sock = 0;
	c->data = NULL;
	c->messages = NULL;

	c->sock = sock;
	c->backlight_state = backlight; /*By default we get the server setting*/

	/*Set up message list...*/
	c->messages = LL_new ();
	if (!c->messages) {
		report (RPT_ERR, "client_create: error allocating message list");
		free (c);
		return NULL;
	}
	/*TODO:  allocate and init client data...*/
	c->data = malloc (sizeof (client_data));
	if (!c->data) {
		report (RPT_ERR, "client_create: error allocating client data");
		free (c->messages);
		free (c);
		return NULL;
	} else if (client_data_init (c->data) < 0) {
		return NULL;
	}
	/*TODO:  Check for errors while adding the client to the list?*/
	LL_Push (clients, (void *) c);

	return c;
}

int
client_destroy (client * c)
{
	int err;

	char *str;

	debug (RPT_INFO, "client_destroy()");

	if (!c)
		return -1;

	/*Eat the rest of the incoming requests...*/
	debug (RPT_DEBUG, "client_destroy: get_messages");
	while ((str = client_get_message (c))) {
		if (str) {
			debug (RPT_DEBUG, "client_destroy: kill message %s", str);
			free (str);
		}
	}

	/*close socket...*/
	if (c->sock) {
		/*sock_send_string (c->sock, "bye\n");*/
		close(c->sock);
		report(RPT_NOTICE, "closed socket for #%d", c->sock);
	}

	err = LL_Destroy (c->messages);

	/*Free client's other data*/
	client_data_destroy (c->data);

	/*Remove the client from the clients list...*/
	LL_Remove (clients, c);

	free (c);

	return 0;
}

/*Add and remove messages from the client's queue...*/
int
client_add_message (client * c, char *message)
{
	int err = 0;
	char *dup;
	char *str, *cp;
	char delimiters[] = "\n\r\0";
/*  int len;*/

	debug(RPT_DEBUG, "client_add_message(%s)", message);

	if (!c)
		return -1;
	if (!message)
		return -1;

/*  len = strlen(message);
 *   if(len < 1) return 0;
 */

	/* Copy the string to avoid overwriting the original...*/
	dup = strdup (message);
	if (!dup) {
		report(RPT_ERR, "client_add_message: Error allocating new string");
		return -1;
	}
	/* Now split the string into lines and enqueue each one...*/
	for (str = strtok (dup, delimiters); str; str = strtok (NULL, delimiters)) {
		cp = strdup (str);
		debug (RPT_DEBUG, "client_add_message: %s", cp);
		err += LL_Enqueue (c->messages, (void *) cp);
	}

	/*debug(RPT_DEBUG, "client_add_message(%s): %i errors", message, err);*/
	free (dup);						  /* Fixed memory leak...*/

	/* Err is the number of errors encountered...*/
	return err;

}

/* Woo-hoo!  A simple function.  :)*/
char *
client_get_message (client * c)
{
	char *str;

	debug(RPT_DEBUG, "client_get_message()");

	if (!c)
		return NULL;

	str = (char *) LL_Dequeue (c->messages);

	/*debug(RPT_DEBUG, "client_get_message:  \"%s\"", str);*/

	return str;
}

/* Get and set the client's data...*/
int
client_set (client * c, void *data)
{
	/* You know, I really doubt this function will be useful...*/
	return 0;
}

void *
client_get (client * c)
{
	/* But this one might be handy...*/

	return NULL;
}

client *
client_find_sock (int sock)
{
	client *c;

/*   debug(RPT_INFO, "client_find_sock(%i)", sock);*/

	LL_Rewind (clients);
	do {
		c = (client *) LL_Get (clients);
/*      debug(RPT_DEBUG, "client_find_sock: ... %i ...", c->sock);*/
		if (c->sock == sock) {
/*       debug(RPT_DEBUG, "client_find_sock: ..! %i !..", c->sock);*/
			return c;
		}
	} while (LL_Next (clients) == 0);

	debug (RPT_ERR, "client_find_sock: failed");

	return NULL;
}
