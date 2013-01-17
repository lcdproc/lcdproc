/** \file server/menuitem.c
 * Handles a menuitem and all actions that can be performed on it.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include "shared/report.h"
#include "shared/defines.h"

#include "widget.h"
#include "screen.h"
#include "menuitem.h"
#include "menu.h"
#include "drivers.h"

/* this is needed for verify_ipv4 and verify_ipv6. */
#include "sock.h"

#define MAX_NUMERIC_LEN 40

extern Menu *main_menu;		/* Access to the main menu */

char *error_strs[] = {"", "Out of range", "Too long", "Too short", "Invalid Address"};
char *menuitemtypenames[] = {"menu", "action", "checkbox", "ring", "slider", "numeric", "alpha", "ip"};
char *menueventtypenames[] = {"select", "update", "plus", "minus", "enter", "leave"};

void menuitem_destroy_action(MenuItem *item);
void menuitem_destroy_checkbox(MenuItem *item);
void menuitem_destroy_ring(MenuItem *item);
void menuitem_destroy_slider(MenuItem *item);
void menuitem_destroy_numeric(MenuItem *item);
void menuitem_destroy_alpha(MenuItem *item);
void menuitem_destroy_ip(MenuItem *item);

void menuitem_reset_numeric(MenuItem *item);
void menuitem_reset_alpha(MenuItem *item);
void menuitem_reset_ip(MenuItem *item);

void menuitem_rebuild_screen_slider(MenuItem *item, Screen *s);
void menuitem_rebuild_screen_numeric(MenuItem *item, Screen *s);
void menuitem_rebuild_screen_alpha(MenuItem *item, Screen *s);
void menuitem_rebuild_screen_ip(MenuItem *item, Screen *s);

void menuitem_update_screen_slider(MenuItem *item, Screen *s);
void menuitem_update_screen_numeric(MenuItem *item, Screen *s);
void menuitem_update_screen_alpha(MenuItem *item, Screen *s);
void menuitem_update_screen_ip(MenuItem *item, Screen *s);

MenuResult menuitem_process_input_slider(MenuItem *item, MenuToken token, const char *key, unsigned int keymask);
MenuResult menuitem_process_input_numeric(MenuItem *item, MenuToken token, const char *key, unsigned int keymask);
MenuResult menuitem_process_input_alpha(MenuItem *item, MenuToken token, const char *key, unsigned int keymask);
MenuResult menuitem_process_input_ip(MenuItem *item, MenuToken token, const char *key, unsigned int keymask);


/* information about string representation of IP addresses */
typedef struct {
	int  maxlen;		// max length of the string represenation;
	char sep;		// separators between numeric strings
	int  base;		// numeric base
	int  width;		// width of each numeric string
	int  limit;		// upper limit of each numeric string
	int  posValue[5];	// digit-value of the digits at the approp. position in the numeric string
	char format[5];		// printf-format string to print a numeric string
	int (*verify)(const char *);	// verify function: returns 1 = OK, 0 = error
	char dummy[16];		// dummy value (if provided value is no IP address)
} IpSstringProperties;


const IpSstringProperties IPinfo[] = {
	// IPv4: 15 char long '.'-separated sequence of 3-digit decimal strings in range 000 - 255
	{ 15, '.', 10, 3,   255, {  100,  10,  1, 0, 0 }, "%03d", verify_ipv4, "0.0.0.0" },
	// IPv6: 39 char long ':'-separated sequence of 4-digit hex strings in range 0000 - ffff
	{ 39, ':', 16, 4, 65535, { 4096, 256, 16, 1, 0 }, "%04x", verify_ipv6, "0:0:0:0:0:0:0:0" }
};


/******** MENU UTILITY FUNCTIONS ********/

/** returns default_result if predecessor_id is NULL or the respective value
 * otherwise. */
MenuResult menuitem_predecessor2menuresult(char *predecessor_id, MenuResult default_result)
{
	if (predecessor_id == NULL)
		return default_result;
	if (strcmp("_quit_", predecessor_id) == 0)
		return MENURESULT_QUIT;
	else if (strcmp("_close_", predecessor_id) == 0)
		return MENURESULT_CLOSE;
	else if (strcmp("_none_", predecessor_id) == 0)
		return MENURESULT_NONE;
	else
		return MENURESULT_PREDECESSOR;
}

/** returns default_result if successor_id is NULL or the respective value
 * otherwise. */
MenuResult menuitem_successor2menuresult(char *successor_id, MenuResult default_result)
{
	if (successor_id == NULL)
		return default_result;
	if (strcmp("_quit_", successor_id) == 0)
		return MENURESULT_QUIT;
	else if (strcmp("_close_", successor_id) == 0)
		return MENURESULT_CLOSE;
	else if (strcmp("_none_", successor_id) == 0)
		return MENURESULT_NONE;
	else
		return MENURESULT_SUCCESSOR;
}

/** Returns the MenuItem with the specified id if found or NULL. The search
 * for itemid is restricted to the client's menus if the preprocessor macro
 * LCDPROC_PERMISSIVE_MENU_GOTO is *not* set. */
MenuItem *menuitem_search(char *menu_id, Client *client)
{
# ifdef LCDPROC_PERMISSIVE_MENU_GOTO
	MenuItem *top = main_menu;
# else
	MenuItem *top = client->menu;
# endif /* LCDPROC_PERMISSIVE_MENU_GOTO */

	return menu_find_item(top, menu_id, true);
}

/******** FUNCTION TABLES ********/
/*-
 * Tables with functions to call for all different item types. The order is:
 * "menu", "action", "checkbox", "ring", "slider", "numeric", "alpha", "ip".
 */

