/** \file server/drivers.c
 * Manage the lists of loaded drivers and perform actions on all drivers.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright(c) 2001, Joris Robijn
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "shared/LL.h"
#include "shared/report.h"
#include "shared/configfile.h"

#include "driver.h"
#include "drivers.h"
#include "widget.h"


LinkedList *loaded_drivers = NULL;		/**< list of loaded drivers */
DisplayProps *display_props = NULL;		/**< properties of the display */

#define ForAllDrivers(drv) for (drv = LL_GetFirst(loaded_drivers); drv; drv = LL_GetNext(loaded_drivers))


/**
 * Load driver based on "DriverPath" config setting and section name or
 * "File" configuration setting in the driver's section.
 * \param name  Driver section name.
 * \retval  <0  error.
 * \retval   0  OK, driver is an input driver only.
 * \retval   1  OK, driver is an output driver.
 * \retval   2  OK, driver is an output driver that needs to run in the foreground.
 */
int
drivers_load_driver(const char *name)
{
	Driver *driver;
	const char *s;
	char *driverpath;
	char *filename;

	debug(RPT_DEBUG, "%s(name=\"%.40s\")", __FUNCTION__, name);

	/* First driver ? */
	if (!loaded_drivers) {
		/* Create linked list */
		loaded_drivers = LL_new();
		if (!loaded_drivers) {
			report(RPT_ERR, "Error allocating driver list.");
			return -1;
		}
	}

	/* Retrieve data from config file */
	s = config_get_string("server", "DriverPath", 0, "");
	driverpath = malloc(strlen(s) + 1);
	strcpy(driverpath, s);

	s = config_get_string(name, "File", 0, NULL);
	if (s) {
		filename = malloc(strlen(driverpath) + strlen(s) + 1);
		strcpy(filename, driverpath);
		strcat(filename, s);
	} else {
		filename = malloc(strlen(driverpath) + strlen(name) + strlen(MODULE_EXTENSION) + 1);
		strcpy(filename, driverpath);
		strcat(filename, name);
		strcat(filename, MODULE_EXTENSION);
	}

	/* Load the module */
	driver = driver_load(name, filename);
	if (driver == NULL) {
		/* It failed. The message has already been given by driver_load() */
		report(RPT_INFO, "Module %.40s could not be loaded", filename);
		free(driverpath);
		free(filename);
		return -1;
	}

	/* Add driver to list */
	LL_Push(loaded_drivers, driver);

	free(driverpath);
	free(filename);

	/* If first driver, store display properties */
	if (driver_does_output(driver) && !display_props) {
		if (driver->width(driver) <= 0 || driver->width(driver) > LCD_MAX_WIDTH
		|| driver->height(driver) <= 0 || driver->height(driver) > LCD_MAX_HEIGHT) {
			report(RPT_ERR, "Driver [%.40s] has invalid display size", driver->name);
		}

		/* Allocate new DisplayProps structure */
		display_props = malloc(sizeof(DisplayProps));
		display_props->width      = driver->width(driver);
		display_props->height     = driver->height(driver);

		if (driver->cellwidth != NULL && driver->cellwidth(driver) > 0)
			display_props->cellwidth  = driver->cellwidth(driver);
		else
			display_props->cellwidth  = LCD_DEFAULT_CELLWIDTH;

		if (driver->cellheight != NULL && driver->cellheight(driver) > 0)
			display_props->cellheight = driver->cellheight(driver);
		else
			display_props->cellheight = LCD_DEFAULT_CELLHEIGHT;
	}

	/* Return the driver type */
	if (driver_does_output(driver)) {
		if (driver_stay_in_foreground(driver))
			return 2;
		else
			return 1;
	}
	return 0;
}


/**
 * Unload all loaded drivers.
 * \retval  0
 */
int
drivers_unload_all(void)
{
	Driver *driver;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	while ((driver = LL_Pop(loaded_drivers)) != NULL) {
		driver_unload(driver);
	}

	return 0;
}


/**
 * Get information from loaded drivers.
 * \return  Pointer to information string of first driver with get_info() function defined,
 *          or the empty string if no driver has a get_info() function.
 */
const char *
drivers_get_info(void)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	ForAllDrivers(drv) {
		if (drv->get_info) {
			return drv->get_info(drv);
		}
	}
	return "";
}


/**
 * Clear screen on all loaded drivers.
 * Call clear() function of all loaded drivers that have a clear() function defined.
 */
void
drivers_clear(void)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	ForAllDrivers(drv) {
		if (drv->clear)
			drv->clear(drv);
	}
}


/**
 * Flush data on all loaded drivers to LCDs.
 * Call flush() function of all loaded drivers that have a flush() function defined.
 */
void
drivers_flush(void)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	ForAllDrivers(drv) {
		if (drv->flush)
			drv->flush(drv);
	}
}


/**
 * Write string to all loaded drivers.
 * Call string() function of all loaded drivers that have a flush() function defined.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
void
drivers_string(int x, int y, const char *string)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, string=\"%.40s\")", __FUNCTION__, x, y, string);

	ForAllDrivers(drv) {
		if (drv->string)
			drv->string(drv, x, y, string);
	}
}


/**
 * Write a character to all loaded drivers.
 * Call chr() function of all loaded drivers that have a chr() function defined.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
void
drivers_chr(int x, int y, char c)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, c='%c')", __FUNCTION__, x, y, c);

	ForAllDrivers(drv) {
		if (drv->chr)
			drv->chr(drv, x, y, c);
	}
}


/**
 * Draw a vertical bar to all drivers.
 * For drivers that define a vbar() function, call it;
 * otherwise call the general driver_alt_vbar() function from the server core.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param pattern  Options (currently unused).
 */
