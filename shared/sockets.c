#include "config.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef WINSOCK2
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <fcntl.h>

#include "report.h"
#include "sockets.h"

/**************************************************
  LCDproc client sockets code...

  Feel free to use this in your own clients... :)
**************************************************/

// Length of longest transmission allowed at once...
#define MAXMSG 8192

typedef struct sockaddr_in sockaddr_in;

static int
sock_init_sockaddr (sockaddr_in * name, const char *hostname, unsigned short int port)
{
	struct hostent *hostinfo;

	memset (name, 0, sizeof (*name));
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

 // Client functions...
int
sock_connect (char *host, unsigned short int port)
{
	struct sockaddr_in servername;
	int sock;
	int err = 0;

	report (RPT_DEBUG, "sock_connect: Creating socket");
	sock = socket (PF_INET, SOCK_STREAM, 0);
#ifdef WINSOCK2        
        if (sock == INVALID_SOCKET) {
#else
	if (sock < 0) {
#endif
		report (RPT_ERR, "sock_connect: Error creating socket");
		return sock;
	}
	debug (RPT_DEBUG, "sock_connect: Created socket (%i)", sock);

	if( sock_init_sockaddr (&servername, host, port) < 0 )
		return -1;

	err = connect (sock, (struct sockaddr *) &servername, sizeof (servername));
#ifdef WINSOCK2        
	if (err == INVALID_SOCKET) {
#else
	if (err < 0) {
#endif
		report (RPT_ERR, "sock_connect: connect failed");
		shutdown (sock, 2);
		return 0;					  // Normal exit if server doesn't exist...
	}

#ifndef WINSOCK2        
	fcntl (sock, F_SETFL, O_NONBLOCK);
#else
        {
                unsigned long tmp = 1;
                if (ioctlsocket(sock, FIONBIO, &tmp) == SOCKET_ERROR)
                {
                        report(RPT_ERR, "sock_connect: Error setting socket to non-blocking");
                }
        }
#endif

	return sock;
}

int
sock_close (int fd)
{
	int err;

	err = shutdown (fd, 2);
	if (!err)
		close (fd);

	return err;
}

// Send/receive lines of text
int
sock_send_string (int fd, char *string)
{
	int len;
	int offset = 0;

	if (!string)
		return -1;

	len = strlen (string) ;
	while (offset != len) {
		// write isn't guaranteed to send the entire string at once,
		// so we have to sent it in a loop like this
#ifndef WINSOCK2
		int sent = write (fd, string + offset, len - offset);
#else
                int sent = send(fd, string + offset, len - offset, 0);
#endif
		if (sent == -1) {
			if (errno != EAGAIN) {
				report (RPT_ERR, "sock_send_string: socket write error");
				report (RPT_DEBUG, "Message was: %s", string);
				//shutdown(fd, 2);
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

// Recv gives only one line per call...
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
#ifndef WINSOCK2
		int err = read (fd, ptr, 1);
#else
                int err = recv(fd, ptr, 1, 0);
#endif
		if (err == -1) {
			if (errno == EAGAIN) {
				if (recv) {
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

		if (recvBytes == maxlen || *ptr == 0 || *ptr == 10) {
			*ptr = 0;
			break;
		}
		ptr++;
	}

	if (recvBytes == 1 && dest[0] == 0) {
		// Don't return a null string
		return 0;
	}

	if (recvBytes < maxlen - 1) {
		dest[recvBytes] = 0;
	}

	return recvBytes;
}

// Send/receive raw data
int
sock_send (int fd, void *src, size_t size)
{
	int offset = 0;

	if (!src)
		return -1;

	while (offset != size) {
		// write isn't guaranteed to send the entire string at once,
		// so we have to sent it in a loop like this
#ifndef WINSOCK2
                int sent = write (fd, ((char *) src) + offset, size - offset);
#else
                int sent = send(fd, ((char *) src) + offset, size - offset, 0);
#endif
		if (sent == -1) {
			if (errno != EAGAIN) {
				report (RPT_ERR, "sock_send: socket write error");
				//shutdown(fd, 2);
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

int
sock_recv (int fd, void *dest, size_t maxlen)
{
	int err;

	if (!dest)
		return -1;
	if (maxlen <= 0)
		return 0;

#ifndef WINSOCK2
	err = read (fd, dest, maxlen);
#else
        err = recv(fd, dest, maxlen, 0);
#endif
	if (err < 0) {
		//report (RPT_DEBUG,"sock_recv: socket read error");
		//shutdown(fd, 2);
		return err;
	}
	//debug(RPT_DEBUG, "sock_recv: Got message \"%s\"", (char *)dest);

	return err;
}

/*****************************************************************************/
static char retString[256];

char*
sock_geterror(void)
{
#ifndef WINSOCK2
    return strerror(errno);
#else
    long err;
    char* tmp;

    err = WSAGetLastError();

    sprintf(retString, "Error code %ld: ", err);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  err,
                  0, /* Default language */
                  (LPTSTR) &tmp,
                  0,
                  NULL);

    /* append the message text after the error code and ensure a terminating
       character ends the string */
    strncpy(retString + strlen(retString), tmp, 
            sizeof(retString) - strlen(retString) - 1);
    retString[sizeof(retString) - 1] = '\0';

    return retString;
#endif
}

