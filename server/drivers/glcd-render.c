/** \file server/drivers/glcd-render.c
 * Render glyphs to a framebuffer using FreeType2 or fixed 5x8 font.
 *
 * \todo  Add character encoding option.
 */

/*-
 * Copyright (c) 2010 Bernhard Walle <bernhard@bwalle.de>
 *               2011 Markus Dolze <bsdfan@nurfuerspam.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#ifdef HAVE_FT2
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"
#include "glcd-render.h"
#include "glcd_font5x8.h"
#include "sed1520fm.h"
#include "shared/defines.h"

#ifdef HAVE_FT2
/** Configuration for the Freetype renderer */
typedef struct glcd_render_data {
	FT_Library ft_library;		/**< freetype library handle */
	FT_Face ft_normal_font;		/**< handle for the normal font */
	char ft_has_icons;		/**< flag is the font has icons */
} RenderConfig;

static int icon2unicode(int icon);
#endif


/**
 * Initializes rendering code. Any rendering related configuration settings
 * from LCDd.conf should be read here.
 *
 * \param drvthis  Pointer to driver
 * \return         0 on success, -1 on error
 *
 * \note This function must be implemented, even if not compiled with Freetype!
 */
int
glcd_render_init(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	p->cellwidth = GLCD_DEFAULT_CELLWIDTH;
	p->cellheight = GLCD_DEFAULT_CELLHEIGHT;

	debug(RPT_DEBUG, "%s: render_init()", drvthis->name);

#ifdef HAVE_FT2
	int rc;
	const char *tmp;
	char font_file[255];
	RenderConfig *rconf;
	int w, h;

	/* Allocate memory structures */
	rconf = (RenderConfig *) calloc(1, sizeof(RenderConfig));
	if (rconf == NULL) {
		report(RPT_ERR, "%s: error allocating rendering config", drvthis->name);
		return -1;
	}
	p->render_config = rconf;

	/* use_ft2 is available in PrivateDate for easy use! */
	p->use_ft2 = drvthis->config_get_bool(drvthis->name, "useFT2", 0, 1);

	/* Only configure FreeType if enabled */
	if (p->use_ft2) {
		debug(RPT_INFO, "%s: render_init: Using FreeType", drvthis->name);

		/* get font from config file */
		tmp = drvthis->config_get_string(drvthis->name, "normal_font", 0, NULL);
		if (tmp == NULL) {
			report(RPT_ERR, "%s: normal_font missing in configuration", drvthis->name);
			goto err_out;
		}
		strncpy(font_file, tmp, sizeof(font_file));
		font_file[sizeof(font_file) - 1] = '\0';

		/* initialize FreeType library */
		rc = FT_Init_FreeType(&rconf->ft_library);
		if (rc != 0) {
			report(RPT_ERR, "s: Freetype initialisation failed", drvthis->name);
			goto err_out;
		}

		/* load the font face for freetype */
		rc = FT_New_Face(rconf->ft_library, font_file, 0, &rconf->ft_normal_font);
		if (rc != 0) {
			report(RPT_ERR, "%s: Creation of font '%s' failed", drvthis->name, font_file);
			goto err_out;
		}

		/* If the font does not have icons use default 5x8 font */
		rconf->ft_has_icons = drvthis->config_get_bool(drvthis->name, "fontHasIcons", 0, 1);

		/* Read display size in pixels */
		tmp = drvthis->config_get_string(drvthis->name, "CellSize", 0, "6x8");
		if ((sscanf(tmp, "%dx%d", &w, &h) != 2)
		    || (w <= 4) || (w > 24)
		    || (h <= 6) || (h > 32)) {
			report(RPT_WARNING, "%s: cannot read CellSize: %s, Using default %dx%d",
			       drvthis->name, tmp, GLCD_DEFAULT_CELLWIDTH, GLCD_DEFAULT_CELLHEIGHT);
			w = GLCD_DEFAULT_CELLWIDTH;
			h = GLCD_DEFAULT_CELLHEIGHT;
		}
		p->cellwidth = w;
		p->cellheight = h;
	}
#endif
	debug(RPT_INFO, "%s: using cellsize %dx%d", drvthis->name, p->cellwidth, p->cellheight);

	return 0;

#ifdef HAVE_FT2
err_out:
	glcd_render_close(drvthis);
	return -1;
#endif
}


/**
 * Releases on resources used by the rendering code.
 *
 * \param drvthis  Pointer to driver
 * \note This function must be implemented, even if not compiled with Freetype!
 */
