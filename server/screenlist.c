/** \file server/screenlist.c
 * All actions that can be performed on the list of screens.
 * This file also manages the rotation of screens.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2003, Joris Robijn
 */

#include <stdlib.h>
#include <stdio.h>

#include "shared/LL.h"
#include "shared/sockets.h"
#include "shared/report.h"

#include "client.h"
#include "screen.h"
#include "screenlist.h"

#include "main.h" /* for timer */

/* Local functions */
int compare_priority(void *one, void *two);

bool autorotate = UNSET_INT;	/* If on, INFO and FOREGROUND screens will rotate */
LinkedList *screenlist = NULL;
Screen *current_screen = NULL;
long int current_screen_start_time = 0;


int
screenlist_init(void)
{
	report(RPT_DEBUG, "%s()", __FUNCTION__);

	screenlist = LL_new();
	if (!screenlist) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return -1;
	}
	return 0;
}


int
screenlist_shutdown(void)
{
	report(RPT_DEBUG, "%s()", __FUNCTION__);

	if (!screenlist) {
		/* Program shutdown before completed startup */
		return -1;
	}
	LL_Destroy(screenlist);

	return 0;
}


int
screenlist_add(Screen *s)
{
	if (!screenlist)
		return -1;
	return LL_Push(screenlist, s);
}


int
screenlist_remove(Screen *s)
{
	debug(RPT_DEBUG, "%s(s=[%.40s])", __FUNCTION__, s->id);

	if (!screenlist)
		return -1;

	/* Are we trying to remove the current screen ? */
	if (s == current_screen) {
		screenlist_goto_next();
		if (s == current_screen) {
			/* Hmm, no other screen had same priority */
			void *res = LL_Remove(screenlist, s, NEXT);
			/* And now once more */
			screenlist_goto_next();
			return (res == NULL) ? -1 : 0;
		}
	}
	return (LL_Remove(screenlist, s, NEXT) == NULL) ? -1 : 0;
}


void
screenlist_process(void)
{
	Screen *s;
	Screen *f;

	report(RPT_DEBUG, "%s()", __FUNCTION__);

	if (!screenlist)
		return;
	/* Sort the list according to priority class */
	LL_Sort(screenlist, compare_priority);
	f = LL_GetFirst(screenlist);

	/**** First we need to check out the current situation. ****/

	/* Check whether there is an active screen */
	s = screenlist_current();
	if (!s) {
		/* We have no active screen yet.
		 * Try to switch to the first screen in the list... */

		s = f;
		if (!s) {
			/* There was no screen in the list */
			return;
		}
		screenlist_switch(s);
		return;
	}
	else {
		/* There already was an active screen.
		 * Check to see if it has an expiry time. If so, decrease it
		 * and then check to see if it has expired. Remove the screen
		 * if expired. */
		if (s->timeout != -1) {
			--(s->timeout);
			report(RPT_DEBUG, "Active screen [%.40s] has timeout->%d", s->id, s->timeout);
			if (s->timeout <= 0) {
				/* Expired, we can destroy it */
				report(RPT_DEBUG, "Removing expired screen [%.40s]", s->id);
				client_remove_screen(s->client, s);
				screen_destroy(s);
			}
		}
	}

	/**** OK, current situation examined. We can now see if we need to switch. */

	/* Is there a screen of a higher priority class than the
	 * current one ? */
	if (f->priority > s->priority) {
		/* Yes, switch to that screen, job done */
		report(RPT_DEBUG, "%s: High priority screen [%.40s] selected", __FUNCTION__, f->id);
		screenlist_switch(f);
		return;
	}

	/* Current screen has been visible long enough and is it of 'normal'
	 * priority ?
	 */
	if (autorotate && (timer - current_screen_start_time >= s->duration)
	&& s->priority > PRI_BACKGROUND && s->priority <= PRI_FOREGROUND) {
		/* Ah, rotate! */
		screenlist_goto_next();
	}
}


void
screenlist_switch(Screen *s)
{
	Client *c;
	char str[256];

	if (!s) return;

	report(RPT_DEBUG, "%s(s=[%.40s])", __FUNCTION__, s->id);

	if (s == current_screen) {
		/* Nothing to be done */
		return;
	}

	if (current_screen) {
		c = current_screen->client;
		if (c) {
			/* Tell the client we're not listening any more...*/
			snprintf(str, sizeof(str), "ignore %s\n", current_screen->id);
			sock_send_string(c->sock, str);
		} else {
			/* It's a server screen, no need to inform it. */
		}
	}
	c = s->client;
	if (c) {
		/* Tell the client we're paying attention...*/
		snprintf(str, sizeof(str), "listen %s\n", s->id);
		sock_send_string(c->sock, str);
	} else {
		/* It's a server screen, no need to inform it. */
	}
	report(RPT_INFO, "%s: switched to screen [%.40s]", __FUNCTION__, s->id);
	current_screen = s;
	current_screen_start_time = timer;
}


Screen *
screenlist_current(void)
{
	return current_screen;
}


int
screenlist_goto_next(void)
{
	Screen *s;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	if (!current_screen)
		return -1;

	/* Find current screen in screenlist */
	for (s = LL_GetFirst(screenlist); s && s != current_screen; s = LL_GetNext(screenlist))
		;

	/* One step forward */
	s = LL_GetNext(screenlist);
	if (!s || s->priority < current_screen->priority) {
		/* To far, go back to start of screenlist */
		s = LL_GetFirst(screenlist);
	}
	screenlist_switch(s);
	return 0;
}


int
screenlist_goto_prev(void)
{
	Screen *s;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	if (!current_screen)
		return -1;

	/* Find current screen in screenlist */
	for (s = LL_GetFirst(screenlist); s && s != current_screen; s = LL_GetNext(screenlist));

	/* One step back */
	s = LL_GetPrev(screenlist);
	if (!s) {
		/* We're at the start of the screenlist. We should find the
		 * last screen with the same priority as the first screen.
		 */
		Screen *f = LL_GetFirst(screenlist);
		Screen *n;

		s = f;
		while ((n = LL_GetNext(screenlist)) && n->priority == f->priority) {
			s = n;
		}
	}
	screenlist_switch(s);
	return 0;
}

/* Internal function for sorting. */
int
compare_priority(void *one, void *two)
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

	return (b->priority - a->priority);
}
