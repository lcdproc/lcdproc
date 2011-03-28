/** \file shared/sockets.c
 * Socket functions available to server and clients.
 */

/*-
 * This file is part of LCDproc.
 *
 * Feel free to use this in your own clients... :)
 */

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "report.h"
#include "sockets.h"

// Length of longest transmission allowed at once...
#define MAXMSG 8192

typedef struct sockaddr_in sockaddr_in;

/**
 * Tries to resolve a resolve a hostname.
 * \param name      Pointer to resolves IP-address
 * \param hostname  Hostname or IP-address (as string)
 * \param port      Port number
 * \return  0 on success, -1 on error.
 */
static int
sock_init_sockaddr (sockaddr_in *name, const char *hostname, unsigned short int port)
{
	struct hostent *hostinfo;

	memset (name, '\0', sizeof (*name));
	name->sin_family = AF_INET;
	name->sin_port = htons (port);
	hostinfo = gethostbyname (hostname);
	if (hostinfo == NULL) {
		report (RPT_ERR, "sock_init_sockaddr: Unknown host %s.", hostname);
		return -1;
	}
	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

	return 0;
}

/**
 * Connect to server.
 * \param host  Hostname or IP-address
 * \param port  Port number
 * \return  socket file descriptor on success, -1 on error
 */
int
sock_connect (char *host, unsigned short int port)
{
	struct sockaddr_in servername;
	int sock;
	int err = 0;

	report (RPT_DEBUG, "sock_connect: Creating socket");
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		report (RPT_ERR, "sock_connect: Error creating socket");
		return sock;
	}
	debug (RPT_DEBUG, "sock_connect: Created socket (%i)", sock);

	if (sock_init_sockaddr (&servername, host, port) < 0)
		return -1;

	err = connect (sock, (struct sockaddr *) &servername, sizeof (servername));
	if (err < 0) {
		report (RPT_ERR, "sock_connect: connect failed");
		shutdown (sock, SHUT_RDWR);
		return -1;
	}

	fcntl (sock, F_SETFL, O_NONBLOCK);

	return sock;
}

/**
 * Disconnect from server.
 * \param fd  Socket file descriptor
 * \return  0 on success, -1 on error.
 */
int
sock_close (int fd)
{
	int err;

	err = shutdown (fd, SHUT_RDWR);
	if (!err)
		close (fd);

	return err;
}


/**
 * Send printf-like formatted output.
 * \param fd      Socket file descriptor
 * \param format  Format string
 * \param ...     Arguments to the format string
 * \return  Number of bytes sent.
 */
int
sock_printf(int fd, const char *format, .../*args*/ )
{
	char buf[MAXMSG];
	va_list ap;
	int size = 0;

	va_start(ap, format);
	size = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (size < 0) {
		report(RPT_ERR, "sock_printf: vsnprintf failed");
		return -1;
	}
	if (size > sizeof(buf))
		report(RPT_WARNING, "sock_printf: vsnprintf truncated message");

	return sock_send_string(fd, buf);
}

/**
 * Send lines of text.
 * \param fd      Socket file descriptor
 * \param string  Pointer to the string to send.
 * \return  Number of bytes sent.
 */
int
sock_send_string (int fd, char *string)
{
	return sock_send(fd, string, strlen(string));
}

/**
 * Receive a line of text.
 * Recv gives only one line per call...
 * \param fd      Socket file descriptor
 * \param dest    Pointer to buffer to store the received data
 * \param maxlen  Number of bytes to read at most (size of buffer)
 * \return  Number of bytes received.
 */
