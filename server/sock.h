/** \file server/sock.h
 * function declarations for LCDproc sockets code
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2004, F5 Networks, Inc. - IP-address verification
 *               2008, Peter Marschall
 */

#ifndef SOCK_H
#define SOCK_H

#include "shared/sockets.h"
#define INC_TYPES_ONLY 1
#include "client.h"
#undef INC_TYPES_ONLY

/* Server functions...*/
int sock_init(char* bind_addr, int bind_port);
int sock_shutdown(void);
int sock_create_inet_socket(char* bind_addr, unsigned int port);
int sock_poll_clients(void);
int sock_destroy_client_socket(Client *client);
int verify_ipv4(const char *addr);
int verify_ipv6(const char *addr);

#endif