void
glcd_render_close(Driver *drvthis)
{
#ifdef HAVE_FT2
	PrivateData *p = drvthis->private_data;
	RenderConfig *rconf = p->render_config;

	if (rconf != NULL) {
		if (rconf->ft_normal_font != NULL)
			FT_Done_Face(rconf->ft_normal_font);
		if (rconf->ft_library != NULL)
			FT_Done_FreeType(rconf->ft_library);

		free(rconf);
		p->render_config = NULL;
	}
#endif
}


#ifdef HAVE_FT2
/**
 * Draws character c to the framebuffer at position x,y using Freetype 2 for
 * font rendering. Top left corner is (1/1).
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 * \param yscale   Use multiple of cellheight
 * \param xscale   Use multiple of cellwidth
 */
void
glcd_render_char_unicode(Driver *drvthis, int x, int y, int c, int yscale, int xscale)
{
	static int last_font_size = -1;
	PrivateData *p = drvthis->private_data;
	RenderConfig *rconf = p->render_config;
	int col, row;		/* Position in the font bitmap */
	int px, py;		/* Pixel position on the display */
	int r_width, r_height;	/* Size of the cell used to render char into */
	int rc;
	FT_Face face;
	FT_GlyphSlot glyph;
	FT_Bitmap *bitmap;
	unsigned char *bitmap_buf;

	if (x < 1 || x > p->width || y < 1 || y > p->height)
		return;

	x--;			/* convert coordinates to zero-based */

	/*
	 * Implementation note: This function can be used to render characters
	 * that are multiple the size of one cell. Currently this is used to
	 * draw big numbers. Therefore use a scaled cell (render_cell) for all
	 * calculations.
	 */
	r_height = p->cellheight * yscale;
	r_width = p->cellwidth * xscale;

	/*
	 * Set the font size. We set the font pixel width and height to the
	 * same value (r_height), otherwise characters look too much condensed.
	 */
	if (last_font_size != r_height) {
		debug(RPT_INFO, "%s: Setting font size to %d",  drvthis->name, r_height);
		rc = FT_Set_Pixel_Sizes(rconf->ft_normal_font, r_height, r_height);
		if (rc != 0) {
			report(RPT_ERR, "%s: Failed to set pixel size (%dx%x)", drvthis->name,
			       p->cellwidth, p->cellheight);
			return;
		}

		last_font_size = r_height;
	}

	/* load the glyph and render it */
	rc = FT_Load_Char(rconf->ft_normal_font, c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
	if (rc != 0) {
		report(RPT_ERR, "%s: loading char '%c' (0x%x) failed", drvthis->name, c, c);
		return;
	}

	/* set some data elements for convenience */
	face = rconf->ft_normal_font;
	glyph = rconf->ft_normal_font->glyph;
	bitmap = &glyph->bitmap;
	bitmap_buf = bitmap->buffer;

	/* Clear the cell. */
	py = max(y * p->cellheight - r_height, 0);
	for (row = 0; row < r_height; row++, py++) {
		px = x * p->cellwidth;
		for (col = 0; col < r_width; col++, px++) {
			fb_draw_pixel(&(p->framebuf), px, py, 0);
		}
	}

	/*
	 * Copy the pixels. Important: The font metrics may result in negative
	 * py value! So protect it by restricting it to 0.
	 */
	py = max(y * p->cellheight + (face->size->metrics.descender >> 6) - glyph->bitmap_top, 0);
	for (row = 0; (row < bitmap->rows) && (row < r_height); row++) {
		px = x * p->cellwidth;
		/*
		 * Hack: If scales are not the same, ignore Freetype's idea of
		 * character position, but just center it. Currently only used
		 * for the ':' of the bignum.
		 */
		if (yscale == xscale)
			px += glyph->bitmap_left;
		else
			px += (r_width - bitmap->width)/2;

		for (col = 0; (col < bitmap->width) && (col < r_width); col++) {
			fb_draw_pixel(&(p->framebuf), px, py, bitmap_buf[col / 8] >> (7 - (col % 8)) & 1);
			px++;
		}
		bitmap_buf += bitmap->pitch;
		py++;
	}
}
#endif

/**
 * Draws char c from our standard 5x8 font definition to the framebuffer at
 * position x,y. Top left corner is (1/1).
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
void
glcd_render_char(Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;
	int font_x, font_y;	/* Position in the font definition array */
	int px, py;		/* Pixel position on the display */

	if (x < 1 || x > p->width || y < 1 || y > p->height)
		return;

	x--;			/* convert coordinates to zero-based */
	y--;

	/*
	 * Algorithm: For each row in the font definition check if bit (dot)
	 * at each column is set. If yes, plot a dot into the framebuffer. If
	 * the bit is not set then clear the dot. Currently it is wrong to
	 * assume the framebuffer is clear (e.g. the heartbeat does not clear
	 * it's contents in advance).
	 */
	/* FIXME: What happens if font is larger than cell size? */
	py = y * p->cellheight;
	for (font_y = 0; font_y < GLCD_FONT_HEIGHT; font_y++) {
		px = x * p->cellwidth;
		/*
		 * Note: Initializing font_x with font's width leaves one
		 * empty column to the left.
		 */
		for (font_x = GLCD_FONT_WIDTH; font_x >= 0; font_x--) {
			if (glcd_iso8859_1[c][font_y] & (1 << font_x))
				fb_draw_pixel(&(p->framebuf), px, py, 1);
			else
				fb_draw_pixel(&(p->framebuf), px, py, 0);
			px++;
		}
		py++;
	}
}


