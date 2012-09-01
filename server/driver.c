/** \file server/driver.c
 * This code does all actions on the driver object.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "main.h" /* for timer  */

#include "shared/report.h"
#include "shared/configfile.h"

#include "widget.h"
#include "driver.h"
#include "drivers.h"
#include "drivers/lcd.h"
/* lcd.h is used for the driver API definition */


/** property / method symbols in a Driver structure */
typedef struct driver_symbols {
	const char *name;	/**< symbol name */
	short offset;		/**< offset in Driver structure */
	short required;		/**< is the symbol mandatory */
} DriverSymbols;

DriverSymbols driver_symbols[] = {
	{ "api_version",        offsetof(Driver, api_version),        1 },
	{ "stay_in_foreground", offsetof(Driver, stay_in_foreground), 1 },
	{ "supports_multiple",  offsetof(Driver, supports_multiple),  1 },
	{ "symbol_prefix",      offsetof(Driver, symbol_prefix),      1 },
	{ "init",               offsetof(Driver, init),               1 },
	{ "close",              offsetof(Driver, close),              1 },
	{ "width",              offsetof(Driver, width),              0 },
	{ "height",             offsetof(Driver, height),             0 },
	{ "clear",              offsetof(Driver, clear),              0 },
	{ "flush",              offsetof(Driver, flush),              0 },
	{ "string",             offsetof(Driver, string),             0 },
	{ "chr",                offsetof(Driver, chr),                0 },
	{ "vbar",               offsetof(Driver, vbar),               0 },
	{ "hbar",               offsetof(Driver, hbar),               0 },
	{ "num",                offsetof(Driver, num),                0 },
	{ "heartbeat",          offsetof(Driver, heartbeat),          0 },
	{ "icon",               offsetof(Driver, icon),               0 },
	{ "cursor",             offsetof(Driver, cursor),             0 },
	{ "set_char",           offsetof(Driver, set_char),           0 },
	{ "get_free_chars",     offsetof(Driver, get_free_chars),     0 },
	{ "cellwidth",          offsetof(Driver, cellwidth),          0 },
	{ "cellheight",         offsetof(Driver, cellheight),         0 },
	{ "get_contrast",       offsetof(Driver, get_contrast),       0 },
	{ "set_contrast",       offsetof(Driver, set_contrast),       0 },
	{ "get_brightness",     offsetof(Driver, get_brightness),     0 },
	{ "set_brightness",     offsetof(Driver, set_brightness),     0 },
	{ "backlight",          offsetof(Driver, backlight),          0 },
	{ "output",             offsetof(Driver, output),             0 },
	{ "get_key",            offsetof(Driver, get_key),            0 },
	{ "get_info",           offsetof(Driver, get_info),           0 },
	{ NULL, 0, 0 }
};



/* Functions for the driver */
static int request_display_width(void);
static int request_display_height(void);
static int driver_store_private_ptr(Driver *driver, void *private_data);


/** Create a driver object.
 * Allocate memory for the driver object, load it from file and bind its symbols.
 * \param name      Name under which the driver shall be known further on.
 * \param filename  Name of the file containing the drivers object code.
 * \return          Pointer to the freshly created driver; \c NULL on error.
 */
