/** \file clients/lcdexec/menu.h
 * Declare constants, data types and functions for menu functions in clients/lcdexec/menu.c.
 */

/* This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006-7, Peter Marschall
 */

#ifndef LCDEXEC_MENU_H
#define LCDEXEC_MENU_H

/* boolean values */
#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif


/**  Symbolic names for the types of a MenuEntry */
typedef enum {
	MT_UNKNOWN      = 0x00,	/**< Unknown MenuEntry type. */
	MT_MENU         = 0x10,	/**< MenuEntry representing a menu. */
	MT_EXEC         = 0x20,	/**< MenuEntry representing an executable command. */
	MT_ARGUMENT     = 0x40,	/**< Mask denoting a parameter of any type */
	MT_AUTOMATIC	= 0x80,	/**< BitFlag denoting automatically generated entries */

	MT_ARG_SLIDER   = 0x41,	/**< MenuEntry representing a slider parameter. */
	MT_ARG_RING     = 0x42,	/**< MenuEntry representing a ring parameter. */
	MT_ARG_NUMERIC  = 0x43,	/**< MenuEntry representing a numeric input parameter. */
	MT_ARG_ALPHA    = 0x44,	/**< MenuEntry representing a alpha input parameter. */
	MT_ARG_IP       = 0x45,	/**< MenuEntry representing a IP input parameter. */
	MT_ARG_CHECKBOX = 0x46,	/**< MenuEntry representing a checkbox input parameter. */
	MT_ARG_ACTION   = 0x47,	/**< MenuEntry representing a checkbox input parameter. */
} MenuType;	


/** Data structure to hold a menu entry in \c lcdexec */
typedef struct menu_entry {
	// Variables necessary for multiple/all types
	char *name;		/**< Name of the menu entry (from section name). */
	char *displayname;	/**< Visible name of the entry. */
	int id;			/**< Internal ID of the entry. */
	MenuType type;		/**< Type of the entry. */
	struct menu_entry *parent;	/**< Parent menu entry. */
	int numChildren;		/**< # of child entries. */
	struct menu_entry *children;	/**< Subordinate menu entries (for type \c MT_MENU & \c MT_EXEC). */
	struct menu_entry *next;	/**< Next sibling menu entry (for type \c MT_MENU). */

	// Variables specific to one special type
	union data {
		struct exec{	// elements necessary for type MT_EXEC
			char *command;	/**< Command to execute. */
			int feedback;	/**< Feedback flag. */
		} exec;
		struct slider {	// elements necessary for type MT_ARG_SLIDER
			int value;	/**< Numeric value of slider. */
			int minval;	/**< Minimal allowed value. */
			int maxval;	/**< Maximal allowed value. */
			int stepsize;	/**< Increments/decrement for value. */
			char *mintext;	/**< Label for the min. value. */
			char *maxtext;	/**< Label for the max. value. */
		} slider;
		struct ring {	// elements necessary for type MT_ARG_RING
			int value;	/**< Index into list of alternatives. */
			char **strings;	/**< List of alternatives. */
		} ring;
		struct numeric {	// elements necessary for type MT_ARG_NUMERIC
			int value;	/**< Numeric input value. */
			int minval;	/**< Minimal allowed value. */
			int maxval;	/**< Maximal allowed value. */
		} numeric;
		struct alpha {	// elements necessary for type MT_ARG_ALPHA
			char *value;	/**< Text value. */
			int minlen;	/**< Maximal allowed length. */
			int maxlen;	/**< Minimal required length. */
			char *allowed;	/**< Characters allowed in input. */
		} alpha;
		struct ip {	// elements necessary for type MT_ARG_IP
			char *value;	/**< IP address value. */
			int v6;		/**< Flag: use IPv6 editing logic. */
		} ip;
		struct checkbox {	// elements necessary for type MT_ARG_CHECKBOX
			int value;	/**< Checkbox value. */
			int allow_gray;	/**< Flag for tristate chechboxes. */
			char *map[3];	/**< Replacement strings for each value. */
		} checkbox;
	} data;
} MenuEntry;


MenuEntry *menu_read(MenuEntry *parent, const char *name);
int menu_sock_send(MenuEntry *me, MenuEntry *parent, int sock);
MenuEntry *menu_find_by_id(MenuEntry *me, int id);
const char *menu_command(MenuEntry *me);
void menu_free(MenuEntry *me);
void menu_dump(MenuEntry *me);

#endif
