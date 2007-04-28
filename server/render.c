/*
 * render.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *		 2007, Peter Marschall
 *
 * Draws screens on the LCD.
 *
 * This needs to be greatly expanded and redone for greater flexibility.
 * For example, it should support multiple screen sizes, more flexible
 * widgets, and multiple simultaneous screens.
 *
 * This will probably take a while to do.  :(
 *
 * THIS FILE IS MESSY!  Anyone care to rewrite it nicely?  Please??  :)
 *
 * NOTE: (from David Douthitt) Multiple screen sizes?  Multiple simultaneous
 * screens?  Horrors of horrors... next thing you know it'll be making coffee...
 * Better believe it'll take a while to do...
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/report.h"
#include "shared/LL.h"

#include "drivers.h"

#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "render.h"

int heartbeat = HEARTBEAT_OPEN;
static int heartbeat_fallback = HEARTBEAT_ON; /* If no heartbeat setting has been set at all */
int backlight = BACKLIGHT_OPEN;
static int backlight_fallback = BACKLIGHT_ON; /* If no backlight setting has been set at all */
int output_state = 0;
char *server_msg_text;
int server_msg_expire = 0;


#define BUFSIZE 1024	/* larger than display width => large enough */


static int render_frame(LinkedList *list, char fscroll, int left, int top, int right, int bottom, int fwid, int fhgt, int fspeed, long timer);

int
render_screen(Screen *s, long timer)
{
	int tmp_state = 0;

	debug(RPT_DEBUG, "%s(screen=[%.40s], timer=%ld)  ==== START RENDERING ====", __FUNCTION__, s->id, timer);

	if (s == NULL)
		return -1;

	/* Clear the LCD screen... */
	drivers_clear();

	/* FIXME drivers_backlight --
	 *
	 * If the screen's backlight isn't set (default) then we
	 * inherit the backlight state from the parent client. This allows
	 * the client to override it's childrens settings.
	 * The server can also override the clients and screens settings.
	 */
	if (backlight != BACKLIGHT_OPEN) {
		tmp_state = backlight;
	}
	else if ((s->client != NULL) && (s->client->backlight != BACKLIGHT_OPEN)) {
		tmp_state = s->client->backlight;
	}
	else if (s->backlight != BACKLIGHT_OPEN) {
		tmp_state = s->backlight;
	}
	else {
		tmp_state = backlight_fallback;
	}

	/* Set up backlight to the correct state... */
	/* NOTE: dirty stripping of other options... */
	/* Backlight flash: check timer and flip backlight as appropriate */
	if (tmp_state & BACKLIGHT_FLASH) {
		drivers_backlight(
			(
				(tmp_state & BACKLIGHT_ON)
				^ ((timer & 7) == 7)
			) ? BACKLIGHT_ON : BACKLIGHT_OFF);
	}
	/* Backlight blink: check timer and flip backlight as appropriate */
	else if (tmp_state & BACKLIGHT_BLINK) {
		drivers_backlight(
			(
				(tmp_state & BACKLIGHT_ON)
				^ ((timer & 14) == 14)
			) ? BACKLIGHT_ON : BACKLIGHT_OFF);
	}
	else {
		/* Simple: Only send lowest bit then... */
		drivers_backlight(tmp_state & BACKLIGHT_ON);
	}

	/* Output ports from LCD - outputs depend on the current screen */
	drivers_output(output_state);

	/* Draw a frame... */
	render_frame(s->widgetlist, 'v', 0, 0,
			display_props->width, display_props->height,
			s->width, s->height, max(s->duration / s->height, 1), timer);

	/* Set the cursor */
	drivers_cursor(s->cursor_x, s->cursor_y, s->cursor);

	if (heartbeat != HEARTBEAT_OPEN) {
		tmp_state = heartbeat;
	}
	else if ((s->client != NULL) && (s->client->heartbeat != HEARTBEAT_OPEN)) {
		tmp_state = s->client->heartbeat;
	}
	else if (s->heartbeat != HEARTBEAT_OPEN) {
		tmp_state = s->heartbeat;
	}
	else {
		tmp_state = heartbeat_fallback;
	}
	drivers_heartbeat(tmp_state);

	/* If there is an server message that is not expired, display it */
	if (server_msg_expire > 0) {
		drivers_string(display_props->width - strlen(server_msg_text) + 1,
				display_props->height, server_msg_text);
		server_msg_expire--;
		if (server_msg_expire == 0) {
			free(server_msg_text);
		}
	}

	/* flush display out, frame and all... */
	drivers_flush();

	debug(RPT_DEBUG, "==== END RENDERING ====");
	return 0;

}

