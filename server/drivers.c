/*
 * drivers.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *
 *
 * This code manages the lists of loaded drivers and does actions on all drivers.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/LL.h"
#include "shared/report.h"
#include "configfile.h"

#include "drivers.h"
#include "driver.h"
#include "drivers/lcd.h"
/* lcd.h is used for the driver API definition */


LinkedList * loaded_drivers = NULL;
DisplayProps * display_props = NULL;

#define ForAllDrivers(drv) for( drv = LL_GetFirst(loaded_drivers); drv; drv = LL_GetNext(loaded_drivers) )


int
drivers_load_driver( char * name )
{
	Driver * driver;
	char * s;
	char * driverpath;
	char * filename;
	char * args;

	report( RPT_INFO, "drivers_load_driver( name=\"%.40s\")", name );

	/* First driver ? */
	if( !loaded_drivers ) {
		/* Create linked list */
		loaded_drivers = LL_new ();
		if( !loaded_drivers ) {
			report( RPT_ERR, "Error allocating driver list." );
			return -1;
		}
	}

	/* Retrieve data from config file */
	s = config_get_string( "server", "driverpath", 0, "" );
	driverpath = malloc( strlen(s) + 1 );
	strcpy( driverpath, s );

	s = config_get_string( name, "file", 0, NULL );
	if( s ) {
		filename = malloc( strlen(driverpath) + strlen(s) + 1 );
		strcpy( filename, driverpath );
		strcat( filename, s );
	} else {
		filename = malloc( strlen(driverpath) + strlen(name) + strlen(MODULE_EXTENSION) + 1 );
		strcpy( filename, driverpath );
		strcat( filename, name  );
		strcat( filename, MODULE_EXTENSION );
	}

	s = config_get_string( name, "arguments", 0, "" );
	args = malloc( strlen(s)+1 );
	strcpy( args, s );

	/* Load the module */
	driver = driver_load( name, filename, args );
	if( driver == NULL ) {
		/* It failed. The message has already been given by driver_load() */
		report( RPT_INFO, "Module %.40s could not be loaded", filename );
		free( driverpath );
		free( filename );
		free( args );
		return -1;
	}

	/* Add driver to list */
	LL_Push( loaded_drivers, driver );

	free( driverpath );
	free( filename );
	free( args );

	/* If first driver, store display properties */
	if( driver_does_output(driver) && !display_props ) {
		if( driver->width(driver) <= 0 || driver->width(driver) > LCD_MAX_WIDTH
		|| driver->height(driver) <= 0 || driver->height(driver) > LCD_MAX_HEIGHT ) {
			report( RPT_ERR, "Driver [%.40s] has invalid display size", driver->name );
		}

		/* Allocate new DisplayProps structure */
		display_props = malloc( sizeof( DisplayProps ));
		display_props->width      = driver->width(driver);
		display_props->height     = driver->height(driver);

		if( driver->cellwidth != NULL && display_props->cellwidth > 0 )
			display_props->cellwidth  = driver->cellwidth(driver);
		else
			display_props->cellwidth  = LCD_DEFAULT_CELLWIDTH;

		if( driver->cellheight != NULL && driver->cellheight(driver) > 0 )
			display_props->cellheight = driver->cellheight(driver);
		else
			display_props->cellheight = LCD_DEFAULT_CELLHEIGHT;
	}

	/* Return the driver type */
	if( driver_does_output(driver) ) {
		if( driver_stay_in_foreground(driver) )
			return 2;
		else
			return 1;
	}
	return 0;
}


int
drivers_unload_all()
{
	Driver * driver;

	report( RPT_INFO, "unload_all_driver()");

	while( (driver = LL_Pop( loaded_drivers )) != NULL ) {
		driver_unload( driver );
	}

	return 0;
}


char *
drivers_get_info()
{
	Driver *drv;

	report( RPT_INFO, "drivers_getinfo()" );

	ForAllDrivers(drv) {
		if( drv->get_info ) {
			return drv->get_info( drv );
		}
	}
	return "";
}


void
drivers_clear()
{
	Driver *drv;

	report( RPT_INFO, "drivers_clear()" );

	ForAllDrivers(drv) {
		if( drv->clear )
			drv->clear( drv );
	}
}


void
drivers_flush()
{
	Driver *drv;

	report( RPT_INFO, "drivers_flush()" );

	ForAllDrivers(drv) {
		if( drv->flush )
			drv->flush( drv );
	}
}


