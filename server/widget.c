#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../shared/sockets.h"
#include "../shared/debug.h"

#include "screen.h"
#include "widget.h"

char *types[] = { "none",
   "string",
   "hbar",
   "vbar",
   "icon",
   "title",
   "scroller",
   "frame",
   "num",
   //"",
   NULL,
};

static widget *widget_finder (LL * list, char *id);
static int widget_remover (LL * list, widget * w);

widget *
widget_create ()
{
   widget *w;

   debug ("widget_create()\n");

   w = malloc (sizeof (widget));
   if (!w) {
      fprintf (stderr, "widget_create: Error allocating widget\n");
      return NULL;
   }

   w->id = NULL;
   w->type = 0;
   w->x = 1;
   w->y = 1;
   w->wid = 0;
   w->hgt = 0;
   w->left = 1;
   w->top = 1;
   w->right = 0;
   w->bottom = 0;
   w->length = 1;
   w->speed = 1;
   w->text = NULL;
   w->kids = NULL;

   return w;
}

int
widget_destroy (widget * w)
{
   LL *list;
   widget *foo;

   if (!w)
      return -1;

   debug ("widget_destroy(%s)\n", w->id);

   if (w->id)
      free (w->id);
   //debug("widget_destroy: id...\n");
   if (w->text)
      free (w->text);
   //debug("widget_destroy: text...\n");

   // TODO: Free kids!
   if (w->kids) {
      list = w->kids;
      LL_Rewind (list);
      do {
	 foo = LL_Get (list);
	 if (foo)
	    widget_destroy (foo);
      } while (0 == LL_Next (list));

      LL_Destroy (list);
      w->kids = NULL;
   }

   free (w);
   //debug("widget_destroy: widget...\n");

   return 0;
}

widget *
widget_find (screen * s, char *id)
{
   //widget *w, *err;

   if (!s)
      return NULL;
   if (!id)
      return NULL;

   debug ("widget_find(%s)\n", id);

   return widget_finder (s->widgets, id);
/*
   LL_Rewind(s->widgets);
   do {
      w = LL_Get(s->widgets);
      if( (w)  &&  (0 == strcmp(w->id, id)))
      {
	 debug("widget_find:  Found %s\n", id);
	 return w;
      }
      // TODO:  Search kids too!
      if( w->type == WID_FRAME )
      {
	 err = widget_finder(w->kids, id);
	 if(err) return err;
      }
      
   } while(LL_Next(s->widgets) == 0);

   return NULL;
*/
}

widget *
widget_finder (LL * list, char *id)
{
   widget *w, *err;

   if (!list)
      return NULL;
   if (!id)
      return NULL;

   debug ("widget_finder(%s)\n", id);

   LL_Rewind (list);
   do {
      //debug("widget_finder: Iteration\n");
      w = LL_Get (list);
      if (w) {
	 //debug("widget_finder: ...\n");
	 if (0 == strcmp (w->id, id)) {
	    debug ("widget_finder:  Found %s\n", id);
	    return w;
	 }
	 // Search kids recursively
	 //debug("widget_finder: ...\n");
	 if (w->type == WID_FRAME) {
	    err = widget_finder (w->kids, id);
	    if (err)
	       return err;
	 }
	 //debug("widget_finder: ...\n");
      }

   } while (LL_Next (list) == 0);

   return NULL;
}

