/** \file server/menuitem.h
 * Defines all the menuitem data and actions.
 *
 * There are a few different menuitems:
 * - action
 * - checkbox (on/off and optionally open)
 * - slider (the user can increase/decrease a value)
 * - numeric input
 * - alphanumeric input (in short: alpha)
 * - menu (a menu is a menuitem itself too)
 *
 * The slider, numeric & string input and menu have their own screen,
 * that comes to front when the items are selected.
 * One menuitem is in a different file: Menu data is in menu.h.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 */

#ifndef MENUITEM_H
#define MENUITEM_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif
#include "shared/defines.h"

#include "shared/LL.h"

/*********************************************************************
 * Data definitions of the menustuff
 */

/** These values are used in the function tables in menuitem.c ! */
typedef enum MenuItemType {
	MENUITEM_INVALID  = -1,
	MENUITEM_MENU     = 0,
	MENUITEM_ACTION   = 1,
	MENUITEM_CHECKBOX = 2,
	MENUITEM_RING     = 3,
	MENUITEM_SLIDER   = 4,
	MENUITEM_NUMERIC  = 5,
	MENUITEM_ALPHA    = 6,
	MENUITEM_IP       = 7,
        NUM_ITEMTYPES = 8
} MenuItemType;

typedef enum CheckboxValue {
	CHECKBOX_OFF = 0, CHECKBOX_ON, CHECKBOX_GRAY
} CheckboxValue;

/** Recognized input token codes: they need to be bit values */
typedef enum MenuToken {
	MENUTOKEN_NONE  = 0x0000,	/**< no key */
	MENUTOKEN_MENU  = 0x0001,	/**< MenuKey */
	MENUTOKEN_ENTER = 0x0002,	/**< EnterKey */
	MENUTOKEN_UP    = 0x0004,	/**< UpKey */
	MENUTOKEN_DOWN  = 0x0008,	/**< DownKey */
	MENUTOKEN_LEFT  = 0x0010,	/**< LeftKey */
	MENUTOKEN_RIGHT = 0x0020,	/**< RightKey */
	MENUTOKEN_OTHER = 0x1000	/**< any other key */
} MenuToken;

/** Return codes from an input handler */
typedef enum MenuResult {
	MENURESULT_ERROR = -1,	/**< Something has gone wrong */
	MENURESULT_NONE  = 0,	/**< Token handled OK, no extra action */
	MENURESULT_ENTER,	/**< Token handled OK, enter the selected
				 * menuitem now */
	MENURESULT_CLOSE,	/**< Token handled OK, close the current
				 * menuitem now */
	MENURESULT_QUIT,	/**< Token handled OK, close ALL menus now */
	MENURESULT_PREDECESSOR, /**< Token handled OK, goto registered
				 * predecessor */
	MENURESULT_SUCCESSOR    /**< Token handled OK, goto registered
				 * successor */
} MenuResult;

/** Events caused by a menuitem */
typedef enum MenuEventType {
	MENUEVENT_INVALID = -1,	/**< Special value for error handling */
	MENUEVENT_SELECT = 0,	/**< Item has been selected
				 * (action chosen) */
	MENUEVENT_UPDATE = 1,	/**< Item has been modified
				 * (checkbox, numeric, alphanumeric) */
	MENUEVENT_PLUS   = 2,	/**< Item has been modified in positive direction
				 * (slider moved) */
	MENUEVENT_MINUS  = 3,	/**< Item has been modified in negative direction
				 * (slider moved) */
	MENUEVENT_ENTER  = 4,	/**< Menu has been entered */
	MENUEVENT_LEAVE  = 5,	/**< Menu has been left */
	NUM_EVENTTYPES   = 6
} MenuEventType;

#define MenuEventFunc(f) int (f) (struct MenuItem *item, MenuEventType event)

