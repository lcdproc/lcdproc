/*
 * driver.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *
 *
 * This code does all actions on the driver object.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include <dlfcn.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "screenlist.h" /* for timer  */

#include "shared/report.h"
#include "configfile.h"

#include "widget.h"
#include "driver.h"
#include "drivers.h"
#include "drivers/lcd.h"
/* lcd.h is used for the driver API definition */


typedef struct driver_symbols {
	char *name;
	short offset; /* offset in Driver structure */
	short required;
} DriverSymbols;

DriverSymbols driver_symbols[] = {
	{ "api_version", offsetof(Driver, api_version), 1 },
	{ "stay_in_foreground", offsetof(Driver, stay_in_foreground), 1 },
	{ "supports_multiple", offsetof(Driver, supports_multiple), 1 },
	{ "symbol_prefix", offsetof(Driver, symbol_prefix), 1 },
	{ "init", offsetof(Driver, init), 1 },
	{ "close", offsetof(Driver, close), 1 },
	{ "width", offsetof(Driver, width), 0 },
	{ "height", offsetof(Driver, height), 0 },
	{ "clear", offsetof(Driver, clear), 0 },
	{ "flush", offsetof(Driver, flush), 0 },
	{ "string", offsetof(Driver, string), 0 },
	{ "chr", offsetof(Driver, chr), 0 },
	{ "vbar", offsetof(Driver, vbar), 0 },
	{ "hbar", offsetof(Driver, hbar), 0 },
	{ "num", offsetof(Driver, num), 0 },
	{ "heartbeat", offsetof(Driver, heartbeat), 0 },
	{ "icon", offsetof(Driver, icon), 0 },
	{ "cursor", offsetof(Driver, cursor), 0 },
	{ "set_char", offsetof(Driver, set_char), 0 },
	{ "get_free_chars", offsetof(Driver, get_free_chars), 0 },
	{ "cellwidth", offsetof(Driver, cellwidth), 0 },
	{ "cellheight", offsetof(Driver, cellheight), 0 },
	{ "get_contrast", offsetof(Driver, get_contrast), 0 },
	{ "set_contrast", offsetof(Driver, set_contrast), 0 },
	{ "get_brightness", offsetof(Driver, get_brightness), 0 },
	{ "set_brightness", offsetof(Driver, set_brightness), 0 },
	{ "backlight", offsetof(Driver, backlight), 0 },
	{ "output", offsetof(Driver, output), 0 },
	{ "get_key", offsetof(Driver, get_key), 0 },
	{ "get_info", offsetof(Driver, get_info), 0 },
	{ NULL, 0, 0 }
};



/* Functions for the driver */
static int request_display_width();
static int request_display_height();
static int driver_store_private_ptr(Driver * driver, void * private_data);


Driver *
driver_load( char * name, char * filename, char * args )
{
	Driver * driver = NULL;
	int res;

	report( RPT_INFO, "driver_load( name=\"%.40s\", filename=\"%.80s\", args=\"%.80s\")", name, filename, args );

	/* Allocate memory for new driver struct */
	driver = malloc( sizeof( Driver ));
	memset( driver, 0, sizeof (Driver));

	/* And store its name and filename */
	driver->name = malloc( strlen( name ) + 1 );
	strcpy( driver->name, name );
	driver->filename = malloc( strlen( filename ) + 1 );
	strcpy( driver->filename, filename );

	/* Load and bind the driver module and locate the symbols */
	if( driver_bind_module( driver ) < 0 ) {
		report( RPT_ERR, "Driver [%.40s] binding failed", name );
		free( driver->name );
		free( driver->filename );
		free( driver );
		return NULL;
	}

	/* Check module version */
	if( strcmp( *(driver->api_version), API_VERSION ) != 0 ) {
		report( RPT_ERR, "Driver [%.40s] is of an incompatible version", name );
		driver_unbind_module( driver );
		free( driver->name );
		free( driver->filename );
		free( driver );
		return NULL;
	}

	/* Call the init function */
	debug( RPT_DEBUG, "Calling driver [%.40s] init function", driver->name );
	res = driver->init( driver, args );
	if( res < 0 ) {
		report( RPT_ERR, "Driver [%.40s] init failed, return code < 0", driver->name );
		/* Driver load failed, driver should not be added to list
		 * Free driver structure again
		 */
		driver_unbind_module( driver );
		free( driver->name );
		free( driver->filename );
		free( driver );
		return NULL;
	}

	debug( RPT_NOTICE, "Driver [%.40s] loaded", driver->name );

	return driver;
}


int
driver_unload( Driver * driver )
{
	debug( RPT_NOTICE, "Closing driver [%.40s]", driver->name );
	if( driver->close )
		driver->close (driver);

	/* Unlaod the module */
	driver_unbind_module( driver );

	/* Free its data */
	free( driver->filename );
	free( driver->name );
	free( driver );
	debug( RPT_DEBUG, "Driver unloaded" );

	return 0;
}


