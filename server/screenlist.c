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
#include "render.h"
#include "screen.h"
#include "serverscreens.h"
#include "clients.h"

#include "main.h" /* for timer */

int screenlist_action = 0;
long int current_screen_start_time;

LinkedList *screenlist = NULL;
Screen * current_screen = NULL;

int screenlist_add_end (Screen * screen);
Screen *screenlist_next_roll ();
Screen *screenlist_prev_roll ();
Screen *screenlist_next_priority ();

int compare_priority (void *one, void *two);
int compare_addresses (void *one, void *two);

int
screenlist_init ()
{
	report (RPT_DEBUG, "%s()", __FUNCTION__);

	screenlist = LL_new ();
	if (!screenlist) {
		report(RPT_ERR, "%s: error allocating list", __FUNCTION__);
		return -1;
	}
	screenlist_action = 0;
	return 0;
}

int
screenlist_shutdown ()
{
	report (RPT_DEBUG, "%s()", __FUNCTION__);

	if( !screenlist ) {
		/* Program shutdown before completed startup */
		return -1;
	}
	LL_Destroy (screenlist);

	return 0;
}

int
screenlist_remove (Screen * s)
{
	debug (RPT_DEBUG, "%s( s=[%.40s] )", __FUNCTION__, s->id);

	if (!LL_Remove (screenlist, s))
		return -1;
	else
		return 0;
}

int
screenlist_remove_all (Screen * s)
{
	int i = 0;

	debug (RPT_DEBUG, "%s( s=[%.40s] )", __FUNCTION__, s->id);

	while (LL_Remove (screenlist, s))
		i++;

	debug (RPT_DEBUG, "screenlist_remove_all()... got %i", i);

	return i;
}

void
screenlist_update ()
/* Decide if we need to switch to an other screen.
 */
{
	Screen * s;

	report (RPT_DEBUG, "%s()", __FUNCTION__);

	s = screenlist_current ();
	if (!s) {
		/* Try to switch to the first screen in the list */

		s = LL_GetFirst(screenlist);
		if (!s) {
			/* There was no screen in the list */
			return;
		}
		screenlist_switch (s);
	}
	if (timer - current_screen_start_time >= s->duration) {
		screenlist_switch( screenlist_next ());
	}

	/* Check to see if the screen has a timeout value, if it does
	 * decrese it and then check to see if it has excpired.
	 * Remove if expired.
	 */
	if (s && s->timeout != -1) {
		--(s->timeout);
		report(RPT_DEBUG, "Timeout matches check, screen %s has timeout->%d", s->name, s->timeout);
		if (s->timeout <= 0) {
			client_remove_screen (s->client, s);
			screen_destroy (s);
			report(RPT_DEBUG, "Removing screen %s which has timeout->%d", s->name, s->timeout);
		}
	}
	for (s=LL_GetFirst(screenlist); s; s=LL_GetNext(screenlist)) {
		report( RPT_DEBUG, "%s: [%s] %d %d", __FUNCTION__, s->id, s->priority, s->duration );
	}
}

void
screenlist_switch (Screen * s)
/* Switch to an other screen */
{
	Client * c;
	char str[256];

	if (!s) return;

	report (RPT_DEBUG, "%s( s=[%.40s] )", __FUNCTION__, s->id );

	if (s == current_screen) {
		/* Nothing to be done */
		return;
	}

	if (current_screen) {
		c = current_screen->client;
		if (c) {
			/* Tell the client we're not listening any more...*/
			snprintf (str, sizeof(str), "ignore %s\n", current_screen->id);
			sock_send_string (c->sock, str);
		} else {
			/* It's a server screen, no need to inform it. */
		}
	}
	c = s->client;
	if (c) {
		/* Tell the client we're paying attention...*/
		snprintf (str, sizeof(str), "listen %s\n", s->id);
		sock_send_string (c->sock, str);
	} else {
		/* It's a server screen, no need to inform it. */
	}
	report (RPT_INFO, "%s: switched to screen [%.40s]", __FUNCTION__, s->id );
	current_screen = s;
	current_screen_start_time = timer;
}

Screen *
screenlist_current ()
{
	return current_screen;
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

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

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

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

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
	debug (RPT_DEBUG, "%s( screen=[%.40s] )", __FUNCTION__, screen->id);

	return LL_Push (screenlist, (void *) screen);
}

/* Simple round-robin approach to screen cycling...*/
Screen *
screenlist_next_roll ()
{
	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	return screenlist_current ();
}

/* Strict priority queue approach...*/
Screen *
screenlist_next_priority ()
{
	/*screen *s, *t;*/
	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	if (LL_UnRoll (screenlist) != 0)
		return NULL;

	LL_Sort (screenlist, compare_priority);

	return LL_Get (screenlist);
}

/* Simple round-robin approach to screen cycling...*/
Screen *
screenlist_prev_roll ()
{
	debug (RPT_DEBUG, "%s()", __FUNCTION__);

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