/** I've used a union in the struct below. Why? And why not for Widget?
 *
 * There are different types of menuitems. There are also types of widgets.
 * Menuitems have, just like widgets, different datafields per subtype.
 * The difference is that menuitems have, unlike widgets _many__different_
 * attributes. Widgets share many attributes like x, y, text.
 * The code would become unreadable if we used the 'widget way', or it would
 * get large if we define datafields that we use for only one type of
 * menuitem. (Joris)
 */
typedef struct MenuItem {
	MenuItemType type;	/**< Type as defined above */
	char *id;		/**< Internal name for client supplied menus */
	char *successor_id;	/**< next menuitem after hitting "Enter" on
				 * this one. (Special values are "_quit_",
				 * "_close_", "_none_"). */
	char *predecessor_id;	/**< next menuitem after hitting "Escape" on
				 * this one. (Special values are "_quit_",
				 * "_close_", "_none_"). */
	struct MenuItem *parent; /**< Parent of this menuitem */
	MenuEventFunc (*event_func);
			/**< Defines event_func to be an event function */
	char *text;	/**< Visible name of the item */
	void* client;	/**< The owner of this menuitem. */
	bool is_hidden; /**< If the item currently should not appear in a menu. */
	union data {
		struct menu {
			int selector_pos;	/**< At what menuitem is the
						   selector (0 for first) */
			int scroll;		/**< How much has the menu been
						   scrolled down */
			void *association;      /**< To associate an object
                                                   with this menu */
			LinkedList *contents;	/**< What's in this menu */
		} menu;
		struct action {
			/* nothing */
		} action;
		struct checkbox {
			bool allow_gray;	/**< Is CHECKBOX_GRAY allowed ? */
			CheckboxValue value;	/**< Current value */
		} checkbox;
		struct ring {
			LinkedList *strings;	/**< The selectable strings */
			short value;		/**< Current index */
		} ring;
		struct slider {
			char *mintext;		/**< Text at minimal value */
			char *maxtext;		/**< Text at minimal value */
			int minvalue;
			int maxvalue;
			int stepsize;
			int value;		/**< Current value */
		} slider;
		struct numeric {
			int maxvalue;
			int minvalue;
			//short allowed_decimals; /**< Number of numbers behind dot */
			int value;		/**< Current value */
			char *edit_str;		/**< Value while being edited */
			short edit_pos;		/**< Position while editing */
			short edit_offs;	/**< Offset while editing */
			short error_code;
		} numeric;
		struct alpha {
			char password_char;	/**< For passwords */
			short minlength;
			short maxlength;
			bool allow_caps;	/**< Caps allowed ? */
			bool allow_noncaps;	/**< Non-caps allowed ? */
			bool allow_numbers;	/**< Numbers allowed ? */
			char *allowed_extra;	/**< Allowed extra characters */
			char *value;		/**< Current value */
			char *edit_str;		/**< Value while being edited */
			short edit_pos;		/**< Position while editing */
			short edit_offs;	/**< Offset while editing */
			short error_code;
		} alpha;
		struct ip {
			char *value;		/**< Current value */
			char *edit_str;		/**< Value while being edited */
			short maxlength;
			bool v6;		/**< true if editing ipv6 addr */
			short edit_pos;		/**< Position while editing */
			short edit_offs;	/**< Offset while editing */
			short error_code;
		} ip;
	} data;
} MenuItem;


/*********************************************************************
 * Functions to use the menustuff
 */

/** translates a predecessor_id into a MenuResult. */
MenuResult menuitem_predecessor2menuresult(char *predecessor_id, MenuResult default_result);

/** translates a successor_id into a MenuResult. */
MenuResult menuitem_successor2menuresult(char *successor_id, MenuResult default_result);

MenuItem *menuitem_search(char *menu_id, Client *client);

/** YOU SHOULD NOT CALL THIS FUNCTION BUT THE TYPE SPECIFIC ONE INSTEAD */
MenuItem *menuitem_create(MenuItemType type, char *id,
		MenuEventFunc(*event_func), char *text, Client *client);

