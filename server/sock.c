/*
 * sock.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * LCDproc sockets code...
 *
 * This is messy, and needs to be finished.
 *
 */

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>

#include "shared/sockets.h"
#include "sock.h"
#include "client.h"
#include "clients.h"
#include "screen.h"
#include "shared/report.h"
#include "screenlist.h"

extern char bind_addr[64];
extern int lcd_port;


fd_set active_fd_set, read_fd_set;
int orig_sock;

/* Length of longest transmission allowed at once...*/
#define MAXMSG 8192

int read_from_client (int filedes);

/* Creates a socket in internet space*/
int
sock_create_inet_socket (char * addr, unsigned int port)
{
	struct sockaddr_in name;
	int sock, sockopt=1;

	debug (RPT_DEBUG, "%s( addr=\"%s\", port=%i )", __FUNCTION__, addr, port);

	/* Create the socket. */
	/*debug(RPT_DEBUG, "Creating Inet Socket");*/
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		report(RPT_ERR, "%s: Could not create socket", __FUNCTION__);
		return -1;
	}
	/* Set the socket so we can re-use it*/
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(sockopt)) < 0) {
		report(RPT_ERR, "%s: Error setting socket option SO_REUSEADDR", __FUNCTION__);
		return -1;
	}

	/* Give the socket a name. */
	memset (&name, 0, sizeof (name));
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	inet_aton(addr, &name.sin_addr);

	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
		report(RPT_ERR, "Could not bind to port %d", port);
		return -1;
	} else {
		report(RPT_NOTICE, "Listening for queries on %s:%d", addr, port);
	}

	return sock;

}

/*int StartSocketServer()*/
int
sock_create_server (char *bind_addr, int lcd_port)
{
	int sock;

	debug (RPT_DEBUG, "%s( bind_addr=\"%s\", port=%d )", __FUNCTION__, bind_addr, lcd_port);

	/* Create the socket and set it up to accept connections. */
	sock = sock_create_inet_socket (bind_addr, lcd_port);
	if (sock < 0) {
		report (RPT_ERR, "%s: Error creating socket", __FUNCTION__);
		return -1;
	}

	if (listen (sock, 1) < 0) {
		report(RPT_ERR, "%s: error in attempting to listen to port", __FUNCTION__);
		return -1;
	}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);

	orig_sock = sock;

/*
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

	return sock;
}

/* Service all clients with input pending...*/

int
sock_poll_clients ()
{
	int i;
	int err;
	struct sockaddr_in clientname;
	size_t size;
	struct timeval t;
	Client * c;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Block until input arrives on one or more active sockets. */
	read_fd_set = active_fd_set;

	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &t) < 0) {
		report (RPT_ERR, "%s: Select error", __FUNCTION__);
		return -1;
	}

	/* Service all the sockets with input pending. */
	for (i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET (i, &read_fd_set)) {
			if (i == orig_sock) {
				/* Connection request on original socket. */
				int new_sock;
				size = sizeof (clientname);
				new_sock = accept (orig_sock, (struct sockaddr *) &clientname, &size);
				if (new_sock < 0) {
					report (RPT_ERR, "%s: Accept error", __FUNCTION__);
					return -1;
				}
				report (RPT_NOTICE, "Connect from host %s:%hd on socket %i",
					inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port), new_sock);
				FD_SET (new_sock, &active_fd_set);

				fcntl (new_sock, F_SETFL, O_NONBLOCK);

				/* Create new client */
				if ((c = client_create (new_sock)) == NULL) {
					report( RPT_ERR, "%s: Error creating client on socket %i", __FUNCTION__, i);
					return -1;
				}
				if (clients_add_client (c) != 0) {
					report( RPT_ERR, "%s: Could not add client on socket %i", __FUNCTION__, i);
					return -1;
				}
			} else {
				/* Data arriving on an already-connected socket. */
				err = 0;
				do {
					debug (RPT_DEBUG, "%s: reading...", __FUNCTION__);
					err = read_from_client (i);
					debug (RPT_DEBUG, "%s: ...done", __FUNCTION__);
					if (err < 0) {
						/* Client disconnected, destroy client data */
						c = clients_find_client_by_sock (i);
						if (c) {
							/*sock_send_string(i, "bye\n");*/
							report (RPT_NOTICE, "Client on socket %i disconnected", i);
							client_destroy (c);
							clients_remove_client (c);
							close (i);
							FD_CLR (i, &active_fd_set);
						} else
							report (RPT_ERR, "%s: Can't find client of socket %i", __FUNCTION__, i);
					}
				} while (err > 0);
			}
		}
	}
	return 0;
}

int
read_from_client (int filedes)
{
	char buffer[MAXMSG];
	int nbytes, i;
	Client * c;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	errno = 0;
	if ((nbytes = sock_recv (filedes, buffer, MAXMSG)) < 0) {
		if (errno != EAGAIN)
			report (RPT_DEBUG, "%s: Error on socket %d: %s", __FUNCTION__, filedes, strerror(errno));
		return 0;
	} else if (nbytes == 0)		  /* EOF*/
		return -1;
	else if (nbytes > (MAXMSG - (MAXMSG / 8)))	/* Very noisy client...*/
	{
		sock_send_string (filedes, "huh? Too much data received... quiet down!\n");
		report (RPT_WARNING, "%s: Too much data received on socket %d", __FUNCTION__, filedes);
		return -1;
	} else							  /* Data Read*/
	{
		buffer[nbytes] = 0;
		/* Now, replace zeros with linefeeds...*/
		for (i = 0; i < nbytes; i++)
			if (buffer[i] == 0)
				buffer[i] = '\n';
		/* Enqueue a "client message" here...*/
		c = clients_find_client_by_sock (filedes);
		if (c) {
			client_add_message (c, buffer);
		} else
			report (RPT_DEBUG, "%s:  Can't find client %d", __FUNCTION__, filedes);

		report (RPT_DEBUG, "%s: got message from client %d: \"%s\"", __FUNCTION__, filedes, buffer);
		return nbytes;
	}
	return nbytes;
}
