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
#include <syslog.h>

#include <linux/joystick.h>
#ifndef JSIOCGNAME
#define JSIOCGNAME(len)           _IOC(_IOC_READ, 'j', 0x13, len)         /* get identifier string */
#endif

#include "shared/debug.h"
#include "shared/str.h"

#define NAME_LENGTH 128
#define JOY_DEFAULT_DEVICE "/dev/js0"

#include "lcd.h"
#include "joy.h"

lcd_logical_driver *joy;

int fd;
extern int debug_level;

struct js_event js;

char axes = 2;
char buttons = 2;
int jsversion = 0x000800;
char jsname[NAME_LENGTH] = "Unknown";

int *axis;
int *button;

// Configured for a Gravis Gamepad  (2 axis, 4 button)
char *axismap = "EFGHIJKLMNOPQRST";
char *buttonmap = "BDACEFGHIJKLMNOP";

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
int
joy_init (struct lcd_logical_driver *driver, char *args)
{
	char device[256];
	char *argv[64];
	int argc, i;

	joy = driver;

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

	driver->getkey = joy_getkey;
	driver->close = joy_close;

	if ((fd = open (device, O_RDONLY)) < 0)
		return -1;

	fcntl (fd, F_SETFL, O_NONBLOCK);
	ioctl (fd, JSIOCGVERSION, &jsversion);
	ioctl (fd, JSIOCGAXES, &axes);
	ioctl (fd, JSIOCGBUTTONS, &buttons);
	ioctl (fd, JSIOCGNAME (NAME_LENGTH), jsname);

	if (debug_level > 2) {
		syslog(LOG_DEBUG, "Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d.\n",
			jsname, axes, buttons,
			jsversion >> 16, (jsversion >> 8) & 0xff, jsversion & 0xff);
	}

	if ((axis = calloc (axes, sizeof (int))) == NULL) {
		syslog(LOG_ERR, "joystick: could not allocate memory for axes");
		return -1;
	}

	if ((button = calloc (buttons, sizeof (char))) == NULL) {
		syslog(LOG_ERR, "joystick: could not allocate memory for buttons");
		return -1;
	}

	return fd;						  // 200 is arbitrary.  (must be 1 or more)
}

void
joy_close ()
{
	if (joy->framebuf != NULL)
		free (joy->framebuf);

	close (fd);

	joy->framebuf = NULL;

	// Why do I have so much trouble getting memory freed without segfaults??
	// Use gdb and find out :) In preliminary testing, this seemed to work...

	if (axis) free(axis);
	if (button) free(button);

}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
joy_getkey ()
{
	int i;
	int err;

	if ((err = read (fd, &js, sizeof (struct js_event))) <= 0) {
		return 0;
	} else
		if (err != sizeof (struct js_event)) {
			syslog(LOG_ERR, "error reading joystick input");
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