/* For all constructor functions below the following:
 *
 * id:		internal name of the item. Never visible. String will be
 *		copied.
 * event_func:	the event function that should be called upon actions on this
 *		item.
 * text:	the displayed text.
 *
 * All strings will be copied !
 *
 * Return value: the new item, or NULL on error.
 *
 * To create a Menu (which is also an ItemType), call menu_create.
 *
 */

/** Creates a an action item (a string only).  Generated events:
 * MENUEVENT_SELECT when user selects the item.
 */
MenuItem *menuitem_create_action(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, MenuResult menu_result);

/** Creates a checkbox.
 * Generated events: MENUEVENT_UPDATE when user changes value (immediately).
 */
MenuItem *menuitem_create_checkbox(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, bool allow_gray, bool value);

/** Creates a ring with the given string, separated by tabs.
 * value is the (initial) index in the strings.
 * eg: if strings="abc\\tdef" the value=1 means that "def" is selected.
 * Generated events: MENUEVENT_UPDATE when user changes value (immediately).
 */
MenuItem *menuitem_create_ring(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char *strings, short value);

/** Creates a slider with the given min and max values.
 * If the display is big enough the mintext and maxtext will be placed
 * at the end positions of the slider.
 * You can set the step size. Make it 0 to disable the automatic value chaning,
 * and update the value yourself.
 * MENUEVENT_PLUS, MENUEVENT_MINUS when slider is moved (immediately).
 */
MenuItem *menuitem_create_slider(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char *mintext, char *maxtext,
	int minvalue, int maxvalue, int stepsize, int value);

/** Creates a numeric value box.
 * Value can range from minvalue to maxvalue.
 * MENUEVENT_UPDATE when user finishes the value (no immediate update).
 */
MenuItem *menuitem_create_numeric(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, int minvalue, int maxvalue, int value);

/** Creates a string value box.
 * Value should have given minimal and maximal length. You can set whether
 * caps, non-caps and numbers are allowed. Also you can alow other characters.
 * If password char is non-zero, you will only see this char, not the actual
 * input.
 * MENUEVENT_UPDATE when user finishes the value (no immediate update).
 */
MenuItem *menuitem_create_alpha(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, char password_char, short minlength, short maxlength,
	bool allow_caps, bool allow_noncaps, bool allow_numbers,
	char *allowed_extra, char *value);

/** Creates an ip value box.  can be either v4 or v6
 * MENUEVENT_UPDATE when user finishes the value (no immediate update).
 */
MenuItem *menuitem_create_ip(char *id, MenuEventFunc(*event_func),
	char *text, Client *client, bool v6, char *value);

/** Deletes item from memory.
 * All allocated extra data (like strings) will be freed.
 */
void menuitem_destroy(MenuItem *item);

/** Resets the item to the initial state.
 * You should call menuitem_update after this to see the effects.
 * This call is useless on items that have immediate effect, like a slider.
 * Those items do not keep temporary data.
 */
void menuitem_reset(MenuItem *item);

/** (Re)builds the selected menuitem on screen using widgets.
 * Should be re-called if menuitem data has been changed.
 * There are a few (logical) exceptions to this:
 * - the values
 * - the menu scroll and menu index
 */
void menuitem_rebuild_screen(MenuItem *item, Screen *s);

/** Updates the widgets of the selected menuitem
 * Fills all widget attributes with the corrrect values.
 */
void menuitem_update_screen(MenuItem *item, Screen *s);

/** Does something with the given input.
 * key is only used if token is MENUTOKEN_OTHER.
 */
MenuResult menuitem_process_input(MenuItem *item, MenuToken token, const char *key, unsigned int keymask);

/** returns the Client that owns the MenuItem. item must not be null */
Client *menuitem_get_client(MenuItem *item);

/** Converts a tab-separated list to a LinkedList. */
LinkedList *tablist2linkedlist(char *strings);

MenuItemType menuitem_typename_to_type(char *name);

char *menuitem_type_to_typename(MenuItemType type);

MenuEventType menuitem_eventtypename_to_eventtype(char *name);

char *menuitem_eventtype_to_eventtypename(MenuEventType type);

#endif