int
driver_bind_module( Driver * driver )
{
	int i;
	int missing_symbols = 0;

	/* Load the module */
	driver->module_handle = dlopen( driver->filename, RTLD_NOW );
	if( driver->module_handle == NULL ) {
		report( RPT_ERR, "Could not dlopen driver module %.40s: %s", driver->filename, dlerror() );
		return -1;
	}

	/* And locate the symbols */
	for( i=0; driver_symbols[i].name; i++ ) {
		void (**p)();
		p = (void(**)() ) ((size_t)driver + (driver_symbols[i].offset));
		*p = NULL;

		/* Add the symbol_prefix */
		if( driver->symbol_prefix ) {
			char *s = malloc( strlen( *(driver->symbol_prefix) ) + strlen( driver_symbols[i].name ) + 1 );
			strcpy( s, *(driver->symbol_prefix) ) ;
			strcat( s, driver_symbols[i].name );
			debug( RPT_DEBUG, "finding symbol: %s", s );
			*p = dlsym( driver->module_handle, s );
			free( s );
		}
		/* Retrieve the symbol */
		if( !*p ) {
			debug( RPT_DEBUG, "finding symbol: %s", driver_symbols[i].name );
			*p = dlsym( driver->module_handle, driver_symbols[i].name );
		}

		if( *p ) {
			debug( RPT_DEBUG, "found symbol at: %p", *p );
		}

		/* Was the symbol required but not found ? */
		if( !*p && driver_symbols[i].required ) {
			report( RPT_ERR, "Module [%.40s] does not have required symbol: %s", driver->name, driver_symbols[i].name );
			missing_symbols = 1;
		}
	}

	/* If errors, leave now while we can :) */
	if( missing_symbols ) {
  		report( RPT_ERR, "Driver [%.40s] does not have all obligatory symbols", driver->name );
		dlclose( driver->module_handle );
		return -1;
	}


	/* Add our exported functions */

	/* Config file functions */
	driver->config_get_bool		= config_get_bool;
	driver->config_get_int		= config_get_int;
	driver->config_get_float	= config_get_float;
	driver->config_get_string	= config_get_string;
	driver->config_has_section	= config_has_section;
	driver->config_has_key		= config_has_key;

	/* Reporting */
	driver->report			= report;

	/* Driver private data */
	driver->store_private_ptr	= driver_store_private_ptr;

	/* Display size request */
	driver->request_display_width	= request_display_width;
	driver->request_display_height	= request_display_height;

	return 0;
}


int
driver_unbind_module( Driver * driver )
{
	dlclose( driver->module_handle );

	return 0;
}


bool
driver_does_output( Driver * driver )
{
	return (driver->width != NULL
		|| driver->height != NULL
		|| driver->clear != NULL
		|| driver->string != NULL
		|| driver->chr != NULL ) ? 1 : 0;
}


bool
driver_does_input( Driver * driver )
{
	return (driver->getkey != NULL
		|| driver->get_key != NULL ) ? 1 : 0;
}


bool
driver_stay_in_foreground( Driver * driver )
{
	return *driver->stay_in_foreground;
}


bool
driver_supports_multiple( Driver * driver )
{
	return *driver->supports_multiple;
}


static int
driver_store_private_ptr(Driver * driver, void * private_data)
{
	report( RPT_INFO, "driver_store_private_ptr( driver=%p, ptr=%p )", driver, private_data );

	driver->private_data = private_data;
	return 0;
}


static int
request_display_width()
{
	if( !display_props )
		return 0;
	return display_props->width;
}

static int
request_display_height()
{
	if( !display_props )
		return 0;
	return display_props->height;
}

