/*
 * render.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
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

#include "shared/report.h"
#include "shared/LL.h"

#include "drivers.h"
#include "drivers.h"

#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "render.h"

int heartbeat = HEARTBEAT_OPEN;
int backlight = BACKLIGHT_OPEN;
int backlight_state = BACKLIGHT_OPEN;
int backlight_brightness = 255;
int backlight_off_brightness = 0;
int output_state = 0;

static int reset;

#define BUFSIZE 1024

static int draw_frame (LinkedList * list, char fscroll, int left, int top, int right, int bottom, int fwid, int fhgt, int fspeed, int timer);

int
draw_screen (Screen * s, int timer)
{
	static Screen * old_s = NULL;
	int tmp = 0, tmp_state = 0;

	report(RPT_INFO, "draw_screen( screen=\"%.40s\", timer=%d )  ==== START RENDERING ====", s->name, timer );

	reset = 1;

	if (!s)
		return -1;

	if (s == old_s)
		reset = 0;
	old_s = s;

	/* Clear the LCD screen... */
	drivers_clear ();

	/* FIXME drivers_backlight --
	 *
	 * This should be in a separate function altogether.
	 * Perhaps several: drivers_backlight_off, drivers_backlight_on,
	 * drivers_backlight_brightness, drivers_backlight_flash ...
	 *
	 * If the screen's backlight_state isn't set (default) then we
	 * inherit the backlight state from the parent client. This allows
	 * the client to override it's childrens settings.
	 */
	if (s->backlight == BACKLIGHT_OPEN) {
		if (s->client) tmp_state = s->client->backlight;
	} else {
		tmp_state = s->backlight;
	}

	/* Set up backlight to the correct state... */
	/* NOTE: dirty stripping of other options... */
	switch (tmp_state & 1) {
		case BACKLIGHT_OFF:
			drivers_backlight (BACKLIGHT_OFF);
			break;
		/* Backlight on (easy) */
		case BACKLIGHT_ON:
			drivers_backlight (BACKLIGHT_ON);
			break;
		default:
			/* Backlight flash: check timer and flip backlight as appropriate */
			if (tmp_state & BACKLIGHT_FLASH) {
				tmp = (!((timer & 7) == 7));
				if (tmp_state & 1)
					drivers_backlight (tmp ? backlight_brightness : backlight_off_brightness);
				/*drivers_backlight(backlight_brightness * (!((timer&7) == 7))); */
				else
					drivers_backlight (!tmp ? backlight_brightness : backlight_off_brightness);
				/*drivers_backlight(backlight_brightness * ((timer&7) == 7)); */

			/* Backlight blink: check timer and flip backlight as appropriate */
			} else if (tmp_state & BACKLIGHT_BLINK) {
				tmp = (!((timer & 14) == 14));
				if (tmp_state & 1)
					drivers_backlight (tmp ? backlight_brightness : backlight_off_brightness);
				/*drivers_backlight(backlight_brightness * (!((timer&14) == 14))); */
				else
					drivers_backlight (!tmp ? backlight_brightness : backlight_off_brightness);
				/*drivers_backlight(backlight_brightness * ((timer&14) == 14)); */
			}
			break;
	}

	/* Output ports from LCD - outputs depend on the current screen */
	drivers_output (output_state);

	/* Draw a frame... */
	draw_frame (s->widgetlist, 'v', 0, 0, display_props->width, display_props->height, s->width, s->height, (((s->duration / s->height) < 1) ? 1 : (s->duration / s->height)), timer);

	/*debug(RPT_DEBUG, "draw_screen done"); */

	if (heartbeat) {
		drivers_heartbeat(s->heartbeat);
		/*if ((s->heartbeat == HEART_ON) || heartbeat == HEART_ON) { */
			/* Set this to pulsate like a real heart beat... */
			/* (binary is fun...  :) */
			/* drivers_heartbeat (); */
			/*drivers_icon (!((timer + 4) & 5), 0); */
			/*drivers_chr (display_props->width, 1, 0); */
		/*} */
		/* else */
		/* This seems unnecessary... heartbeat is nicer... */
		/* if ((s->heartbeat == HEART_OPEN) && heartbeat != HEART_OFF) { */
		/* 	char *phases = "-\\|/"; */
		/* 	drivers_chr (ldisplay_props->width, 1, phases[timer & 3]); */
		/* } */
	}

	/* flush display out, frame and all... */
	drivers_flush ();

	/*debug(RPT_DEBUG, "draw_screen: %8x, %i", s, timer); */

	report(RPT_INFO, "==== END RENDERING ====" );
	return 0;

}

