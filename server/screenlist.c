#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "shared/LL.h"
#include "shared/sockets.h"
#include "shared/debug.h"
#include "screenlist.h"
#include "screen.h"
#include "clients.h"

int screenlist_action = 0;

int timer = 0;

LL *screenlist;

int screenlist_add_end (screen * screen);
screen *screenlist_next_roll ();
screen *screenlist_prev_roll ();
screen *screenlist_next_priority ();

int compare_priority (void *one, void *two);
int compare_addresses (void *one, void *two);

int
screenlist_init ()
{
	debug ("screenlist_init()\n");

	screenlist = LL_new ();
	if (!screenlist) {
		syslog(LOG_ERR, "screenlist_init: error allocating list");
		return -1;
	}

	screenlist_action = 0;
	timer = 0;

	return 0;
}

int
screenlist_shutdown ()
{
	debug ("screenlist_shutdown()\n");

	LL_Destroy (screenlist);

	return 0;
}

int
screenlist_remove (screen * s)
{
	debug ("screenlist_remove()\n");

	if (!LL_Remove (screenlist, s))
		return -1;
	else
		return 0;
}

int
screenlist_remove_all (screen * s)
{
	int i = 0;

	debug ("screenlist_remove_all()\n");

	while (LL_Remove (screenlist, s))
		i++;

	debug ("screenlist_remove_all()... got %i\n", i);

	return i;
}

LL *
screenlist_getlist ()
{
	debug ("screenlist_getlist()\n");

	return screenlist;
}

screen *
screenlist_current ()
{
	char str[256];
	screen *s;
	static screen *old_s = NULL;
	client *c;

	//debug("screenlist_current:\n");
	//LL_dprint(screenlist);

	s = (screen *) LL_GetFirst (screenlist);

	// FIXME:  Make sure the screen/client exists!
	if (s != old_s) {
		debug ("screenlist_current: new screen\n");
		timer = 0;

		// Tell the client we're done with the current screen
		if (old_s) {
			//debug("screenlist_current: ignoring old screen\n");
			LL_Rewind (screenlist);
			if (old_s != LL_Find (screenlist, compare_addresses, old_s)) {
				debug ("screenlist: Didn't find screen 0x%8x!\n", (int) old_s);
			} else {
				//debug("screenlist_current: ... sending ignore\n");
				c = old_s->parent;
				if (c)				  // Tell the client we're not listening any more...
				{
					snprintf (str, sizeof(str), "ignore %s\n", old_s->id);
					sock_send_string (c->sock, str);
				} else				  // The server has the display, so do nothing
				{
					;
				}
				//debug("screenlist_current: ... sent ignore\n");
			}
		}
		if (s) {
			//debug("screenlist_current: listening to new screen\n");
			c = s->parent;
			if (c)					  // Tell the client we're paying attention...
			{
				snprintf (str, sizeof(str), "listen %s\n", s->id);
				sock_send_string (c->sock, str);
			} else					  // The server has the display, so do nothing
			{
				;
			}
		}
	}

	old_s = s;

	//debug("screenlist_current: return %8x\n", s);

	return s;
}

int
screenlist_add (screen * s)
{
	// TODO:  Different queueing modes...
	return screenlist_add_end (s);
}

screen *
screenlist_next ()
{
	screen *s;

	//debug("Screenlist_next()\n");

	s = screenlist_current ();

	// If we're on hold, don't advance!
	if (screenlist_action == SCR_HOLD)
		return s;
	if (screenlist_action == RENDER_HOLD)
		return s;

	// Otherwise, reset it to regular operation
	screenlist_action = 0;

	//debug("Screenlist_next: calling handler...\n");

	// Call the selected queuing function...
	// TODO:  Different queueing modes...
	s = screenlist_next_priority ();
	//s = screenlist_next_roll();

	//debug("Screenlist_next() done\n");

	return s;
}

screen *
screenlist_prev ()
{
	screen *s;

	s = screenlist_current ();

	// If we're on hold, don't advance!
	if (screenlist_action == SCR_HOLD)
		return s;
	if (screenlist_action == RENDER_HOLD)
		return s;

	// Otherwise, reset it no regular operation
	screenlist_action = 0;

	// Call the selected queuing function...
	// TODO:  Different queueing modes...
	s = screenlist_prev_roll ();

	return s;
}

// Adds new screens to the end of the screenlist...
int
screenlist_add_end (screen * screen)
{
	debug ("screenlist_add_end()\n");

	return LL_Push (screenlist, (void *) screen);
}

// Simple round-robin approach to screen cycling...
screen *
screenlist_next_roll ()
{
	//debug("screenlist_next_roll()\n");

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	return screenlist_current ();
}

// Strict priority queue approach...
screen *
screenlist_next_priority ()
{
	//screen *s, *t;
	//debug("screenlist_next_priority\n");

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	LL_Sort (screenlist, compare_priority);

	return screenlist_current ();
}

// Simple round-robin approach to screen cycling...
screen *
screenlist_prev_roll ()
{
	//debug("screenlist_prev_roll()\n");

	if (LL_Roll (screenlist) != 0)
		return NULL;

	return screenlist_current ();
}

int
compare_priority (void *one, void *two)
{
	screen *a, *b;

	//debug("compare_priority: %8x %8x\n", one, two);

	if (!one)
		return 0;
	if (!two)
		return 0;

	a = (screen *) one;
	b = (screen *) two;

	//debug("compare_priority: done?\n");

	return (a->priority - b->priority);
}

int
compare_addresses (void *one, void *two)
{
	//printf("compare_addresses: 0x%x == 0x%x ???\n", one, two);
	//if(one == two) printf("Yes!\n");
	//else printf("No!\n");
	return (one != two);
}
