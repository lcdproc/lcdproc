/*
 * sock.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef SOCKETS_H
#define SOCKETS_H

#include "shared/sockets.h"

typedef struct sockaddr_in sockaddr_in;

/* Server functions...*/
int sock_create_server ();
int sock_create_inet_socket (unsigned short int port);
int sock_poll_clients ();
int sock_close_all ();
int read_from_client (int filedes);

#endif
