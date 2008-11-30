/** \file server/widget.c
 * Does all actions on widgets
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2002, Joris Robijn
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "screen.h"
#include "widget.h"
#include "render.h"
#include "drivers/lcd.h"

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

struct icontable {
	int icon;
	char *iconname;
} icontable[] = {
	{ICON_BLOCK_FILLED, "BLOCK_FILLED"},
	{ICON_HEART_OPEN, "HEART_OPEN"},
	{ICON_HEART_FILLED, "HEART_FILLED"},
	{ICON_ARROW_UP, "ARROW_UP"},
	{ICON_ARROW_DOWN, "ARROW_DOWN"},
	{ICON_ARROW_LEFT, "ARROW_LEFT"},
	{ICON_ARROW_RIGHT, "ARROW_RIGHT"},
	{ICON_CHECKBOX_OFF, "CHECKBOX_OFF"},
	{ICON_CHECKBOX_ON, "CHECKBOX_ON"},
	{ICON_CHECKBOX_GRAY, "CHECKBOX_GRAY"},
	{ICON_SELECTOR_AT_LEFT, "SELECTOR_AT_LEFT"},
	{ICON_SELECTOR_AT_RIGHT, "SELECTOR_AT_RIGHT"},
	{ICON_ELLIPSIS, "ELLIPSIS"},
	{ICON_STOP, "STOP"},
	{ICON_PAUSE, "PAUSE"},
	{ICON_PLAY, "PLAY"},
	{ICON_PLAYR, "PLAYR"},
	{ICON_FF, "FF"},
	{ICON_FR, "FR"},
	{ICON_NEXT, "NEXT"},
	{ICON_PREV, "PREV"},
	{ICON_REC, "REC"},
	{0,NULL}
};


Widget *
widget_create(char *id, WidgetType type, Screen *screen)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(id=\"%s\", type=%d, screen=[%s])", __FUNCTION__, id, type, screen->id);

	/* Create it */
	w = malloc(sizeof(Widget));
	if (!w) {
		report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}

	w->id = strdup(id);
	if (!w->id) {
		report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
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
		char *frame_name;
		frame_name = malloc(strlen("frame_") + strlen(id) + 1);
		strcpy(frame_name, "frame_");
		strcat(frame_name, id);

		w->frame_screen = screen_create(frame_name, screen->client);

		free(frame_name); /* not needed anymore */
	}
	return w;
}

int
widget_destroy(Widget *w)
{
	debug(RPT_DEBUG, "%s(w=[%s])", __FUNCTION__, w->id);

	if (!w)
		return -1;

	if (w->id)
		free(w->id);
	if (w->text)
		free(w->text);

	/* Free subscreen of frame widget too */
	if (w->type == WID_FRAME) {
		screen_destroy(w->frame_screen);
	}

	free(w);

	return 0;
}

WidgetType
widget_typename_to_type(char *typename)
{
	WidgetType wid_type = WID_NONE;
	int i;

	for (i = 0; typenames[i]; i++) {
		if (strcmp(typenames[i], typename) == 0) {
			wid_type = i;
			break; /* it's valid: skip out...*/
		}
	}
	return wid_type;
}

char *
widget_type_to_typename(WidgetType t)
{
	return typenames[t];
}

Widget *
widget_search_subs(Widget *w, char *id)
{
	if (w->type == WID_FRAME) {
		return screen_find_widget(w->frame_screen, id);
	} else {
		return NULL; /* no kids */
	}
}

char *widget_icon_to_iconname(int icon)
{
	int i;

	for (i = 0; icontable[i].iconname; i++) {
		if (icontable[i].icon == icon) {
			return icontable[i].iconname;
		}
	}

	return NULL;
}

int widget_iconname_to_icon(char *iconname)
{
	int i;

	for (i = 0; icontable[i].iconname; i++) {
		if (strcasecmp(icontable[i].iconname, iconname) == 0) {
			return icontable[i].icon;
		}
	}

	return -1;
}

