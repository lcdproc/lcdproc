/*
 * screenlist.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * All actions that can be performed on the list of screens
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "shared/LL.h"
#include "shared/sockets.h"
#include "shared/report.h"
#include "screenlist.h"
#include "screen.h"
#include "clients.h"

int screenlist_action = 0;

int timer = 0;

LinkedList *screenlist;

int screenlist_add_end (Screen * screen);
Screen *screenlist_next_roll ();
Screen *screenlist_prev_roll ();
Screen *screenlist_next_priority ();

int compare_priority (void *one, void *two);
int compare_addresses (void *one, void *two);

int
screenlist_init ()
{
	report (RPT_INFO, "screenlist_init()");

	screenlist = LL_new ();
	if (!screenlist) {
		report(RPT_ERR, "screenlist_init: error allocating list");
		return -1;
	}

	screenlist_action = 0;
	timer = 0;

	return 0;
}

int
screenlist_shutdown ()
{
	report (RPT_INFO, "screenlist_shutdown()");

	LL_Destroy (screenlist);

	return 0;
}

int
screenlist_remove (Screen * s)
{
	report (RPT_INFO, "screenlist_remove()");

	if (!LL_Remove (screenlist, s))
		return -1;
	else
		return 0;
}

int
screenlist_remove_all (Screen * s)
{
	int i = 0;

	report (RPT_INFO, "screenlist_remove_all()");

	while (LL_Remove (screenlist, s))
		i++;

	debug (RPT_DEBUG, "screenlist_remove_all()... got %i", i);

	return i;
}

LinkedList *
screenlist_getlist ()
{
	report (RPT_INFO, "screenlist_getlist()");

	return screenlist;
}

Screen *
screenlist_current ()
{
	char str[256];
	Screen * s;
	static Screen * old_s = NULL;
	Client * c;

	debug( RPT_INFO, "screenlist_current:");

	/*LL_dprint(screenlist);*/

	s = (Screen *) LL_GetFirst (screenlist);

	/* FIXME:  Make sure the screen/client exists!*/
	if (s != old_s) {
		/*debug (RPT_DEBUG, "screenlist_current: new screen");*/
		timer = 0;

		/* Tell the client we're done with the current screen*/
		if (old_s) {
			/*debug(RPT_DEBUG, "screenlist_current: ignoring old screen");*/
			LL_Rewind (screenlist);
			if (old_s != LL_Find (screenlist, compare_addresses, old_s)) {
				report (RPT_WARNING, "screenlist: Didn't find screen 0x%8x! Client crashed?", (int) old_s);
			} else {
				/*debug(RPT_DEBUG, "screenlist_current: ... sending ignore");*/
				c = old_s->client;
				if (c)				  /* Tell the client we're not listening any more...*/
				{
					snprintf (str, sizeof(str), "ignore %s\n", old_s->id);
					sock_send_string (c->sock, str);
				} else				  /* The server has the display, so do nothing*/
				{
					;
				}
				/*debug(RPT_DEBUG, "screenlist_current: ... sent ignore");*/
			}
		}
		if (s) {
			/*debug(RPT_DEBUG, "screenlist_current: listening to new screen");*/
			c = s->client;
			if (c)					  /* Tell the client we're paying attention...*/
			{
				snprintf (str, sizeof(str), "listen %s\n", s->id);
				sock_send_string (c->sock, str);
			} else					  /* The server has the display, so do nothing*/
			{
				;
			}
		}
	}

	old_s = s;

	/*debug(RPT_DEBUG, "screenlist_current: return %8x", s);*/

	return s;
}

int
screenlist_add (Screen * s)
{
	/* TODO:  Different queueing modes...*/
	return screenlist_add_end (s);
}

Screen *
screenlist_next ()
{
	Screen *s;

	/*debug(RPT_DEBUG, "Screenlist_next()");*/

	s = screenlist_current ();

	/* If we're on hold, don't advance!*/
	if (screenlist_action == SCR_HOLD)
		return s;
	if (screenlist_action == RENDER_HOLD)
		return s;

	/* Otherwise, reset it to regular operation*/
	screenlist_action = 0;

	/*debug(RPT_DEBUG, "Screenlist_next: calling handler...");*/

	/* Call the selected queuing function...*/
	/* TODO:  Different queueing modes...*/
	s = screenlist_next_priority ();
	/*s = screenlist_next_roll();*/

	/*debug(RPT_DEBUG, "Screenlist_next() done");*/

	return s;
}

Screen *
screenlist_prev ()
{
	Screen *s;

	s = screenlist_current ();

	/* If we're on hold, don't advance!*/
	if (screenlist_action == SCR_HOLD)
		return s;
	if (screenlist_action == RENDER_HOLD)
		return s;

	/* Otherwise, reset it no regular operation*/
	screenlist_action = 0;

	/* Call the selected queuing function...*/
	/* TODO:  Different queueing modes...*/
	s = screenlist_prev_roll ();

	return s;
}

/* Adds new screens to the end of the screenlist...*/
int
screenlist_add_end (Screen * screen)
{
	debug (RPT_DEBUG, "screenlist_add_end()");

	return LL_Push (screenlist, (void *) screen);
}

/* Simple round-robin approach to screen cycling...*/
Screen *
screenlist_next_roll ()
{
	/*debug(RPT_DEBUG, "screenlist_next_roll()");*/

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	return screenlist_current ();
}

/* Strict priority queue approach...*/
Screen *
screenlist_next_priority ()
{
	/*screen *s, *t;*/
	/*debug(RPT_DEBUG, "screenlist_next_priority");*/

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	LL_Sort (screenlist, compare_priority);

	return screenlist_current ();
}

/* Simple round-robin approach to screen cycling...*/
Screen *
screenlist_prev_roll ()
{
	/*debug(RPT_DEBUG, "screenlist_prev_roll()");*/

	if (LL_Roll (screenlist) != 0)
		return NULL;

	return screenlist_current ();
}

int
compare_priority (void *one, void *two)
{
	Screen *a, *b;

	/*debug(RPT_DEBUG, "compare_priority: %8x %8x", one, two);*/

	if (!one)
		return 0;
	if (!two)
		return 0;

	a = (Screen *) one;
	b = (Screen *) two;

	/*debug(RPT_DEBUG, "compare_priority: done?");*/

	return (a->priority - b->priority);
}

int
compare_addresses (void *one, void *two)
{
	/*debug(RPT_DEBUG, "compare_addresses: %p == %p ???", one, two);*/
	return (one != two);
}