#ifdef HAVE_FT2
/**
 * Maps a LCDproc icon to an Unicode codepoint for an icon.
 *
 * \param icon  The LCDproc icon constant
 * \return      Unicode value
 */
static int
icon2unicode(int icon)
{
	switch (icon) {
	    case ICON_BLOCK_FILLED:
		return 0x2588;	/* Full block */
	    case ICON_HEART_FILLED:
		return 0x2665;	/* Black heart suit */
	    case ICON_HEART_OPEN:
		return 0x2661;	/* White heart suit */
	    case ICON_ARROW_UP:
		return 0x2191;	/* Upwards arrow */
	    case ICON_ARROW_DOWN:
		return 0x2193;	/* Downwards arrow */
	    case ICON_ARROW_LEFT:
		return 0x2190;	/* Leftwards arrow */
	    case ICON_ARROW_RIGHT:
		return 0x2192;	/* Rightwards arrow */
//	    case ICON_CHECKBOX_OFF:
//		return 0x2610;	/* Ballot box */
//	    case ICON_CHECKBOX_ON:
//		return 0x2611;	/* Ballot box with check */
//	    case ICON_CHECKBOX_GRAY:
//		return 0x2612;	/* Ballot box with x */
	    case ICON_ELLIPSIS:
		return 0x2026;	/* Horizontal ellipsis */
	    default:
		return -1;
	}
}
#endif


/**
 * Place an icon on the screen using either our built-in standard font or
 * an Unicode icon rendered with Freetype.
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
int
glcd_render_icon(Driver *drvthis, int x, int y, int icon)
{
	int icon_char;

#ifdef HAVE_FT2
	PrivateData *p = drvthis->private_data;
	RenderConfig *rconf = p->render_config;

	if (p->use_ft2 && rconf->ft_has_icons) {
		if ((icon_char = icon2unicode(icon)) != -1) {
			glcd_render_char_unicode(drvthis, x, y, icon_char, 1, 1);
			return 0;
		}
		return -1;
	}
#endif
	if ((icon_char = glcd_icon5x8(icon)) != -1) {
		glcd_render_char(drvthis, x, y, icon_char);
		return 0;
	}
	return -1;
}


/**
 * Draw a big digit (or colon) using the built-in 16x24 font. Although the font
 * is stored in different pixel layout (column format, LSB top) this does not
 * matter as we need to address pixels in frame buffer anyway. The digit is
 * centered vertically.
 *
 * \note  Works only for displays with pixel height >= 24! Smaller displays are
 *        not supported and nothing will be drawn.
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      The digit to draw (0-10 with 10 = colon)
 */
void
glcd_render_bignum(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int c, z;		/* Column and byte within font definition */
	int px, py;		/* Pixel coordinates within the frame buffer */

	if (p->framebuf.px_height < chr_hgt_NUM)
		return;

	x--;

	px = x * p->cellwidth;
	for (c = 0; c < widtbl_NUM[num]; c++) {
		/* center vertically */
		py = (p->framebuf.px_height - chr_hgt_NUM) / 2;
		for (z = 0; z < chr_hgt_NUM; z++) {
			/* Test if pixel bit is set and draw it */
			if (chrtbl_NUM[num][c * 3 + z / 8] & (1 << (z % 8)))
				fb_draw_pixel(&(p->framebuf), px, py, 1);
			else
				fb_draw_pixel(&(p->framebuf), px, py, 0);
			py++;
		}
		px++;
	}
}