void
driver_alt_vbar( Driver * drv, int x, int y, int len, int promille, int pattern )
{
	int pos;

	debug (RPT_DEBUG, "%s( drv=[%.40s], x=%d, y=%d, len=%d, promille=%d, pattern=%d", __FILE__, drv->name, x, y, len, promille, pattern);

	if (!drv->chr)
		return;
	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			drv->chr (drv, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}

void
driver_alt_hbar( Driver * drv, int x, int y, int len, int promille, int pattern )
{
	int pos;

	debug (RPT_DEBUG, "%s( drv=[%.40s], x=%d, y=%d, len=%d, promille=%d, pattern=%d", __FILE__, drv->name, x, y, len, promille, pattern);

	if (!drv->chr)
		return;

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			drv->chr (drv, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}

void
driver_alt_num( Driver * drv, int x, int num )
{
	/* Ugly code extracted by David GLAUDE from lcdm001.c ;)*/
	/* Moved to driver.c by Joris Robijn */
	static char num_map [10][4][3] = {
	{
		{' ','_',' '}, /*0*/
		{'|',' ','|'},
		{'|','_','|'},
		{' ',' ',' '}},
	{
		{' ',' ',' '},/*1*/
		{' ',' ','|'},
		{' ',' ','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*2*/
		{' ','_','|'},
		{'|','_',' '},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*3*/
		{' ','_','|'},
		{' ','_','|'},
		{' ',' ',' '}},
	{
		{' ',' ',' '},/*4*/
		{'|','_','|'},
		{' ',' ','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*5*/
		{'|','_',' '},
		{' ','_','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*6*/
		{'|','_',' '},
		{'|','_','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*7*/
		{' ',' ','|'},
		{' ',' ','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*8*/
		{'|','_','|'},
		{'|','_','|'},
		{' ',' ',' '}},
	{
		{' ','_',' '},/*9*/
		{'|','_','|'},
		{' ','_','|'},
		{' ',' ',' '}}
	};
	/* End of ugly code ;) by Rene Wagner */
	/* I like this code !  Joris */

	int y, dx;

	debug (RPT_DEBUG, "%s( drv=[%.40s], x=%d, num=%d", __FILE__, drv->name, x, num);

	if (!drv->chr)
		return;

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			drv->chr (drv, x + dx, y, num_map[num][y-1][dx]);
}

void
driver_alt_heartbeat( Driver * drv, int state )
{
	int icon;

	debug (RPT_DEBUG, "%s( drv=[%.40s], state=%d", __FILE__, drv->name, state);

	if (state == HEARTBEAT_OFF)
		return;
		/* Don't display anything */

	if (!drv->width)
		return;

	/* Hmm, is this a good method ?
	 * Or should we use clock() ? Or ftime ? Or gettimeofday ?
	 */
	icon = (timer & 5) ? ICON_HEART_FILLED : ICON_HEART_OPEN;

	if (drv->icon)
		drv->icon( drv, drv->width(drv), 1, icon);
	else
		driver_alt_icon( drv, drv->width(drv), 1, icon);
}

void
driver_alt_icon( Driver * drv, int x, int y, int icon )
{
	char ch1 = '?';
	char ch2 = 0;

	debug (RPT_DEBUG, "%s( drv=[%.40s], x=%d, y=%d, icon=ICON_%s", __FILE__, drv->name, x, y, widget_icon_to_iconname (icon) );

	if (!drv->chr)
		return;

	switch (icon) {
	  case ICON_BLOCK_FILLED:	ch1 = '#'; break;
	  case ICON_HEART_OPEN:		ch1 = '-'; break;
	  case ICON_HEART_FILLED:	ch1 = '#'; break;
	  case ICON_ARROW_UP:		ch1 = '^'; break;
	  case ICON_ARROW_DOWN:		ch1 = 'v'; break;
	  case ICON_ARROW_LEFT:		ch1 = '<'; break;
	  case ICON_ARROW_RIGHT:	ch1 = '>'; break;
	  case ICON_CHECKBOX_OFF:	ch1 = 'N'; break;
	  case ICON_CHECKBOX_ON:	ch1 = 'Y'; break;
	  case ICON_CHECKBOX_GRAY:	ch1 = 'o'; break;
	  case ICON_SELECTOR_AT_LEFT:	ch1 = '>'; break;
	  case ICON_SELECTOR_AT_RIGHT:	ch1 = '<'; break;
	  case ICON_ELLIPSIS:		ch1 = '_'; break;
	  case ICON_STOP:		ch1 = '['; ch2 = ']'; break;
	  case ICON_PAUSE:		ch1 = '|'; ch2 = '|'; break;
	  case ICON_PLAY:		ch1 = '>'; ch2 = ' '; break;
	  case ICON_PLAYR:		ch1 = '<'; ch2 = ' '; break;
	  case ICON_FF:			ch1 = '>'; ch2 = '>'; break;
	  case ICON_FR:			ch1 = '<'; ch2 = '<'; break;
	  case ICON_NEXT:		ch1 = '>'; ch2 = '|'; break;
	  case ICON_PREV:		ch1 = '|'; ch2 = '<'; break;
	  case ICON_REC:		ch1 = '('; ch2 = ')'; break;
	}
	drv->chr( drv, x, y, ch1);
	if (ch2)
		drv->chr( drv, x+1, y, ch2);
}

void driver_alt_cursor( Driver * drv, int x, int y, int state )
{
	/* Same question about timer in this function... */

	debug (RPT_DEBUG, "%s( drv=[%.40s], x=%d, y=%d, state=%d", __FILE__, drv->name, x, y, state);

	switch( state ) {
	  case CURSOR_BLOCK:
	  case CURSOR_DEFAULT_ON:
		if (timer & 2)
			driver_alt_icon( drv, x, y, ICON_BLOCK_FILLED );
		break;
	  case CURSOR_UNDER:
		if (timer & 2 && drv->chr)
			drv->chr( drv, x, y, '_');
		break;
	}

}
