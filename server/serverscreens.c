#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../shared/debug.h"

#include "drivers/lcd.h"

#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"

#include "serverscreens.h"

screen *server_screen;
char *id = "ClientList";
char *name = "Client List";

char title[256] = "LCDproc Server";
char one[256]   = "";
char two[256]   = "";
char three[256] = "";

int server_screen_init()
{
   widget *w;

   debug("server_screen_init\n");
   
   server_screen = screen_create();

   if(!server_screen)
   {
      fprintf(stderr, "server_screen_init: Error allocating screen\n");
      return -1;
   }

   server_screen->id = id;
   server_screen->name = name;
   server_screen->duration = 8;  // 1 second, instead of 4...
   
   // TODO:  Error-checking?
   widget_add(server_screen, "title", "title", NULL, 1);
   widget_add(server_screen, "one"  , "string", NULL, 1);
   widget_add(server_screen, "two"  , "string", NULL, 1);
   widget_add(server_screen, "three", "string", NULL, 1);


   // Now, initialize all the widgets...
   w = widget_find(server_screen, "title");
   if(w)
   {
      w->text = title;
   }
   else
   {
      fprintf(stderr, "server_screen_init: Can't find title\n");
   }
   
   w = widget_find(server_screen, "one");
   if(w)
   {
      w->x = 1;
      w->y = 2;
      w->text = one;
   }
   else
   {
      fprintf(stderr, "server_screen_init: Can't find widget one\n");
   }
   
   w = widget_find(server_screen, "two");
   if(w)
   {
      w->x = 1;
      w->y = 3;
      w->text = two;
   }
   else
   {
      fprintf(stderr, "server_screen_init: Can't find widget two\n");
   }
   
   w = widget_find(server_screen, "three");
   if(w)
   {
      w->x = 1;
      w->y = 4;
      w->text = three;
   }
   else
   {
      fprintf(stderr, "server_screen_init: Can't find widget three\n");
   }
   

   
   // And enqueue the screen
   screenlist_add(server_screen);

   debug("server_screen_init done\n");
   
   return 0;
}


int update_server_screen(int timer)
{
   client *c;
   int num_clients;
   screen *s;
   int num_screens;
   
   
   // Draw a title...
   //strcpy(title, "LCDproc Server");


   // Now get info on the number of connected clients...
   num_clients=0; num_screens=0;
   LL_Rewind(clients);
   do {
      c = LL_Get(clients);
      if(c)
      {
	 num_clients++;
	 LL_Rewind(c->data->screenlist);
	 do {
	    s = LL_Get(c->data->screenlist);
	    if(s)
	    {
	       num_screens++;
	    }
	 } while(LL_Next(c->data->screenlist) == 0);
      }
   } while(LL_Next(clients) == 0);

   if(lcd.hgt >= 3)
   {
      sprintf(one, "Clients: %i", num_clients);
      sprintf(two, "Screens: %i", num_screens);
   }
   else
   {
      if(lcd.wid >= 20)
	 sprintf(one, "%i Client%s, %i Screen%s",
		 num_clients, (num_clients==1)?"":"s",
		 num_screens, (num_screens==1)?"":"s");
      else // 16x2 size
	 sprintf(one, "%i Cli%s, %i Scr%s",
		 num_clients, (num_clients==1)?"":"s",
		 num_screens, (num_screens==1)?"":"s");
   }
   
   return 0;
}

int no_screen_screen(int timer)
{
   
   lcd.clear();

   lcd.string(1, 1, "Error:  No screen!");
   
   lcd.flush();
   
   return 0;
}

int goodbye_screen()
{
   char
      *b20 = "                    ",
      *t20 = "  Thanks for using  ",
#ifdef LINUX
      *l20 = " LCDproc and Linux! ",
#else
      *l20 = "      LCDproc!      ",
#endif
      *b16 = "                ",
      *t16 = "Thanks for using",
#ifdef LINUX
      *l16 = " LCDproc+Linux! ",
#else
      *l16 = "    LCDproc!    ",
#endif
      *nil = "";
   
   lcd.clear();

   if(lcd.hgt >= 4)
   {
      if(lcd.wid >= 20)
      {
	 lcd.string(1, 1, b20);
	 lcd.string(1, 2, t20);
	 lcd.string(1, 3, l20);
	 lcd.string(1, 4, b20);
      }
      else
      {
	 lcd.string(1, 1, b16);
	 lcd.string(1, 2, t16);
	 lcd.string(1, 3, l16);
	 lcd.string(1, 4, b16);
      }
   }
   else
   {
      if(lcd.wid >= 20)
      {
	 lcd.string(1, 1, t20);
	 lcd.string(1, 2, l20);
      }
      else
      {
	 lcd.string(1, 1, t16);
	 lcd.string(1, 2, l16);
      }
   }
   
   lcd.flush();
   
   return 0;
}