/* The following function is positively ghastly (as was mentioned above!) */
/* Best thing to do is to remove support for frames... but anyway... */
/* */
static int
draw_frame (LinkedList * list,
		char fscroll,	/* direction of scrolling */
		int left,	/* left edge of frame */
		int top,	/* top edge of frame */
		int right,	/* right edge of frame */
		int bottom,	/* bottom edge of frame */
		int fwid,	/* frame width? */
		int fhgt,	/* frame height? */
		int fspeed,	/* speed of scrolling... */
		int timer)	/* ? */
{

#define	VerticalScrolling (fscroll == 'v')
#define	HorizontalScrolling (fscroll == 'h')

	char str[BUFSIZE];			/* scratch buffer */
	Widget * w;

	int vis_width, vis_height;		/* Width and height of visible frame area */
	int x, y;
	int fx, fy;				/* Scrolling offset for the frame... */
	int length, speed;
	/*int lines; */

	int reset = 1;

	report( RPT_INFO, "draw_frame( list=%p, fscroll='%c', left=%d, top=%d, "
			  "right=%d, bottom=%d, fwid=%d, fhgt=%d, fspeed=%d, timer=%d )",
			  list, fscroll, left,top, right, bottom, fwid, fhgt, fspeed, timer );

	vis_width = right - left;		/* This is the size of the visible frame area */
	vis_height = bottom - top;

	fx = 0;
	fy = 0;
	if (VerticalScrolling) {
		if (fspeed > 0)
			fy = (timer - fspeed) / fspeed;
		else if (fspeed < 0)
			fy = (-fspeed) * timer;

		if (fy < 0)
			fy = 0;

		/* Make sure the whole frame gets displayed, at least...
		 * ...by setting the action to RENDER_HOLD if no other action
		 * is currently defined...
		 */

		if (!screenlist_action)
			screenlist_action = RENDER_HOLD;

		if ((fy) > fhgt - 1) {
			/* Release hold after it has been displayed */
			if (!screenlist_action || screenlist_action == RENDER_HOLD)
				screenlist_action = 0;
		}

		fy %= fhgt;
		if (fy > fhgt - vis_height)
			fy = fhgt - vis_height;

	} else if (HorizontalScrolling) {
		/* TODO:  Frames don't scroll horizontally yet! */
	}

	if (!list)
		return -1;

	/*debug(RPT_DEBUG, "draw_frame: %8x, %i", frame, timer); */

#define PositiveX(a)	((a)->x > 0)
#define PositiveY(a)	((a)->y > 0)
#define ValidPoint(a)	(PositiveX(a) && PositiveY(a))
#define TextPresent(a)	((a)->text)

	LL_Rewind (list);
	do {
		w = (Widget *) LL_Get (list);
		if (!w)
			return -1;

		/* TODO:  Make this cleaner and more flexible!*/
		switch (w->type) {
			case WID_STRING:
				if (ValidPoint(w) && TextPresent(w)) {
					if ((w->y <= vis_height + fy) && (w->y > fy)) {
						if (w->x > vis_width) w->x=vis_width;
						strncpy (str, w->text, vis_width - w->x + 1);
						str[vis_width - w->x + 1] = 0;
						drivers_string (w->x + left, w->y + top - fy, str);
					}
				}
				break;
			case WID_HBAR:
				if (reset) {
					//drivers_init_hbar ();
					reset = 0;
				}
				if ((w->x > 0) && (w->y > 0)) {
					if ((w->y <= vis_height + fy) && (w->y > fy)) {
						if (w->length > 0) {
							if ((w->length / display_props->cellwidth) < vis_width - w->x + 1) {
								/*was: drivers_hbar (w->x + left, w->y + top - fy, w->length); */
								/* improvised len and promille */
								int full_len = display_props->width - w->x - left + 1;
								int promille = (long) 1000 * w->length / ( display_props->cellwidth * full_len );
								drivers_hbar (w->x + left, w->y + top - fy, full_len, promille, BAR_PATTERN_FILLED);
							}
							else {
								/*was: drivers_hbar (w->x + left, w->y + top - fy, wid * display_props->cellwidth); */
								/* Improvised len and promille while we have the old widget language */
								int full_len = ( display_props->width - w->x - left + 1 );
								drivers_hbar (w->x + left, w->y + top - fy, full_len, 1000, BAR_PATTERN_FILLED);
							}
						} else if (w->length < 0) {
							/* TODO:  Rearrange stuff to get left-extending
							 * hbars to draw correctly...
							 * .. er, this'll require driver modifications,
							 * so I'll leave it out for now.
							 */
						}
					}
				}
				break;
			case WID_VBAR:			  /* FIXME:  Vbars don't work in frames!*/
				if (reset) {
					//drivers_init_vbar ();
					reset = 0;
				}
				if ((w->x > 0) && (w->y > 0)) {
					if (w->length > 0) {
						/* Improvised len and promille while we have the old widget language */
						int full_len = display_props->height;
						int promille = (long) 1000 * w->length / display_props->cellheight / full_len;
						drivers_vbar (w->x, display_props->height, full_len, promille, BAR_PATTERN_FILLED);
					} else if (w->length < 0) {
						/* TODO:  Rearrange stuff to get down-extending
						 * vbars to draw correctly...
						 * .. er, this'll require driver modifications,
						 * so I'll leave it out for now.
						 */
					}
				}
				break;
			case WID_ICON:			  /* FIXME:  Not implemented*/
				break;
			case WID_TITLE:			  /* FIXME:  Doesn't work quite right in frames...*/
				if (!w->text)
					break;
				if (vis_width < 8)
					break;

				/*memset (str, 255, width);*/
				memset (str, '*', vis_width);
				/* TODO: Use icon function to draw the bar */

				str[2] = ' ';
				length = strlen (w->text);
				if (length <= vis_width - 6) {
					memcpy (str + 3, w->text, length);
					str[length + 3] = ' ';
				} else					  /* Scroll the title, if it doesn't fit...*/
				{
					speed = 1;
					x = timer / speed;
					y = x / length;

					/* Make sure the whole title gets displayed, at least...*/
					if (!screenlist_action)
						screenlist_action = RENDER_HOLD;
					if (x > length - 6) {
						/* Release hold after it has been displayed*/
						if (!screenlist_action || screenlist_action == RENDER_HOLD)
							screenlist_action = 0;
					}
					x %= (length);
					x -= 3;
					if (x < 0)
						x = 0;
					if (x > length - (vis_width - 6))
						x = length - (vis_width - 6);

					if (y & 1)			  /* Scrolling backwards...*/
					{
						x = (length - (vis_width - 6)) - x;
					}
					strncpy (str + 3, w->text + x, (vis_width - 6));
					str[vis_width - 3] = ' ';
				}
				str[vis_width] = 0;

				drivers_string (1 + left, 1 + top, str);
				break;
			case WID_SCROLLER:		  /* FIXME: doesn't work in frames...*/
				{
					int offset;
					int screen_width;
					if (!w->text)
						break;
					if (w->right < w->left)
						break;
					/*debug(RPT_DEBUG, "rendering: %s %d",w->text,timer);*/
					screen_width = w->right - w->left + 1;
					switch (w->length) {	/* actually, direction...*/
						/* FIXED:  Horz scrollers don't show the
						 * last letter in the string...  (1-off error?)
						 */
					case 'h':
						length = strlen (w->text) + 1;
						if (length <= screen_width) {
							/* it fits within the box, just render it */
							drivers_string (w->left, w->top, w->text);
						} else {
							int effLength = length - screen_width;
							int necessaryTimeUnits = 0;
							if (!screenlist_action)
								screenlist_action = RENDER_HOLD;
							if (w->speed > 0) {
								necessaryTimeUnits = effLength * w->speed;
								if (((timer / (effLength * w->speed)) % 2) == 0) {
									/*wiggle one way*/
									offset = (timer % (effLength * w->speed))
										 / w->speed;
								} else {
									/*wiggle the other*/
									offset = (((timer % (effLength * w->speed))
												  - (effLength * w->speed) + 1)
												 / w->speed) * -1;
								}
							} else if (w->speed < 0) {
								necessaryTimeUnits = effLength / (w->speed * -1);
								if (((timer / (effLength / (w->speed * -1))) % 2) == 0) {
									offset = (timer % (effLength / (w->speed * -1)))
										 * w->speed * -1;
								} else {
									offset = (((timer % (effLength / (w->speed * -1)))
												  * w->speed * -1) - effLength + 1) * -1;
								}
							} else {
								offset = 0;
								if (screenlist_action == RENDER_HOLD)
									screenlist_action = 0;
							}
							if (timer > necessaryTimeUnits) {
								if (screenlist_action == RENDER_HOLD)
									screenlist_action = 0;
							}
							if (offset <= length) {
								strncpy (str, &((w->text)[offset]), screen_width);
								str[screen_width] = '\0';
								/*debug(RPT_DEBUG, "scroller %s : %d", str, length-offset); */
							} else {
								str[0] = '\0';
							}
							drivers_string (w->left, w->top, str);
						}
						break;
						/* FIXME:  Vert scrollers don't always seem to scroll */
						/* back up after hitting the bottom.  They jump back to */
						/* the top instead...  (nevermind?) */
					case 'v':
						{
							int i = 0;
							length = strlen (w->text);
							if (length <= screen_width) {
								/* no scrolling required... */
								drivers_string (w->left, w->top, w->text);
							} else {
								int lines_required = (length / screen_width)
									 + (length % screen_width ? 1 : 0);
								int available_lines = (w->bottom - w->top + 1);
								if (lines_required <= available_lines) {
									/* easy...*/
									for (i = 0; i < lines_required; i++) {
										strncpy (str, &((w->text)[i * screen_width]), screen_width);
										str[screen_width] = '\0';
										drivers_string (w->left, w->top + i, str);
									}
								} else {
									int necessaryTimeUnits = 0;
									int effLines = lines_required - available_lines + 1;
									int begin = 0;
									if (!screenlist_action)
										screenlist_action = RENDER_HOLD;
									/*debug(RPT_DEBUG, "length: %d sw: %d lines req: %d  avail lines: %d  effLines: %d ",length,screen_width,lines_required,available_lines,effLines);*/
									if (w->speed > 0) {
										necessaryTimeUnits = effLines * w->speed;
										if (((timer / (effLines * w->speed)) % 2) == 0) {
											/*debug(RPT_DEBUG, "up ");*/
											begin = (timer % (effLines * w->speed))
												 / w->speed;
										} else {
											/*debug(RPT_DEBUG, "down ");*/
											begin = (((timer % (effLines * w->speed))
														 - (effLines * w->speed) + 1) / w->speed)
												 * -1;
										}
									} else if (w->speed < 0) {
										necessaryTimeUnits = effLines / (w->speed * -1);
										if (((timer / (effLines / (w->speed * -1))) % 2) == 0) {
											begin = (timer % (effLines / (w->speed * -1)))
												 * w->speed * -1;
										} else {
											begin = (((timer % (effLines / (w->speed * -1)))
														 * w->speed * -1) - effLines + 1)
												 * -1;
										}
									} else {
										begin = 0;
									}
									/*debug(RPT_DEBUG, "rendering begin: %d  timer: %d effLines: %d",begin,timer,effLines); */
									for (i = begin; i < begin + available_lines; i++) {
										strncpy (str, &((w->text)[i * (screen_width)]), screen_width);
										str[screen_width] = '\0';
										/*debug(RPT_DEBUG, "rendering: '%s' of %s", */
										/*str,w->text); */
										drivers_string (w->left, w->top + (i - begin), str);
									}
									if (timer > necessaryTimeUnits) {
										if (screenlist_action == RENDER_HOLD)
											screenlist_action = 0;
									}
								}
							}
							break;
						}
					}
					break;
				}
			case WID_FRAME:
				{
					/* FIXME: doesn't handle nested frames quite right!
					 * doesn't handle scrolling in nested frames at all...
					 */
					int new_left, new_top, new_right, new_bottom;
					new_left = left + w->left - 1;
					new_top = top + w->top - 1;
					new_right = left + w->right;
					new_bottom = top + w->bottom;
					if (new_right > right)
						new_right = right;
					if (new_bottom > bottom)
						new_bottom = bottom;
					if (new_left >= right || new_top >= bottom) {	/* Do nothing if it's invisible...*/
					} else {
						draw_frame (w->frame_screen->widgetlist, w->length, new_left, new_top, new_right, new_bottom, w->width, w->height, w->speed, timer);
					}
				}
				break;
			case WID_NUM:				  /* FIXME: doesn't work in frames...*/
				/* NOTE: y=10 means COLON (:)*/
				if ((w->x > 0) && (w->y >= 0) && (w->y <= 10)) {
					if (reset) {
						//drivers_init_num ();
						reset = 0;
					}
					drivers_num (w->x + left, w->y);
				}
				break;
			case WID_NONE:
			default:
				break;
		}
	} while (LL_Next (list) == 0);

	return 0;
}
