#ifndef SOCKETS_H
#define SOCKETS_H

#include "shared/sockets.h"

typedef struct sockaddr_in sockaddr_in;

// Server functions...
int sock_create_server ();
int sock_create_inet_socket (unsigned short int port);
int sock_poll_clients ();
int sock_close_all ();
int read_from_client (int filedes);

#endif
