/** \file clients/lcdexec/menu.h
 * Declare constants, data types and functions for menu functions in clients/lcdexec/menu.c.
 */

/* This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006, Peter Marschall
 *
 */

#ifndef LCDEXEC_MENU_H
#define LCDEXEC_MENU_H

#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif

/**  Symbolic names for the types of a MenuEntry */
typedef enum {
	unknown = 0,		/**< Unknown MenuEntry type. */
	menu = 1,		/**< MenuEntry representing a menu. */
	exec = 2,		/**< MenuEntry representing an executable command. */
} MenuType;	


typedef struct menu_entry {
	char *name;		/**< Name of the menu entry (from section name). */
	char *displayname;	/**< isible name of the entry. */
	int id;			/**< Internal ID of the entry. */
	MenuType type;		/**< Type of the entry. */

	// variables necessary for type menu
	struct menu_entry *entries;	/**< Subordinate menu entries (for MenuType \c menu). */
	struct menu_entry *next;	/**< Next sibling menu entry (for MenuType \c menu). */

	// variables necessary for type exec
	char *command;			/**< Command to execute (for MenuType \c exec). */
} MenuEntry;


MenuEntry *menu_read(MenuEntry *parent, const char *name);
int menu_sock_send(MenuEntry *me, MenuEntry *parent, int sock);
MenuEntry *menu_find_by_id(MenuEntry *me, int id);
const char *menu_command(MenuEntry *me);
void menu_free(MenuEntry *me);
void menu_dump(MenuEntry *me);

#endif
