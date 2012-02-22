/** \file server/sock.c
 * This file contains all the sockets code used by the server. This contains
 * the code called upon by main() to initialize the listening socket, as well
 * as code to deal with sending messages to clients, maintaining connections,
 * accepting new connections, closing dead connections (or connections
 * associated with dying/exiting clients), etc.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 *               2009, Markus Dolze - input ring buffer
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "shared/report.h"
#include "shared/sring.h"
#include "shared/defines.h"

#include "clients.h"
#include "sock.h"


/****************************************************************************/
static fd_set active_fd_set, read_fd_set;
static int listening_fd;

/* For efficiency we maintain a list of open sockets. Nodes in this list
 * are obtained from a pre-allocated pool - this removes heap operations
 * from the polling loop. A list of open sockets is also required under WINSOCK
 * as sockets can be arbitrary values instead of low value integers. */
static LinkedList* openSocketList = NULL;
static LinkedList* freeClientSocketList = NULL;

/* ring buffer for incoming messages */
static sring_buffer *messageRing;

/** Mapping between socket and associated client */
typedef struct _ClientSocketMap
{
	int socket;		/**< Socket for the client */
	Client *client;		/**< Pointer to client representation */
} ClientSocketMap;


/* The memory referenced from \c openSocketList and \c freeSocketList
 * is obtained from the freeClientSocketPool array. */
ClientSocketMap *freeClientSocketPool;


/* Length of longest transmission allowed at once...*/
#define MAXMSG 8192

/**** Internal function declarations ****************************************/
static int sock_read_from_client(ClientSocketMap *clientSocketMap);
static void sock_destroy_socket(void);


/** Initialize sockets.
 * Prepare server socket, and initialize socket management structures.
 * \param bind_addr       Hostname / IP address to bind to.
 * \param bind_port       Port to bind to.
 * \retval  <0            error
 * \retval   0            success
 */
int
sock_init(char* bind_addr, int bind_port)
{
	int i;

	debug(RPT_DEBUG, "%s(bind_addr=\"%s\", port=%d)", __FUNCTION__, bind_addr, bind_port);

	/* Create the socket and set it up to accept connections. */
	listening_fd = sock_create_inet_socket(bind_addr, bind_port);
	if (listening_fd < 0) {
		report(RPT_ERR, "%s: error creating socket - %s",
			__FUNCTION__, sock_geterror());
		return -1;
	}

	/* Create the socket -> Client mapping pool */
	/* How large can FD_SETSIZE be? Even if it is ~2000 this only uses a
	   few kilobytes of memory. Let's trade size for speed! */
	freeClientSocketPool = (ClientSocketMap *)
				calloc(FD_SETSIZE, sizeof(ClientSocketMap));
	if (freeClientSocketPool == NULL) {
		report(RPT_ERR, "%s: Error allocating client sockets.",
			__FUNCTION__);
		return -1;
	}

	freeClientSocketList = LL_new();
	if (freeClientSocketList == NULL) {
		report(RPT_ERR, "%s: error allocating free socket list.",
			 __FUNCTION__);
		return -1;
	}
	for (i = 0; i < FD_SETSIZE; ++i) {
		LL_AddNode(freeClientSocketList, (void*) &freeClientSocketPool[i]);
	}

	/* Create and initialize the open socket list with the server socket */
	openSocketList = LL_new();
	if (openSocketList == NULL) {
		report(RPT_ERR, "%s: error allocating open socket list.",
			 __FUNCTION__);
		return -1;
	}
	else {
		ClientSocketMap *entry;

		entry = (ClientSocketMap*) LL_Pop(freeClientSocketList);
		entry->socket = listening_fd;
		entry->client = NULL;
		LL_AddNode(openSocketList, (void*) entry);
	}

	if ((messageRing = sring_create(MAXMSG)) == NULL) {
		report(RPT_ERR, "%s: error allocating receive buffer.",
			 __FUNCTION__);
		return -1;
	}

	return 0;
}


/** Cleanup socket management structures.
 * \retval  <0    error
 * \retval   0    success
 */
