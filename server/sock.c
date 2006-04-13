/*
 * sock.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2003, Benjamin Tse (blt@ieee.org) - Winsock port
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 *
 * LCDproc sockets code...
 *
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
#ifdef WINSOCK2
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif /* WINSOCK */
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "sock.h"
#include "client.h"
#include "clients.h"
#include "screen.h"
#include "shared/report.h"
#include "screenlist.h"


/****************************************************************************/
static fd_set active_fd_set, read_fd_set;
static int listening_fd;

/* For efficiency we maintain a list of open sockets. Nodes in this list
 * are obtained from a pre-allocated pool - this removes heap operations
 * from the polling loop. A list of open sockets is also required under WINSOCK
 * as sockets can be arbitrary values instead of low value integers. */
static LinkedList* openSocketList = NULL;
static LinkedList* freeClientSocketList = NULL;

struct ClientSocketMap
{
    int socket;
    Client* client;
};

/* The memory referenced from clientSocketPoolList is obtained from the
 * clientSocketPool array. */
struct ClientSocketMap* freeClientSocketPool;

/* Length of longest transmission allowed at once...*/
#define MAXMSG 8192

/**** Internal function declarations ****************************************/
int sock_read_from_client(struct ClientSocketMap* clientSocketMap);


/****************************************************************************/
int
sock_init(char* bind_addr, int bind_port)
{
        int i;

#ifdef WINSOCK2
        /* Initialize the Winsock dll */
        WSADATA wsaData;
        int startup = WSAStartup( MAKEWORD(2, 2), &wsaData );
        if (startup != 0)
        {
            report(RPT_ERR, "%s: Could not start Winsock library - %s", 
                   __FUNCTION__, sock_geterror());
        }
        /* REVISIT: call WSACleanup(); */
#endif

	debug (RPT_DEBUG, "%s( bind_addr=\"%s\", port=%d )", __FUNCTION__, bind_addr, bind_port);

	/* Create the socket and set it up to accept connections. */
	listening_fd = sock_create_inet_socket (bind_addr, bind_port);
	if (listening_fd < 0) {
		report (RPT_ERR, "%s: Error creating socket - %s", 
                        __FUNCTION__, sock_geterror());
		return -1;
	}

        /* Create the socket -> Client mapping pool */
        /* How large can FD_SETSIZE be? Even if it is ~2000 this only uses a
           few kilobytes of memory. Let's trade size for speed! */
        freeClientSocketPool = (struct ClientSocketMap*) 
                        malloc(sizeof(struct ClientSocketMap) * FD_SETSIZE);
        if (!freeClientSocketPool)
        {
            report(RPT_ERR, "%s: Error allocating memory for client sockets.",
                   __FUNCTION__);
            return -1;
        }

        freeClientSocketList = LL_new();
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            LL_AddNode(freeClientSocketList, (void*) &freeClientSocketPool[i]);
        }

        /* Create and initialize the open socket list with the server socket */
        openSocketList = LL_new();
        if (!openSocketList)
        {
            report(RPT_ERR, "%s: Error allocating memory for the open socket "
                   "list.", __FUNCTION__);
            return -1;
        } else {
            struct ClientSocketMap* entry;

            /* LL_DeleteNode removes an entry from the list and returns it */
            entry = (struct ClientSocketMap*) LL_Pop(freeClientSocketList);
            entry->socket = listening_fd;
            entry->client = NULL;
            LL_AddNode(openSocketList, (void*) entry);
        }

	return 0;
}

/*
This code gets the send and receive buffer sizes.
  {
     int val, len, sock;
     sock = new;

     len = sizeof(int);
     getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &val, &len);
     debug(RPT_DEBUG, "SEND buffer: %i bytes", val);

     len = sizeof(int);
     getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &val, &len);
     debug(RPT_DEBUG, "RECV buffer: %i bytes", val);
  }
*/


