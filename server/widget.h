/** \file server/widget.h
 * Public interface to the widget methods.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 */

#ifndef WIDGET_H
#define WIDGET_H

#define INC_TYPES_ONLY 1
#include "screen.h"
#undef INC_TYPES_ONLY

/* These correspond to the index into the "types" array...*/
typedef enum WidgetType {
	WID_NONE = 0,
	WID_STRING,
	WID_HBAR,
	WID_VBAR,
	WID_ICON,
	WID_TITLE,
	WID_SCROLLER,
	WID_FRAME,
	WID_NUM
} WidgetType;


/** Widget structure */
typedef struct Widget {
	char *id;			/**< the widget's name */
	WidgetType type;		/**< the widget's type */
	Screen *screen;			/**< What screen is this widget in ? */
	int x, y;			/**< Position */
	int width, height;		/**< Visible size */
	int left, top, right, bottom;	/**< bounding rectangle */
	int length;			/**< size or direction */
	int speed;			/**< For scroller... */
	char *text;			/**< text or binary data */
	struct Screen *frame_screen;	/**< frame widget get an associated screen */
	//LinkedList *kids;		/* Frames can contain more widgets...*/
} Widget;

#define WID_MAX_DIR 4

/* Create new widget */
Widget *widget_create(char *id, WidgetType type, Screen *screen);

/* Destroy a widget */
int widget_destroy(Widget *w);

/* Convert a widget typename to a widget type */
WidgetType widget_typename_to_type(char *typename);

/* Convert a widget typename to a widget type */
char *widget_type_to_typename(WidgetType t);

/* Search subwidgets of a widget */
Widget *widget_search_subs(Widget *w, char *id);

/* Convert icon number to icon name */
char *widget_icon_to_iconname(int icon);

/* Convert iconname to icon number */
int widget_iconname_to_icon(char *iconname);

#endif
