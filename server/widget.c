/*
 * widget.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Does widget management
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "screen.h"
#include "widget.h"
#include "render.h"

char *typenames[] = {
	"none",
	"string",
	"hbar",
	"vbar",
	"icon",
	"title",
	"scroller",
	"frame",
	"num",
	NULL,
};

//static widget *widget_finder (LinkedList * list, char *id);
//static int widget_remover (LinkedList * list, widget * w);

Widget *
widget_create (char *id, WidgetType type, Screen * screen)
{
	Widget * w;

	report (RPT_INFO, "widget_create(%s,%d)", id, type);

	/* Create it */
	w = malloc (sizeof (Widget));
	if (!w) {
		report (RPT_DEBUG, "widget_create: Error allocating");
		return NULL;
	}

	w->id = strdup(id);
	if (!w->id) {
		report (RPT_DEBUG, "widget_create: Error allocating");
		return NULL;
	}

	w->type = type;
	w->screen = screen;
	w->x = 1;
	w->y = 1;
	w->width = 0;
	w->height = 0;
	w->left = 1;
	w->top = 1;
	w->right = 0;
	w->bottom = 0;
	w->length = 1;
	w->speed = 1;
	w->text = NULL;
	//w->kids = NULL;

	if (w->type == WID_FRAME) {
		/* create a screen for the frame widget */
		char * frame_name;
		frame_name = malloc (strlen("frame_") + strlen(id) + 1);
		strcpy (frame_name, "frame_");
		strcat (frame_name, id);

		w->frame_screen = screen_create (frame_name, screen->client);

		free (frame_name); /* not needed anymore */
	}
	return w;
}

int
widget_destroy (Widget * w)
{
	debug (RPT_INFO, "widget_destroy(%s)", w->id);

	if (!w)
		return -1;

	if (w->id)
		free (w->id);
	/*debug(RPT_DEBUG, "widget_destroy: id...");*/
	if (w->text)
		free (w->text);
	/*debug(RPT_DEBUG, "widget_destroy: text...");*/

	if (w->type == WID_FRAME) {
		/* TODO: create a screen for the frame widget */
	}

	/* Free subscreen of frame widget too */
	if (w->type == WID_FRAME) {
		screen_destroy (w->frame_screen);
	}

	free (w);
	/*debug(RPT_DEBUG, "widget_destroy: widget...");*/

	return 0;
}

WidgetType
widget_typename_to_type (char * typename)
{
	WidgetType wid_type = WID_NONE;
	int i;

	for (i = 0; typenames[i]; i++) {
		if (strcmp (typenames[i], typename) == 0) {
			wid_type = i;
			break; /* it's valid: skip out...*/
		}
	}
	return wid_type;
}

char *
widget_type_to_typename (WidgetType t)
{
	return typenames[t];
}

Widget *
widget_search_subs (Widget * w, char * id)
{
	if (w->type == WID_FRAME) {
		return screen_find_widget (w->frame_screen, id);
	} else {
		return NULL; /* no kids */
	}
}