int
sock_shutdown ()
{
        int retVal = 0;

	debug( RPT_DEBUG, "%s()", __FUNCTION__ );

        /*struct ClientSocketMap* clientIt;*/

        /* delete all clients */
        /* This should be done by calling clients_shutdown */
        /*
          LL_Rewind(openSocketList);
          for (clientIt = (struct ClientSocketMap*) LL_Get(openSocketList);
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
	close( listening_fd );
        LL_Destroy(freeClientSocketList);
        free(freeClientSocketPool);

#ifdef WINSOCK2
        if (WSACleanup() != 0)
        {
            report(RPT_ERR, "%s: Error closing Winsock library - %s",
                   __FUNCTION__, sock_geterror());
            retVal = -1;
        }
#endif
    
        return retVal;
}


/****************************************************************************/
/* Creates a socket in internet space */
int
sock_create_inet_socket (char * addr, unsigned int port)
{
	struct sockaddr_in name;
	int sock, sockopt=1;

	debug (RPT_DEBUG, "%s( addr=\"%s\", port=%i )", __FUNCTION__, addr, port);

	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
#ifdef WINSOCK2
        if (sock == INVALID_SOCKET)
#else
	if (sock < 0)
#endif
        {
		report(RPT_ERR, "%s: Could not create socket - %s", 
                       __FUNCTION__, sock_geterror());
		return -1;
	}
	/* Set the socket so we can re-use it*/
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&sockopt, sizeof(sockopt)) < 0) {
		report(RPT_ERR, "%s: Error setting socket option SO_REUSEADDR - %s", 
                       __FUNCTION__, sock_geterror());
		return -1;
	}

	/* Give the socket a name. */
	memset (&name, 0, sizeof (name));
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
#ifndef WINSOCK2
        /* REVISIT: can probably use the same code as under winsock */
	inet_aton(addr, &name.sin_addr);
#else
        name.sin_addr.S_un.S_addr = inet_addr(addr);
#endif

	if (bind(sock, (struct sockaddr *) &name, sizeof (name)) < 0) 
        {
		report(RPT_ERR, "%s: Could not bind to port %d at address %s - %s", 
                       __FUNCTION__, port, addr, sock_geterror());
		return -1;
	} else {
		report(RPT_NOTICE, "Listening for queries on %s:%d", addr, port);
	}

	if (listen (sock, 1) < 0) {
		report(RPT_ERR, "%s: error in attempting to listen to port "
                       "%d at %s - %s", 
                       __FUNCTION__, port, addr, sock_geterror());
		return -1;
	}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);

	return sock;
}


