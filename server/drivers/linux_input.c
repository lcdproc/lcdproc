/** \file server/drivers/linux_input.c
 * LCDd \c linux event device driver for inputting data from the input
 * subsystem of the linux kernel..
 */

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "linux_input.h"
#include "shared/report.h"
#include "shared/LL.h"

#define LINUXINPUT_DEFAULT_DEVICE	"/dev/input/event0"

/** describe the button of a keycode */
struct keycode {
	unsigned short code;
	char *button;
};

/**
 * Parse key definition from config file
 * \param configvalue value part of the config file entry
 * \retval NULL Error.
 * \retval else Pointer to newly allocated struct keycode.
 */
static struct keycode *
keycode_create(const char *configvalue)
{
	long code;
	char *button;
	struct keycode *ret;

	code = strtol(configvalue, NULL, 0);
	if (code < 0 || code > UINT16_MAX)
		return NULL;

	button = strchr(configvalue,',');
	if (!button)
		return NULL;
	button = strdup(&button[1]);
	if (!button)
		return NULL;

	ret = malloc(sizeof(*ret));
	if (ret) {
		ret->code = code;
		ret->button = button;
	}

	return ret;
}

/** private data for the linux event device driver */
typedef struct linuxInput_private_data {
	int fd;
	/* For re-acquiring the device on connection loss when openen by name */
	const char *name;
	LinkedList *buttonmap;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "linuxInput_";

static int linuxInput_open_with_name(const char *device, const char *name)
{
	char buf[256];
	int err, fd;

	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd == -1)
		return -1;

	err = ioctl(fd, EVIOCGNAME(256), buf);
	if (err == -1) {
		close(fd);
		return -1;
	}
	/* The kernel does not add a 0 terminator if the name is too long */
	buf[255] = 0;

	if (strcmp(buf, name)) {
		/* Not the device we are looking for */
		close(fd);
		return -1;
	}

	return fd;
}

