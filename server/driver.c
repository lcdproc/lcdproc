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

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/report.h"
#include "configfile.h"

#include "driver.h"
#include "drivers.h"
#include "drivers/lcd.h"
/* lcd.h is used for the driver API definition */

void * lcd_find_init (char *driver); /* HACK TO USE THIS WHILE NO LOADABLE DRIVERS */


/* Functions for the driver */
static int request_display_width();
static int request_display_height();
static int driver_store_private_ptr(Driver * driver, void * private_data);


Driver *
driver_load( char * name, char * filename, char * args )
{
	void (*driver_init)();
	Driver * driver = NULL;
	int res;

	debug( RPT_DEBUG, "Loading driver [%.40s]", name );

	/* Allocate memory for new driver struct */
	driver = malloc( sizeof( Driver ));
	memset( driver, 0, sizeof (Driver));

	/* Load the driver modules and fill the symbols */
	if( driver_bind_module( driver ) < 0 ) {
		report( RPT_ERR, "Driver [%.40s] load failed", name );
		free( driver );
		return NULL;
	}

	/* Find the driver in the array of driver types    OLD CODE IS CALLED HERE */
	if ((driver_init = lcd_find_init(name)) == NULL) {
		/* Driver not found */
		report( RPT_ERR, "Unknown driver [%.40s]", name);
		return NULL;
	}
	driver->init = (int (*)(Driver*,char*)) driver_init;

	/* And store its name and filename */
	driver->name = malloc( strlen( name ) + 1 );
	strcpy( driver->name, name );
	driver->filename = malloc( strlen( filename ) + 1 );
	strcpy( driver->filename, filename );

	/* Call the init function */
	report( RPT_DEBUG, "Calling driver [%.40s] init function", driver->name );
	res = driver->init( driver, args );
	if( res < 0 ) {
		report( RPT_ERR, "Driver [%.40s] init failed, return code < 0", driver->name );
		/* Driver load failed, don't add driver to list
		 * Free driver structure again
		 */
		free( driver->name );
		free( driver->filename );
		free( driver );
		return NULL;
	}

	/* Check if necesary functions are filled
	 * SHOULD BE DONE BEFORE CALLING INIT WHEN WE HAVE LOADABLE DRIVERS
	 */
	if( ! driver_has_obligatory_symbols( driver ) ) {
		report( RPT_ERR, "Driver [%.40s] does not have all obligatory symbols", driver->name );
		driver_unload( driver );
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

	/* FUTURE: UNLOAD THE LOADED MODULE */
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
	/* Clear the struct, including all functions */
	memset( driver, 0, sizeof(Driver) );

	/* FUTURE: GET THE SYMBOLS FROM THE DRIVER MODULE */



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
	return 0;
}


bool
driver_has_obligatory_symbols( Driver * driver )
{
	if( driver->api_version == NULL
	|| driver->stay_in_foreground == NULL
	|| driver->supports_multiple == NULL ) {
		report( RPT_ERR, "Driver [%.40s] misses symbols", driver->name );
		return 0;
	}

	if( driver_does_output(driver)
	&& ( driver->width == NULL
	  || driver->height == NULL
	  || driver->clear == NULL
	  || driver->string == NULL
	  || driver->chr == NULL )) {
		report( RPT_ERR, "Driver [%.40s] does output but misses a obligatory function", driver->name );
		return 0;
	}
	return 1;
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