Driver *
driver_load(const char *name, const char *filename)
{
	Driver *driver = NULL;
	int res;

	report(RPT_DEBUG, "%s(name=\"%.40s\", filename=\"%.80s\")",
		__FUNCTION__, name, filename);

	/* fail on wrong / missing parameters */
	if ((name == NULL) || (filename == NULL))
		return NULL;

	/* Allocate memory for new driver struct */
	driver = calloc(1, sizeof(Driver));
	if (driver == NULL) {
		report(RPT_ERR, "%s: error allocating driver", __FUNCTION__);
		return NULL;
	}

	/* And store its name and filename */
	driver->name = malloc(strlen(name) + 1);
	if (driver->name == NULL) {
		report(RPT_ERR, "%s: error allocating driver name", __FUNCTION__);
		free(driver);
		return NULL;
	}
	strcpy(driver->name, name);

	driver->filename = malloc(strlen(filename) + 1);
	if (driver->filename == NULL) {
		report(RPT_ERR, "%s: error allocating driver filename", __FUNCTION__);
		free(driver->name);
		free(driver);
		return NULL;
	}
	strcpy(driver->filename, filename);

	/* Load and bind the driver module and locate the symbols */
	if (driver_bind_module(driver) < 0) {
		report(RPT_ERR, "Driver [%.40s] binding failed", name);
		free(driver->name);
		free(driver->filename);
		free(driver);
		return NULL;
	}

	/* Check module version */
	if (strcmp(*(driver->api_version), API_VERSION) != 0) {
		report(RPT_ERR, "Driver [%.40s] is of an incompatible version", name);
		driver_unbind_module(driver);
		free(driver->name);
		free(driver->filename);
		free(driver);
		return NULL;
	}

	/* Call the init function */
	debug(RPT_DEBUG, "%s: Calling driver [%.40s] init function",
		__FUNCTION__, driver->name);

	res = driver->init(driver);
	if (res < 0) {
		report(RPT_ERR, "Driver [%.40s] init failed, return code %d",
			driver->name, res);
		/* Driver load failed, driver should not be added to list
		 * Free driver structure again
		 */
		driver_unbind_module(driver);
		free(driver->name);
		free(driver->filename);
		free(driver);
		return NULL;
	}

	debug(RPT_NOTICE, "Driver [%.40s] loaded", driver->name);

	return driver;
}


/** Unload driver from memory.
 * \param driver  Driver to unload.
 * \retval <0     Error.
 * \retval  0     Success.
 */
int
driver_unload(Driver *driver)
{
	debug(RPT_NOTICE, "Closing driver [%.40s]", driver->name);

	/* close the driver, if its \c close method is [already] defined */
	if (driver->close != NULL)
		driver->close(driver);

	/* unload the module */
	driver_unbind_module(driver);

	/* free its data */
	free(driver->filename);
	driver->filename = NULL;
	free(driver->name);
	driver->name = NULL;
	free(driver);
	driver = NULL;

	debug(RPT_DEBUG, "%s: Driver unloaded", __FUNCTION__);

	return 0;
}


/** Dynamically load a module and bind it to the Driver's symbols.
 * \param driver  Pointer to the Driver object.
 * \retval <0     Error.
 * \retval  0     Success.
 */
