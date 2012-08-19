/** \file server/client.h
 * Defines all the client data and actions.
 *
 * \note If you only need 'struct Client' to work with, you should use the
 *       following code (which does not create an indirect dependency on
 *       'struct Screen'):
 *
 * \code
 * #define INC_TYPES_ONLY 1
 * #include "client.h"
 * #undef INC_TYPES_ONLY
 * \endcode
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2002, Joris Robijn
 */

#ifndef CLIENT_H_TYPES
#define CLIENT_H_TYPES

#include "shared/LL.h"

#define CLIENT_NAME_SIZE 256

/** Possible states of a client. */
typedef enum _clientstate {
	NEW,			/**< Client did not yet send \c hello. */
	ACTIVE,			/**< Client sent \c hello, but not yet \c bye. */
	GONE			/**< Client sent \c bye. */
} ClientState;


/** The structure representing a client in the server. */
typedef struct Client {
	char *name;
	ClientState state;
	int sock;
	int backlight;
	int heartbeat;

	LinkedList *messages;		/**< Messages that the client sent. */
	LinkedList *screenlist;		/**< List of client's screens. */

	void* menu;			/**< Menu hierarchy, if any */
} Client;

#endif

#ifndef INC_TYPES_ONLY
#ifndef CLIENT_H_FNCS
#define CLIENT_H_FNCS

#define INC_TYPES_ONLY 1
#include "screen.h"
#undef INC_TYPES_ONLY

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
#endif