int
sock_recv_string (int fd, char *dest, size_t maxlen)
{
	char *ptr = dest;
	int recvBytes = 0;

	if (!dest)
		return -1;
	if (maxlen <= 0)
		return 0;

	while (1) {
		int err = read (fd, ptr, 1);
		if (err == -1) {
			if (errno == EAGAIN) {
				if (recvBytes) {
					// We've begun to read a string, but no bytes are
					// available.  Loop.
					continue;
				}
				return 0;
			} else {
				report (RPT_ERR, "sock_recv_string: socket read error");
				return err;
			}
		} else if (err == 0) {
			return recvBytes;
		}

		recvBytes++;

		// stop at max. bytes allowed, at NUL or at LF
		if (recvBytes == maxlen || *ptr == '\0' || *ptr == '\n') {
			*ptr = '\0';
			break;
		}
		ptr++;
	}

	// Don't return an empty string
	if (recvBytes == 1 && dest[0] == '\0')
		return 0;

	if (recvBytes < maxlen - 1)
		dest[recvBytes] = '\0';

	return recvBytes;
}

/**
 * Send raw data.
 * \param fd    Socket file descriptor
 * \param src   Buffer holding the data to send
 * \param size  Number of bytes to send at most
 * \return  Number of bytes sent.
 */
int
sock_send (int fd, void *src, size_t size)
{
	int offset = 0;

	if (!src)
		return -1;

	while (offset != size) {
		// write isn't guaranteed to send the entire string at once,
		// so we have to sent it in a loop like this
                int sent = write (fd, ((char *) src) + offset, size - offset);
		if (sent == -1) {
			if (errno != EAGAIN) {
				report (RPT_ERR, "sock_send: socket write error");
				report (RPT_DEBUG, "Message was: '%.*s'", size-offset, (char *) src);
				return sent;
			}
			continue;
		} else if (sent == 0) {
			// when this returns zero, it generally means
			// we got disconnected
			return sent + offset;
		}

		offset += sent;
	}

	return offset;
}

/**
 * Receive raw data.
 * \param fd      Socket file descriptor
 * \param dest    Pointer to buffer to store the received data
 * \param maxlen  Number of bytes to read at most (size of buffer)
 * \return  Number of bytes received.
 */
int
sock_recv (int fd, void *dest, size_t maxlen)
{
	int err;

	if (!dest)
		return -1;
	if (maxlen <= 0)
		return 0;

	err = read (fd, dest, maxlen);
	if (err < 0) {
		//report (RPT_DEBUG,"sock_recv: socket read error");
		return err;
	}
	//debug(RPT_DEBUG, "sock_recv: Got message \"%s\"", (char *)dest);

	return err;
}

/*****************************************************************************/

/**
 * Return the error message for the last error occured.
 * \return  Error message string
 */
char*
sock_geterror(void)
{
    return strerror(errno);
}

/**
 * Send an already formatted error message to the client.
 * \param fd  socket
 * \param message  the message to send (without the "huh? ") */
int sock_send_error(int fd, char* message)
{
	// simple: performance penalty isn't worth more work...
	return sock_printf_error(fd, message);
}

/**
 * Print printf-like formatted output to logfile and sends it to the client.
 * \note don't add a the "huh? " to the message. This is done by this
 *   method
 * \param fd socket
 * \param format a printf format */
int
sock_printf_error(int fd, const char *format, .../*args*/ )
{
	static const char huh[] = "huh? ";
	char buf[MAXMSG];
	va_list ap;
	int size = 0;

	strncpy(buf, huh, sizeof(huh)); // note: sizeof(huh) < MAXMSG

	va_start(ap, format);
	size = vsnprintf(buf + (sizeof(huh)-1), sizeof(buf) - (sizeof(huh)-1), format, ap);
	buf[sizeof(buf)-1] = '\0';
	va_end(ap);

	if (size < 0) {
		report(RPT_ERR, "sock_printf_error: vsnprintf failed");
		return -1;
	}
	if (size >= sizeof(buf) - (sizeof(huh)-1))
		report(RPT_WARNING, "sock_printf_error: vsnprintf truncated message");

	report(RPT_ERR, "error: %s", buf);
	return sock_send_string(fd, buf);
}