void (*destructor_table[NUM_ITEMTYPES]) (MenuItem *item) =
{
	menu_destroy,
	NULL,
	NULL,
	menuitem_destroy_ring,
	menuitem_destroy_slider,
	menuitem_destroy_numeric,
	menuitem_destroy_alpha,
	menuitem_destroy_ip
};
void (*reset_table[NUM_ITEMTYPES]) (MenuItem *item) =
{
	menu_reset,
	NULL,
	NULL,
	NULL,
	NULL,
	menuitem_reset_numeric,
	menuitem_reset_alpha,
	menuitem_reset_ip
};
void (*build_screen_table[NUM_ITEMTYPES]) (MenuItem *item, Screen *s) =
{
	menu_build_screen,
	NULL,
	NULL,
	NULL,
	menuitem_rebuild_screen_slider,
	menuitem_rebuild_screen_numeric,
	menuitem_rebuild_screen_alpha,
	menuitem_rebuild_screen_ip
};
void (*update_screen_table[NUM_ITEMTYPES]) (MenuItem *item, Screen *s) =
{
	menu_update_screen,
	NULL,
	NULL,
	NULL,
	menuitem_update_screen_slider,
	menuitem_update_screen_numeric,
	menuitem_update_screen_alpha,
	menuitem_update_screen_ip
};

MenuResult (*process_input_table[NUM_ITEMTYPES]) (MenuItem *item, MenuToken token, const char *key, unsigned int keymask) =
{
	menu_process_input,
	NULL,
	NULL,
	NULL,
	menuitem_process_input_slider,
	menuitem_process_input_numeric,
	menuitem_process_input_alpha,
	menuitem_process_input_ip
};

/******** METHODS ********/

MenuItem *menuitem_create(MenuItemType type, char *id, MenuEventFunc(*event_func),
	char *text, Client *client)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(type=%d, id=\"%s\", event_func=%p, text=\"%s\")",
			__FUNCTION__, type, id, event_func, text);

	if ((id == NULL) || (text == NULL)) {
		// report(RPT_ERR, "%s: illegal id or text", __FUNCTION__);
		return NULL;
	}

	/* Allocate space and fill struct */
	new_item = malloc(sizeof(MenuItem));
	if (!new_item) {
		report(RPT_ERR, "%s: Could not allocate memory", __FUNCTION__);
		return NULL;
	}
	new_item->type = type;
	new_item->id = strdup(id);
	if (!new_item->id) {
		report(RPT_ERR, "%s: Could not allocate memory", __FUNCTION__);
		free(new_item);
		return NULL;
	}
	new_item->successor_id = NULL;
	new_item->predecessor_id = NULL;
	new_item->parent = NULL;
	new_item->event_func = event_func;
	new_item->text = strdup(text);
	if (!new_item->text) {
		report(RPT_ERR, "%s: Could not allocate memory", __FUNCTION__);
		free(new_item->id);
		free(new_item);
		return NULL;
	}
	new_item->client = client;
	new_item->is_hidden = false;

	/* Clear the type specific data part */
	memset(&(new_item->data), '\0', sizeof(new_item->data));

	return new_item;
}

// fixme: the menu_result arg is obsoleted (use char* successor_id)
MenuItem *menuitem_create_action(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, MenuResult menu_result)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=[%s], event_func=%p, text=\"%s\", close_menu=%d)",
			__FUNCTION__, id, event_func, text, menu_result);

	new_item = menuitem_create(MENUITEM_ACTION, id, event_func, text, client);
	if (new_item != NULL)
	{
		switch (menu_result)
		{
		case MENURESULT_NONE:
			new_item->successor_id = strdup("_none_");
			break;
		case MENURESULT_CLOSE:
			new_item->successor_id = strdup("_close_");
			break;
		case MENURESULT_QUIT:
			new_item->successor_id = strdup("_quit_");
			break;
		default:
			assert(!"unexpected MENURESULT");
		}
	}

	return new_item;
}

MenuItem *menuitem_create_checkbox(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, bool allow_gray, bool value)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=[%s], event_func=%p, text=\"%s\", allow_gray=%d, value=%d)",
			__FUNCTION__, id, event_func, text, allow_gray, value);

	new_item = menuitem_create(MENUITEM_CHECKBOX, id, event_func, text, client);
	if (new_item != NULL) {
		new_item->data.checkbox.allow_gray = allow_gray;
		new_item->data.checkbox.value = value;
	}

	return new_item;
}

MenuItem *menuitem_create_ring(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char *strings, short value)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=[%s], event_func=%p, text=\"%s\", strings=\"%s\", value=%d)",
			__FUNCTION__, id, event_func, text, strings, value);

	new_item = menuitem_create(MENUITEM_RING, id, event_func, text, client);
	if (new_item != NULL) {
		new_item->data.ring.strings = tablist2linkedlist(strings);
		new_item->data.ring.value = value;
	}

	return new_item;
}

MenuItem *menuitem_create_slider(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char *mintext, char *maxtext,
	int minvalue, int maxvalue, int stepsize, int value)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=[%s], event_func=%p, text=\"%s\", mintext=\"%s\", maxtext=\"%s\", minvalue=%d, maxvalue=%d, stepsize=%d, value=%d)",
			__FUNCTION__, id, event_func, text, mintext, maxtext, minvalue, maxvalue, stepsize, value);

	new_item = menuitem_create(MENUITEM_SLIDER, id, event_func, text, client);
	if (new_item != NULL) {
		new_item->data.slider.mintext = strdup(mintext);
		new_item->data.slider.maxtext = strdup(maxtext);
		if (new_item->data.slider.mintext == NULL ||
		    new_item->data.slider.maxtext == NULL) {
			menuitem_destroy(new_item);
			return NULL;
		}
		new_item->data.slider.minvalue = minvalue;
		new_item->data.slider.maxvalue = maxvalue;
		new_item->data.slider.stepsize = stepsize;
		new_item->data.slider.value = value;
	}

	return new_item;
}

