/** \file server/screen.c
 * This file stores all the screen definition-handling code. Functions here
 * provide means to create new screens and destroy existing ones. Screens are
 * identified by client and by the client's own identifiers for screens.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2003, Joris Robijn
 *               2008, Peter Marschall
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/report.h"

#include "drivers.h"

#include "clients.h"
#include "widget.h"
#include "screenlist.h"
#include "menuscreens.h"
#include "main.h"
#include "render.h"

int  default_duration = 0;
int  default_timeout  = -1;

char *pri_names[] = {
	"hidden",
	"background",
	"info",
	"foreground",
	"alert",
	"input",
	NULL,
};


/** Create a screen.
 * \param id      Screen id; it's name.
 * \param client  Client, the screen belongs to.
 * \return        Pointer to freshly created screen.
 */
Screen *
screen_create(char *id, Client *client)
{
	Screen *s;

	debug(RPT_DEBUG, "%s(id=\"%.40s\", client=[%d])",
		 __FUNCTION__, id, (client?client->sock:-1));

	if (!id) {
		report(RPT_ERR, "%s: Need id string", __FUNCTION__);
		return NULL;
	}
	/* Client can be NULL for serverscreens and other client-less screens */

	s = malloc(sizeof(Screen));
	if (s == NULL) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}

	s->id = strdup(id);
	if (s->id == NULL) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		free(s);
		return NULL;
	}

	s->name = NULL;
	s->priority = PRI_INFO;
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

	s->widgetlist = LL_new();
	if (s->widgetlist == NULL) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		free(s->id);
		free(s);
		return NULL;
	}

	menuscreen_add_screen(s);

	return s;
}


/** Destroy a screen.
 * \param s    Screen to destroy.
 * \retval <0  Error; no screen given.
 * \retval  0  Success.
 */
int
screen_destroy(Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(s=[%.40s])", __FUNCTION__, s->id);

	menuscreen_remove_screen(s);

	screenlist_remove(s);

	for (w = LL_GetFirst(s->widgetlist); w; w = LL_GetNext(s->widgetlist)) {
		/* Free a widget...*/
		widget_destroy(w);
	}
	LL_Destroy(s->widgetlist);
	s->widgetlist = NULL;

	if (s->id != NULL) {
		free(s->id);
		s->id = NULL;
	}
	if (s->name != NULL) {
		free(s->name);
		s->name = NULL;
	}

	free(s);
	s = NULL;

	return 0;
}


/** Add a widget to a screen.
 * \param s  Screen to add the widget \c w to.
 * \param w  Widget to be added to \c s.
 * \retval <0  Error.
 * \retval  0  Success.
 */
int
screen_add_widget(Screen *s, Widget *w)
{
	debug(RPT_DEBUG, "%s(s=[%.40s], widget=[%.40s])", __FUNCTION__, s->id, w->id);

	LL_Push(s->widgetlist, (void *) w);

	return 0;
}


/** Remove a widget from a screen.
 * \param s  Screen to remove the widget \c w from.
 * \param w  Widget to be removed from \c s.
 * \retval <0  Error.
 * \retval  0  Success.
 */
int
screen_remove_widget(Screen *s, Widget *w)
{
	debug(RPT_DEBUG, "%s(s=[%.40s], widget=[%.40s])", __FUNCTION__, s->id, w->id);

	LL_Remove(s->widgetlist, (void *) w, NEXT);

	return 0;
}


/** Find a widget on a screen by its id.
 * \param s   Screen where to look for the widget.
 * \param id  Identifier of the widget.
 * \return    Pointerr to the widget; \c NULL if widget was not found or error.
 */
Widget *
screen_find_widget(Screen *s, char *id)
{
	Widget *w;

	if (!s)
		return NULL;
	if (!id)
		return NULL;

	debug(RPT_DEBUG, "%s(s=[%.40s], id=\"%.40s\")", __FUNCTION__, s->id, id);

	for (w = LL_GetFirst(s->widgetlist); w != NULL; w = LL_GetNext(s->widgetlist)) {
		if (0 == strcmp(w->id, id)) {
			debug(RPT_DEBUG, "%s: Found %s", __FUNCTION__, id);
			return w;
		}
		/* Search subscreens recursively */
		if (w->type == WID_FRAME) {
			w = widget_search_subs(w, id);
			if (w != NULL)
				return w;
		}
	}
	debug(RPT_DEBUG, "%s: Not found", __FUNCTION__);
	return NULL;
}


/** Convert a priority name to the priority id.
 * \param priname  Name of the screen priority.
 * \return  Priority id associated with \c priname, -1 if no matching priority
 *          id could be found.
 */
Priority
screen_pri_name_to_pri(char *priname)
{
	Priority pri = -1;
	int i;

	for (i = 0; pri_names[i]; i++) {
		if (strcmp(pri_names[i], priname) == 0) {
			pri = i;
			break; /* it's valid: skip out...*/
		}
	}
	return pri;
}


/** Convert a priority id to the associated name.
 * \param pri  Priority id.
 * \return     Priority name associated with \c pri.
 */
char *
screen_pri_to_pri_name(Priority pri)
{
	return pri_names[pri];
}
