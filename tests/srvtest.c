#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// For debugging purposes...
// ... should not be defined in a release.
// (defaults to 13666)
//#define LCDPORT 13667

#include "../server/drivers/lcd.h"
#include "../server/sock.h"
#include "../server/clients.h"
#include "../server/screenlist.h"
#include "../server/screen.h"
#include "../server/parse.h"
#include "../server/render.h"
#include "../server/serverscreens.h"


void exit_program(int val);


int main()
{
   char string[1024];
   int sock;
   screen *s;
   int timer=0;
   

   
   // Ctrl-C will cause a clean exit...
   signal(SIGINT, exit_program);
   // and "kill"...
   signal(SIGTERM, exit_program);
   // and "kill -HUP" (hangup)...
   signal(SIGHUP, exit_program);
   // and just in case, "kill -KILL" (which cannot be trapped; but oh well)
   signal(SIGKILL, exit_program);

   
   memset(string, 0, 1024);

   lcd_init("");

   // Feel free to add as many drivers as you like...  :)
   //lcd_add_driver("joy", "");
   //lcd_add_driver("curses", "Booger");
   lcd_add_driver("MtxOrb", "");


   
   sock = sock_create_server();

   if(sock <= 0)
   {
      printf("Error opening socket.\n");
      return 1;
   }

   if(client_init() < 0)
   {
      printf("Error initializing client list\n");
      return 1;
   }
   
   if(screenlist_init() < 0)
   {
      printf("Error initializing screen list\n");
      return 1;
   }

   // Make sure the server screen shows up every once in a while..
   server_screen_init();
   
   // Main loop...
   while(1)
   {
      sock_poll_clients();
      parse_all_client_messages();
      // TODO: Check for and handle keypresses here...
      //handle_input();

      timer++;
      if(timer >= 32)
      {
	 timer = 0;
	 // TODO: Notify clients of "listen" and "ignore"
	 s = screenlist_next();
	 if(s)
	 {
	    printf("Screen: %s\n", s->id);
	 }
      }
      draw_server_screen(timer);
      s = screenlist_current();
      if(s)
      {
	 // render something here...
	 draw_screen(s, timer);
      }
      else
      {
	 no_screen_screen(timer);
      }
      
      usleep(125000);
   }

   // Say goodbye!
   goodbye_screen();

   // Can go anywhere...
   lcd_shutdown();
   // Must come before screenlist_shutdown
   client_shutdown();
   // Must come after client_shutdown
   screenlist_shutdown();
   // Should come after client_shutdown
   sock_close_all();
   
   return 0;
}


void exit_program(int val)
{
   // Say goodbye!
   goodbye_screen();
   
   // Can go anywhere...
   lcd_shutdown();
   // Must come before screenlist_shutdown
   client_shutdown();
   // Must come after client_shutdown
   screenlist_shutdown();
   // Should come after client_shutdown
   sock_close_all();
   
   exit(0);

}