MenuItem *menuitem_create_numeric(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, int minvalue, int maxvalue, int value)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=[%s], event_func=%p, text=\"%s\", minvalue=%d, maxvalue=%d, value=%d)",
			__FUNCTION__, id, event_func, text, minvalue, minvalue, value);

	new_item = menuitem_create(MENUITEM_NUMERIC, id, event_func, text, client);
	if (new_item != NULL) {
		new_item->data.numeric.maxvalue = maxvalue;
		new_item->data.numeric.minvalue = minvalue;
		new_item->data.numeric.value = value;
		new_item->data.numeric.edit_str = malloc(MAX_NUMERIC_LEN);
		if (new_item->data.numeric.edit_str == NULL) {
			menuitem_destroy(new_item);
			return NULL;
		}
	}

	return new_item;
}

MenuItem *menuitem_create_alpha(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char password_char, short minlength, short maxlength,
	bool allow_caps, bool allow_noncaps, bool allow_numbers,
	char *allowed_extra, char *value)
{
	MenuItem *new_item;

	debug(RPT_DEBUG, "%s(id=\"%s\", event_func=%p, text=\"%s\", password_char=%d, maxlength=%d, value=\"%s\")",
			__FUNCTION__, id, event_func, text, password_char, maxlength, value);

	new_item = menuitem_create(MENUITEM_ALPHA, id, event_func, text, client);
	if (new_item != NULL) {
		new_item->data.alpha.password_char = password_char;
		new_item->data.alpha.minlength = minlength;
		new_item->data.alpha.maxlength = maxlength;

		new_item->data.alpha.allow_caps = allow_caps;
		new_item->data.alpha.allow_noncaps = allow_noncaps;
		new_item->data.alpha.allow_numbers = allow_numbers;
		new_item->data.alpha.allowed_extra = strdup(allowed_extra);
		if (new_item->data.alpha.allowed_extra == NULL) {
			menuitem_destroy(new_item);
			return NULL;
		}

		new_item->data.alpha.value = malloc(maxlength + 1);
		if (new_item->data.alpha.value == NULL) {
			menuitem_destroy(new_item);
			return NULL;
		}

		strncpy(new_item->data.alpha.value, value, maxlength);
		new_item->data.alpha.value[maxlength] = 0;

		new_item->data.alpha.edit_str = malloc(maxlength + 1);
		if (new_item->data.alpha.edit_str == NULL) {
			menuitem_destroy(new_item);
			return NULL;
		}
	}

	return new_item;
}

MenuItem *menuitem_create_ip(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, bool v6, char *value)
{
	MenuItem *new_item;
	const IpSstringProperties *ipinfo;

	debug(RPT_DEBUG, "%s(id=\"%s\", event_func=%p, text=\"%s\", v6=%d, value=\"%s\")",
			__FUNCTION__, id, event_func, text, v6, value);

	new_item = menuitem_create(MENUITEM_IP, id, event_func, text, client);
	if (new_item == NULL)
		return NULL;

	new_item->data.ip.v6 = v6;
	ipinfo = (v6) ? &IPinfo[1] : &IPinfo[0];

	new_item->data.ip.maxlength = ipinfo->maxlen;;
	new_item->data.ip.value = malloc(new_item->data.ip.maxlength + 1);
	if (new_item->data.ip.value == NULL)
	{
		menuitem_destroy(new_item);
		return NULL;
	}

	strncpy(new_item->data.ip.value, value, new_item->data.ip.maxlength);
	new_item->data.ip.value[new_item->data.ip.maxlength] = '\0';

	if (ipinfo->verify != NULL) {
		char *start = new_item->data.ip.value;

		while (start != NULL) {
			char *skip = start;

			while ((*skip == ' ') || ((*skip == '0') && (skip[1] != ipinfo->sep) && (skip[1] != '\0')))
				skip++;
			memccpy(start, skip, '\0', new_item->data.ip.maxlength + 1);
			skip = strchr(start, ipinfo->sep);
			start = (skip != NULL) ? (skip + 1) : NULL;
		}

		if (!ipinfo->verify(new_item->data.ip.value)) {
			report(RPT_WARNING, "%s(id=\"%s\") ip address not verified: \"%s\"",
					    __FUNCTION__, id, value);
			strncpy(new_item->data.ip.value, ipinfo->dummy, new_item->data.ip.maxlength);
			new_item->data.ip.value[new_item->data.ip.maxlength] = '\0';
		}
	}

	new_item->data.ip.edit_str = malloc(new_item->data.ip.maxlength + 1);
	if (new_item->data.ip.edit_str == NULL) {
		menuitem_destroy(new_item);
		return NULL;
	}

	return new_item;
}

void menuitem_destroy(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		void (*destructor) (MenuItem *);

		/* First destroy type specific data */
		destructor = destructor_table[item->type];
		if (destructor)
			destructor(item);

		/* Following strings should always be allocated */
		free(item->text);
		free(item->id);

		/* And finally...*/
		free(item);
	}
}

void menuitem_destroy_ring(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
	char *s;

		/* deallocate the strings */
		for (s = LL_GetFirst(item->data.ring.strings);
		     s != NULL;
		     s = LL_GetNext(item->data.ring.strings)) {
			free(s);
		}
		/* and the list */
		LL_Destroy(item->data.ring.strings);
	}
}

void menuitem_destroy_slider(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		/* These strings should always be allocated */
		free(item->data.slider.mintext);
		free(item->data.slider.maxtext);
	}
}

void menuitem_destroy_numeric(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		/* This string should always be allocated */
		free(item->data.numeric.edit_str);
	}
}

void menuitem_destroy_alpha(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		/* These strings should always be allocated */
		free(item->data.alpha.allowed_extra);
		free(item->data.alpha.value);
		free(item->data.alpha.edit_str);
	}
}

void menuitem_destroy_ip(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	/* These strings should always be allocated */
	free(item->data.ip.value);
	free(item->data.ip.edit_str);
}


/******** MENU ITEM RESET FUNCTIONS ********/

void menuitem_reset(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		void (*func) (MenuItem *);

		/* First destroy type specific data */
		func = reset_table[item->type];
		if (func)
			func(item);
	}
}

