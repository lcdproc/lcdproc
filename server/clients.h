#ifndef CLIENTS_H
#define CLIENTS_H

#include "client_data.h"
#include "shared/LL.h"

typedef struct client {
	int sock;
	char addr[64];
	int backlight_state;
	LinkedList *messages;
	client_data *data;

} client;

extern LinkedList *clients;

// Initialize and kill client list...
int client_init ();
int client_shutdown ();

// Create and destroy clients....
client *client_create (int sock);
int client_destroy (client * c);

// Add and remove messages from the client's queue...
int client_add_message (client * c, char *message);
char *client_get_message (client * c);

// Get and set the client's data...
// Not used at all yet, and may never be.  Oh, well.
int client_set (client * c, void *data);
void *client_get (client * c);

// Search for a client with a particular filedescriptor...
client *client_find_sock (int sock);

#endif
