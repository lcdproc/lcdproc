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

#define NAME_LENGTH 128
#define JOY_DEFAULT_DEVICE "/dev/js0"

int fd;

struct js_event js;

char axes = 2;
char buttons = 2;
int jsversion = 0x000800;
char jsname[NAME_LENGTH] = "Unknown";

int *axis = NULL;
int *button = NULL;

// Configured for a Gravis Gamepad  (2 axis, 4 button)
char *axismap = "EFGHIJKLMNOPQRST";
char *buttonmap = "BDACEFGHIJKLMNOP";

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "joy_";


////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
int
joy_init (Driver *drvthis, char *args)
{
	char device[256];
	char *argv[64];
	int argc, i;

	strcpy (device, JOY_DEFAULT_DEVICE);

	argc = get_args (argv, args, 64);

	for (i = 0; i < argc; i++) {
		//printf("Arg(%i): %s\n", i, argv[i]);
		if (0 == strcmp (argv[i], "-d") || 0 == strcmp (argv[i], "--device")) {
			if (i + 1 > argc) {
				fprintf (stderr, "joy_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strcpy (device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-a") || 0 == strcmp (argv[i], "--axes")) {
			if (i + 1 > argc) {
				fprintf (stderr, "joy_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strncpy (axismap, argv[++i], 16);
		} else if (0 == strcmp (argv[i], "-b") || 0 == strcmp (argv[i], "--buttons")) {
			if (i + 1 > argc) {
				fprintf (stderr, "joy_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strncpy (buttonmap, argv[++i], 16);
		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			printf ("LCDproc Joystick input driver\n" "\t-d\t--device\tSelect the input device to use [/dev/js0]\n" "\t-a\t--axes\t\tModify the axis map [%s]\n" "\t-b\t--buttons\tModify the button map [%s]\n" "\t-h\t--help\t\tShow this help information\n", axismap, buttonmap);
			return -1;
		} else {
			printf ("Invalid parameter: %s\n", argv[i]);
		}

	}

	// Set variables for server
	drvthis->api_version = api_version;
	drvthis->stay_in_foreground = &stay_in_foreground;
	drvthis->supports_multiple = &supports_multiple;

	// Set the functions the driver supports
	drvthis->getkey = joy_getkey;
	drvthis->close = joy_close;

	if ((fd = open (device, O_RDONLY)) < 0)
		return -1;

	fcntl (fd, F_SETFL, O_NONBLOCK);
	ioctl (fd, JSIOCGVERSION, &jsversion);
	ioctl (fd, JSIOCGAXES, &axes);
	ioctl (fd, JSIOCGBUTTONS, &buttons);
	ioctl (fd, JSIOCGNAME (NAME_LENGTH), jsname);

	report (RPT_NOTICE, "Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d.\n",
		jsname, axes, buttons,
		jsversion >> 16, (jsversion >> 8) & 0xff, jsversion & 0xff);

	if ((axis = calloc (axes, sizeof (int))) == NULL) {
		report (RPT_ERR, "joystick: could not allocate memory for axes");
		return -1;
	}

	if ((button = calloc (buttons, sizeof (char))) == NULL) {
		report (RPT_ERR, "joystick: could not allocate memory for buttons");
		return -1;
	}

	return 0;
}

MODULE_EXPORT void
joy_close (Driver *drvthis)
{
	close (fd);

	// Why do I have so much trouble getting memory freed without segfaults??
	// Use gdb and find out :) In preliminary testing, this seemed to work...

	if(axis) free(axis);
	if(button) free(button);

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

	if ((err = read (fd, &js, sizeof (struct js_event))) <= 0) {
		return 0;
	} else
		if (err != sizeof (struct js_event)) {
			report(RPT_ERR, "error reading joystick input");
			return 0;
		}

//   if(js.type & JS_EVENT_INIT) return 0;

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
