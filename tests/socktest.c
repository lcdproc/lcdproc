#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define LCDPORT 13667

#include "../shared/sockets.h"


int main()
{
   char string[1024];
   char string2[1024];
   int sock;
   int err;
   
   memset(string, 0, 1024);

   fcntl(stdin, F_SETFL, O_NONBLOCK);
   
   sock = sock_connect("localhost", LCDPORT);

   if(sock <= 0)
   {
      printf("Error opening socket.\n");
      return 1;
   }


   while(string[0] != 'q')
   {
      do {
	 err = sock_recv(sock, string2, 1024);
	 if (err > 0) printf("Received message: %s\n", string2);
      } while(err > 0);
      printf("Enter a string to send:\n\t");
      gets(string);
      sock_send_string(sock, string);
   }
   sock_close(sock);

   return 0;
}
