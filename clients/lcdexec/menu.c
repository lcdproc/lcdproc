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

#include "menu.h"

Menu * menu_find_submenu( Menu * menu, char * submenu_id );

Menu * menu_read( char * menu_id, char * progname )
{
	char buf[100];
	char * str;
	char * parts[3];
	int i;

	Menu * menu;

	report( RPT_DEBUG, "Reading menu: [%s]", menu_id );

	menu = malloc( sizeof(Menu) );

	/* Read the commands */
	str = "";
	while( str && menu->num_menucmds < MAX_NUM_MENUCMDS ) {

		snprintf( buf, sizeof(buf)-1, "%s%sMenuCommand",
				menu_id, menu_id[0]?"_":"" );
		buf[sizeof(buf)-1] = 0;
		str = config_get_string( progname, buf, menu->num_menucmds, NULL );
		if( !str ) {
			; /* No more menucommands */
		} else {
			char * str2 = strdup( str );
			if( split( str2, ',', parts, 2 ) == 2 ) {
				menu->menucmd_name[menu->num_menucmds] = parts[0];
				while( parts[1][0] == ' ' ) parts[1]++; /* Skip spaces */
				menu->menucmd_exec[menu->num_menucmds] = parts[1];
				report( RPT_DEBUG, "Found command: [%s] [%s]", parts[0], parts[1] );
			} else {
				report( RPT_ERR, "Cannot read MenuCommand: \"%s\"", str );
				return NULL;
			}
			menu->num_menucmds ++;
		}
	}

	/* Read the submenus */
	str = "";
	while( str && menu->num_submenus < MAX_NUM_SUBMENUS ) {

		snprintf( buf, sizeof(buf)-1, "%s%sSubmenu",
				menu_id, menu_id[0]?"_":"" );
		buf[sizeof(buf)-1] = 0;
		str = config_get_string( progname, buf, menu->num_submenus, NULL );
		if( !str ) {
			; /* No more submenus */
		} else {
			char * str2 = strdup( str );
			if( split( str2, ',', parts, 2 ) == 2 ) {
				menu->submenu_name[menu->num_submenus] = parts[0];
				while( parts[1][0] == ' ' ) parts[1]++; /* Skip spaces */
				menu->submenu_id[menu->num_submenus] = parts[1];
				report( RPT_DEBUG, "Found submenu: [%s] [%s]", parts[0], parts[1] );
			} else {
				report( RPT_ERR, "Cannot read Submenu: \"%s\"", str );
				return NULL;
			}
			menu->num_submenus ++;
		}
	}

	/* And do the same for all submenus */
	for( i = 0; i < menu->num_submenus; i++ ) {
		menu->submenu[i] = menu_read (menu->submenu_id[i], progname );
	}

	return menu;
}

int menu_send_to_LCDd( Menu * menu, char * id, int sock )
{
	int i;
	char buf[100];

	for( i = 0; i < menu->num_menucmds; i++ ) {
		snprintf( buf, sizeof(buf)-1,
			"menu_add_item \"%s\" \"%s%s%d\" action \"%s\"\n",
			id, id, id[0]?"_":"", i, menu->menucmd_name[i] );
		buf[sizeof(buf)-1] = 0;
		if( sock_send_string( sock, buf ) < 0 )
			return -1;
		snprintf( buf, sizeof(buf)-1,
			"menu_set_item \"%s\" \"%s%s%d\" -menu_result quit\n",
			id, id, id[0]?"_":"", i );
		buf[sizeof(buf)-1] = 0;
		if( sock_send_string( sock, buf ) < 0 )
			return -1;
	}
	for( i = 0; i < menu->num_submenus; i++ ) {
		snprintf( buf, sizeof(buf)-1,
			"menu_add_item \"%s\" \"%s\" menu \"%s\"\n",
			id, menu->submenu_id[i], menu->submenu_name[i] );
		buf[sizeof(buf)-1] = 0;
		if( sock_send_string( sock, buf ) < 0 )
			return -1;
	}

	/* And do the same for all submenus */
	for( i = 0; i < menu->num_submenus; i++ ) {
		if( menu_send_to_LCDd( menu->submenu[i], menu->submenu_id[i], sock ) < 0 )
			return -1;
	}
	return 0;
}

char * menu_find_cmd_of_id( Menu * menu, char * id )
{
	char * p;
	char * submenu_id;
	char * item_id;
	int i;

	/* Find the submenu_id part of the id */
	submenu_id = strdup( id );
	p = strrchr( submenu_id, '_' );

	/* Do we need to search the submenus ? */
	if( p ) {
		/* There is a menu id prepended to the id */
		*p = 0; /* Crop the submenu_id string */
		item_id = p + 1;
		menu = menu_find_submenu( menu, submenu_id );
		if( !menu ) {
			report( RPT_WARNING, "Server reported an unknown id: %s", id );
			free( submenu_id );
			return NULL;
		}
	}
	else {
		item_id = id;
	}

	/* Determine the number at the end of the id */
	i = strtol( item_id, &p, 10 );
	if( *item_id != 0 && *p == 0
	&& i >= 0 && i < menu->num_menucmds ) {
		/* OK */
		free( submenu_id );
		return menu->menucmd_exec[i];
	} else {
		report( RPT_WARNING, "Server reported an unknown id: %s", id );
		free( submenu_id );
		return NULL;
	}

}

Menu * menu_find_submenu( Menu * menu, char * submenu_id )
{
	int i;
	Menu * found;

	/* Search for the submenu in this menu and all submenus */
	for( i = 0; i < menu->num_submenus; i++ ) {
		if( strcmp( menu->submenu_id[i], submenu_id ) == 0 )
			return menu->submenu[i];
		found = menu_find_submenu( menu->submenu[i], submenu_id );
		if( found )
			return found;
	}

	return NULL;
}

int split( char * str, char delim, char * parts[], int maxparts )
/* Splits a string into parts, to which pointers will be returned in &parts.
 * The return value is the number of parts.
 * maxparts is the maximum number of parts returned. If more parts exist
 * they are (unsplit) in the last part.
 * The parts are split at the character delim.
 * No new space will be allocated, the string str will be mutated !
 */
{
	char * p1 = str;
	char * p2;
	int part_nr = 0;

	/* Find the delim char to end the current part */
	while( part_nr < maxparts - 1 && (p2 = strchr( p1, delim )) ) {

		/* subsequent parts... */
		*p2 = 0;
		parts[part_nr] = p1;

		p1 = p2 + 1; /* Just after the delim char */
		part_nr ++;
	}
	/* and the last part... */
	parts[part_nr] = p1;
	part_nr ++;

	return part_nr;
}

