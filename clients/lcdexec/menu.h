/*
 * lcdexec.c
 * This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#define MAX_NUM_MENUCMDS 40
#define MAX_NUM_SUBMENUS 20

typedef struct Menu {
	char * menucmd_name[MAX_NUM_MENUCMDS];
	char * menucmd_exec[MAX_NUM_MENUCMDS];
	int num_menucmds;
	char * submenu_name[MAX_NUM_SUBMENUS];
	char * submenu_id[MAX_NUM_SUBMENUS];
	struct Menu * submenu[MAX_NUM_SUBMENUS];
	int num_submenus;
} Menu;

Menu *	menu_read (char * menu_id, char * progname);
int	menu_send_to_LCDd( Menu * menu, char * id, int sock );
char *	menu_find_cmd_of_id( Menu * menu, char * id );

int split( char * str, char delim, char * parts[], int maxparts );