/* The following function is positively ghastly (as was mentioned above!) */
/* Best thing to do is to remove support for frames... but anyway... */
/* */
static int
render_frame(LinkedList *list,
		char fscroll,	/* direction of scrolling */
		int left,	/* left edge of frame */
		int top,	/* top edge of frame */
		int right,	/* right edge of frame */
		int bottom,	/* bottom edge of frame */
		int fwid,	/* frame width? */
		int fhgt,	/* frame height? */
		int fspeed,	/* speed of scrolling... */
		long timer)	/* current timer tick */
{
	int vis_width = right - left;		/* width of visible frame area */
	int vis_height = bottom - top;		/* height of visible frame area */
	int /*fx = 0,*/ fy = 0;			/* Scrolling offset for the frame... */

	debug(RPT_DEBUG, "%s(list=%p, fscroll='%c', left=%d, top=%d, "
			  "right=%d, bottom=%d, fwid=%d, fhgt=%d, fspeed=%d, timer=%ld)",
			  __FUNCTION__, list, fscroll, left,top, right, bottom,
			  fwid, fhgt, fspeed, timer);

	/* return on no data or illegal height */
	if ((list == NULL) || (fhgt <= 0))
		return -1;

	if (fscroll == 'v') {		/* vertical scrolling */
		// only set offset !=0 when fspeed is != 0 and there is something to scroll
		if (fspeed && (fhgt > vis_height)) {
			int fy_max = fhgt - vis_height + 1;

			fy = (fspeed > 0)
			     ? (timer / fspeed) % fy_max
			     : (-fspeed * timer) % fy_max;

			fy = max(fy, 0);	// safeguard against negative values
		}	
	}
	else if (fscroll == 'h') {	/* horizontal scrolling */
		/* TODO:  Frames don't scroll horizontally yet! */
	}

	/* reset widget list */
	LL_Rewind(list);

	/* loop over all widgets */
	do {
		char str[BUFSIZE];			/* scratch buffer */
		Widget *w = (Widget *) LL_Get(list);

		if (w == NULL)
			return -1;

		/* TODO:  Make this cleaner and more flexible! */
		switch (w->type) {
			case WID_STRING:
				if ((w->x > 0) && (w->y > 0) && (w->text) &&
				    (w->y <= vis_height + fy) && (w->y > fy)) {
					int length;

					w->x = min(w->x, vis_width);
					length = min(vis_width - w->x + 1, sizeof(str));
					strncpy(str, w->text, length);
					str[length] = '\0';
					drivers_string(w->x + left, w->y + top - fy, str);
				}
				break;
			case WID_HBAR:
				if ((w->x > 0) && (w->y > 0) &&
				    (w->y <= vis_height + fy) && (w->y > fy)) {
					if (w->length > 0) {
						if ((w->length / display_props->cellwidth) < vis_width - w->x + 1) {
							/*was: drivers_hbar(w->x + left, w->y + top - fy, w->length); */
							/* improvised len and promille */
							int full_len = display_props->width - w->x - left + 1;
							int promille = (long) 1000 * w->length / (display_props->cellwidth * full_len);
							drivers_hbar(w->x + left, w->y + top - fy, full_len, promille, BAR_PATTERN_FILLED);
						}
						else {
							/*was: drivers_hbar(w->x + left, w->y + top - fy, wid * display_props->cellwidth); */
							/* Improvised len and promille while we have the old widget language */
							int full_len = (display_props->width - w->x - left + 1);
							drivers_hbar(w->x + left, w->y + top - fy, full_len, 1000, BAR_PATTERN_FILLED);
						}
					}
					else if (w->length < 0) {
						/* TODO:  Rearrange stuff to get left-extending
						 * hbars to draw correctly...
						 * .. er, this'll require driver modifications,
						 * so I'll leave it out for now.
						 */
					}
				}
				break;
			case WID_VBAR:			  /* FIXME:  Vbars don't work in frames! */
				if ((w->x > 0) && (w->y > 0)) {
					if (w->length > 0) {
						/* Improvised len and promille while we have the old widget language */
						int full_len = display_props->height;
						int promille = (long) 1000 * w->length / display_props->cellheight / full_len;
						drivers_vbar(w->x, display_props->height, full_len, promille, BAR_PATTERN_FILLED);
					}
					else if (w->length < 0) {
						/* TODO:  Rearrange stuff to get down-extending
						 * vbars to draw correctly...
						 * .. er, this'll require driver modifications,
						 * so I'll leave it out for now.
						 */
					}
				}
				break;
			case WID_ICON:
				drivers_icon(w->x, w->y, w->length);

				break;
			case WID_TITLE:			  /* FIXME:  Doesn't work quite right in frames... */
				if ((w->text != NULL) && (vis_width >= 8)) {
					int length = strlen(w->text);
					int x;

					drivers_icon(w->x + left, w->y + top, ICON_BLOCK_FILLED);
					drivers_icon(w->x + left + 1, w->y + top, ICON_BLOCK_FILLED);

					length = min(length, sizeof(str));
					if (length <= vis_width - 6) {
						strncpy(str, w->text, length);
						str[length] = '\0';

						x = length + 4;
					}
					else {			/* Scroll the title, if it doesn't fit... */
						int speed = 1;
						int offset = timer / speed;
						int reverse = offset / length;

						offset %= length;
						offset = max(offset, 0);
						if (offset > length - (vis_width - 6))
							offset = length - (vis_width - 6);

						if (reverse & 1)	  /* Scrolling backwards... */
							offset = (length - (vis_width - 6)) - offset;
						length = abs(vis_width - 6);
						length = min(length, sizeof(str));
						strncpy(str, w->text + offset, length);
						str[length] = '\0';

						x = vis_width - 2;
					}

					drivers_string(w->x + 3 + left, w->y + top, str);

					for ( ; x < vis_width; x++) {
						drivers_icon(w->x + x + left, w->y + top, ICON_BLOCK_FILLED);
					}
				}
				break;
			case WID_SCROLLER:		  /* FIXME: doesn't work in frames... */
				if ((w->text != NULL) && (w->right >= w->left)) {
					int length;
					int offset;
					int screen_width;

					/*debug(RPT_DEBUG, "%s: %s %d",__FUNCTION__,w->text,timer);*/
					screen_width = abs(w->right - w->left + 1);
					screen_width = min(screen_width, sizeof(str));
					switch (w->length) {	/* actually, direction... */
						/* FIXED:  Horz scrollers don't show the
						 * last letter in the string...  (1-off error?)
						 */
					case 'm': // Marquee
						length = strlen(w->text);
						if (length <= screen_width) {
							/* it fits within the box, just render it */
							drivers_string(w->left, w->top, w->text);
						}
						else {
							int necessaryTimeUnits = 0;

							if (w->speed > 0) {
								necessaryTimeUnits = length * w->speed;
								offset = (timer % (length * w->speed)) / w->speed;
							}
							else if (w->speed < 0) {
								necessaryTimeUnits = length / (w->speed * -1);
								offset = (timer % (length / (w->speed * -1))) * w->speed * -1;
							}
							else {
								offset = 0;
							}
							if (offset <= length) {
								int room = screen_width - (length - offset);

								strncpy(str, &w->text[offset], screen_width);

								// if there's more room, restart at the beginning
								if (room > 0) {
									strncat(str, w->text, room);
								}

								str[screen_width] = '\0';

								/*debug(RPT_DEBUG, "scroller %s : %d", str, length-offset);*/
							}
							else {
								str[0] = '\0';
							}
							drivers_string(w->left, w->top, str);
						}
						break;
					case 'h':
						length = strlen(w->text) + 1;
						if (length <= screen_width) {
							/* it fits within the box, just render it */
							drivers_string(w->left, w->top, w->text);
						}
						else {
							int effLength = length - screen_width;
							int necessaryTimeUnits = 0;

							if (w->speed > 0) {
								necessaryTimeUnits = effLength * w->speed;
								if (((timer / (effLength * w->speed)) % 2) == 0) {
									/* wiggle one way */
									offset = (timer % (effLength * w->speed))
										 / w->speed;
								}
								else {
									/* wiggle the other */
									offset = (((timer % (effLength * w->speed))
										  - (effLength * w->speed) + 1)
										 / w->speed) * -1;
								}
							}
							else if (w->speed < 0) {
								necessaryTimeUnits = effLength / (w->speed * -1);
								if (((timer / (effLength / (w->speed * -1))) % 2) == 0) {
									offset = (timer % (effLength / (w->speed * -1)))
										 * w->speed * -1;
								}
								else {
									offset = (((timer % (effLength / (w->speed * -1)))
										  * w->speed * -1)
										  - effLength + 1) * -1;
								}
							}
							else {
								offset = 0;
							}
							if (offset <= length) {
								strncpy(str, &((w->text)[offset]), screen_width);
								str[screen_width] = '\0';
								/*debug(RPT_DEBUG, "scroller %s : %d", str, length-offset); */
							}
							else {
								str[0] = '\0';
							}
							drivers_string(w->left, w->top, str);
						}
						break;
						/* FIXME:  Vert scrollers don't always seem to scroll */
						/* back up after hitting the bottom.  They jump back to */
						/* the top instead...  (nevermind?) */
					case 'v':
						length = strlen(w->text);
						if (length <= screen_width) {
							/* no scrolling required... */
							drivers_string(w->left, w->top, w->text);
						}
						else {
							int lines_required = (length / screen_width)
								 + (length % screen_width ? 1 : 0);
							int available_lines = (w->bottom - w->top + 1);

							if (lines_required <= available_lines) {
								/* easy... */
								int i;

								for (i = 0; i < lines_required; i++) {
									strncpy(str, &((w->text)[i * screen_width]), screen_width);
									str[screen_width] = '\0';
									drivers_string(w->left, w->top + i, str);
								}
							}
							else {
								int necessaryTimeUnits = 0;
								int effLines = lines_required - available_lines + 1;
								int begin = 0;
								int i = 0;

								/*debug(RPT_DEBUG, "length: %d sw: %d lines req: %d  avail lines: %d  effLines: %d ",length,screen_width,lines_required,available_lines,effLines);*/
								if (w->speed > 0) {
									necessaryTimeUnits = effLines * w->speed;
									if (((timer / (effLines * w->speed)) % 2) == 0) {
										/*debug(RPT_DEBUG, "up ");*/
										begin = (timer % (effLines * w->speed))
											 / w->speed;
									}
									else {
										/*debug(RPT_DEBUG, "down ");*/
										begin = (((timer % (effLines * w->speed))
											 - (effLines * w->speed) + 1) / w->speed)
											 * -1;
									}
								}
								else if (w->speed < 0) {
									necessaryTimeUnits = effLines / (w->speed * -1);
									if (((timer / (effLines / (w->speed * -1))) % 2) == 0) {
										begin = (timer % (effLines / (w->speed * -1)))
											 * w->speed * -1;
									}
									else {
										begin = (((timer % (effLines / (w->speed * -1)))
											 * w->speed * -1) - effLines + 1)
											 * -1;
									}
								}
								else {
									begin = 0;
								}
								/*debug(RPT_DEBUG, "rendering begin: %d  timer: %d effLines: %d",begin,timer,effLines); */
								for (i = begin; i < begin + available_lines; i++) {
									strncpy(str, &((w->text)[i * (screen_width)]), screen_width);
									str[screen_width] = '\0';
									/*debug(RPT_DEBUG, "rendering: '%s' of %s", */
									/*str,w->text); */
									drivers_string(w->left, w->top + (i - begin), str);
								}
							}
						}
						break;
					}
				}
				break;
			case WID_FRAME:
				{
					/* FIXME: doesn't handle nested frames quite right!
					 * doesn't handle scrolling in nested frames at all...
					 */
					int new_left = left + w->left - 1;
					int new_top = top + w->top - 1;
					int new_right = min(left + w->right, right);
					int new_bottom = min(top + w->bottom, bottom);

					if ((new_left < right) && (new_top < bottom))	/* Render only if it's visible... */
						render_frame(w->frame_screen->widgetlist, w->length, new_left, new_top,
								new_right, new_bottom, w->width, w->height, w->speed, timer);
				}
				break;
			case WID_NUM:				  /* FIXME: doesn't work in frames... */
				/* NOTE: y=10 means COLON (:) */
				if ((w->x > 0) && (w->y >= 0) && (w->y <= 10)) {
					drivers_num(w->x + left, w->y);
				}
				break;
			case WID_NONE:
			default:
				break;
		}
	} while (LL_Next(list) == 0);

	return 0;
}

int
server_msg(const char *text, int expire)
{
	debug(RPT_DEBUG, "%s(text=\"%.40s\", expire=%d)", __FUNCTION__, text, expire);

	if (strlen(text) > 15 || expire <= 0) {
		return -1;
	}

	/* Still a message active ? */

	if (server_msg_expire > 0) {
		free(server_msg_text);
	}

	/* Store new message */
	server_msg_text = malloc(strlen(text) + 3);
	strcpy(server_msg_text, "| ");
	strcat(server_msg_text, text);

	server_msg_expire = expire;

	return 0;
}