static int linuxInput_search_by_name(const char *name)
{
	char devname[PATH_MAX];
	struct dirent *dirent;
	int fd = -1;
	DIR *dir;

	dir = opendir("/dev/input");
	if (dir == NULL)
		return -1;

	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_type != DT_CHR ||
		    strncmp(dirent->d_name, "event", 5))
			continue;

		strcpy(devname, "/dev/input/");
		strcat(devname, dirent->d_name);

		fd = linuxInput_open_with_name(devname, name);
		if (fd != -1)
			break;
	}

	closedir(dir);

	return fd;
}

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
linuxInput_init (Driver *drvthis)
{
	PrivateData *p;
	const char *s;
	struct keycode *key;
	int i;

        /* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->fd = -1;
	if ((p->buttonmap = LL_new()) == NULL) {
		report(RPT_ERR, "%s: cannot allocate memory for buttons", drvthis->name);
		return -1;
	}

	/* Read config file */

	/* What device should be used */
	s = drvthis->config_get_string(drvthis->name, "Device", 0,
						   LINUXINPUT_DEFAULT_DEVICE);
	report(RPT_INFO, "%s: using Device %s", drvthis->name, s);


	/* Open the device, eiher by path or by name */
	if (s[0] == '/') {
		if ((p->fd = open(s, O_RDONLY | O_NONBLOCK)) == -1) {
			report(RPT_ERR, "%s: open(%s) failed (%s)",
					drvthis->name, s, strerror(errno));
			return -1;
		}
	} else {
		if ((p->fd = linuxInput_search_by_name(s)) == -1) {
			report(RPT_ERR, "%s: could not find '%s' input-device",
					drvthis->name, s);
			return -1;
		}
		p->name = s;
	}

	for (i = 0; (s = drvthis->config_get_string(drvthis->name, "key", i, NULL)) != NULL; i++) {
		if ((key = keycode_create(s)) == NULL) {
			report(RPT_ERR, "%s: parsing configvalue '%s' failed",
					drvthis->name, s);
			continue;
		}
		LL_AddNode(p->buttonmap, key);
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
linuxInput_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	struct keycode *k;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->buttonmap != NULL) {
			while ((k = LL_Pop(p->buttonmap)) != NULL) {
				free(k->button);
				free(k);
			}
			LL_Destroy(p->buttonmap);
		}

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Helper function to check LL items against a given key code.
 * \param data   Pointer to data of list item
 * \param codep  Pointer to value holding the received key code
 * \retval 0     We found the right item.
 * \retval else  We need to continue searching.
 */
static int
compare_with_keycode (void *data, void *codep)
{
	uint16_t code = *(uint16_t *)codep;
	struct keycode *k = data;

	return k->code != code;
}

/**
 * Helper function to read a key code from the linux input device.
 * \param p      Pointer to driver linuxInput PrivateData structure
 * \retval > 0   Linux KEY_ key-code
 * \retval 0     Non key-press event read
 * \retval -1    No events are queued
 */
static int
linuxInput_get_key_code (PrivateData *p)
{
	struct input_event event;
	int result = -1;

	if (p->fd != -1) {
		result = read(p->fd, &event, sizeof(event));
		/* Device unplugged / lost connection ? */
		if (result == -1 && errno == ENODEV) {
			report(RPT_WARNING, "Lost input device connection");
			close(p->fd);
			p->fd = -1;
		}
	}

	/*
	 * We may temporary loose access to the device. Possible causes are e.g.:
	 * 1. A temporary connection loss (Bluetooth); or
	 * 2. The device dropping of the bus to re-appear with another prod-id
	 *    (the G510 keyboard does this when (un)plugging the headphones).
	 * If the device was opened by name, we try to re-acquire the device
	 * here to deal with these kinda temporary device losses.
	 */
	if (p->fd == -1 && p->name) {
		p->fd = linuxInput_search_by_name(p->name);
		if (p->fd != -1) {
			report(RPT_WARNING, "Successfully re-opened input device '%s'", p->name);
			result = read(p->fd, &event, sizeof(event));
		}
	}

	if (result != sizeof(event))
		return -1;

	/* Ignore release events and not-key events */
	return (event.type == EV_KEY && event.value) ? event.code : 0;
}

/**
 * Helper function to convert a key code to a key name.
 * \param p      Pointer to driver linuxInput PrivateData structure
 * \param code   Linux KEY_ key-code to convert
 * \retval       String representation of the key;
 *               \c NULL for nothing available / error.
 */
static const char *
linuxInput_key_code_to_key_name (PrivateData *p, uint16_t code)
{
	struct keycode *k;

	if (code == 0)
		return NULL;

	if (LL_GetFirst(p->buttonmap)) {
		/* Use user config for button mapping */
		k = LL_Find(p->buttonmap, compare_with_keycode, &code);
		if (k)
			return k->button;
	} else {
		/* No user config, fallback to defaults. */
		switch (code) {
		case KEY_ESC:
			return "Escape";

		case KEY_UP:
			return "Up";

		case KEY_LEFT:
			return "Left";

		case KEY_RIGHT:
			return "Right";

		case KEY_DOWN:
			return "Down";

		case KEY_ENTER:
		case KEY_KPENTER:
			return "Enter";
		}
	}

	report(RPT_INFO, "linux_input: Unknown key code: %d", code);
	return NULL;
}

/**
 * Read the next input event.
 * \param drvthis  Pointer to driver structure.
 * \retval         String representation of the key;
 *                 \c NULL for nothing available / error.
 */
MODULE_EXPORT const char *
linuxInput_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	const char *retval = NULL;
	int code;

	/*
	 * We may have buildup a backlog of events between polls, keep reading
	 * events until we are out of events, or we get a valid key-name.
	 */
	do {
		code = linuxInput_get_key_code(p);
	} while(code >= 0 &&
		(retval = linuxInput_key_code_to_key_name(p, code)) == NULL);

	return retval;
}
