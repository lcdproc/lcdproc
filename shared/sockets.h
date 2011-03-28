/** \file shared/sockets.h
 * Socket functions available to server and clients.
 */

#ifndef SOCKETS_H
#define SOCKETS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef LCDPORT
# define LCDPORT 13666
#endif

#ifndef SHUT_RDWR
# define SHUT_RDWR 2
#endif

/** Connect to server on host, port */
int sock_connect (char *host, unsigned short int port);
/** Disconnect from server */
int sock_close (int fd);
/** Send printf-like formatted output */
int sock_printf (int fd, const char *format, .../*args*/);
/** Send lines of text */
int sock_send_string (int fd, char *string);
/** Send raw data */
int sock_send (int fd, void *src, size_t size);
/** Receive a line of text */
int sock_recv_string (int fd, char *dest, size_t maxlen);
/** Receive raw data */
int sock_recv (int fd, void *dest, size_t maxlen);


/** Return the error message for the last error occured */
char *sock_geterror(void);
/** Send an already formatted error message to the client */
int sock_send_error(int fd, char* message);
/** Print printf-like formatted output to logfile and send it to the client */
int sock_printf_error(int fd, const char *format, .../*args*/);

#endif
