#ifndef WIDGET_H
#define WIDGET_H

#include "screen.h"

typedef struct widget {
	char *id;
	int type;
	// some sort of data here...
	int x, y;						  // Position
	int wid, hgt;					  // Size
	int left, top, right, bottom;	// bounding rectangle
	int length;						  // size or direction
	int speed;						  // For scroller...
	char *text;						  // text or binary data
	LL *kids;						  // Frames can contain more widgets...
} widget;

// These correspond to the index into the "types" array...
#define WID_NONE 0
#define WID_STRING 1
#define WID_HBAR 2
#define WID_VBAR 3
#define WID_ICON 4
#define WID_TITLE 5
#define WID_SCROLLER 6
#define WID_FRAME 7
#define WID_NUM 8

#define WID_MAX_DIR 4

extern char *types[];

// RESERVED widget ID for keys active on a screen
#define KEYS_WIDGETID "screenkeys"

widget *widget_create ();
int widget_destroy (widget * w);

widget *widget_find (screen * s, char *id);

int widget_add (screen * s, char *id, char *type, char *in, int sock);
int widget_remove (screen * s, char *id, int sock);

#endif