void menuitem_reset_numeric(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		item->data.numeric.edit_pos = 0;
		item->data.numeric.edit_offs = 0;
		memset(item->data.numeric.edit_str, '\0', MAX_NUMERIC_LEN);
		if (item->data.numeric.minvalue < 0) {
			snprintf(item->data.numeric.edit_str, MAX_NUMERIC_LEN,
					"%+d", item->data.numeric.value);
		} else {
			snprintf(item->data.numeric.edit_str, MAX_NUMERIC_LEN,
					"%d", item->data.numeric.value);
		}
	}
}

void menuitem_reset_alpha(MenuItem *item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	if (item != NULL) {
		item->data.alpha.edit_pos = 0;
		item->data.alpha.edit_offs = 0;
		memset(item->data.alpha.edit_str, '\0', item->data.alpha.maxlength+1);
		strcpy(item->data.alpha.edit_str, item->data.alpha.value);
	}
}

void menuitem_reset_ip(MenuItem *item)
{
	char *start = item->data.ip.value;
	const IpSstringProperties *ipinfo = (item->data.ip.v6) ? &IPinfo[1] : &IPinfo[0];

	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"));

	item->data.ip.edit_pos = 0;
	item->data.ip.edit_offs = 0;
	memset(item->data.ip.edit_str, '\0', item->data.ip.maxlength+1);

	// normalize IP address string to e.g. 010.002.250.002 / 0001:0203:0405:0607:0809:0a0b:0c0d:0e0f
	while (start != NULL) {
		char *end;
    		char tmpstr[5];
		int num = (int) strtol(start, (char **) NULL, ipinfo->base);

		snprintf(tmpstr, 5, ipinfo->format, num);
		strcat(item->data.ip.edit_str, tmpstr);
		end = strchr(start, ipinfo->sep);
		start = (end != NULL) ? (end + 1) : NULL;
		if (start != NULL) {
			tmpstr[0] = ipinfo->sep;
			tmpstr[1] = '\0';
			strcat(item->data.ip.edit_str, tmpstr);
		}
	}
}


/******** MENU SCREEN BUILD FUNCTIONS ********/

void menuitem_rebuild_screen(MenuItem *item, Screen *s)
{
	Widget *w;
	void (*build_screen) (MenuItem *item, Screen *s);

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if (!display_props) {
		/* Nothing to build if no display size is known */
		report(RPT_ERR, "%s: display size unknown", __FUNCTION__);
		return;
	}

	if (s != NULL) {
		/* First remove all widgets from the screen */
		while ((w = screen_getfirst_widget(s)) != NULL) {
			/* We know these widgets don't have subwidgets, so we can
			 * easily remove them
			 */
			screen_remove_widget(s, w);
			widget_destroy(w);
		}

		if (item != NULL) {
			/* Call type specific screen building function */
			build_screen = build_screen_table [item->type];
			if (build_screen) {
				build_screen(item, s);
			} else {
				report(RPT_ERR, "%s: given menuitem cannot be active", __FUNCTION__);
				return;
			}

			/* Also always call update_screen */
			menuitem_update_screen(item, s);
		}
	}
}

