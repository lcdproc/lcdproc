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


static int sock_init_sockaddr (sockaddr_in *name,
		   const char *hostname,
		   unsigned short int port)
{
  struct hostent *hostinfo;
     
  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr,"sock_init_sockaddr: Unknown host %s.\n", hostname);
      return -1;
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

  return 0;
  
}

 // Client functions...
int sock_connect(char *host, unsigned short int port)
{
  struct sockaddr_in servername;
  int sock;
  int err=0;
  
  
  debug("sock_connect: Creating socket\n");
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0)
    {
      perror("sock_connect: Error creating socket");
      return sock;
    }
  debug("sock_connect: Created socket (%i)\n", sock);
  
  sock_init_sockaddr(&servername, host, port);
  
  err = connect (sock,
		 (struct sockaddr *) &servername,
		 sizeof (servername));
  if(err<0)
    {
      perror("sock_connect: connect failed");
      shutdown(sock, 2);
      return 0;  // Normal exit if server doesn't exist...
    }
  
  fcntl(sock, F_SETFL, O_NONBLOCK);

  
  return sock;
}

int sock_close(int fd)
{
   int err;
   
   err=shutdown(fd, 2);

   return err;
}

// Send/receive lines of text
int sock_send_string(int fd, char *string)
{
   int err;

   if(!string) return -1;
   
   err = write (fd, string, strlen(string) + 1);
   if (err < 0)
   {
      perror ("sock_send_string: socket write error");
      printf("Message was: %s\n", string);
      //shutdown(fd, 2);
      return err;
   }

   //printf("sock_send_string: %i bytes\n", err);
   
   return err;
}

// Recv gives only one line per call...
int sock_recv_string(int fd, char *dest, size_t maxlen)
{
   char * err;
   int i;

   // TODO:  Get this function to work right somehow...
   return -1;
   
   if(!dest) return -1;
   if(maxlen <= 0) return 0;

   // Read in characters until the end of the line...
   for(i=0;
       i<maxlen && (read(fd, dest+i, 1) > 0);
       i++)
      if(dest[i] == 0  ||  dest[i] == '\n') break;
      
   if (err == NULL)
   {
      perror("sock_recv_string: socket read error");
      //shutdown(fd, 2);
      return -1;
   }
   printf("sock_recv_string: Got message \"%s\"\n", dest);
   
   return strlen(dest);
}

// Send/receive raw data
int sock_send(int fd, void *src, size_t size)
{
   int err;
   
   if(!src) return -1;
   
   err = write (fd, src, size);
   if (err < 0)
   {
      perror("sock_send: socket write error");
      //shutdown(fd, 2);
      return err;
   }

   return err;
}

int sock_recv(int fd, void *dest, size_t maxlen)
{
   int err;
   
   if(!dest) return -1;
   if(maxlen <= 0) return 0;
   
   err = read (fd, dest, maxlen);
   if (err < 0)
   {
      //fprintf (stderr,"sock_recv: socket read error\n");
      //shutdown(fd, 2);
      return err;
   }
   //debug("sock_recv: Got message \"%s\"\n", (char *)dest);
   
   return err;
}
