#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "screen.h"
#include "widget.h"
#include "render.h"

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

static widget *widget_finder (LinkedList * list, char *id);
static int widget_remover (LinkedList * list, widget * w);

widget *
widget_create ()
{
	widget *w;

	report (RPT_INFO, "widget_create()");

	w = malloc (sizeof (widget));
	if (!w) {
		report (RPT_DEBUG, "widget_create: Error allocating widget");
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
	LinkedList *list;
	widget *foo;

	if (!w)
		return -1;

	debug (RPT_INFO, "widget_destroy(%s)", w->id);

	if (w->id)
		free (w->id);
	//debug(RPT_DEBUG, "widget_destroy: id...");
	if (w->text)
		free (w->text);
	//debug(RPT_DEBUG, "widget_destroy: text...");

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
	//debug(RPT_DEBUG, "widget_destroy: widget...");

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

	debug (RPT_DEBUG, "widget_find(%s)", id);

	return widget_finder (s->widgets, id);
}

widget *
widget_finder (LinkedList * list, char *id)
{
	widget *w, *err;

	if (!list)
		return NULL;
	if (!id)
		return NULL;

	debug (RPT_DEBUG, "widget_finder(%s)", id);

	LL_Rewind (list);
	do {
		//debug(RPT_DEBUG, "widget_finder: Iteration");
		w = LL_Get (list);
		if (w) {
			//debug(RPT_DEBUG, "widget_finder: ...");
			if (0 == strcmp (w->id, id)) {
				debug (RPT_DEBUG, "widget_finder:  Found %s", id);
				return w;
			}
			// Search kids recursively
			//debug(RPT_DEBUG, "widget_finder: ...");
			if (w->type == WID_FRAME) {
				err = widget_finder (w->kids, id);
				if (err)
					return err;
			}
			//debug(RPT_DEBUG, "widget_finder: ...");
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
	LinkedList *list;

	report (RPT_INFO, "widget_add(%s, %s, %s)", id, type, in);

	if (!s)
		return -1;
	if (!id)
		return -1;

	list = s->widgets;

	if (0 == strcmp (id, "heartbeat")) {
		s->heartbeat = HEARTBEAT_ON; // was 1
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
					report (RPT_DEBUG, "widget_add: Parent has no kids");
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
			break; // it's valid: skip out...
		}
	}

	if (!valid) {
		// invalid widget type
		sock_send_string (sock, "huh? Invalid widget type\n");
		return 2;
	}

	debug (RPT_DEBUG, "widget_add: making widget");

	// Now, make it...
	w = widget_create ();
	if (!w) {
		report (RPT_ERR, "widget_add:  Error creating widget");
		return -1;
	}

	w->id = strdup (id);
	if (!w->id) {
		report (RPT_ERR, "widget_add:  Error allocating id");
		return -1;
	}

	w->type = wid_type;

	// Set up the container's widget list...
	if (w->type == WID_FRAME) {
		if (!w->kids) {
			w->kids = LL_new ();
			if (!w->kids) {
				report (RPT_ERR, "widget_add: error allocating kids for frame");
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
	LinkedList *list;

	report (RPT_INFO, "widget_remove(%s)", id);

	if (!s)
		return -1;
	if (!id)
		return -1;

	list = s->widgets;

	if (0 == strcmp (id, "heartbeat")) {
		s->heartbeat = HEARTBEAT_OFF; // was 0
		return 0;
	}
	// Make sure this screen *does* exist...
	w = widget_find (s, id);
	if (!w) {
		sock_send_string (sock, "huh? You don't have a widget with that id#\n");
		report (RPT_ERR, "widget_remove:  Error finding widget %s", id);
		return 1;
	}

//	TODO: Check for errors here?
//	TODO: Make this work with frames...
//	LL_Remove(list, (void *)w);


//	TODO: Check for errors here?
//	widget_destroy(w);

	return widget_remover (list, w);

//	return 0;
}

int
widget_remover (LinkedList * list, widget * w)
{
	widget *foo, *bar;
	int err;

	debug (RPT_DEBUG, "widget_remover");

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
						debug (RPT_DEBUG, "widget_remover: frame has no kids");
					} else			  // Kill the kids...
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

				} else				  // Otherwise, search the frame recursively...
				{
					if (!foo->kids) {
						debug (RPT_DEBUG, "widget_remover: frame has no kids");
					} else
						err = widget_remover (foo->kids, w);

				}
			} else					  // If not a frame, remove it if it matches...
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