int
sock_shutdown(void)
{
	int retVal = 0;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

        /*ClientSocketMap* clientIt;*/

        /* delete all clients */
        /* This should be done by calling clients_shutdown */
        /*
          LL_Rewind(openSocketList);
          for (clientIt = (ClientSocketMap*) LL_Get(openSocketList);
          clientIt;
          clientIt = LL_GetNext(openSocketList))
          {
          if (clientIt->client)
          {
        */
        /* destroying a client also closes its socket */
        /*        client_destroy(clientIt->client);
                  }
                  }
                  LL_Destroy(openSocketList);
        */
	close(listening_fd);
	LL_Destroy(freeClientSocketList);
	free(freeClientSocketPool);
	sring_destroy(messageRing);

	return retVal;
}


/** Create an INET socket, bind to it and listen on it.
 * \param addr       Hostname / IP address to bind to.
 * \param port       Port to bind to.
 * \retval  <0       error
 * \retval   0       success
 */
int
sock_create_inet_socket(char *addr, unsigned int port)
{
	struct sockaddr_in name;
	int sock;
	int sockopt = 1;

	debug(RPT_DEBUG, "%s(addr=\"%s\", port=%i)", __FUNCTION__, addr, port);

	/* Create the socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		report(RPT_ERR, "%s: cannot create socket - %s",
			__FUNCTION__, sock_geterror());
		return -1;
	}
	/* Set the socket so we can re-use it*/
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &sockopt, sizeof(sockopt)) < 0) {
		report(RPT_ERR, "%s: error setting socket option SO_REUSEADDR - %s",
			__FUNCTION__, sock_geterror());
		return -1;
	}

	/* Give the socket a name. */
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	inet_aton(addr, &name.sin_addr);

	if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
		report(RPT_ERR, "%s: cannot bind to port %d at address %s - %s",
                       __FUNCTION__, port, addr, sock_geterror());
		return -1;
	}

	if (listen(sock, 1) < 0) {
		report(RPT_ERR, "%s: error in attempting to listen to port "
			"%d at %s - %s",
			__FUNCTION__, port, addr, sock_geterror());
		return -1;
	}

	report(RPT_NOTICE, "Listening for queries on %s:%d", addr, port);

	/* Initialize the set of active sockets. */
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	return sock;
}


/** Service all clients with pending input.
 * \retval  <0       error
 * \retval   0       success
 */
int
sock_poll_clients(void)
{
	struct timeval t;
	ClientSocketMap* clientSocket;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Block until input arrives on one or more active sockets. */
	read_fd_set = active_fd_set;

	if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &t) < 0) {
		report(RPT_ERR, "%s: Select error - %s",
			__FUNCTION__, sock_geterror());
		return -1;
	}

	/* Service all the sockets with input pending. */
	LL_Rewind(openSocketList);
	for (clientSocket = (ClientSocketMap *) LL_Get(openSocketList);
	     clientSocket != NULL;
	     clientSocket = LL_GetNext(openSocketList)) {

		if (FD_ISSET(clientSocket->socket, &read_fd_set)) {
			if (clientSocket->socket == listening_fd) {
				/* Connection request on original socket. */
				Client *c;
				int new_sock;
				struct sockaddr_in clientname;
				socklen_t size = sizeof(clientname);

				new_sock = accept(listening_fd, (struct sockaddr *) &clientname, &size);
				if (new_sock < 0) {
					report(RPT_ERR, "%s: Accept error - %s",
						__FUNCTION__, sock_geterror());
					return -1;
				}
				report(RPT_NOTICE, "Connect from host %s:%hu on socket %i",
					inet_ntoa(clientname.sin_addr), ntohs(clientname.sin_port), new_sock);
				FD_SET(new_sock, &active_fd_set);

				fcntl(new_sock, F_SETFL, O_NONBLOCK);

				/* Create new client */
				if ((c = client_create(new_sock)) == NULL) {
					report(RPT_ERR, "%s: Error creating client on socket %i - %s",
						__FUNCTION__, clientSocket->socket, sock_geterror());
					return -1;
				}
				else {
					/* add new_sock */
					ClientSocketMap *newClientSocket;
					newClientSocket = (ClientSocketMap *) LL_Pop(freeClientSocketList);
					if (newClientSocket != NULL) {
						newClientSocket->socket = new_sock;
						newClientSocket->client = c;
						LL_InsertNode(openSocketList, (void *) newClientSocket);
						/* advance past the new node - check it on the next pass */
						LL_Next(openSocketList);
					}
					else {
						report(RPT_ERR, "%s: Error - free client socket list exhausted - %d clients.",
							__FUNCTION__, FD_SETSIZE);
						return -1;
					}
				}
				if (clients_add_client(c) == NULL) {
					report(RPT_ERR, "%s: Could not add client on socket %i",
						 __FUNCTION__, clientSocket->socket);
					return -1;
				}
			}
			else {	/* Data arriving on an already-connected socket. */
				int err = 0;
				debug(RPT_DEBUG, "%s: reading...", __FUNCTION__);
				err = sock_read_from_client(clientSocket);
				debug(RPT_DEBUG, "%s: ...done", __FUNCTION__);
				if (err < 0)
					sock_destroy_socket();
			}
		}
	}
	return 0;
}


