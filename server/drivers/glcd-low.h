/** \file server/drivers/glcd-low.h
 * Interface to low level code to access graphical displays.
 *
 * \note This file should not include hardware dependent header files (libusb,
 * libftdi, libhid, etc.) Those shall be included by the sub drivers.
 */

#ifndef GLCD_LOW_H
#define GLCD_LOW_H

#define GLCD_DEFAULT_SIZE	"128x64"
#define GLCD_DEFAULT_CELLWIDTH	6
#define GLCD_DEFAULT_CELLHEIGHT	8
#define GLCD_MAX_WIDTH		640
#define GLCD_MAX_HEIGHT		480
#define GLCD_DEFAULT_CONTRAST	600
#define GLCD_DEFAULT_BRIGHTNESS		800
#define GLCD_DEFAULT_OFFBRIGHTNESS	100

/** private data for the \c glcd driver */
typedef struct glcd_private_data {
	unsigned char *framebuf;	/**< frame buffer */
	int px_width;			/**< display width in dots */
	int px_height;			/**< display height in dots */
	int cellwidth;			/**< character cell width */
	int cellheight;			/**< character cell height */
	int width;			/**< display width in characters */
	int height;			/**< display height in characters */
	int contrast;			/**< current contrast */
	int brightness;			/**< current brightness (for backlight on) */
	int offbrightness;		/**< current brightness (for backlight off) */
	int last_output_state;		/**< cache last value to output() */
	int backlightstate;		/**< state of backlight currently used */
	struct hwDependentFns *glcd_functions;	/**< pointers to low-level functions */
	void *ct_data;			/**< Connection type specific data */
	void *render_config;		/**< Settings for the font renderer */
	char use_ft2;			/**< (Not) use FreeType if available */
} PrivateData;

/** Structure holding pointers to display specific functions */
typedef struct hwDependentFns {
	/* report and debug helper. Set by global glcd init */
	void (*drv_report)(const int level, const char *format,... /* args */ );
	void (*drv_debug)(const int level, const char *format,... /* args */ );

	/* Transfer the framebuffer to the display */
	void (*blit)(PrivateData *p);

	/* Switch the backlight on or off */
	void (*set_backlight)(PrivateData *p, int state);

	/* Set contrast */
	void (*set_contrast)(PrivateData *p, int value);

	/* Output "data" to output latch if there is one */
	void (*output)(PrivateData *p, int data);

	/* Close the interface on shutdown */
	void (*close)(PrivateData *p);
} GLCD_functions;

/* ================== Framebuffer functions and macros =================== */

#define BYTES_PER_LINE (p->px_width / 8)
#define FB_BYTES_TOTAL (p->px_height * BYTES_PER_LINE)

/**
 * Draw one pixel into the framebuffer using 1bpp (black and white). This
 * function actually decides about the format of the framebuffer. Using this
 * implementation (0,0) is top left and bytes contain pixels from left to right.
 *
 * \param p      Pointer to driver's private data.
 * \param x      X-position
 * \param y      Y-position
 * \param color  Pixel color: 1 = set (black), 0 = not set (blank/white)
 */
static inline void
fb_draw_pixel(PrivateData *p, int x, int y, int color)
{
	unsigned int pos;	/* Byte within the framebuffer */
	unsigned char bit;	/* Bit within the framebuffer byte */

	if (x < 0 || x >= p->px_width || y < 0 || y >= p->px_height)
		return;

	pos = y * BYTES_PER_LINE + (x / 8);
	bit = 0x80 >> (x % 8);

	if (color == 1)
		p->framebuf[pos] |= bit;
	else
		p->framebuf[pos] &= ~bit;
}

#endif
