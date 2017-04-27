/** \file server/drivers/linux_input.c
 * LCDd \c linux event device driver for inputting data from the input
 * subsystem of the linux kernel..
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
	int code;
	char *button;
	struct keycode *ret;

	code = atoi(configvalue);
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
	LinkedList *buttonmap;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "linuxInput_";


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


	if ((p->fd = open(s, O_RDONLY | O_NONBLOCK)) < 0) {
		report(RPT_ERR, "%s: open(%s) failed (%s)",
				drvthis->name, s, strerror(errno));
		return -1;
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
 * Read the next input event.
 * \param drvthis  Pointer to driver structure.
 * \retval         String representation of the key;
 *                 \c NULL for nothing available / error.
 */
MODULE_EXPORT const char *
linuxInput_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	struct input_event event;
	struct keycode *k;

	if (read(p->fd, &event, sizeof(event)) != sizeof(event))
		return NULL;

	/* Ignore release events and not-key events */
	if (event.type != EV_KEY || event.value == 0)
		return NULL;

	switch (event.code) {
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

	default:
		LL_Rewind(p->buttonmap);
		k = LL_Find(p->buttonmap, compare_with_keycode, &event.code);
		if (k)
			return k->button;
		else
			report(RPT_INFO, "linux_input: Unknown key code: %d", event.code);

		return NULL;
	}
}
