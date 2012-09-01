/** \file server/widget.c
 * This file houses code that handles the creation and destruction of widget
 * objects for the server. These functions are called from the command parser
 * storing the specified widget in a generic container that is parsed later
 * by the screen renderer.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2002, Joris Robijn
 *		 2008, Peter Marschall
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "screen.h"
#include "widget.h"
#include "render.h"
#include "drivers/lcd.h"

char *typenames[] = {
	"none",		/* WID_NONE */
	"string",	/* WID_STRING */
	"hbar",		/* WID_HBAR */
	"vbar",		/* WID_VBAR */
	"icon",		/* WID_ICON */
	"title",	/* WID_TITLE */
	"scroller",	/* WID_SCROLLER */
	"frame",	/* WID_FRAME */
	"num",		/* WID_NUM */
	NULL,		/* WID_NONE */
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


/** Create a widget.
  * \param id       Widget identifier; it's name.
  * \param type     Widget type.
  * \param screen   Screen on which the widget is to be placed.
  * \return         Pointer to the freshly created widget.
  */
Widget *
widget_create(char *id, WidgetType type, Screen *screen)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(id=\"%s\", type=%d, screen=[%s])", __FUNCTION__, id, type, screen->id);

	/* Create it */
	w = malloc(sizeof(Widget));
	if (w == NULL) {
		report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}

	w->id = strdup(id);
	if (w->id == NULL) {
		report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
		free(w);
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
		char *frame_name = malloc(strlen("frame_") + strlen(id) + 1);

		if (frame_name == NULL) {
			report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
			free(w->id);
			free(w);
			return NULL;
		}
		strcpy(frame_name, "frame_");
		strcat(frame_name, id);

		w->frame_screen = screen_create(frame_name, screen->client);
		if (w->frame_screen == NULL) {
			report(RPT_DEBUG, "%s: Error allocating", __FUNCTION__);
			free(w->id);
			free(w);
			/* return NULL after cleaning up */
			w = NULL;
		}

		free(frame_name); /* not needed anymore */
	}
	return w;
}


/** Destroy a widget.
 * \param w    Widget to destroy.
 * \retval <0  Error; no widget given.
 * \retval  0  Success.
 */
int
widget_destroy(Widget *w)
{
	debug(RPT_DEBUG, "%s(w=[%s])", __FUNCTION__, w->id);

	if (!w)
		return -1;

	if (w->id != NULL) {
		free(w->id);
		w->id = NULL;
	}
	if (w->text != NULL) {
		free(w->text);
		w->text = NULL;
	}

	/* Free subscreen of frame widget too */
	if (w->type == WID_FRAME) {
		screen_destroy(w->frame_screen);
		w->frame_screen = NULL;
	}

	free(w);
	w = NULL;

	return 0;
}


/** Convert a widget type name to a widget type.
 * \param typename  Name of the widget type.
 * \return          Widget type.
 */
WidgetType
widget_typename_to_type(char *typename)
{
	WidgetType wid_type = WID_NONE;
	int i;

	for (i = 0; typenames[i] != NULL; i++) {
		if (strcmp(typenames[i], typename) == 0) {
			wid_type = i;
			break; /* it's valid: skip out...*/
		}
	}
	return wid_type;
}


/** Convert a widget type to the associated type name.
 * \param t  Widget type.
 * \return   Widget type's name.
 */
char *
widget_type_to_typename(WidgetType t)
{
	return typenames[t];
}


/** Find subordinate widgets of a widget by name.
 * \param w   Widget.
 * \param id  Name of the subiordinate widget.
 * \return    Pointer to the sub-widget; \c NULL if not found or on error.
 */
Widget *
widget_search_subs(Widget *w, char *id)
{
	if (w->type == WID_FRAME) {
		return screen_find_widget(w->frame_screen, id);
	} else {
		return NULL; /* no kids */
	}
}


/** Find a widget icon by type.
 * \param icon    Icon type.
 * \return        Pointer to constant string containing the icon name.
 */
char *widget_icon_to_iconname(int icon)
{
	int i;

	for (i = 0; icontable[i].iconname != NULL; i++) {
		if (icontable[i].icon == icon) {
			return icontable[i].iconname;
		}
	}

	return NULL;
}


/** Find a widget icon by name.
 * \param iconname  Icon name.
 * \return          Icon type
 */
int widget_iconname_to_icon(char *iconname)
{
	int i;

	for (i = 0; icontable[i].iconname != NULL; i++) {
		if (strcasecmp(icontable[i].iconname, iconname) == 0) {
			return icontable[i].icon;
		}
	}

	return -1;
}

