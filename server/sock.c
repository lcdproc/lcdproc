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
#include "clients.h"
#include "shared/report.h"

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

	report (RPT_INFO, "sock_create_inet_socket(%i)", port);

	/* Create the socket. */
	/*debug(RPT_DEBUG, "Creating Inet Socket");*/
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		report(RPT_ERR, "Could not create socket");
		return -1;
	}
	/* Set the socket so we can re-use it*/
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(sockopt)) < 0) {
		report(RPT_ERR, "Error setting socket option SO_REUSEADDR");
		return -1;
	}

	/* Give the socket a name. */
	/*debug(RPT_DEBUG, "Binding Inet Socket");*/
	memset (&name, 0, sizeof (name));
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	inet_aton(addr, &name.sin_addr);

	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
		report(RPT_ERR, "Could not bind to port %d", port);
		return -1;
	} else {
		report(RPT_NOTICE, "listening for queries on port %d", port);
	}

	return sock;

}

/*int StartSocketServer()*/
int
sock_create_server (char *bind_addr, int lcd_port)
{
	int sock;

	report (RPT_INFO, "sock_create_server()");

	/* Create the socket and set it up to accept connections. */
	sock = sock_create_inet_socket (bind_addr, lcd_port);
	if (sock < 0) {
		report (RPT_ERR, "sock_create_server: Error creating socket");
		return -1;
	}

	if (listen (sock, 1) < 0) {
		report(RPT_ERR, "error in attempting to listen to port");
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
	client *c;

	debug(RPT_INFO, "sock_poll_clients()");

	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Block until input arrives on one or more active sockets. */
	read_fd_set = active_fd_set;

	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &t) < 0) {
		report (RPT_ERR, "sock_poll_clients: Select error");
		return -1;
	}

	/* Service all the sockets with input pending. */
	for (i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET (i, &read_fd_set)) {
			if (i == orig_sock) {
				/* Connection request on original socket. */
				int new;
				size = sizeof (clientname);
				new = accept (orig_sock, (struct sockaddr *) &clientname, &size);
				if (new < 0) {
					report (RPT_ERR, "sock_poll_clients: Accept error");
					return -1;
				}
				report (RPT_INFO, "sock_poll_clients: Connect from host %s:%hd on #%d",
					inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port), new);
				FD_SET (new, &active_fd_set);

				fcntl (new, F_SETFL, O_NONBLOCK);

				/* TODO:  Create new "client" here...  (done?)*/
				if (client_create (new) == NULL) {
					report( RPT_ERR, "sock_poll_clients: error creating client %i", i);
					return -1;
				}
			} else {
				/* Data arriving on an already-connected socket. */
				err = 0;
				do {
					debug (RPT_DEBUG, "sock_poll_clients: reading...");
					err = read_from_client (i);
					debug (RPT_DEBUG, "sock_poll_clients: ...done");
					if (err < 0) {
						/* TODO:  Destroy a "client" here... (done?)*/
						c = client_find_sock (i);
						if (c) {
							/*sock_send_string(i, "bye\n");*/
							client_destroy (c);
							close (i);
							FD_CLR (i, &active_fd_set);
							report (RPT_INFO, "sock_poll_clients: Closed connection %i", i);
						} else
							report (RPT_ERR, "sock_poll_clients: Can't find client %i", i);
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
	client *c;

	report(RPT_DEBUG, "read_from_client()" );

	/*nbytes = read (filedes, buffer, MAXMSG);*/
	/*debug(RPT_DEBUG, "read_from_client(%i): reading...", filedes);*/
	/*nbytes = sock_recv (filedes, buffer, MAXMSG);*/
	/*debug(RPT_DEBUG, "read_from_client(%i): ...done", filedes);*/
	/*debug (RPT_DEBUG, "read_from_client(%i): %i bytes", filedes, nbytes);*/

	errno = 0;
	if ((nbytes = sock_recv (filedes, buffer, MAXMSG)) < 0) {
		if (errno != EAGAIN)
			report (RPT_DEBUG, "read_from_client: (fd %d) %s", filedes, strerror(errno));
		return 0;
	} else if (nbytes == 0)		  /* EOF*/
		return -1;
	else if (nbytes > (MAXMSG - (MAXMSG / 8)))	/* Very noisy client...*/
	{
		sock_send_string (filedes, "huh? Too much data received... quiet down!\n");
		return -1;
	} else							  /* Data Read*/
	{
		buffer[nbytes] = 0;
		/* Now, replace zeros with linefeeds...*/
		for (i = 0; i < nbytes; i++)
			if (buffer[i] == 0)
				buffer[i] = '\n';
		/* Enqueue a "client message" here...*/
		c = client_find_sock (filedes);
		if (c) {
			client_add_message (c, buffer);
		} else
			report (RPT_DEBUG, "read_from_client:  Can't find client %i", filedes);

		debug (RPT_DEBUG, "read_from_client: got message: `%s'", buffer);
		return nbytes;
	}
	return nbytes;
}

/* FIXME: This talks to all open files, including
 * stdin, stdout, stderr, the LCD, etc...
 * BUT it should only talk to sockets!
 */
int
sock_close_all ()
{
	int fd;

	report (RPT_INFO, "sock_close_all()");

	for (fd = 0; fd < FD_SETSIZE; fd++) {
		/* TODO:  Destroy a "client" here...?  Nope.*/

		/* Instead of using STDIN_FILENO, STDOUT_FILENO,
		 * and STDERR_FILENO, one could use "fd = 4" in the
		 * for() call - but this would probably not be good
		 * practice...
		 */

		if (	fd == STDIN_FILENO ||
			fd == STDOUT_FILENO ||
			fd == STDERR_FILENO)
			continue;
		else {
			/*sock_send_string (fd, "bye\n");*/
			close (fd);
			FD_CLR (fd, &active_fd_set);
			debug (RPT_DEBUG, "sock_close_all: Closed connection %i", fd);
		}
	}

	return 0;

}