void
drivers_vbar(int x, int y, int len, int promille, int pattern)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, len=%d, promille=%d, pattern=%d)",
	      __FUNCTION__, x, y, len, promille, pattern);

	/* NEW FUNCTIONS
	 *
	 * We need more data in the widget. Requires language update...
	 */


	ForAllDrivers(drv) {
		if (drv->vbar)
			drv->vbar(drv, x, y, len, promille, pattern);
		else
			driver_alt_vbar(drv, x, y, len, promille, pattern);
	}
}


/**
 * Draw a horizontal bar to all drivers.
 * For drivers that define a hbar() function, call it;
 * otherwise call the general driver_alt_hbar() function from the server core.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param pattern  Options (currently unused).
 */
void
drivers_hbar(int x, int y, int len, int promille, int pattern)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, len=%d, promille=%d, pattern=%d)",
	      __FUNCTION__, x, y, len, promille, pattern);

	ForAllDrivers(drv) {
		if (drv->hbar)
			drv->hbar(drv, x, y, len, promille, pattern);
		else
			driver_alt_hbar(drv, x, y, len, promille, pattern);
	}
}


/**
 * Write a big number to all output drivers.
 * For drivers that define a num() function, call it;
 * otherwise call the general driver_alt_num() function from the server core.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
void
drivers_num(int x, int num)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, num=%d)", __FUNCTION__, x, num);

	ForAllDrivers(drv) {
		if (drv->num)
			drv->num(drv, x, num);
		else
			driver_alt_num(drv, x, num);
	}
}


/**
 * Perform heartbeat on all drivers.
 * For drivers that define a heartbeat() function, call it;
 * otherwise call the general driver_alt_heartbeat() function from the server core.
 * \param state    Heartbeat state.
 */
void
drivers_heartbeat(int state)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(state=%d)", __FUNCTION__, state);

	ForAllDrivers(drv) {
		if (drv->heartbeat)
			drv->heartbeat(drv, state);
		else
			driver_alt_heartbeat(drv, state);
	}
}


/**
 * Write icon to all drivers.
 * For drivers that define a icon() function, call it;
 * otherwise call the general driver_alt_icon() function from the server core.
 * If the driver's locally defined icon() function returns -1, then also
 * call the server core's driver_alt_icon().
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 */
void
drivers_icon(int x, int y, int icon)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, icon=ICON_%s)", __FUNCTION__, x, y, widget_icon_to_iconname(icon));

	ForAllDrivers(drv) {
		/* Does the driver have the icon function ? */
		if (drv->icon) {
			/* Try driver call */
			if (drv->icon(drv, x, y, icon) == -1) {
				/* do alternative call if driver's function does not know the icon */
				driver_alt_icon(drv, x, y, icon);
			}
		} else {
			/* Also do alternative call if the driver does not have icon function */
			driver_alt_icon(drv, x, y, icon);
		}
	}
}


/**
 * Set cursor on all loaded drivers.
 * For drivers that define a cursor() function, call it;
 * otherwise call the general driver_alt_cursor() function from the server core.
 * \param x        Horizontal cursor position (column).
 * \param y        Vertical cursor position (row).
 * \param state    New cursor state.
 */
void
drivers_cursor(int x, int y, int state)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(x=%d, y=%d, state=%d)", __FUNCTION__, x, y, state);

	ForAllDrivers(drv) {
		if (drv->cursor)
			drv->cursor(drv, x, y, state);
		else
			driver_alt_cursor(drv, x, y, state);
	}
}


/**
 * Set backlight on all drivers.
 * Call backlight() function of all drivers that have a backlight() function defined.
 * \param state    New backlight status.
 */
void
drivers_backlight(int state)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(state=%d)", __FUNCTION__, state);

	ForAllDrivers(drv) {
		if (drv->backlight)
			drv->backlight(drv, state);
	}
}


/**
 * Set output on all drivers.
 * Call ouptput() function of all drivers that have an ouptput() function defined.
 * \param state    New ouptut status.
 */
void
drivers_output(int state)
{
	Driver *drv;

	debug(RPT_DEBUG, "%s(state=%d)", __FUNCTION__, state);

	ForAllDrivers(drv) {
		if (drv->output)
			drv->output(drv, state);
	}
}


/**
 * Get key presses from loaded drivers.
 * \return  Pointer to key string for first driver ithat has a get_key() function defined
 *          and for which the get_key() function returns a key; otherwise \c NULL.
 */
const char *
drivers_get_key(void)
{
	/* Find the first input keystroke, if any */
	Driver *drv;
	const char *keystroke;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	ForAllDrivers(drv) {
		if (drv->get_key) {
			keystroke = drv->get_key(drv);
			if (keystroke != NULL) {
				report(RPT_INFO, "Driver [%.40s] generated keystroke %.40s", drv->name, keystroke);
				return keystroke;
			}
		}
	}
	return NULL;
}

