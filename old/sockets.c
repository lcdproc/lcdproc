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
#include "sockets.h"


/**************************************************
  LCDproc sockets code...

  This is messy, and needs to be finished.
**************************************************/

fd_set active_fd_set, read_fd_set;
int sock;

// Length of longest transmission allowed at once...
#define MAXMSG 8192



int init_sockaddr (sockaddr_in *name,
		   const char *hostname,
		   unsigned short int port)
{
  struct hostent *hostinfo;
     
  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr,"Unknown host %s.\n", hostname);
      return -1;
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

  return 0;
  
}


#if 0
// Creates a socket as a file...
int CreateNamedSocket(char *filename)
{
  struct sockaddr_un name;
  size_t size;

  sock=socket(PF_FILE, SOCK_STREAM, 0);
  if(sock < 0) return -1;


  name.sun_family = AF_FILE;
  strcpy (name.sun_path, filename);
     
  /* The size of the address is
     the offset of the start of the filename,
     plus its length,
     plus one for the terminating null byte. */
  size = (offsetof (struct sockaddr_un, sun_path)
	  + strlen (name.sun_path) + 1);
     
  if (bind (sock, (struct sockaddr *) &name, size) < 0)
    return -1;
     
  return sock;
}
#endif

// Creates a socket in internet space
int CreateInetSocket(unsigned short int port)
{
  struct sockaddr_in name;
     
  /* Create the socket. */
  //fprintf(stderr,"Creating Inet Socket\n");
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;
     
  /* Give the socket a name. */
  //fprintf(stderr,"Binding Inet Socket\n");
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    return -1;
     
  return sock;
 
}

// Checks the LCDproc port for a response...
int PingLCDport()
{
  struct sockaddr_in servername;
  int err=0;
  char ping[16] = {0xFE, 0x01, 0};
  
  
  //fprintf(stderr,"Creating socket (client)\n");
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0)
    {
      fprintf(stderr,"Error creating socket (client)\n");
      return sock;
    }
  //else fprintf(stderr,"Created socket (%i) (client)\n", sock);
  

  init_sockaddr(&servername, "localhost", LCDPORT);
  
  err = connect (sock,
		 (struct sockaddr *) &servername,
		 sizeof (servername));
  if(err<0)
    {
      //fprintf (stderr,"connect failed (client)\n");
      shutdown(sock, 2);
      return 0;  // Normal exit if server doesn't exist...
    }
  
  err = write (sock, ping, strlen(ping) + 1);
  if (err < 0)
    {
      fprintf (stderr,"socket write error (client)");
      shutdown(sock, 2);
      return err;
    }

  shutdown(sock, 2);
  
  return 1;
}

int ConnectAsClient()
{
  return 0;
}

int StartSocketServer()
{
     
  /* Create the socket and set it up to accept connections. */
  sock = CreateInetSocket (LCDPORT);
  if(sock < 0)
    {
      fprintf(stderr,"Error creating socket (server)\n");
      return -1;
    }
  
  
  if (listen (sock, 1) < 0)
    {
      fprintf (stderr,"Listen error (server)\n");
      return -1;
    }
  
  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  return sock;
}


int PollSockets()
{
  int i;
  struct sockaddr_in clientname;
  size_t size;
  struct timeval t;

  
  t.tv_sec = 0;
  t.tv_usec = 0;

  /* Block until input arrives on one or more active sockets. */
  read_fd_set = active_fd_set;

  if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &t) < 0)
    {
      fprintf (stderr,"Select error (server)\n");
      return -1;
    }

  /* Service all the sockets with input pending. */
  for (i = 0; i < FD_SETSIZE; ++i)
    if (FD_ISSET (i, &read_fd_set))
      {
	if (i == sock)
	  {
	    /* Connection request on original socket. */
	    int new;
	    size = sizeof (clientname);
	    new = accept (sock,
			  (struct sockaddr *) &clientname,
			  &size);
	    if (new < 0)
	      {
		fprintf (stderr,"Accept error (server)\n");
		return -1;
	      }
	    fprintf (stderr,"Server: connect from host %s, port %hd.\n",
		     inet_ntoa (clientname.sin_addr),
		     ntohs (clientname.sin_port));
	    FD_SET (new, &active_fd_set);
	  }
	else
	  {
	    /* Data arriving on an already-connected socket. */
	    if (read_from_client (i) < 0)
	      {
		close (i);
		FD_CLR (i, &active_fd_set);
		fprintf(stderr,"Closed connection %i\n", i);
	      }
	  }
      }
  return 0;
}


int read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;
  
  nbytes = read (filedes, buffer, MAXMSG);
  buffer[nbytes] = 0;
  buffer[nbytes+1] = 0;
  
  if (nbytes < 0)  // Read error
    {
      fprintf (stderr,"Read error (server)\n");
      return -1;
    }
  else if (nbytes == 0)  // EOF
    return -1;
  else         // Data Read
    {
      fprintf (stderr,"Server: got message: `%s'\n", buffer);
      return 0;
    }
}

int CloseAllConnections()
{
  int i;
  
    /* Service all the sockets with input pending. */
  for (i = 0; i < FD_SETSIZE; ++i)
    if (FD_ISSET (i, &read_fd_set))
      {
	/* Data arriving on an already-connected socket. */
	close (i);
	FD_CLR (i, &active_fd_set);
	fprintf(stderr,"Closed connection %i\n", i);
      }

  return 0;
  
}
