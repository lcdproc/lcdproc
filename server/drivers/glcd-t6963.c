/** \file server/drivers/glcd-t6963.c
 * Connection type to drive displays based on the Toshiba T6963 controller
 * connected to the parallel port. The display is driven in graphic mode. See
 * \c t6963_low.c for details of the parallel port connection.
 *
 * \note  The display must be wired to use 8x8 font. Check with your datasheet.
 */

/*-
 * Copyright (c) 2011 Markus Dolze, based on the t6963 driver by
 *               2001 Manuel Stahl <mythos@xmythos.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"
#include "t6963_low.h"

#define DEFAULT_PORT 0x378

static void t6963_graphic_clear(PrivateData *p);
void glcd_t6963_blit(PrivateData *p);
void glcd_t6963_close(PrivateData *p);

/** Data local to the t6963 connection type */
typedef struct glcd_t6963_data {
	unsigned char *backingstore;	/**< backing buffer */
	T6963_port *port_config;	/**< parallel port configuration */
} CT_t6963_data;

/**
 * API: Initialize the connection type driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
int
glcd_t6963_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	T6963_port *port_config;
	CT_t6963_data *ct_data;

	report(RPT_INFO, "GLCD/T6963: intializing");

	/* Check the size before doing anything else! */
	if ((p->framebuf.px_width > T6963_MAX_WIDTH) || (p->framebuf.px_height > T6963_MAX_HEIGHT)) {
		report(RPT_ERR, "GLCD/T6963: Size %dx%d not supported by connection type",
		       p->framebuf.px_width, p->framebuf.px_height);
		return -1;
	}

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd_t6963_blit;
	p->glcd_functions->close = glcd_t6963_close;

	/* Allocate memory structures */
	ct_data = (CT_t6963_data *) calloc(1, sizeof(CT_t6963_data));
	if (ct_data == NULL) {
		report(RPT_ERR, "GLCD/T6963: error allocating connection data");
		return -1;
	}
	p->ct_data = ct_data;

	port_config = (T6963_port *) calloc(1, sizeof(T6963_port));
	if (port_config == NULL) {
		report(RPT_ERR, "GLCD/T6963: error allocating port config");
		return -1;
	}
	ct_data->port_config = port_config;

	ct_data->backingstore = malloc(p->framebuf.size);
	if (ct_data->backingstore == NULL) {
		report(RPT_ERR, "GLCD/T6963: unable to allocate backing store");
		return -1;
	}
	memset(ct_data->backingstore, 0x00, p->framebuf.size);

	/* Get port from config */
	port_config->port = drvthis->config_get_int(drvthis->name, "Port", 0, DEFAULT_PORT);
	if ((port_config->port < 0x200) || (port_config->port > 0x400)) {
		port_config->port = DEFAULT_PORT;
		report(RPT_WARNING, "GLCD/T6963: Port value must be between 0x200 and 0x400. Using default 0x%03X",
		       DEFAULT_PORT);
	}

	/* Use bi-directional mode of LPT port? Default: yes */
	port_config->bidirectLPT = drvthis->config_get_bool(drvthis->name, "bidirectional", 0, 1);
	/* Additional delay necessary? Default: no */
	port_config->delayBus = drvthis->config_get_bool(drvthis->name, "delayBus", 0, 0);

	/* Now initialize port */
	if (t6963_low_init(port_config) == -1) {
		report(RPT_ERR, "GLCD/T6963: Error initializing port 0x%03X: %s",
		       port_config->port, strerror(errno));
		return -1;
	}

	debug(RPT_INFO, "GLCD/T6963: Sending init to display...");
	/* Set graphic address (and text address even though not needed) */
	t6963_low_command_word(port_config, SET_GRAPHIC_HOME_ADDRESS, GRAPHIC_BASE);
	t6963_low_command_word(port_config, SET_GRAPHIC_AREA, p->framebuf.bytesPerLine);
	t6963_low_command_word(port_config, SET_TEXT_HOME_ADDRESS, TEXT_BASE);
	t6963_low_command_word(port_config, SET_TEXT_AREA, p->framebuf.bytesPerLine);

	/* Use OR-mode for text and graphics */
	t6963_low_command(port_config, SET_MODE | OR_MODE);
	t6963_graphic_clear(p);

	/* Turn on display, text off, graphics on, cursor off */
	t6963_low_command(port_config, SET_DISPLAY_MODE | GRAPHIC_ON);

	debug(RPT_DEBUG, "GLCD/T6963: init() done");

	return 0;
}


/**
 * API: Write the framebuffer to the display
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_t6963_blit(PrivateData *p)
{
	CT_t6963_data *ct_data = (CT_t6963_data *) p->ct_data;
	int x, y;

	for (y = 0; y < p->framebuf.px_height; y++) {
		/* set pointers to start of the line */
		unsigned char *sp = p->framebuf.data + (y * p->framebuf.bytesPerLine);
		unsigned char *sq = ct_data->backingstore + (y * p->framebuf.bytesPerLine);

		/* set pointers to end of the line */
		unsigned char *ep = sp + (p->framebuf.bytesPerLine - 1);
		unsigned char *eq = sq + (p->framebuf.bytesPerLine - 1);

		/* find begin and end of differences */
		x = 0;
		while ((sp <= ep) && (*sp == *sq)) {
			sp++, sq++, x++;
		}
		while ((ep >= sp) && (*ep == *eq)) {
			ep--, eq--;
		}

		/* there are differences, ... */
		if (sp <= ep) {
			t6963_low_command_word(ct_data->port_config, SET_ADDRESS_POINTER,
				  GRAPHIC_BASE + (y * p->framebuf.bytesPerLine) + x);
			t6963_low_command(ct_data->port_config, AUTO_WRITE);
			while (sp <= ep) {
				t6963_low_auto_write(ct_data->port_config, *sp);
				/* Update backing store and advance */
				*sq++ = *sp++;
			}
			t6963_low_command(ct_data->port_config, AUTO_RESET);
		}
	}
}


/**
 * API: Release low-level resources.
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_t6963_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_t6963_data *ct_data = (CT_t6963_data *) p->ct_data;

		if (ct_data->port_config != NULL) {
			t6963_low_close(ct_data->port_config);
			free(ct_data->port_config);
		}

		if (ct_data->backingstore != NULL)
			free(ct_data->backingstore);

		free(p->ct_data);
		p->ct_data = NULL;
	}
}


/**
 * Clears graphic memory.
 * \param p  Pointer to glcd driver's private date structure.
 */
static void
t6963_graphic_clear(PrivateData *p)
{
	CT_t6963_data *ct_data = (CT_t6963_data *) p->ct_data;
	int num = p->framebuf.size;
	int i;

	p->glcd_functions->drv_debug(RPT_DEBUG, "GLCD/T6963: Clearing graphic: %d bytes", num);

	t6963_low_command_word(ct_data->port_config, SET_ADDRESS_POINTER, GRAPHIC_BASE);
	t6963_low_command(ct_data->port_config, AUTO_WRITE);
	for (i = 0; i < num; i++)
		t6963_low_auto_write(ct_data->port_config, 0);
	t6963_low_command(ct_data->port_config, AUTO_RESET);
}