void menuitem_rebuild_screen_slider(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	if (display_props->height >= 2) {
		/* Only add a title if enough space... */
		w = widget_create("text", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup(item->text);
		w->x = 1;
		w->y = 1;
	}

	w = widget_create("bar", WID_HBAR, s);
	screen_add_widget(s, w);
	w->width = display_props->width;
	if (display_props->height > 2) {
		/* This is option 1: we have enought space, so the bar and
		 * min/max texts can be on separate lines.
		 */
		w->x = 2;
		w->y = display_props->height / 2 + 1;
		w->width = display_props->width - 2;
	}

	w = widget_create("min", WID_STRING, s);
	screen_add_widget(s, w);
	w->text = NULL;
	w->x = 1;
	if (display_props->height > 2) {
		w->y = display_props->height / 2 + 2;
	} else {
		w->y = display_props->height / 2 + 1;
	}

	w = widget_create("max", WID_STRING, s);
	screen_add_widget(s, w);
	w->text = NULL;
	w->x = 1;
	if (display_props->height > 2) {
		w->y = display_props->height / 2 + 2;
	} else {
		w->y = display_props->height / 2 + 1;
	}
}

void menuitem_rebuild_screen_numeric(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	if (display_props->height >= 2) {
		/* Only add a title if enough space... */
		w = widget_create("text", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup(item->text);
		w->x = 1;
		w->y = 1;
	}

	w = widget_create("value", WID_STRING, s);
	screen_add_widget(s, w);
	w->text = malloc(MAX_NUMERIC_LEN);
	w->x = 2;
	w->y = display_props->height / 2 + 1;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = widget_create("error", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup("");
		w->x = 1;
		w->y = display_props->height;
	}
}

void menuitem_rebuild_screen_alpha(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	if (display_props->height >= 2) {
		/* Only add a title if enough space... */
		w = widget_create("text", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup(item->text);
		w->x = 1;
		w->y = 1;
	}

	w = widget_create("value", WID_STRING, s);
	screen_add_widget(s, w);
	w->text = malloc(item->data.alpha.maxlength+1);
	w->x = 2;
	w->y = display_props->height / 2 + 1;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = widget_create("error", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup("");
		w->x = 1;
		w->y = display_props->height;
	}
}

void menuitem_rebuild_screen_ip(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	if (display_props->height >= 2) {
		/* Only add a title if enough space... */
		w = widget_create("text", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup(item->text);
		w->x = 1;
		w->y = 1;
	}

	w = widget_create("value", WID_STRING, s);
	screen_add_widget(s, w);
	w->text = malloc(item->data.ip.maxlength+1);
	w->x = 2;
	w->y = display_props->height / 2 + 1;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = widget_create("error", WID_STRING, s);
		screen_add_widget(s, w);
		w->text = strdup("");
		w->x = 1;
		w->y = display_props->height;
	}
}

/******** MENU SCREEN UPDATE FUNCTIONS ********/

void menuitem_update_screen(MenuItem *item, Screen *s)
{
	void (*update_screen) (MenuItem *item, Screen *s);

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	/* Disable the cursor by default */
	s->cursor = CURSOR_OFF;

	/* Call type specific screen building function */
	update_screen = update_screen_table [item->type];
	if (update_screen) {
		update_screen(item, s);
	} else {
		report(RPT_ERR, "%s: given menuitem cannot be active", __FUNCTION__);
		return;
	}
}

void menuitem_update_screen_slider(MenuItem *item, Screen *s)
{
	Widget *w;
	int min_len, max_len;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	/* Calculate the bar position and length by filling buffers */
	min_len = strlen(item->data.slider.mintext);
	max_len = strlen(item->data.slider.maxtext);

	/* And adjust the data */

	w = screen_find_widget(s, "bar");
	if (display_props->height <= 2) {
		/* This is option 2: we're tight on lines, so we put the bar
		 * and min/max texts on the same line.
		 */
		w->x = 1 + min_len;
		w->y = display_props->height;
		w->width = display_props->width -
				min_len - max_len;
	}
	/* FUTURE: w->promille = 1000 * (item->data.slider.value - item->data.slider.minvalue) / (item->data.slider.maxvalue - item->data.slider.minvalue) */;
	w->length = w->width * display_props->cellwidth
		* (item->data.slider.value - item->data.slider.minvalue)
		/ (item->data.slider.maxvalue - item->data.slider.minvalue);

	w = screen_find_widget(s, "min");
	if (w->text) free(w->text);
	w->text = strdup(item->data.slider.mintext);

	w = screen_find_widget(s, "max");
	if (w->text) free(w->text);
	w->x = 1 + display_props->width - max_len;
	w->text = strdup(item->data.slider.maxtext);
}

void menuitem_update_screen_numeric(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	w = screen_find_widget(s, "value");
	strcpy(w->text, item->data.numeric.edit_str + item->data.numeric.edit_offs);

	s->cursor = CURSOR_DEFAULT_ON;
	s->cursor_x = w->x + item->data.numeric.edit_pos - item->data.numeric.edit_offs;
	s->cursor_y = w->y;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = screen_find_widget(s, "error");
		free(w->text);
		w->text = strdup(error_strs[item->data.numeric.error_code]);
	}
}

void menuitem_update_screen_alpha(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	w = screen_find_widget(s, "value");
	if (item->data.alpha.password_char == '\0') {
		strcpy(w->text, item->data.alpha.edit_str + item->data.alpha.edit_offs);
	} else {
		int len = strlen(item->data.alpha.edit_str) - item->data.alpha.edit_offs;

		memset(w->text, item->data.alpha.password_char, len);
		w->text[len] = '\0';
	}

	s->cursor = CURSOR_DEFAULT_ON;
	s->cursor_x = w->x + item->data.alpha.edit_pos - item->data.alpha.edit_offs;
	s->cursor_y = w->y;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = screen_find_widget(s, "error");
		free(w->text);
		w->text = strdup(error_strs[item->data.alpha.error_code]);
	}
}

void menuitem_update_screen_ip(MenuItem *item, Screen *s)
{
	Widget *w;

	debug(RPT_DEBUG, "%s(item=[%s], screen=[%s])", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((item == NULL) || (s == NULL))
		return;

	w = screen_find_widget(s, "value");
	if (w != NULL)
		strcpy(w->text, item->data.ip.edit_str + item->data.ip.edit_offs);

	s->cursor = CURSOR_DEFAULT_ON;
	s->cursor_x = w->x + item->data.ip.edit_pos - item->data.ip.edit_offs;
	s->cursor_y = w->y;

	/* Only display error string if enough space... */
	if (display_props->height > 2) {
		w = screen_find_widget(s, "error");
		free(w->text);
		w->text = strdup(error_strs[item->data.ip.error_code]);
	}
}

/******** MENU SCREEN INPUT HANDLING FUNCTIONS ********/

MenuResult menuitem_process_input(MenuItem *item, MenuToken token, const char *key, unsigned int keymask)
{
	MenuResult (*process_input) (MenuItem *item, MenuToken token, const char *key, unsigned int keymask);

	debug(RPT_DEBUG, "%s(item=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"), token, key);

	if (item == NULL)
		return MENURESULT_ERROR;

	/* Call type specific screen building function */
	process_input  = process_input_table [item->type];
	if (process_input) {
		return process_input(item, token, key, keymask);
	} else {
		report(RPT_ERR, "%s: given menuitem cannot be active", __FUNCTION__);
		return MENURESULT_ERROR;
	}
}

MenuResult menuitem_process_input_slider(MenuItem *item, MenuToken token, const char *key, unsigned int keymask)
{
	debug(RPT_DEBUG, "%s(item=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"), token, key);

	if (item == NULL)
		return MENURESULT_ERROR;

	switch (token) {
	  case MENUTOKEN_MENU:
		return menuitem_predecessor2menuresult(
			item->predecessor_id, MENURESULT_CLOSE);
	  case MENUTOKEN_ENTER:
		return menuitem_successor2menuresult(
			item->successor_id, MENURESULT_CLOSE);
	  case MENUTOKEN_UP:
	  case MENUTOKEN_RIGHT:
	  	/* Wrap slider around if max value is reached.
		 * Note: The max value is actually reached,
		 * because of min(maxvalue, value + stepsize) below.
		 * Wrapping then happens on the next key press.
		 */
		if ((!(keymask & (MENUTOKEN_LEFT | MENUTOKEN_DOWN))) &&
		    (item->data.slider.value == item->data.slider.maxvalue))
			item->data.slider.value = item->data.slider.minvalue;
		else
		  	item->data.slider.value = min(item->data.slider.maxvalue,
	  			item->data.slider.value + item->data.slider.stepsize);
		if (item->event_func)
			item->event_func(item, MENUEVENT_PLUS);
	  	return MENURESULT_NONE;
	  case MENUTOKEN_DOWN:
	  case MENUTOKEN_LEFT:
		if ((!(keymask & (MENUTOKEN_RIGHT | MENUTOKEN_UP))) &&
		    (item->data.slider.value == item->data.slider.minvalue))
			item->data.slider.value = item->data.slider.maxvalue;
		else
	  		item->data.slider.value = max(item->data.slider.minvalue,
	  			item->data.slider.value - item->data.slider.stepsize);
		if (item->event_func)
			item->event_func(item, MENUEVENT_MINUS);
	  	return MENURESULT_NONE;
	  case MENUTOKEN_OTHER:
          default:
		break;
	}

	return MENURESULT_ERROR;
}