int
driver_bind_module(Driver *driver)
{
	int i;
	int missing_symbols = 0;

	debug(RPT_DEBUG, "%s(driver=[%.40s])", __FUNCTION__, driver->name);

	/* Load the module */
	driver->module_handle = dlopen(driver->filename, RTLD_NOW);
	if (driver->module_handle == NULL) {
		report(RPT_ERR, "Could not open driver module %.40s: %s",
			driver->filename, dlerror());
		return -1;
	}

	/* And locate the symbols */
	for (i = 0; driver_symbols[i].name != NULL; i++) {
		void (**p)();

		p = (void(**)()) ((size_t)driver + (driver_symbols[i].offset));
		*p = NULL;

		/* 1) try to retrieve the symbol with the driver's symbol_prefix added */
		if (driver->symbol_prefix != NULL) {
			char *s = malloc(strlen(*(driver->symbol_prefix)) + strlen(driver_symbols[i].name) + 1);
			strcpy(s, *(driver->symbol_prefix));
			strcat(s, driver_symbols[i].name);
			debug(RPT_DEBUG, "%s: finding symbol: %s", __FUNCTION__, s);
			*p = dlsym(driver->module_handle, s);
			free(s);
		}
		/* 2) try to retrieve the symbol without the symbol prefix */
		if (*p == NULL) {
			debug(RPT_DEBUG, "%s: finding symbol: %s", __FUNCTION__, driver_symbols[i].name);
			*p = dlsym(driver->module_handle, driver_symbols[i].name);
		}

		if (*p != NULL) {
			debug(RPT_DEBUG, "%s: found symbol at: %p", __FUNCTION__, *p);
		}
		else {
			/* Was the symbol required but not found ? */
			if (driver_symbols[i].required) {
				report(RPT_ERR, "Driver [%.40s] does not have required symbol: %s",
					driver->name, driver_symbols[i].name);
				missing_symbols++;
			}
		}
	}

	/* If errors, leave now while we can :) */
	if (missing_symbols > 0) {
  		report(RPT_ERR, "Driver [%.40s]  misses %d required symbols",
			driver->name, missing_symbols);
		dlclose(driver->module_handle);
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


/** Unload a Driver's module.
 * \param driver  Pointer to the driver object.
 * \retval <0     Error.
 * \retval  0     Success.
 */
int
driver_unbind_module(Driver *driver)
{
	debug(RPT_DEBUG, "%s(driver=[%.40s])", __FUNCTION__, driver->name);

	dlclose(driver->module_handle);

	return 0;
}


/** Determine if the driver is an output driver.
 * This is done by checking whether the driver knows dimensions
 * and supports the methods that write to the screen.
 * \param driver  Pointer to the driver object.
 * \retval 0      No, it is not an output driver.
 * \retval 1      Yes, it is an output driver.
 */
bool
driver_does_output(Driver *driver)
{
	return (driver->width != NULL
		|| driver->height != NULL
		|| driver->clear != NULL
		|| driver->string != NULL
		|| driver->chr != NULL) ? 1 : 0;
}


/** Determine if the driver is an input driver.
 * This is done by checking whether the driver supports the \c get_key method.
 * \param driver  Pointer to the driver object.
 * \retval 0      No, it is not an input driver.
 * \retval 1      Yes, it is an input driver.
 */
bool
driver_does_input(Driver *driver)
{
	return (driver->get_key != NULL) ? 1 : 0;
}


/** Tell if the driver needs to stay in the foreground.
 * \param driver  Pointer to the driver object.
 * \retval 0      No, the driver does not need to stay in the foreground.
 * \retval 1      Yes, the driver needs to stay in the foreground.
 */
bool
driver_stay_in_foreground(Driver *driver)
{
	return *driver->stay_in_foreground;
}


/** Tell if the driver supports multiple instances.
 * \param driver  Pointer to the driver object.
 * \retval 0      No, it doesn't.
 * \retval 1      Yes, it does.
 */
bool
driver_supports_multiple(Driver *driver)
{
	return *driver->supports_multiple;
}


static int
driver_store_private_ptr(Driver *driver, void *private_data)
{
	debug(RPT_DEBUG, "%s(driver=[%.40s], ptr=%p)",
		__FUNCTION__, driver->name, private_data);

	driver->private_data = private_data;
	return 0;
}


static int
request_display_width(void)
{
	if (!display_props)
		return 0;
	return display_props->width;
}


static int
request_display_height(void)
{
	if (!display_props)
		return 0;
	return display_props->height;
}


/** Draw a vertical bar bottom-up.
 * Fallback for the driver's \c vbar method if the driver does not provide one.
 * \param drv      Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
void
driver_alt_vbar(Driver *drv, int x, int y, int len, int promille, int options)
{
	int pos;

	debug(RPT_DEBUG, "%s(drv=[%.40s], x=%d, y=%d, len=%d, promille=%d, options=%d)",
		__FUNCTION__, drv->name, x, y, len, promille, options);

	/* if the driver does not support output, do nothing */
	if (drv->chr == NULL)
		return;

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			drv->chr(drv, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}


/** Draw a horizontal bar to the right.
 * Fallback for the driver's \c hbar method if the driver does not provide one.
 * \param drv      Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
void
driver_alt_hbar(Driver *drv, int x, int y, int len, int promille, int options)
{
	int pos;

	debug(RPT_DEBUG, "%s(drv=[%.40s], x=%d, y=%d, len=%d, promille=%d, options=%d)",
		__FUNCTION__, drv->name, x, y, len, promille, options);

	/* if the driver does not support output, do nothing */
	if (drv->chr == NULL)
		return;

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			drv->chr(drv, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}


/** Write a big number to the screen.
 * Fallback for the driver's \c num method if the driver does not provide one.
 * \param drv  Pointer to driver structure.
 * \param x    Horizontal character position (column).
 * \param num  Character to write (0 - 10 with 10 representing ':')
 */
void
driver_alt_num(Driver *drv, int x, int num)
{
	/* Ugly code extracted by David GLAUDE from lcdm001.c ;)*/
	/* Moved to driver.c by Joris Robijn */
	static char num_map[][4][4] = {
	{ /* 0 */
		" _ ",
		"| |",
		"|_|",
		"   " },
	{ /* 1 */
		"   ",
		"  |",
		"  |",
		"   " },
	{ /* 2 */
		" _ ",
		" _|",
		"|_ ",
		"   " },
	{ /* 3 */
		" _ ",
		" _|",
		" _|",
		"   " },
	{ /* 4 */
		"   ",
		"|_|",
		"  |",
		"   " },
	{ /* 5 */
		" _ ",
		"|_ ",
		" _|",
		"   " },
	{ /* 6 */
		" _ ",
		"|_ ",
		"|_|",
		"   " },
	{ /* 7 */
		" _ ",
		"  |",
		"  |",
		"   " },
	{ /* 8 */
		" _ ",
		"|_|",
		"|_|",
		"   " },
	{ /* 9 */
		" _ ",
		"|_|",
		" _|",
		"   " },
	{ /* colon */
		" ",
		".",
		".",
		" " }
	};
	/* End of ugly code ;) by Rene Wagner */
	/* I like this code !  Joris */

	int y, dx;

	debug(RPT_DEBUG, "%s(drv=[%.40s], x=%d, num=%d)",
		__FUNCTION__, drv->name, x, num);

	if ((num < 0) || (num > 10))
		return;
	/* if the driver does not support output, do nothing */
	if (drv->chr == NULL)
		return;

	for (y = 0; y < 4; y++)
		for (dx = 0; num_map[num][y][dx] != '\0'; dx++)
			drv->chr(drv, x + dx, y+1, num_map[num][y][dx]);
}


/** Show the heartbeat.
 * Fallback for the driver's \c heartbeat method if the driver does not provide one.
 * \param drv    Pointer to driver structure.
 * \param state  Current heartbeat state.
 */
void
driver_alt_heartbeat(Driver *drv, int state)
{
	int icon;

	debug(RPT_DEBUG, "%s(drv=[%.40s], state=%d)",
		__FUNCTION__, drv->name, state);

	if (state == HEARTBEAT_OFF)
		return;
		/* Don't display anything */

	/* if the driver does not support output, do nothing */
	if (drv->width == NULL)
		return;

	/* Hmm, is this a good method ?
	 * Or should we use clock() ? Or ftime ? Or gettimeofday ?
	 */
	icon = (timer & 5) ? ICON_HEART_FILLED : ICON_HEART_OPEN;

	if (drv->icon)
		drv->icon(drv, drv->width(drv), 1, icon);
	else
		driver_alt_icon(drv, drv->width(drv), 1, icon);
}


/** Place an icon on the screen.
 * Fallback for the driver's \c icon method, in case either the driver does not
 * provide one or the driver's method indicates the icon needs to be handled
 * by the server core.
 * \param drv   Pointer to driver structure.
 * \param x     Horizontal character position (column).
 * \param y     Vertical character position (row).
 * \param icon  synbolic value representing the icon.
 *
 * \note Icons used for 'play control' actually are composed of two
 *       characters. Drivers must check for bounds on calls to chr()!
 */
void
driver_alt_icon(Driver *drv, int x, int y, int icon)
{
	char ch1 = '?';
	char ch2 = '\0';

	debug(RPT_DEBUG, "%s(drv=[%.40s], x=%d, y=%d, icon=ICON_%s)",
		__FUNCTION__, drv->name, x, y, widget_icon_to_iconname(icon));

	/* if the driver does not support output, do nothing */
	if (drv->chr == NULL)
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

	drv->chr(drv, x, y, ch1);
	if (ch2 != '\0')
		drv->chr(drv, x+1, y, ch2);
}


/** Set cursor position and state.
 * Fallback for the driver's \c cursor method if the driver does not provide one.
 * \param drv    Pointer to driver structure.
 * \param x      Horizontal cursor position (column).
 * \param y      Vertical cursor position (row).
 * \param state  New cursor state.
 */
void driver_alt_cursor(Driver *drv, int x, int y, int state)
{
	/* Same question about timer in this function... */

	debug(RPT_DEBUG, "%s(drv=[%.40s], x=%d, y=%d, state=%d)",
		__FUNCTION__, drv->name, x, y, state);

	switch (state) {
	  case CURSOR_BLOCK:
	  case CURSOR_DEFAULT_ON:
	  	if ((timer & 2) && (drv->chr != NULL)) {
	  		if (drv->icon != NULL) {
	  			drv->icon(drv, x, y, ICON_BLOCK_FILLED);
	  		} else {
				driver_alt_icon(drv, x, y, ICON_BLOCK_FILLED);
			}
		}
		break;
	  case CURSOR_UNDER:
		if ((timer & 2) && (drv->chr != NULL)) {
			drv->chr(drv, x, y, '_');
		}
		break;
	}

}