/* Service all clients with input pending...*/
int
sock_poll_clients ()
{
	int err;
	struct sockaddr_in clientname;
	struct timeval t;
        struct ClientSocketMap* clientSocket;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Block until input arrives on one or more active sockets. */
	read_fd_set = active_fd_set;

	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &t) < 0) {
		report(RPT_ERR, "%s: Select error - %s", 
                       __FUNCTION__, sock_geterror());
		return -1;
	}

	/* Service all the sockets with input pending. */
        LL_Rewind(openSocketList);
        for (clientSocket = (struct ClientSocketMap*) LL_Get(openSocketList); 
             clientSocket; 
             clientSocket = LL_GetNext(openSocketList))
        {
		if (FD_ISSET(clientSocket->socket, &read_fd_set)) 
                {
			if (clientSocket->socket == listening_fd) 
                        {
				/* Connection request on original socket. */
                                Client* c;
				int new_sock;
				socklen_t size = sizeof(clientname);
				new_sock = accept (listening_fd, (struct sockaddr *) &clientname, &size);
#ifdef WINSOCK2
                                if (new_sock == INVALID_SOCKET)
#else
				if (new_sock < 0) 
#endif
                                {
					report(RPT_ERR, "%s: Accept error - %s", 
                                               __FUNCTION__, sock_geterror());
					return -1;
				}
				report (RPT_NOTICE, "Connect from host %s:%hd on socket %i",
					inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port), new_sock);
				FD_SET (new_sock, &active_fd_set);

#ifdef WINSOCK2        
                                {
                                    unsigned long tmp;
                                    ioctlsocket(new_sock, FIONBIO, &tmp);
                                }
#else
				fcntl(new_sock, F_SETFL, O_NONBLOCK);
#endif

				/* Create new client */
				if ((c = client_create (new_sock)) == NULL) {
					report( RPT_ERR, "%s: Error creating client on socket %i - %s",
                                                __FUNCTION__, clientSocket->socket, sock_geterror());
					return -1;
				} else {
                                        /* add new_sock */
                                        struct ClientSocketMap* newClientSocket;
                                        newClientSocket = (struct ClientSocketMap*) LL_Pop(freeClientSocketList);
                                        if (newClientSocket)
                                        {
                                            newClientSocket->socket = new_sock;
                                            newClientSocket->client = c;
                                            LL_InsertNode(openSocketList, (void*) newClientSocket);
                                            /* advance past the new node - check it on the next pass */
                                            LL_Next(openSocketList);
                                        } else {
                                            report(RPT_ERR, "%s: Error - free client socket list exhausted - %d clients.", 
                                                   __FUNCTION__, FD_SETSIZE);
                                            return -1;
                                        }
                                }
				if (clients_add_client (c) != 0) {
					report( RPT_ERR, "%s: Could not add client on socket %i", __FUNCTION__, clientSocket->socket);
					return -1;
				}
			} else {
				/* Data arriving on an already-connected socket. */
				err = 0;
				do {
					debug (RPT_DEBUG, "%s: reading...", __FUNCTION__);
					err = sock_read_from_client(clientSocket);
					debug (RPT_DEBUG, "%s: ...done", __FUNCTION__);
					if (err < 0) 
                                        {
                                            /* Client disconnected, destroy client data */
                                            /* c = clients_find_client_by_sock(); - Deprecated by clientsocketmap*/
                                            if (clientSocket->client) 
                                            {
                                                struct ClientSocketMap* entry;

                                                /*sock_send_string(i, "bye\n");*/
                                                report (RPT_NOTICE, "Client on socket %i disconnected",
                                                        clientSocket->socket);
                                                client_destroy(clientSocket->client);
                                                clients_remove_client(clientSocket->client);
                                                FD_CLR(clientSocket->socket, &active_fd_set);
                                                close(clientSocket->socket);

                                                entry = (struct ClientSocketMap*) LL_DeleteNode(openSocketList);
                                                LL_Push(freeClientSocketList, (void*) entry);
                                            } else {
                                                report (RPT_ERR, "%s: Can't find client of socket %i",
                                                        __FUNCTION__, clientSocket->socket);
                                            }
					}
				} while (err > 0);
			}
		}
	}
	return 0;
}

int
sock_read_from_client(struct ClientSocketMap* clientSocketMap)
{
	char buffer[MAXMSG];
	int nbytes, i;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	errno = 0;
        nbytes = sock_recv(clientSocketMap->socket, buffer, MAXMSG);
	if (nbytes < 0) 
        {
		if (errno != EAGAIN)
			report(RPT_DEBUG, "%s: Error on socket %d - %s", 
                               __FUNCTION__, clientSocketMap->socket, sock_geterror());
		return 0;
	} else if (nbytes == 0)		  /* EOF*/
        {
		return -1;
	} else if (nbytes > (MAXMSG - (MAXMSG / 8)))	/* Very noisy client...*/
	{
		sock_send_error(clientSocketMap->socket, "Too much data received... quiet down!\n");
		return -1;
	} else							  /* Data Read*/
	{
		buffer[nbytes] = 0;
		/* Now, replace zeros with linefeeds...*/
		for (i = 0; i < nbytes; i++)
			if (buffer[i] == 0)
				buffer[i] = '\n';
		/* Enqueue a "client message" here...*/
		/* c = clients_find_client_by_sock (filedes);  - Deprecated by clientsocketmap*/
		if (clientSocketMap->client) {
			client_add_message(clientSocketMap->client, buffer);
		} else {
			report(RPT_DEBUG, "%s:  Can't find client %d", 
                               __FUNCTION__, clientSocketMap->socket);
                }

		report(RPT_DEBUG, "%s: got message from client %d: \"%s\"", 
                       __FUNCTION__, clientSocketMap->socket, buffer);
		return nbytes;
	}
	return nbytes;
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