MenuResult menuitem_process_input_numeric(MenuItem *item, MenuToken token, const char *key, unsigned int keymask)
{
	char buf1[MAX_NUMERIC_LEN];
	char buf2[MAX_NUMERIC_LEN];

	int max_len;

	debug(RPT_DEBUG, "%s(item=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"), token, key);

	if (item != NULL) {
		/* To make life easy... */
		char *str = item->data.numeric.edit_str;
		int pos = item->data.numeric.edit_pos;
		int allow_signed = (item->data.numeric.minvalue < 0);
		char *format_str = (allow_signed) ? "%+d" : "%d";

		snprintf(buf1, MAX_NUMERIC_LEN, format_str, item->data.numeric.minvalue);
		snprintf(buf2, MAX_NUMERIC_LEN, format_str, item->data.numeric.maxvalue);

		max_len = max(strlen(buf1), strlen(buf2));

		/* Clear the error */
		item->data.numeric.error_code = 0;

		switch (token) {
		  case MENUTOKEN_MENU:
		  	if (pos == 0) {
				return menuitem_predecessor2menuresult(
					item->predecessor_id, MENURESULT_CLOSE);
			}
			else {
				/* Reset data */
				menuitem_reset_numeric(item);
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_ENTER:
			if ((keymask & MENUTOKEN_RIGHT) || (str[pos] == '\0')) {
				int value;
				/* The user completed his input */

				/* ...scan it */
				if (sscanf(str, "%d", &value) != 1) {
					return MENURESULT_ERROR;
				}
				/* Test the value */
				if ((value < item->data.numeric.minvalue) ||
				    (value > item->data.numeric.maxvalue)) {
					/* Out of range !
					 * We can't exit this screen now
					 */
					item->data.numeric.error_code = 1;
					item->data.numeric.edit_pos = 0;
					item->data.numeric.edit_offs = 0;
					return MENURESULT_NONE;
				}

				/* OK, store value */
				item->data.numeric.value = value;

				/* Inform client */
				if (item->event_func)
					item->event_func(item, MENUEVENT_UPDATE);

				return menuitem_successor2menuresult(
					item->successor_id, MENURESULT_CLOSE);
			}
			else {
				/* The user wants to go to next digit */
				if (pos < max_len) {
					item->data.numeric.edit_pos++;
					if (pos >= display_props->width - 2)
						item->data.numeric.edit_offs++;
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_UP:
			if (pos >= max_len) {
				/* We're not allowed to add anything anymore */
				item->data.numeric.error_code = 2;
				item->data.numeric.edit_pos = 0;
				item->data.numeric.edit_offs = 0;
				return MENURESULT_NONE;
			}
			if (allow_signed && pos == 0) {
				/* make negative */
				str[0] = (str[0] == '-') ? '+' : '-';
			}
			else {
				if (str[pos] >= '0' && str[pos] < '9') {
					str[pos] ++;
				} else if (str[pos] == '9') {
					str[pos] = '\0';
				} else if (str[pos] == '\0') {
					str[pos] = '0';
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_DOWN:
			if (pos >= max_len) {
				/* We're not allowed to add anything anymore */
				item->data.numeric.error_code = 2;
				item->data.numeric.edit_pos = 0;
				item->data.numeric.edit_offs = 0;
				return MENURESULT_NONE;
			}
			if (allow_signed && pos == 0) {
				/* make negative */
				str[0] = (str[0] == '-') ? '+' : '-';
			}
			else {
				if (str[pos] > '0' && str[pos] <= '9') {
					str[pos] --;
				} else if (str[pos] == '0') {
					str[pos] = '\0';
				} else if (str[pos] == '\0') {
		  			str[pos] = '9';
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_RIGHT:
			/* The user wants to go to next digit */
			if (str[pos] != '\0' && pos < max_len) {
				item->data.numeric.edit_pos++;
				if (pos >= display_props->width - 2)
					item->data.numeric.edit_offs++;
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_LEFT:
			/* The user wants to go to back a digit */
			if (pos > 0) {
				item->data.numeric.edit_pos--;
				if (item->data.numeric.edit_offs > item->data.numeric.edit_pos)
					item->data.numeric.edit_offs = item->data.numeric.edit_pos;
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_OTHER:
			if (pos >= max_len) {
				/* We're not allowed to add anything anymore */
				item->data.numeric.error_code = 2;
				item->data.numeric.edit_pos = 0;
				item->data.numeric.edit_offs = 0;
				return MENURESULT_NONE;
			}
			/* process numeric keys */
  			if ((strlen(key) == 1) && isdigit(key[0])) {
				str[pos] = key[0];
				item->data.numeric.edit_pos++;
				if (pos >= display_props->width - 2)
					item->data.numeric.edit_offs++;
			}
		  default:
			return MENURESULT_NONE;
		}
	}
	return MENURESULT_ERROR;
}

MenuResult menuitem_process_input_alpha(MenuItem *item, MenuToken token, const char *key, unsigned int keymask)
{
	char *p;
	static char *chars = NULL;

	debug(RPT_DEBUG, "%s(item=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"), token, key);

	if (item != NULL) {
		/* To make life easy... */
		char *str = item->data.alpha.edit_str;
		int pos = item->data.alpha.edit_pos;

		/* Create list of allowed chars */
		chars = realloc(chars, 26 + 26 + 10 + strlen(item->data.alpha.allowed_extra) + 1);
		chars[0] = '\0'; /* clear string */
		if (item->data.alpha.allow_caps)
			strcat(chars, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		if (item->data.alpha.allow_noncaps)
			strcat(chars, "abcdefghijklmnopqrstuvwxyz");
		if (item->data.alpha.allow_numbers)
			strcat(chars, "0123456789");
		strcat(chars, item->data.alpha.allowed_extra);

		/* Clear the error */
		item->data.alpha.error_code = 0;

		switch (token) {
		  case MENUTOKEN_MENU:
	  		if (pos == 0) {
				return menuitem_predecessor2menuresult(
					item->predecessor_id, MENURESULT_CLOSE);
			}
			else {
				/* Reset data */
				menuitem_reset_alpha(item);
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_ENTER:
			if ((keymask & MENUTOKEN_RIGHT) || (str[item->data.alpha.edit_pos] == '\0')) {
				/* The user completed his input */

				/* It's not too short ? */
				if (strlen(item->data.alpha.edit_str) < item->data.alpha.minlength) {
					item->data.alpha.error_code = 3;
					return MENURESULT_NONE;
				}

				/* Store value */
				strcpy(item->data.alpha.value, item->data.alpha.edit_str);

				/* Inform client */
				if (item->event_func)
					item->event_func(item, MENUEVENT_UPDATE);

				return menuitem_successor2menuresult(
					item->successor_id, MENURESULT_CLOSE);
			}
			else {
				/* The user wants to go to next digit */
				if (pos < item->data.alpha.maxlength) {
					item->data.alpha.edit_pos++;
					if (pos >= display_props->width - 2)
						item->data.alpha.edit_offs++;
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_UP:
			if (pos >= item->data.alpha.maxlength) {
				/* We're not allowed to add anything anymore */
				item->data.alpha.error_code = 2;
				item->data.alpha.edit_pos = 0;
				item->data.alpha.edit_offs = 0;
				return MENURESULT_NONE;
			}
			if (str[pos] == '\0') {
				/* User goes past EOL */
				str[pos] = chars[0];
			} else {
				/* We should have a symbol from our list */
				p = strchr(chars, str[pos]);
				if (p != NULL) {
					str[pos] = *(++p); /* next symbol on list */
					/* Might be '\0' now */
				} else {
					str[pos] = '\0';
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_DOWN:
			if (pos >= item->data.alpha.maxlength) {
				/* We're not allowed to add anything anymore */
				item->data.alpha.error_code = 2;
				item->data.alpha.edit_pos = 0;
				item->data.alpha.edit_offs = 0;
				return MENURESULT_NONE;
			}
			if (str[pos] == '\0') {
				/* User goes past EOL */
				str[pos] = chars[strlen(chars)-1];
			} else {
				/* We should have a symbol from our list */
				p = strchr(chars, str[pos]);
				if ((p != NULL) && (p != chars)) {
					str[pos] = *(--p); /* previous symbol on list */
				} else {
					str[pos] = '\0';
				}
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_RIGHT:
			/* The user wants to go to next digit */
			if (str[item->data.alpha.edit_pos] != '\0' &&
			    pos < item->data.alpha.maxlength - 1) {
				item->data.alpha.edit_pos++;
				if (pos >= display_props->width - 2)
					item->data.alpha.edit_offs++;
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_LEFT:
			/* The user wants to go to back a digit */
			if (pos > 0) {
				item->data.alpha.edit_pos--;
				if (item->data.alpha.edit_offs > item->data.alpha.edit_pos)
					item->data.alpha.edit_offs = item->data.alpha.edit_pos;
			}
			return MENURESULT_NONE;
		  case MENUTOKEN_OTHER:
			if (pos >= item->data.alpha.maxlength) {
				/* We're not allowed to add anything anymore */
				item->data.alpha.error_code = 2;
				item->data.alpha.edit_pos = 0;
				item->data.alpha.edit_offs = 0;
				return MENURESULT_NONE;
			}
			/* process other keys */
  			if ((strlen(key) == 1) && (key[0] >= ' ') && (strchr(chars, key[0]) != NULL)) {
				str[pos] = key[0];
				item->data.alpha.edit_pos++;
				if (pos >= display_props->width - 2)
					item->data.alpha.edit_offs++;
			}
		  default:
			return MENURESULT_NONE;
		}
	}
	return MENURESULT_ERROR;
}


MenuResult menuitem_process_input_ip(MenuItem *item, MenuToken token, const char *key, unsigned int keymask)
{
	/* To make life easy... */
	char *str = item->data.ip.edit_str;
	char numstr[5];
	int  num;
	int pos = item->data.ip.edit_pos;
	const IpSstringProperties *ipinfo = (item->data.ip.v6) ? &IPinfo[1] : &IPinfo[0];

	debug(RPT_DEBUG, "%s(item=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((item != NULL) ? item->id : "(null)"), token, key);

	/* Clear the error */
	item->data.ip.error_code = 0;

	switch (token) {
		  case MENUTOKEN_MENU:
		  	if (pos == 0) {
				return menuitem_predecessor2menuresult(
					item->predecessor_id, MENURESULT_CLOSE);
			}
			else {
				/* Reset data */
				menuitem_reset_ip(item);
			}
			return MENURESULT_NONE;
		case MENUTOKEN_ENTER:
			if ((keymask & MENUTOKEN_RIGHT) || (pos >= item->data.ip.maxlength - 1)) {
				// remove the leading spaces/zeros in each octet-representing string
				char tmp[40];	// 40 = max. length of IPv4 & IPv6 addresses incl. '\0'
				char *start = tmp;

				memccpy(tmp, str, '\0', sizeof(tmp));

				while (start != NULL) {
					char *skip = start;

					while ((*skip == ' ') || ((*skip == '0') && (skip[1] != ipinfo->sep) && (skip[1] != '\0')))
						skip++;
					memccpy(start, skip, '\0', item->data.ip.maxlength + 1);
					skip = strchr(start, ipinfo->sep);
					start = (skip != NULL) ? (skip + 1) : NULL;
				}

				// check IP address entered
				if ((ipinfo->verify != NULL) && (!ipinfo->verify(tmp))) {
					report(RPT_WARNING, "%s(id=\"%s\") ip address not verified: \"%s\"",
					       __FUNCTION__, item->id, tmp);
					item->data.ip.error_code = 4;
					return MENURESULT_NONE;
				}

				// store value
				strcpy(item->data.ip.value, tmp);

				// Inform client
				if (item->event_func)
					item->event_func(item, MENUEVENT_UPDATE);

				return menuitem_successor2menuresult(
					item->successor_id, MENURESULT_CLOSE);
			}
			else {
				item->data.ip.edit_pos++;
				if (str[item->data.ip.edit_pos] == ipinfo->sep)
					item->data.ip.edit_pos++;
				while (item->data.ip.edit_pos - item->data.ip.edit_offs > display_props->width - 2)
					item->data.ip.edit_offs++;
				return MENURESULT_NONE;
			}
		case MENUTOKEN_UP:
			// convert string starting at the beginning / previous dot into a number
			num = (int) strtol(&str[pos - (pos % (ipinfo->width + 1))], (char **) NULL, ipinfo->base);
			// increase the number depending on the position
			num += ipinfo->posValue[(pos - (pos / (ipinfo->width + 1))) % ipinfo->width];
			// wrap around upper limit
			if (num > ipinfo->limit)
				num = 0;
			snprintf(numstr, 5, ipinfo->format, num);
			memcpy(&str[pos - (pos % (ipinfo->width + 1))], numstr, ipinfo->width);
			return MENURESULT_NONE;
		case MENUTOKEN_DOWN:
			// convert string starting at the beginning / previous dot into a number
			num = (int) strtol(&str[pos - (pos % (ipinfo->width + 1))], (char **) NULL, ipinfo->base);
			// decrease the number depending on the position
			num -= ipinfo->posValue[(pos - (pos / (ipinfo->width + 1))) % ipinfo->width];
			// wrap around lower limit 0
			if (num < 0)
				num = ipinfo->limit;
			snprintf(numstr, 5, ipinfo->format, num);
			memcpy(&str[pos - (pos % (ipinfo->width + 1))], numstr, ipinfo->width);
			return MENURESULT_NONE;
		case MENUTOKEN_RIGHT:
			if (pos < item->data.ip.maxlength - 1) {
				item->data.ip.edit_pos++;
				if (str[item->data.ip.edit_pos] == ipinfo->sep)
					item->data.ip.edit_pos++;
				while (item->data.ip.edit_pos - item->data.ip.edit_offs > display_props->width - 2)
					item->data.ip.edit_offs++;
			}
			return MENURESULT_NONE;
		case MENUTOKEN_LEFT:
			/* The user wants to go to back a digit */
			if (pos > 0) {
				item->data.ip.edit_pos--;
				if (str[item->data.ip.edit_pos] == ipinfo->sep)
					item->data.ip.edit_pos--;
				if (item->data.ip.edit_offs > item->data.ip.edit_pos)
					item->data.ip.edit_offs = item->data.ip.edit_pos;
			}
			return MENURESULT_NONE;
		case MENUTOKEN_OTHER:
			/* process other keys */
  			if ((strlen(key) == 1) &&
			    ((item->data.ip.v6) ? isxdigit(key[0]) : isdigit(key[0]))) {
				str[pos] = tolower(key[0]);
				if (pos < item->data.ip.maxlength - 1) {
					item->data.ip.edit_pos++;
					if (str[item->data.ip.edit_pos] == ipinfo->sep)
						item->data.ip.edit_pos++;
					while (item->data.ip.edit_pos - item->data.ip.edit_offs > display_props->width - 2)
						item->data.ip.edit_offs++;
				}
			}
			/* FALLTHROUGH */
		default:
			return MENURESULT_NONE;
	}
	/* NOTREACHED */
	return MENURESULT_ERROR;
}

/**
 * get the Client that owns item. extracted from menu_commands_handler().
 */
Client *menuitem_get_client(MenuItem *item)
{
        return item->client;
}

LinkedList *tablist2linkedlist(char *strings)
{
	LinkedList *list;

	list = LL_new();

	/* Parse strings */
	if (strings != NULL) {
		char *p = strings;
		char *tabptr, *new_s;

		while ((tabptr = strchr(p, '\t')) != NULL) {
			int len = (int)(tabptr - p);

			/* Alloc and copy substring */
			new_s = malloc(len + 1);
			if (new_s != NULL) {
				strncpy(new_s, p, len);
				new_s[len] = 0;

				LL_Push(list, new_s);
			}

			/* Go to next string */
			p = tabptr + 1;
		}
		/* Add last string */
		new_s = strdup(p);
		if (new_s != NULL)
			LL_Push(list, new_s);
	}

	return list;
}

MenuItemType menuitem_typename_to_type(char *name)
{
	if (name != NULL) {
		MenuItemType type;

		for (type = 0; type < NUM_ITEMTYPES; type ++) {
			if (strcmp(menuitemtypenames[type], name) == 0) {
				return type;
			}
		}
	}
	return MENUITEM_INVALID;
}

char *menuitem_type_to_typename(MenuItemType type)
{
	return ((type >= 0 && type < NUM_ITEMTYPES)
		? menuitemtypenames[type]
		: NULL);
}

MenuEventType menuitem_eventtypename_to_eventtype(char *name)
{
	if (name != NULL) {
		MenuEventType type;

		for (type = 0; type < NUM_EVENTTYPES; type ++) {
			if (strcmp(menueventtypenames[type], name) == 0) {
				return type;
			}
		}
	}
	return MENUEVENT_INVALID;
}

char *menuitem_eventtype_to_eventtypename(MenuEventType type)
{
	return ((type >= 0 && type < NUM_EVENTTYPES)
		? menueventtypenames[type]
		: NULL);
}

