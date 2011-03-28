/** \file server/drivers/irmanin.c
 * LCDd \c irman input driver for the IrMan IR remote control.
 */

/* irmanin.c - test/demo of LIBIR's to interface with lcdproc (LCDd) */
/* Copyright (C) 1999 David Glaude loosely based on workmanir.c */
/* workmanir.c - test/demo of LIBIR's high level command functions */
/* Copyright (C) 1998 Tom Wheeley, see file COPYING for details    */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define __u32 unsigned int
#define __u8 unsigned char

#define NAME_LENGTH 128

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lcd.h"
#include "irmanin.h"
#include "report.h"
#include "irman.h"


/** mapping between IrMan code names and LCDd key names */
typedef struct _codemap {
	const char *irman;	/**< IrMan code name */
	const char *lcdproc;	/**< LCDproc key name */
} CodeMap;

/** private data for the \c irman driver */
typedef struct irmanin_private_data {
	char device[256];	/**< IrMan device name */
	char config[256];	/**< IrMan config file */
	char *portname;		/**< IrMan port name */
} PrivateData;


CodeMap codemap[] = {
	{ "",  "" },	/* dummy: ir_register_command() needs offset > 0 */
	{ "lcdproc-Up",     "Up"     },
	{ "lcdproc-Down",   "Down"   },
	{ "lcdproc-Left",   "Left"   },
	{ "lcdproc-Right",  "Right"  },
	{ "lcdproc-Enter",  "Enter"  },
	{ "lcdproc-Escape", "Escape" },
	{ NULL, NULL }
};


/* vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "irmanin_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
irmanin_init (Driver *drvthis)
{
	PrivateData *p;
	char *ptrdevice = NULL;
	char *ptrconfig = NULL;
	int i;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Read config file */

	/* What device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   ""), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	if (p->device[0] != '\0') {
		report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);
		ptrdevice = p->device;
	}

	/* What config file should be used */
	strncpy(p->config, drvthis->config_get_string(drvthis->name, "Config", 0,
						   ""), sizeof(p->config));
	p->config[sizeof(p->config)-1] = '\0';
	if (p->config[0] != '\0')
		ptrconfig = p->config;

	/* End of config file parsing */

	if (ir_init_commands(ptrconfig, 1) < 0) {
		report(RPT_ERR, "%s: error initialising commands: %s", drvthis->name, strerror(errno));
		return -1;
	}

	p->portname = ir_default_portname();
	if (p->portname == NULL) {
		if (ptrdevice != NULL) {
			p->portname = ptrdevice;
		} else {
			report(RPT_ERR, "%s: error no device defined", drvthis->name);
			return -1;
		}
	}

	for (i = 1; codemap[i].irman != NULL; i++) {
		if (ir_register_command((char *) codemap[i].irman, i) < 0) {
			if (errno == ENOENT) {
				report(RPT_WARNING, "%s: no code set for `%s'",
					drvthis->name, codemap[i].irman);
			} else {
				report(RPT_WARNING, "%s: error registering `%s': %s",
					drvthis->name, codemap[i].irman, strerror(errno));
			}
		}
	}

	errno = 0;
	if (ir_init(p->portname) < 0) {
		report(RPT_ERR, "%s: error initialising Irman %s: %s",
			drvthis->name, p->portname, strerror(errno));
		return -1;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;  // return success
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
irmanin_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL)
		free(p);
	drvthis->store_private_ptr(drvthis, NULL);

	ir_free_commands();
	ir_finish();
}

/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
irmanin_get_key (Driver *drvthis)
{
	int cmd;
	const char *key = NULL;

	switch (cmd = ir_poll_command()) {
	case IR_CMD_ERROR:
		report(RPT_WARNING, "%s: error reading command: %s",
			drvthis->name, strerror(errno));
		break;
	case IR_CMD_UNKNOWN:
		break;
	default:
		if ((cmd > 0) && (cmd < 7))	 // only 6 keys, startinig at ofset 1
			key = codemap[cmd].lcdproc;
		break;
	}

	return key;
}

/* end of irmanin.c */
