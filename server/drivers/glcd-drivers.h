/** \file server/drivers/glcd-drivers.h
 * Connection type registry for glcd driver.
 *
 * File file contains prototypes and pointers to the connection type's init()
 * function.
 */

#ifndef GLCD_DRIVERS_H
#define GLCD_DRIVERS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Include prototypes for initialization functions below */
#ifdef HAVE_PCSTYLE_LPT_CONTROL
int glcd_t6963_init(Driver *drvthis);
#endif
#ifdef HAVE_LIBPNG
int glcd_png_init(Driver *drvthis);
#endif
#ifdef HAVE_SERDISPLIB
int glcd_serdisp_init(Driver *drvthis);
#endif
#ifdef HAVE_LIBUSB
int glcd2usb_init(Driver *drvthis);
int glcd_picolcdgfx_init(Driver *drvthis);
#endif
#ifdef HAVE_LIBX11
int glcd_x11_init(Driver *drvthis);
#endif

/* symbolic names for connection types */
#define GLCD_CT_UNKNOWN		0
#define GLCD_CT_T6963		1
#define GLCD_CT_PNG		2
#define GLCD_CT_SERDISP		3
#define GLCD_CT_GLCD2USB	4
#define GLCD_CT_X11		5
#define GLCD_CT_PICOLCDGFX	6

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
#ifdef HAVE_LIBPNG
	{"png", GLCD_CT_PNG, glcd_png_init},
#endif
#ifdef HAVE_SERDISPLIB
	{"serdisplib", GLCD_CT_SERDISP, glcd_serdisp_init},
#endif
#ifdef HAVE_LIBUSB
	{"glcd2usb", GLCD_CT_GLCD2USB, glcd2usb_init},
	{"picolcdgfx", GLCD_CT_PICOLCDGFX, glcd_picolcdgfx_init},
#endif
#ifdef HAVE_LIBX11
	{"x11", GLCD_CT_X11, glcd_x11_init},
#endif
	/* default, end of structure element (do not delete) */
	{NULL, GLCD_CT_UNKNOWN, NULL}
};

#endif