/** Read from a client's socket and store the messages in the client for further parsing.
 * \retval  <0       error
 * \retval   0       success
 */
static int
sock_read_from_client(ClientSocketMap *clientSocketMap)
{
	char buffer[MAXMSG];
	int nbytes;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	errno = 0;
	nbytes = sock_recv(clientSocketMap->socket, buffer, MAXMSG);

	while (nbytes > 0) {		/* Data available */
		int fr;
		char *str;

		debug(RPT_DEBUG, "%s: received %4d bytes", __FUNCTION__, nbytes);

		/* Append to ring buffer */
		sring_write(messageRing, buffer, nbytes);

		/* Process all available message in ring buffer */
		do {
			str = sring_read_string(messageRing);
			if (clientSocketMap->client) {
				client_add_message(clientSocketMap->client, str);
			} else {
				report(RPT_DEBUG, "%s: Can't find client %d",
					__FUNCTION__, clientSocketMap->socket);
			}
		} while (str != NULL);

		/* Read again, but only as much as space is left */
		fr = sring_getMaxWrite(messageRing);
		if (fr == 0)
			report(RPT_WARNING, "%s: Message buffer full", __FUNCTION__);

		nbytes = sock_recv(clientSocketMap->socket, buffer, min(MAXMSG, fr));
	}

	if (sring_getMaxRead(messageRing) > 0) {
		report(RPT_WARNING, "%s: left over bytes in message buffer",
			__FUNCTION__);
		sring_clear(messageRing);
	}

	if (nbytes < 0 && errno == EAGAIN)
		return 0;		/* No data is not an error */

	return -1;			/* EOF */
}


/* comparison function to find a ClientsocketMap entry by client */
int byClient(void *csm, void *client)
{
	return (((ClientSocketMap *) csm)->client == (Client *) client) ? 0 : -1;
}


/** Close an open socket for a given client.
 * \param client  Client whose socket shall be closed.
 * \retval <0     error
 * \retval  0     success.
 */
int
sock_destroy_client_socket(Client *client)
{
	ClientSocketMap *entry;

	LL_Rewind(openSocketList);
	entry = LL_Find(openSocketList, byClient, client);

	if (entry != NULL) {
		sock_destroy_socket();
		return 0;
	}
	return -1;
}


/** Close the socket the openSocketList's \c current pointer points to.
 */
static void
sock_destroy_socket(void)
{
	ClientSocketMap *entry = LL_Get(openSocketList);

	if (entry != NULL) {
		if (entry->client != NULL) {
			report(RPT_NOTICE, "Client on socket %i disconnected",
				entry->socket);
			client_destroy(entry->client);
			clients_remove_client(entry->client, PREV);
			entry->client = NULL;
		}
		else {
			report(RPT_ERR, "%s: Can't find client of socket %i",
				__FUNCTION__, entry->socket);
		}
		/* close socket and remove it from select()'s mask of active sockets */
		FD_CLR(entry->socket, &active_fd_set);
		close(entry->socket);

		/* re-add socket to the free socket pool */
		entry = (ClientSocketMap *) LL_DeleteNode(openSocketList, PREV);
		LL_Push(freeClientSocketList, (void*) entry);
	}
}


/* return 1 if addr is valid IPv4 */
int verify_ipv4(const char *addr)
{
	int result = -1;

	if (addr != NULL) {
		struct in_addr a;

		/* inet_pton returns  positive value if it worked */
		result = inet_pton(AF_INET, addr, &a);
	}
	return (result > 0) ? 1 : 0;
}

/* return 1 if addr is valid IPv6 */
int verify_ipv6(const char *addr)
{
	int result = 0;

	if (addr != NULL) {
		struct in6_addr a;

		/* inet_pton returns  positive value if it worked */
		result = inet_pton(AF_INET6, addr, &a);
	}
	return (result > 0) ? 1 : 0;
}
