/*
 * Joystick input driver for LCDd
 *
 * Only two unique functions are defined:
 *
 * joy_getkey
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
#include <sys/errno.h>
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
#include "shared/str.h"

#define JOY_NAMELENGTH		128
#define JOY_DEFAULT_DEVICE	"/dev/js0"
#define JOY_MAPSIZE		16
#define JOY_DEFAULT_AXISMAP	"EFGHIJKLMNOPQRST"
#define JOY_DEFAULT_BUTTONMAP	"BDACEFGHIJKLMNOP"

int fd = -1;

struct js_event js;

char axes = 2;
char buttons = 2;
int jsversion = 0x000801;
char jsname[JOY_NAMELENGTH] = "Unknown";

int *axis = NULL;
int *button = NULL;

// Configured for a Gravis Gamepad  (2 axis, 4 button)
char axismap[JOY_MAPSIZE+1] = JOY_DEFAULT_AXISMAP;
char buttonmap[JOY_MAPSIZE+1] = JOY_DEFAULT_BUTTONMAP;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "joy_";


////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
joy_init (Driver *drvthis)
{
	char device[256];

	/* Read config file */

	/* What device should be used */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   JOY_DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	/* How does the axis map look like */
	strncpy(axismap, drvthis->config_get_string(drvthis->name, "AxisMap", 0,
						    JOY_DEFAULT_AXISMAP), sizeof(axismap));
	axismap[sizeof(axismap)-1] = '\0';

	/* How does the button map look like */
	strncpy(buttonmap, drvthis->config_get_string(drvthis->name, "ButtonMap", 0,
						      JOY_DEFAULT_BUTTONMAP), sizeof(buttonmap));
	buttonmap[sizeof(buttonmap)-1] = '\0';

	/* End of config file parsing */
	
	if ((fd = open(device, O_RDONLY)) < 0) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", 
				drvthis->name, device, strerror(errno));
		return -1;
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);
	ioctl(fd, JSIOCGVERSION, &jsversion);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(JOY_NAMELENGTH), jsname);

	report(RPT_NOTICE, "%s: Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d",
		drvthis->name, jsname, axes, buttons,
		jsversion >> 16, (jsversion >> 8) & 0xff, jsversion & 0xff);

	if ((axis = calloc(axes, sizeof (int))) == NULL) {
		report(RPT_ERR, "%s: could not allocate memory for axes", drvthis->name);
		return -1;
	}

	if ((button = calloc(buttons, sizeof (char))) == NULL) {
		report(RPT_ERR, "%s: could not allocate memory for buttons", drvthis->name);
		return -1;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

MODULE_EXPORT void
joy_close (Driver *drvthis)
{
	if (fd >= 0)
		close(fd);

	// Why do I have so much trouble getting memory freed without segfaults??
	// Use gdb and find out :) In preliminary testing, this seemed to work...

	if (axis != NULL)
		free(axis);
	if (button != NULL)
		free(button);

}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
MODULE_EXPORT char
joy_getkey (Driver *drvthis)
{
	int i;
	int err;

	if ((err = read(fd, &js, sizeof(struct js_event))) <= 0) {
		return 0;
	} else
		if (err != sizeof(struct js_event)) {
			report(RPT_ERR, "%s: error reading joystick input", drvthis->name);
			return 0;
		}

//   if (js.type & JS_EVENT_INIT) return 0;

	switch (js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_BUTTON:
			button[js.number] = js.value;
			break;
		case JS_EVENT_AXIS:
			axis[js.number] = js.value;
			break;
	}

	if (buttons) {
		//printf("Buttons: ");
		for (i = 0; i < buttons; i++)
			//printf("%2d:%s ", i, button[i] ? "on " : "off");
			if (button[i])
				return buttonmap[i];
	}

	if (axes) {
		//printf("Axes: ");
		for (i = 0; i < axes; i++) {
			//printf("%2d:%6d ", i, axis[i]);
			// Eliminate noise...
			if (axis[i] > 20000)
				return axismap[(2 * i) + 1];
			if (axis[i] < -20000)
				return axismap[(2 * i)];
		}
	}

	return 0;
}