void
drivers_string( int x, int y, char * string )
{
	Driver *drv;

	report( RPT_INFO, "drivers_string( x=%d, y=%d, string=\"%.40s\" )", x, y, string );

	ForAllDrivers(drv) {
		if( drv->string )
			drv->string( drv, x, y, string );
	}
}


void
drivers_chr( int x, int y, char c )
{
	Driver *drv;

	report( RPT_INFO, "drivers_chr( x=%d, y=%d, c='%c' )", x, y, c );

	ForAllDrivers(drv) {
		if( drv->chr )
			drv->chr( drv, x, y, c );
	}
}


void
drivers_vbar( int x, int y, int len, int promille, int pattern )
{
	Driver *drv;

	report( RPT_INFO, "drivers_vbar( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", x, y, len, promille, pattern );

	/* NEW FUNCTIONS
	 *
	 * We need more data in the widget. Requires language update...
	 */


	ForAllDrivers(drv) {
		if( drv->vbar )
			drv->vbar( drv, x, y, len, promille, pattern );
		else
			driver_alt_vbar( drv, x, y, len, promille, pattern );
	}
}


void
drivers_hbar( int x, int y, int len, int promille, int pattern )
{
	Driver *drv;

	report( RPT_INFO, "drivers_hbar( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", x, y, len, promille, pattern );

	ForAllDrivers(drv) {
		if( drv->hbar )
			drv->hbar( drv, x, y, len, promille, pattern );
		else
			driver_alt_hbar( drv, x, y, len, promille, pattern );
	}
}


void
drivers_num( int x, int num )
{
	Driver *drv;

	report( RPT_INFO, "drivers_num( x=%d, num=%d )", x, num );

	ForAllDrivers(drv) {
		if( drv->num )
			drv->num( drv, x, num );
		else
			driver_alt_num( drv, x, num );
	}
}


void
drivers_heartbeat( int state )
{
	Driver *drv;

	report( RPT_INFO, "drivers_heartbeat( state=%d )", state );

	ForAllDrivers(drv) {
		if( drv->heartbeat )
			drv->heartbeat( drv, state );
		else
			driver_alt_heartbeat( drv, state );
	}
}


void
drivers_icon( int x, int y, int icon )
{
	Driver *drv;

	report( RPT_INFO, "drivers_icon( x=%d, y=%d, icon=%d )", x, y, icon );

	ForAllDrivers(drv) {
		if( drv->icon )
			drv->icon( drv, x, y, icon );
		else
			driver_alt_icon( drv, x, y, icon );
	}
}

void
drivers_cursor( int x, int y, int state )
{
	Driver *drv;

	report( RPT_INFO, "drivers_cursor( x=%d, y=%d, state=%d )", x, y, state );

	ForAllDrivers(drv) {
		if( drv->cursor )
			drv->cursor( drv, x, y, state );
		else
			driver_alt_cursor( drv, x, y, state );
	}
}

void
drivers_backlight( int brightness )
{
	Driver *drv;

	report( RPT_INFO, "drivers_backlight( brightness=%d )", brightness );

	ForAllDrivers(drv) {
		if( drv->backlight )
			drv->backlight( drv, brightness );
	}
}


void
drivers_output( int state )
{
	Driver *drv;

	report( RPT_INFO, "drivers_output( state=%d )", state );

	ForAllDrivers(drv) {
		if( drv->output )
			drv->output( drv, state );
	}
}


char *
drivers_get_key()
{
	/* Find the first input keystroke, if any */
	Driver *drv;
	char * keystroke;

	report( RPT_INFO, "drivers_get_key()" );

	ForAllDrivers(drv) {
		if( drv->get_key ) {
			keystroke = drv->get_key( drv );
			if( keystroke != NULL ) {
				report( RPT_INFO, "Driver [%.40s] generated keystroke %.40s", drv->name, keystroke );
				return keystroke;
			}
		}
	}
	return NULL;
}


char
drivers_getkey() /* TO BE REMOVED AS SOON AS INPUT ROUTINES ACCEPT STRINGS */
{
	Driver *drv;
	char * s;
	char ch;

	report( RPT_INFO, "drivers_getkey()" );

	ForAllDrivers(drv) {
		if( drv->get_key ) {
			s = drv->get_key(drv);
			if( s )
				return s[0];	/* It returns the first char only ! a hack ! */
		}
		else if( drv->getkey ) {
			ch = drv->getkey(drv);
			if( ch )
				return ch;
		}
	}
	return 0;
}
