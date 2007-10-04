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


typedef enum {
	unknown = 0,		/**< unknown menu entry. */
	menu = 1,		/**< menu entry representing a menu. */
	exec = 2,		/**< menu entry repesenting a command. */
} MenuType;	


typedef struct menu_entry {
	char *name;		/**< Name of the menu entry. */
	char *displayname;	/**< Display name of the enty. */
	int id;			/**< Internal ID of the entry. */
	MenuType type;		/**< Type of the entry. */

	// variables necessary for type menu
	struct menu_entry *entries;	/**< Subordinate menu entries (for MenuType menu). */
	struct menu_entry *next;	/**< Next sibling menu entry (for MenuType menu). */

	// variables necessary for type exec
	char *command;			/**< Command to execute (for MenuType exec). */
} MenuEntry;


MenuEntry *menu_read(MenuEntry *parent, const char *name);
int menu_sock_send(MenuEntry *me, MenuEntry *parent, int sock);
MenuEntry *menu_find_by_id(MenuEntry *me, int id);
const char *menu_command(MenuEntry *me);
void menu_free(MenuEntry *me);
void menu_dump(MenuEntry *me);

#endif
