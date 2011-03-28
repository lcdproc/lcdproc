/** \file server/drivers/joy.c
 * LCDd \c joy input driver for inputting data using joysticks.
 */

/*
 * Joystick input driver for LCDd
 *
 * Only two unique functions are defined:
 *
 * joy_get_key
 * joy_close
 *
 * All others are at their defaults.
 *
 * The code here is configured for a Gravis Gamepad (2 axis, 4 button)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/joystick.h>
#ifndef JSIOCGNAME
#define JSIOCGNAME(len)           _IOC(_IOC_READ, 'j', 0x13, len)         /* get identifier string */
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "joy.h"
#include "report.h"

#define JOY_NAMELENGTH		128
#define JOY_DEFAULT_DEVICE	"/dev/js0"
#define JOY_MAPSIZE		16
#define JOY_DEFAULT_AXISMAP	"EFGHIJKLMNOPQRST"
#define JOY_DEFAULT_BUTTONMAP	"BDACEFGHIJKLMNOP"


/** private data for the \c joy driver */
typedef struct joy_private_data {
	char device[256];
	int fd;

	char axes;
	char buttons;
	int jsversion;
	char jsname[JOY_NAMELENGTH];

	char **axismap;
	char **buttonmap;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "joy_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
joy_init (Driver *drvthis)
{
	PrivateData *p;
	int i;

        /* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->fd = -1;
	p->axes = 2;
	p->buttons = 2;
	p->jsversion = 0;
	strcpy(p->jsname, "Unknown");
	p->axismap = NULL;
	p->buttonmap = NULL;


	/* Read config file (1st part) */

	/* What device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   JOY_DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* End of config file parsing (1st part) */

	if ((p->fd = open(p->device, O_RDONLY)) < 0) {
		report(RPT_ERR, "%s: open(%s) failed (%s)",
				drvthis->name, p->device, strerror(errno));
		return -1;
	}

	/* init joystick, get values for buttons, exes, name, ... */
	fcntl(p->fd, F_SETFL, O_NONBLOCK);
	ioctl(p->fd, JSIOCGVERSION, &p->jsversion);
	ioctl(p->fd, JSIOCGAXES, &p->axes);
	ioctl(p->fd, JSIOCGBUTTONS, &p->buttons);
	ioctl(p->fd, JSIOCGNAME(JOY_NAMELENGTH), p->jsname);

	report(RPT_NOTICE, "%s: Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d",
		drvthis->name, p->jsname, p->axes, p->buttons,
		p->jsversion >> 16, (p->jsversion >> 8) & 0xff, p->jsversion & 0xff);

	if ((p->axismap = calloc(2 * p->axes, sizeof(char *))) == NULL) {
		report(RPT_ERR, "%s: cannot allocate memory for axes", drvthis->name);
		return -1;
	}

	if ((p->buttonmap = calloc(p->buttons, sizeof(char *))) == NULL) {
		report(RPT_ERR, "%s: cannot allocate memory for buttons", drvthis->name);
		return -1;
	}

	/* Read config file (2nd part) */

	for (i = 0; i < p->axes; i++) {
		char mapkey[50];
		const char *mapval;

		snprintf(mapkey, sizeof(mapkey), "Map_Axis%dneg", i+1);
		mapval = drvthis->config_get_string(drvthis->name, mapkey, 0, NULL);
		if (mapval != NULL) {
			p->axismap[2*i] = strdup(mapval);
			report(RPT_DEBUG, "%s: map Axis%dneg to %s",
					drvthis->name, i+1, p->axismap[2*i]);
		}

		snprintf(mapkey, sizeof(mapkey), "Map_Axis%dpos", i+1);
		mapval = drvthis->config_get_string(drvthis->name, mapkey, 0, NULL);
		if (mapval != NULL) {
			p->axismap[2*i + 1] = strdup(mapval);
			report(RPT_DEBUG, "%s: map Axis%dpos to %s",
					drvthis->name, i+1, p->axismap[2*i + 1]);
		}
	}

	for (i = 0; i < p->buttons; i++) {
		char mapkey[50];
		const char *mapval;

		snprintf(mapkey, sizeof(mapkey), "Map_Button%d", i+1);
		mapval = drvthis->config_get_string(drvthis->name, mapkey, 0, NULL);
		if (mapval != NULL) {
			p->buttonmap[i] = strdup(mapval);
			report(RPT_DEBUG, "%s: map Button%d to %s",
					drvthis->name, i+1, p->buttonmap[i]);
		}
	}

	/* End of config file parsing (2nd part) */

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
joy_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->axismap != NULL)
			free(p->axismap);
		if (p->buttonmap != NULL)
			free(p->buttonmap);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Get key from the joystick.
 * Map the joystick events (move, button press) to a key.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL for nothing available / error.
 */
MODULE_EXPORT const char *
joy_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	struct js_event js;
	int err;

	if ((err = read(p->fd, &js, sizeof(struct js_event))) <= 0) {
		return NULL;
	}
	if (err != sizeof(struct js_event)) {
		report(RPT_ERR, "%s: error reading joystick input", drvthis->name);
		return NULL;
	}

	switch (js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_BUTTON:
			/* ignore button release */
			if ((js.value == 0) || (js.number >= p->buttons))
				return NULL;
			return p->buttonmap[js.number];
		case JS_EVENT_AXIS:
			/* ignore noise */
			if (((js.value > -20000) && (js.value < 20000))
			    || (js.number >= 2 * p->axes))
				return NULL;
			return p->axismap[js.number];
		default:
			return NULL;
	}
}

