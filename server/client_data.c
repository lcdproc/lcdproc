/*
  client_data.c

  Creates and destroys a client's data structures.  These are mainly
  its name, screen list, and menu list.

 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../shared/debug.h"

#include "client_data.h"
#include "screen.h"
#include "screenlist.h"



int client_data_init(client_data *d)
{
   if(!d) return -1;

   d->ack = 0;
   d->name = NULL;

   d->screenlist = LL_new();
   if(!d->screenlist)
   {
      fprintf(stderr, "client_data_init: Error allocating screenlist\n");
      return -1;
   }
   /* TODO:  this section...  (client menus)
   d->menulist = LL_new();
   if(!d->menulist)
   {
      fprintf(stderr, "client_data_init: Error allocating menulist\n");
      return -1;
   }
   */
   return 0;
}

int client_data_destroy(client_data *d)
{
   screen *s;

   debug("client_data_destroy\n");
   
   if(!d) return -1;

   d->ack = 0;

   // Clean up the name...
   if(d->name)
      free(d->name);

   // Clean up the screenlist...
   debug("client_data_destroy: Cleaning screenlist\n");
   LL_Rewind(d->screenlist);
   do {
      s = (screen *)LL_Get(d->screenlist);
      if(s)
      {
	 debug("client_data_destroy: removing screen %s\n", s->id);

	 // FIXME? This shouldn't be handled here...
	 // Now, remove it from the screenlist...
	 if(screenlist_remove_all(s) < 0)
	 {
	    // Not a serious error..
	    fprintf(stderr, "client_data_destroy:  Error dequeueing screen\n");
	    return 0;
	 }

	 // Free its memory...
	 screen_destroy(s);
      }
   } while(LL_Next(d->screenlist) == 0);
   LL_Destroy(d->screenlist);
   
   // TODO:  clean up the rest of the data...

   return 0;
}


