/** \file server/drivers/glcd-png.c
 * This driver writes the framebuffer content to PNG images as
 * /tmp/lcdproc######.png.
 */

/*-
 * Copyright (c) 2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <png.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"

/* Prototypes */
void glcd_png_blit(PrivateData *p);
void glcd_png_close(PrivateData *p);

/** Private data for the PNG connection type */
typedef struct glcd_png_data {
	unsigned char *backingstore;	/**< backing buffer */
} CT_png_data;

/**
 * API: Initialize the connection type driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
int
glcd_png_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	CT_png_data *ct_data;

	report(RPT_INFO, "GLCD/png: intializing");

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd_png_blit;
	p->glcd_functions->close = glcd_png_close;

	/* Allocate memory structures */
	ct_data = (CT_png_data *) calloc(1, sizeof(CT_png_data));
	if (ct_data == NULL) {
		report(RPT_ERR, "GLCD/png: error allocating connection data");
		return -1;
	}
	p->ct_data = ct_data;

	ct_data->backingstore = malloc(p->framebuf.size);
	if (ct_data->backingstore == NULL) {
		report(RPT_ERR, "GLCD/png: unable to allocate backing store");
		return -1;
	}
	memset(ct_data->backingstore, 0x00, p->framebuf.size);

	debug(RPT_DEBUG, "GLCD/png: init() done");

	return 0;
}

/**
 * API: Write the framebuffer to the display
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_png_blit(PrivateData *p)
{
	CT_png_data *ct_data = (CT_png_data *) p->ct_data;
	char filename[256];
	static int num = 0;
	int row;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row_pointer;

	/* Check if framebufer has changed. If not there's nothing to do */
	if (memcmp(p->framebuf.data, ct_data->backingstore, p->framebuf.size) == 0)
		return;

	snprintf(filename, sizeof(filename), "/tmp/lcdproc%06d.png", num++);
	fp = fopen(filename, "wb");
	if (!fp) {
		p->glcd_functions->drv_debug(RPT_ERR, "File %s could not be opened for writing", filename);
		return;
	}

	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		p->glcd_functions->drv_debug(RPT_ERR, "png_create_write_struct failed");
		fclose(fp);
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		p->glcd_functions->drv_debug(RPT_ERR, "png_create_info_struct failed");
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		fclose(fp);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
		goto err_out;

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, p->framebuf.px_width, p->framebuf.px_height,
		     1, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_set_invert_mono(png_ptr);

	png_write_info(png_ptr, info_ptr);

	/* Write the image row by row */
	row_pointer = p->framebuf.data;
	for (row = 0; row < p->framebuf.px_height; row++) {
		png_write_row(png_ptr, row_pointer);
		row_pointer += p->framebuf.bytesPerLine;
	}

	png_write_end(png_ptr, NULL);
	fclose(fp);
	fp = NULL;
	png_destroy_write_struct(&png_ptr, &info_ptr);

	memcpy(ct_data->backingstore, p->framebuf.data, p->framebuf.size);

	return;

err_out:
	p->glcd_functions->drv_debug(RPT_ERR, "Error writing PNG image");
	if (fp != NULL)
		fclose(fp);
	if (png_ptr != NULL)
		png_destroy_write_struct(&png_ptr, &info_ptr);
	return;
}

/**
 * API: Release low-level resources.
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_png_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_png_data *ct_data = (CT_png_data *) p->ct_data;

		if (ct_data->backingstore != NULL)
			free(ct_data->backingstore);

		free(p->ct_data);
		p->ct_data = NULL;
	}
}
