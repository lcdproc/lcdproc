/*
 * screen.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Does screen management
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/report.h"

#include "drivers.h"

#include "clients.h"
#include "client_data.h"
#include "widget.h"
#include "screenlist.h"
#include "screen.h"
#include "main.h"
#include "render.h"

int  default_duration = 0;
int  default_timeout  = -1;

screen *
screen_create ()
{
	screen *s;

	s = malloc (sizeof (screen));
	if (!s) {
		report(RPT_ERR, "screen_create: Error allocating new screen");
		return NULL;
	}

	s->id = NULL;
	s->name = NULL;
	s->priority = DEFAULT_SCREEN_PRIORITY;
	s->duration = default_duration;
	s->heartbeat = DEFAULT_HEARTBEAT;
	s->wid = display_props->width;
	s->hgt = display_props->height;
	s->keys = NULL;
	s->parent = NULL;
	s->widgets = NULL;
	s->timeout = default_timeout; /*ignored unless greater than 0.*/
	s->backlight_state = BACKLIGHT_NOTSET;	/*Lets the screen do it's own*/
						/*or do what the client says.*/

	s->widgets = LL_new ();
	if (!s->widgets) {
		report(RPT_ERR, "screen_create:  Error allocating widget list");
		return NULL;
	}

	return s;
}

int
screen_destroy (screen * s)
{
	widget *w;

	if (!s)
		return -1;

	LL_Rewind (s->widgets);
	do {
		/* Free a widget...*/
		w = LL_Get (s->widgets);
		widget_destroy (w);
	} while (LL_Next (s->widgets) == 0);
	LL_Destroy (s->widgets);

	if (s->id)
		free (s->id);
	if (s->name)
		free (s->name);
	if (s->keys)
		free (s->keys);

	free (s);

	return 0;
}

screen *
screen_find (client * c, char *id)
{
	screen *s;

	if (!c)
		return NULL;
	if (!id)
		return NULL;

	debug (RPT_INFO, "client_find_screen(%s)", id);

	LL_Rewind (c->data->screenlist);
	do {
		s = LL_Get (c->data->screenlist);
		if ((s) && (0 == strcmp (s->id, id))) {
			debug (RPT_DEBUG, "client_find_screen:  Found %s", id);
			return s;
		}
	} while (LL_Next (c->data->screenlist) == 0);

	return NULL;
}

int
screen_add (client * c, char *id)
{
	screen *s;

	if (!c)
		return -1;
	if (!id)
		return -1;

	/* Make sure this screen doesn't already exist...*/
	s = screen_find (c, id);
	if (s) {
		return 1;
	}

	s = screen_create ();
	if (!s) {
		report (RPT_ERR, "screen_add:  Error creating screen");
		return -1;
	}

	s->parent = c;

	s->id = strdup (id);
	if (!s->id) {
		report (RPT_ERR, "screen_add:  Error allocating name");
		return -1;
	}
	/* TODO:  Check for errors here?*/
	LL_Push (c->data->screenlist, (void *) s);

	/* Now, add it to the screenlist...*/
	if (screenlist_add (s) < 0) {
		report (RPT_ERR, "screen_add:  Error queueing new screen");
		return -1;
	}

	return 0;
}

int
screen_remove (client * c, char *id)
{
	screen *s;

	if (!c)
		return -1;
	if (!id)
		return -1;

	/* Make sure this screen *does* exist...*/
	s = screen_find (c, id);
	if (!s) {
		report (RPT_ERR, "screen_remove:  Error finding screen %s", id);
		return 1;
	}
	/* TODO:  Check for errors here?*/
	LL_Remove (c->data->screenlist, (void *) s);

	/* Now, remove it from the screenlist...*/
	if (screenlist_remove_all (s) < 0) {
		/* Not a serious error..*/
		report (RPT_ERR, "screen_remove:  Error dequeueing screen");
		return 0;
	}

	/* TODO:  Check for errors here too?*/
	screen_destroy (s);

	return 0;
}
