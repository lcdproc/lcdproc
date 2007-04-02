/*
 * client.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2002, Joris Robijn
 *
 * Defines all the client data and actions.
 */

#include "menu.h"
#include "menuitem.h"
/* These headers are placed here on purpose ! (circular references) */

#ifndef CLIENT_H
#define CLIENT_H

#include "shared/LL.h"
#include <stdio.h>

#define CLIENT_NAME_SIZE 256

typedef struct Client {
	char *name;
	int ack;
	int sock;
	int backlight;
	int heartbeat;

	/* Messages that the client generated */
	LinkedList *messages;

	/* The list of screens */
	LinkedList *screenlist;

	/* Maybe it has created a menu */
	Menu *menu;

} Client;

#include "screen.h"

/* When a new client connects, set up a new client data struct */
Client *client_create(int sock);

/* Destroys the client data */
int client_destroy(Client *c);

/* Close the socket */
void client_close_sock(Client *c);

/* Add message to the client's queue...*/
int client_add_message(Client *c, char *message);

/* Get message from queue */
char *client_get_message(Client *c);

/* Find a named screen for the client */
Screen *client_find_screen(Client *c, char *id);

int client_add_screen(Client *c, Screen *s);

int client_remove_screen(Client *c, Screen *s);

int client_screen_count(Client *c);

#endif