int
widget_add (screen * s, char *id, char *type, char *in, int sock)
{
   int i;
   int valid = 0;
   int wid_type = 0;
   widget *w, *parent;
   LL *list;

   debug ("widget_add(%s, %s, %s)\n", id, type, in);

   if (!s)
      return -1;
   if (!id)
      return -1;

   list = s->widgets;

   if (0 == strcmp (id, "heartbeat")) {
      s->heartbeat = 1;
      return 0;
   }
   // Make sure this screen doesn't already exist...
   w = widget_find (s, id);
   if (w) {
      // already exists
      sock_send_string (sock, "huh? You already have a widget with that id#\n");
      return 1;
   }
   // Make sure the container, if any, is real
   if (in) {
      parent = widget_find (s, in);
      if (!parent) {
	 // no frame to use as parent
	 sock_send_string (sock, "huh? Frame doesn't exist\n");
	 return 3;
      } else {
	 if (parent->type == WID_FRAME) {
	    list = parent->kids;
	    if (!list)
	       fprintf (stderr, "widget_add: Parent has no kids\n");
	 } else {
	    // no frame to use as parent
	    sock_send_string (sock, "huh? Not a frame\n");
	    return 4;
	 }
      }
   }
   // Make sure it's a valid widget type
   for (i = 1; types[i]; i++) {
      if (0 == strcmp (types[i], type)) {
	 valid = 1;
	 wid_type = i;
      }
   }
   if (!valid) {
      // invalid widget type
      sock_send_string (sock, "huh? Invalid widget type\n");
      return 2;
   }

   debug ("widget_add: making widget\n");

   // Now, make it...
   w = widget_create ();
   if (!w) {
      fprintf (stderr, "widget_add:  Error creating widget\n");
      return -1;
   }

   w->id = strdup (id);
   if (!w->id) {
      fprintf (stderr, "widget_add:  Error allocating id\n");
      return -1;
   }

   w->type = wid_type;

   // Set up the container's widget list...
   if (w->type == WID_FRAME) {
      if (!w->kids) {
	 w->kids = LL_new ();
	 if (!w->kids) {
	    fprintf (stderr, "widget_add: error allocating kids for frame\n");
	    return -1;
	 }
      }
   }
   // TODO:  Check for errors here?
   LL_Push (list, (void *) w);

   return 0;
}

int
widget_remove (screen * s, char *id, int sock)
{
   widget *w;
   LL *list;

   debug ("widget_remove(%s)\n", id);

   if (!s)
      return -1;
   if (!id)
      return -1;

   list = s->widgets;

   if (0 == strcmp (id, "heartbeat")) {
      s->heartbeat = 0;
      return 0;
   }
   // Make sure this screen *does* exist...
   w = widget_find (s, id);
   if (!w) {
      sock_send_string (sock, "huh? You don't have a widget with that id#\n");
      debug ("widget_remove:  Error finding widget %s\n", id);
      return 1;
   }

/*
   // TODO:  Check for errors here?
   // TODO:  Make this work with frames...
//   LL_Remove(list, (void *)w);

   
   // TODO:  Check for errors here?
//   widget_destroy(w);
*/

   return widget_remover (list, w);

//   return 0;
}

int
widget_remover (LL * list, widget * w)
{
   widget *foo, *bar;
   int err;

   debug ("widget_remover\n");

   if (!list)
      return 0;
   if (!w)
      return 0;

   // Search through the list...
   LL_Rewind (list);
   do {
      // Test each item
      foo = LL_Get (list);
      if (foo) {
	 // Frames require recursion to search and/or destroy
	 if (foo->type == WID_FRAME) {
	    // If removing a frame, kill all its kids, too...
	    if (foo == w) {
	       if (!foo->kids) {
		  fprintf (stderr, "widget_remover: frame has no kids\n");
	       } else		// Kill the kids...
	       {
		  LL_Rewind (foo->kids);
		  do {
		     bar = LL_Get (foo->kids);
		     err = widget_remover (foo->kids, bar);
		  } while (0 == LL_Next (foo->kids));

		  // Then kill the parent...
		  LL_Remove (list, (void *) w);
		  widget_destroy (w);
	       }

	    } else		// Otherwise, search the frame recursively...
	    {
	       if (!foo->kids) {
		  fprintf (stderr, "widget_remover: frame has no kids\n");
	       } else
		  err = widget_remover (foo->kids, w);

	    }
	 } else			// If not a frame, remove it if it matches...
	 {
	    if (foo == w) {
	       LL_Remove (list, (void *) w);
	       widget_destroy (w);
	    }
	 }
      }
   } while (0 == LL_Next (list));

   return 0;
}
