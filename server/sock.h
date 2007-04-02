/*
 * sock.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2004, F5 Networks, Inc. - IP-address verification
 *
 */

#ifndef SOCK_H
#define SOCK_H

#include "shared/sockets.h"

typedef struct sockaddr_in sockaddr_in;

/* Server functions...*/
int sock_init(char* bind_addr, int bind_port);
int sock_shutdown(void);
int sock_create_inet_socket(char* bind_addr, unsigned int port);
int sock_poll_clients(void);
int verify_ipv4(const char *addr);
int verify_ipv6(const char *addr);

#endif
