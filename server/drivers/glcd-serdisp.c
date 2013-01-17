/** \file server/drivers/glcd-serdisp.c
 * Connection type using serdisplib for output (bridge driver).
 */

/*-
 * Copyright (c) 2010 Bernhard Walle <bernhard@bwalle.de>
 *               2011 Markus Dolze <bsdfan@nurfuerspam.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <limits.h>
#include <serdisplib/serdisp.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"

#define SERDISPLIB_MAX_DISPLAYNAME	32
#define SERDISPLIB_MAX_DEVICENAME	PATH_MAX

/* Prototypes */
void glcd_serdisp_blit(PrivateData *p);
void glcd_serdisp_close(PrivateData *p);
void glcd_serdisp_backlight(PrivateData *p, int state);

/** Private data for the serdisplib connection type */
typedef struct glcd_serdisp_data {
	/** the name of the display driver in serdisplib, e.g. 'ctinclud' */
	char display_name[SERDISPLIB_MAX_DISPLAYNAME];

	/** the name of the device in serdisplib, e.g. /dev/parport0 */
	char display_device[SERDISPLIB_MAX_DEVICENAME];

	/** the serdisplib connection handle */
	serdisp_CONN_t *serdisplib_conn;

	/** the serdisplib handle */
	serdisp_t *disp;

	/** framebuffer for incremental updates */
	struct glcd_framebuf bsbuf;
} CT_serdisp_data;

/**
 * API: Initialize the connection type driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
int
glcd_serdisp_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	CT_serdisp_data *ct_data;
	const char *s;

	report(RPT_INFO, "glcd/serdisplib: intializing...");

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd_serdisp_blit;
	p->glcd_functions->close = glcd_serdisp_close;
	p->glcd_functions->set_backlight = glcd_serdisp_backlight;

	/* Allocate memory structures */
	ct_data = (CT_serdisp_data *) calloc(1, sizeof(CT_serdisp_data));
	if (ct_data == NULL) {
		report(RPT_ERR, "%s: error allocating connection data",
		       drvthis->name);
		return -1;
	}
	p->ct_data = ct_data;

	/* get the display name */
	s = drvthis->config_get_string(drvthis->name, "serdisp_name", 0, NULL);
	if (s == NULL) {
		report(RPT_ERR, "%s: \'serdisp_name\' missing in configuration",
		       drvthis->name);
		goto err_out;
	}
	strncpy(ct_data->display_name, s, SERDISPLIB_MAX_DISPLAYNAME);
	ct_data->display_name[SERDISPLIB_MAX_DISPLAYNAME - 1] = '\0';

	/* get the display device */
	s = drvthis->config_get_string(drvthis->name, "serdisp_device", 0, NULL);
	if (s == NULL) {
		report(RPT_ERR, "%s: \'serdisp_device\' missing in configuration",
		       drvthis->name);
		goto err_out;
	}
	strncpy(ct_data->display_device, s, SERDISPLIB_MAX_DEVICENAME);
	ct_data->display_device[SERDISPLIB_MAX_DEVICENAME - 1] = '\0';

	/* open the output device */
	ct_data->serdisplib_conn = SDCONN_open(ct_data->display_device);
	if (ct_data->serdisplib_conn == NULL) {
		report(RPT_ERR, "Could not open %s: %s", ct_data->display_device,
		       sd_geterrormsg());
		goto err_out;
	}

	/* open and initialize the display with options */
	s = drvthis->config_get_string(drvthis->name, "serdisp_options", 0, "");
	debug(RPT_INFO, "%s: Using serdisp options: %s", drvthis->name, s);
	ct_data->disp = serdisp_init(ct_data->serdisplib_conn, ct_data->display_name, s);
	if (ct_data->disp == NULL) {
		report(RPT_ERR, "Error opening display %s: %s\n",
		       ct_data->display_name, sd_geterrormsg());
		goto err_out;
	}

	/* set size overriding anything set in serdisp_options */
	serdisp_setoption(ct_data->disp, "WIDTH", p->framebuf.px_width);
	serdisp_setoption(ct_data->disp, "HEIGHT", p->framebuf.px_height);

	/* allocate backing store based on frame buffer */
	ct_data->bsbuf.px_width = p->framebuf.px_width;
	ct_data->bsbuf.px_height = p->framebuf.px_height;
	ct_data->bsbuf.bytesPerLine = p->framebuf.bytesPerLine;
	ct_data->bsbuf.size = p->framebuf.size;
	ct_data->bsbuf.data = malloc(ct_data->bsbuf.size);
	if (ct_data->bsbuf.data == NULL) {
		report(RPT_ERR, "%s: error allocating backing store",
		       drvthis->name);
		goto err_out;
	}
	memset(ct_data->bsbuf.data, 0x00, ct_data->bsbuf.size);

	serdisp_clearbuffer(ct_data->disp);

	debug(RPT_INFO, "glcd/serdisplib: finished");
	return 0;

err_out:
	glcd_serdisp_close(p);
	return -1;
}


/**
 * API: Release low-level resources.
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_serdisp_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_serdisp_data *ct_data = (CT_serdisp_data *) p->ct_data;

		if (ct_data->serdisplib_conn && !ct_data->disp)
			SDCONN_close(ct_data->serdisplib_conn);
		if (ct_data->disp)
			serdisp_quit(ct_data->disp);
		if (ct_data->bsbuf.data) {
			free(ct_data->bsbuf.data);
			ct_data->bsbuf.data = NULL;
		}

		free(p->ct_data);
		p->ct_data = NULL;
	}
}


/**
 * API: Write the framebuffer to the display
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_serdisp_blit(PrivateData *p)
{
	CT_serdisp_data *ct_data = (CT_serdisp_data *) p->ct_data;
	int px, py;
	int pixel_new, pixel_old;

	/*
	 * Update method: go through the whole framebuffer line by line and
	 * compare each pixel with the one in the backing store. If different
	 * draw to serdisplib.
	 */
	for (py = 0; py < p->framebuf.px_height; py++) {
		for (px = 0; px < p->framebuf.px_width; px++) {
			pixel_old = fb_get_pixel(&(ct_data->bsbuf), px, py);
			pixel_new = fb_get_pixel(&(p->framebuf), px, py);
			if (pixel_old != pixel_new) {
				serdisp_setcolour(ct_data->disp, px, py,
						  (pixel_new == FB_BLACK) ? SD_COL_BLACK : SD_COL_WHITE);
				fb_draw_pixel(&(ct_data->bsbuf), px, py, pixel_new);
			}
		}
	}
	serdisp_update(ct_data->disp);
}


/**
 * API: Turn the backlight on or off. The serdisp library does not support
 * backlight levels. Any level set in glcd driver is ignored.
 * \param p
 * \param state
 */
void
glcd_serdisp_backlight(PrivateData *p, int state)
{
	CT_serdisp_data *ct_data = (CT_serdisp_data *) p->ct_data;

	if (state == BACKLIGHT_ON)
		serdisp_setoption(ct_data->disp, "BACKLIGHT", SD_OPTION_YES);
	else
		serdisp_setoption(ct_data->disp, "BACKLIGHT", SD_OPTION_NO);
}
