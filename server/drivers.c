/*
 * Driver loader
 *
 * This code does all driver handling, loading, initializing, unloading.
 *
 */

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/LL.h"
#include "shared/debug.h"
#include "configfile.h"

#include "drivers/lcd.h"
// lcd.h is used for the driver API definition


LinkedList * loaded_drivers = NULL;

long int empty_function() { return 0; }

static int fill_driver_functions( lcd_logical_driver * driver );

static int store_private_ptr(struct lcd_logical_driver * driver, void * private_data);

int
load_driver ( char * name, char * filename, char * args )
{
	int res;
	void (*driver_init)();
	lcd_logical_driver * driver;

	debug ("load_driver(%s,%s,%s)\n", name,filename,args);

	// First driver ?
	if( !loaded_drivers ) {
		// Create linked list
		loaded_drivers = LL_new ();
		if( !loaded_drivers ) {
			fprintf( stderr, "Error allocating driver list.\n" );
			return -1;
		}
	}


	// Find the driver in the array of driver types
	if ((driver_init = (void *) lcd_find_init(name)) == NULL) {
		// Driver not found
		fprintf( stderr, "invalid driver: %s\n", name);
		return -1;
	}


	// Allocate memory for new driver struct
	driver = malloc( sizeof( lcd_logical_driver ));
	//memset( driver, 0, sizeof (lcd_logical_driver ));

	lcd_ptr = driver;

	fill_driver_functions( driver );


	// Rebind the init function and call it
	driver->init = driver_init;

	res = driver->init( driver, args );
	if( res < 0 ) {
		fprintf( stderr, "res<0\n" );
		// driver load failed, don't add driver to list
		return -1;
	}

	// Add driver to list
	LL_Push( loaded_drivers, driver );

	// Check the driver type
	if( driver->daemonize ) {
		return 2;
	}

	return 1; // We can't see if it's an input driver only...
}


int
unload_all_drivers ()
{
	lcd_logical_driver * driver;

	debug ("unload_all_driver()\n");

	while( (driver = LL_Pop( loaded_drivers )) != NULL ) {
		debug ("driver->close %p\n", driver );
		driver->close();
	}

	return 0;
}


static int
fill_driver_functions( lcd_logical_driver * driver )
{
	driver->wid = LCD_STD_WIDTH;
	driver->hgt = LCD_STD_HEIGHT;

	driver->cellwid = LCD_STD_CELL_WIDTH;
	driver->cellhgt = LCD_STD_CELL_HEIGHT;

	driver->framebuf = NULL;
	driver->nextkey = NULL;

	driver->daemonize = 1;

	// Set pointers to empty function

	// Basic functions
	driver->init = empty_function;
	driver->close = empty_function;

	driver->getinfo = empty_function;
	// and don't forget other get_* functions later...

	driver->clear = empty_function;
	driver->flush = empty_function;
	driver->string = empty_function;
	driver->chr = empty_function;

	// Extended functions
	driver->init_vbar = empty_function;
	driver->vbar = empty_function;
	driver->init_hbar = empty_function;
	driver->hbar = empty_function;
	driver->init_num = empty_function;
	driver->num = empty_function;
	driver->heartbeat = empty_function;

	// Hardware functions
	driver->contrast = empty_function;
	driver->backlight = empty_function;
	driver->output = empty_function;

	// Uesrdef character functions
	driver->set_char = empty_function;
	driver->icon = empty_function;

	// Key functions
	driver->getkey = empty_function;

	// Ancient functions
	driver->flush_box = empty_function;
	driver->draw_frame = empty_function;

	// Config file functions
	driver->config_get_bool		= config_get_bool;
	driver->config_get_int		= config_get_int;
	driver->config_get_float	= config_get_float;
	driver->config_get_string	= config_get_string;
	driver->config_has_section	= config_has_section;
	driver->config_has_key		= config_has_key;

	// Driver private data
	driver->store_private_ptr	= store_private_ptr;

	return 0;
}


static int
store_private_ptr(struct lcd_logical_driver * driver, void * private_data)
{
	driver->private_data = private_data;
	return 0;
}
