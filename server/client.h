#ifndef CLIENT_H
#define CLIENT_H

#include "shared/LL.h"
#include <stdio.h>

#define CLIENT_NAME_SIZE 256

typedef struct Client {
	char *name;
	int ack;
	int sock;
	char addr[64];
	int backlight;
	int heartbeat;

	LinkedList *messages;

	/* and other stuff...  doesn't matter yet*/
	LinkedList *screenlist;

	/* TO BE REMOVED */
	char *client_keys;

	/* list of requested keys */
	//LinkedList *keylist;
	/* list of client supplied menus */
	//LinkedList *menulist;

} Client;

#include "screen.h"


/* When a new client connects, set up a new client data struct */
Client * client_create (int sock);

/* Destroys the client data */
int client_destroy (Client * c);

/* Close the socket */
void client_close_sock (Client * c);

/* Add message to the client's queue...*/
int client_add_message (Client * c, char *message);

/* Get message from queue */
char * client_get_message (Client * c);

/* Find a named screen for the client */
Screen * client_find_screen (Client * c, char *id);

int client_add_screen (Client * c, Screen * s);

int client_remove_screen (Client * c, Screen * s);

int client_screen_count (Client * c);

#endif