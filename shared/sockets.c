#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
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

#include "debug.h"
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

   name->sin_family = AF_INET;
   name->sin_port = htons (port);
   hostinfo = gethostbyname (hostname);
   if (hostinfo == NULL) {
      fprintf (stderr, "sock_init_sockaddr: Unknown host %s.\n", hostname);
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


   debug ("sock_connect: Creating socket\n");
   sock = socket (PF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      perror ("sock_connect: Error creating socket");
      return sock;
   }
   debug ("sock_connect: Created socket (%i)\n", sock);

   sock_init_sockaddr (&servername, host, port);

   err = connect (sock, (struct sockaddr *) &servername, sizeof (servername));
   if (err < 0) {
      perror ("sock_connect: connect failed");
      shutdown (sock, 2);
      return 0;			// Normal exit if server doesn't exist...
   }

   fcntl (sock, F_SETFL, O_NONBLOCK);


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

   len = strlen (string) + 1;
   while (offset != len) {
      // write isn't guaranteed to send the entire string at once,
      // so we have to sent it in a loop like this
      int sent = write (fd, string + offset, len - offset);
      if (sent == -1) {
	 if (errno != EAGAIN) {
	    perror ("sock_send_string: socket write error");
	    printf ("Message was: %s\n", string);
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
   int recv = 0;

   if (!dest)
      return -1;
   if (maxlen <= 0)
      return 0;

   while (1) {
      int err = read (fd, ptr, 1);
      if (err == -1) {
	 if (errno == EAGAIN) {
	    if (recv) {
	       // We've begun to read a string, but no bytes are
	       // available.  Loop.
	       continue;
	    }
	    return 0;
	 } else {
	    perror ("sock_recv_string: socket read error");
	    return err;
	 }
      } else if (err == 0) {
	 return recv;
      }

      recv++;

      if (recv == maxlen || *ptr == 0 || *ptr == 10) {
	 *ptr = 0;
	 break;
      }
      ptr++;
   }

   if (recv == 1 && dest[0] == 0) {
      // Don't return a null string
      return 0;
   }

   if (recv < maxlen - 1) {
      dest[recv] = 0;
   }

   return recv;
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
      int sent = write (fd, ((char *) src) + offset, size - offset);
      if (sent == -1) {
	 if (errno != EAGAIN) {
	    perror ("sock_send: socket write error");
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

   err = read (fd, dest, maxlen);
   if (err < 0) {
      //fprintf (stderr,"sock_recv: socket read error\n");
      //shutdown(fd, 2);
      return err;
   }
   //debug("sock_recv: Got message \"%s\"\n", (char *)dest);

   return err;
}
