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

#include <malloc.h>
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

#include "drivers.h"
#include "driver.h"
#include "drivers/lcd.h"
/* lcd.h is used for the driver API definition */


LinkedList * loaded_drivers = NULL;
DisplayProps * display_props = NULL;

#define ForAllDrivers(drv) for( drv = LL_GetFirst(loaded_drivers); drv; drv = LL_GetNext(loaded_drivers) )


int
drivers_load_driver( char * name, char * filename, char * args )
{
	Driver * driver;

	report( RPT_INFO, "drivers_load_driver( name=\"%.40s\", filename=\"%.80s\", args=\"%.80s\")", name, filename, args );

	/* First driver ? */
	if( !loaded_drivers ) {
		/* Create linked list */
		loaded_drivers = LL_new ();
		if( !loaded_drivers ) {
			report( RPT_ERR, "Error allocating driver list." );
			return -1;
		}
	}

	/* Load the module */
	driver = driver_load( name, filename, args );
	if( driver == NULL )
		/* It failed. The message has already been given by driver_load() */
		return -1;

	/* Add driver to list */
	LL_Push( loaded_drivers, driver );

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
drivers_init_vbar()	/* TO BE REMOVED */
{
	Driver *drv;

	report( RPT_INFO, "drivers_init_vbar()" );

	ForAllDrivers(drv) {
		if( drv->init_vbar )
			drv->init_vbar(drv);
	}
}


void
drivers_init_hbar()	/* TO BE REMOVED */
{
	Driver *drv;

	report( RPT_INFO, "drivers_init_hbar()" );

	ForAllDrivers(drv) {
		if( drv->init_hbar )
			drv->init_hbar(drv);
	}
}


void
drivers_init_num()	/* TO BE REMOVED */
{
	Driver *drv;

	report( RPT_INFO, "drivers_init_num()" );

	ForAllDrivers(drv) {
		if( drv->init_num )
			drv->init_num( drv );
	}
}


void
drivers_vbar( int x, int y, int len, int promille, int pattern )
{
	Driver *drv;

	int old_len;

	report( RPT_INFO, "drivers_vbar( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", x, y, len, promille, pattern );

	/* NEW FUNCTIONS
	 *
	 * We need more data in the widget. Requires language update...
	 */


	ForAllDrivers(drv) {
		if( drv->vbar ) {
			drv->vbar( drv, x, y, len, promille, pattern );
		}
	}

	/* OLD FUNCTIONS
	 *
	 * We need to convert the bar lengths, because the displays can
	 * have different pixels per char
	 */

	old_len = (long) display_props->cellheight * len * promille / 1000;

	ForAllDrivers(drv) {
		if( drv->old_vbar ) {
			drv->old_vbar( drv, x, old_len );
		}
	}

}


void
drivers_hbar( int x, int y, int len, int promille, int pattern )
{
	Driver *drv;
	int old_len;

	report( RPT_INFO, "drivers_hbar( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", x, y, len, promille, pattern );

	/* NEW FUNCTIONS */

	ForAllDrivers(drv) {
		if( drv->hbar ) {
			drv->hbar( drv, x, y, len, promille, pattern );
		}
	}

	/* OLD FUNCTIONS
         *
	 * We need to convert the bar lengths, because the displays can
	 * have different pixels per char
	 */

	old_len = (long) display_props->cellwidth * len * promille / 1000;

	ForAllDrivers(drv) {
		if( drv->old_hbar ) {
			drv->old_hbar( drv, x, y, old_len );
		}
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
	}
}


void
drivers_set_char( char ch, char *dat )
{
	Driver *drv;

	report( RPT_INFO, "drivers_set_char( ch=%d, dat=%p )", ch, dat );

	ForAllDrivers(drv) {
		if( drv->set_char )
			drv->set_char( drv, ch, dat );
	}
}


int
drivers_get_contrast()
{
	Driver *drv;
	int res;

	report( RPT_INFO, "drivers_get_contrast()" );

	ForAllDrivers(drv) {
		if( drv->get_contrast ) {
			res = drv->get_contrast( drv );
			report( RPT_INFO, "Driver [%.40s] gave contrast value %d", drv->name, res );
			return res;
		}
	}
	report( RPT_INFO, "Did not get any contrast value" );
	return -1;
}


void
drivers_set_contrast( int promille )
{
	Driver *drv;

	report( RPT_INFO, "drivers_contrast( contrast=%d )", promille );

	ForAllDrivers(drv) {
		if( drv->set_contrast )
			drv->set_contrast( drv, promille );
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
