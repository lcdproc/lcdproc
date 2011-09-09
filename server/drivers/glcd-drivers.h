/** \file server/drivers/glcd-drivers.h
 * Connection type registry for glcd driver.
 *
 * File file contains includes and pointers to the connection type's init()
 * function.
 */

#ifndef GLCD_DRIVERS_H
#define GLCD_DRIVERS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Include connection type header below */
#ifdef HAVE_PCSTYLE_LPT_CONTROL
# include "glcd-t6963.h"
#endif

/* symbolic names for connection types */
#define GLCD_CT_UNKNOWN		0
#define GLCD_CT_T6963		1

/** Structure linking symbolic names to initialization routines */
typedef struct ConnectionMapping {
	char *name;
	int connectiontype;
	int (*init_fn)(Driver *drvthis);
} ConnectionMapping;


/** connectionType mapping table:
 * - string to identify connection in config file
 * - connection type identifier
 * - initialization function
 */
static const ConnectionMapping connectionMapping[] = {
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	{"t6963", GLCD_CT_T6963, glcd_t6963_init},
#endif
	/* default, end of structure element (do not delete) */
	{NULL, GLCD_CT_UNKNOWN, NULL}
};

#endif
