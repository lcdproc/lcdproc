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
#include "widget.h"
#include "screenlist.h"
#include "screen.h"
#include "menuscreens.h"
#include "main.h"
#include "render.h"

int  default_duration = 0;
int  default_timeout  = -1;

Screen *
screen_create (char * id, Client * client)
{
	Screen *s;

	debug( RPT_DEBUG, "%s( id=\"%.40s\", client=[%d] )", __FUNCTION__, id, (client?client->sock:-1));

	s = malloc (sizeof (Screen));
	if (!s) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}
	if (!id) {
		report (RPT_ERR, "%s: Need id string", __FUNCTION__);
		return NULL;
	}
	/* Client can be NULL for serverscreens and other client-less screens */

	s->id = strdup(id);
	if (!s->id) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}

	s->name = NULL;
	s->priority = DEFAULT_SCREEN_PRIORITY;
	s->duration = default_duration;
	s->backlight = BACKLIGHT_OPEN;
	s->heartbeat = HEARTBEAT_OPEN;
	s->width = display_props->width;
	s->height = display_props->height;
	s->keys = NULL;
	s->client = client;
	s->widgetlist = NULL;
	s->timeout = default_timeout; 	/*ignored unless greater than 0.*/
	s->backlight = BACKLIGHT_OPEN;		/*Lets the screen do it's own*/
						/*or do what the client says.*/
	s->cursor = CURSOR_OFF;
	s->cursor_x = 1;
	s->cursor_y = 1;

	s->widgetlist = LL_new ();
	if (!s->widgetlist) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}

	menuscreen_add_screen (s);

	return s;
}

int
screen_destroy (Screen * s)
{
	Widget *w;

	debug( RPT_DEBUG, "%s( s=[%.40s] )", __FUNCTION__, s->id );

	menuscreen_remove_screen (s);

	screenlist_remove (s);

	for (w=LL_GetFirst(s->widgetlist); w; w=LL_GetNext(s->widgetlist)) {
		/* Free a widget...*/
		widget_destroy (w);
	}
	LL_Destroy (s->widgetlist);

	if (s->id)
		free (s->id);
	if (s->name)
		free (s->name);

	free (s);

	return 0;
}

int
screen_add_widget (Screen * s, Widget * w)
{
	debug( RPT_DEBUG, "%s( s=[%.40s], widget=[%.40s] )", __FUNCTION__, s->id, w->id );

	LL_Push (s->widgetlist, (void *) w);

	return 0;
}

int
screen_remove_widget (Screen * s, Widget * w)
{
	debug( RPT_DEBUG, "%s( s=[%.40s], widget=[%.40s] )", __FUNCTION__, s->id, w->id );

	LL_Remove (s->widgetlist, (void *) w);

	return 0;
}

Widget *
screen_find_widget (Screen * s, char *id)
{
	Widget * w;

	if (!s)
		return NULL;
	if (!id)
		return NULL;

	debug( RPT_DEBUG, "%s( s=[%.40s], id=\"%.40s\" )", __FUNCTION__, s->id, id );

	for ( w=LL_GetFirst(s->widgetlist); w; w=LL_GetNext(s->widgetlist) ) {
		if (0 == strcmp (w->id, id)) {
			debug (RPT_DEBUG, "%s: Found %s", __FUNCTION__, id);
			return w;
		}
		/* Search subscreens recursively */
		if (w->type == WID_FRAME) {
			w = widget_search_subs (w, id);
			if (w)
				return w;
		}
	}
	debug (RPT_DEBUG, "%s: Not found", __FUNCTION__);
	return NULL;
}
