/*

  clients.c

  Inits/shuts down client system,
  creates/destroys individual clients,
  enqueues/dequeues messages from clients,
  and searches for clients in the list.

  :)

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#include "sock.h"
#include "clients.h"
#include "client_data.h"
#include "shared/debug.h"

LinkedList *clients;

// Initialize and kill client list...
int
client_init ()
{
	debug ("client_init()\n");

	clients = LL_new ();
	if (!clients) {
		fprintf (stderr, "client_init:  Unable to create client list\n");
		return -1;
	}

	return 0;
}

int
client_shutdown ()
{
	client *c;

	debug ("client_shutdown()\n");

	// Free all client structures...
	// Note that the regular list loop doesn't work here, because
	// client_destroy() calls LL_Remove()
	for (c = LL_Pop (clients); c; c = LL_Pop (clients)) {
		debug ("client_shutdown: ...\n");
		if (c) {
			debug ("client_shutdown: ... %i ...\n", c->sock);
			if (client_destroy (c) != 0) {
				fprintf (stderr, "client_shutdown: Error freeing client\n");
			} else {
				debug ("client_shutdown: Freed client...\n");
			}
		} else {
			debug ("client_shutdown: No client!\n");
		}
	}

	// Then, free the list...
	LL_Destroy (clients);

	debug ("client_shutdown: done\n");

	return 0;
}

// A client is identified by the file descriptor
// associated with it.
//
// Create and destroy clients....
client *
client_create (int sock)
{
	client *c;

	debug ("client_create(%i)\n", sock);

	// Allocate new client...
	c = malloc (sizeof (client));
	if (!c) {
		fprintf (stderr, "client_create: error allocating new client\n");
		return NULL;
	}
	// Init struct members
	c->sock = 0;
	c->data = NULL;
	c->messages = NULL;

	c->sock = sock;

	// Set up message list...
	c->messages = LL_new ();
	if (!c->messages) {
		fprintf (stderr, "client_create: error allocating message list\n");
		free (c);
		return NULL;
	}
	// TODO:  allocate and init client data...
	c->data = malloc (sizeof (client_data));
	if (!c->data) {
		fprintf (stderr, "client_create: error allocating client data\n");
		free (c->messages);
		free (c);
		return NULL;
	} else if (client_data_init (c->data) < 0) {
		return NULL;
	}
	// TODO:  Check for errors while adding the client to the list?
	LL_Push (clients, (void *) c);

	return c;
}

int
client_destroy (client * c)
{
	int err;

	char *str;

	debug ("client_destroy()\n");

	if (!c)
		return -1;

	// Eat the rest of the incoming requests...
	debug ("client_destroy: get_messages\n");
	while ((str = client_get_message (c))) {
		if (str) {
			debug ("client_destroy: kill message %s\n", str);
			free (str);
		}
	}

	// close socket...
	if (c->sock) {
		// sock_send_string (c->sock, "bye\n");
		close(c->sock);
		syslog(LOG_NOTICE, "closed socket for #%d\n", c->sock);
	}

	err = LL_Destroy (c->messages);

	// Free client's other data
	client_data_destroy (c->data);

	// Remove the client from the clients list...
	LL_Remove (clients, c);

	free (c);

	return 0;
}

// Add and remove messages from the client's queue...
int
client_add_message (client * c, char *message)
{
	int err = 0;
	char *dup;
	char *str, *cp;
	char delimiters[] = "\n\r\0";
//   int len;

	//debug("client_add_message(%s)\n", message);

	if (!c)
		return -1;
	if (!message)
		return -1;

//   len = strlen(message);
//   if(len < 1) return 0;

	// Copy the string to avoid overwriting the original...
	dup = strdup (message);
	if (!dup) {
		fprintf (stderr, "client_add_message: Error allocating new string\n");
		return -1;
	}
	// Now split the string into lines and enqueue each one...
	for (str = strtok (dup, delimiters); str; str = strtok (NULL, delimiters)) {
		cp = strdup (str);
		debug ("client_add_message: %s\n", cp);
		err += LL_Enqueue (c->messages, (void *) cp);
	}

	//debug("client_add_message(%s): %i errors\n", message, err);
	free (dup);						  // Fixed memory leak...

	// Err is the number of errors encountered...
	return err;

}

// Woo-hoo!  A simple function.  :)
char *
client_get_message (client * c)
{
	char *str;

	//debug("client_get_message()\n");

	if (!c)
		return NULL;

	str = (char *) LL_Dequeue (c->messages);

	//debug("client_get_message:  \"%s\"\n", str);

	return str;
}

// Get and set the client's data...
int
client_set (client * c, void *data)
{
	// You know, I really doubt this function will be useful...
	return 0;
}

void *
client_get (client * c)
{
	// But this one might be handy...

	return NULL;
}

client *
client_find_sock (int sock)
{
	client *c;

//   debug("client_find_sock(%i)\n", sock);

	LL_Rewind (clients);
	do {
		c = (client *) LL_Get (clients);
//      debug("client_find_sock: ... %i ...\n", c->sock);
		if (c->sock == sock) {
//       debug("client_find_sock: ..! %i !..\n", c->sock);
			return c;
		}
	} while (LL_Next (clients) == 0);

	debug ("client_find_sock: failed\n");

	return NULL;
}
