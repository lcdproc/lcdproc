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
#define GLCD_KEYPAD_MAX			26
#define GLCD_DEFAULT_REPEAT_DELAY	500	/* milliseconds */
#define GLCD_DEFAULT_REPEAT_INTERVAL	300	/* milliseconds */

enum fb_types {
	FB_TYPE_LINEAR = 0,
	FB_TYPE_VPAGED
};

/** The framebuffer and its properties */
struct glcd_framebuf {
	unsigned char *data;	/**< frame buffer */
	int px_width;		/**< display width in dots */
	int px_height;		/**< display height in dots */
	int bytesPerLine;	/**< number of bytes per pixel row */
	int size;		/**< total size in bytes */
	enum fb_types layout;	/**< memory layout */
};

/** private data for the \c glcd driver */
typedef struct glcd_private_data {
	/* framebuffer and size settings */
	struct glcd_framebuf framebuf;	/**< the main framebuffer */
	int cellwidth;			/**< character cell width */
	int cellheight;			/**< character cell height */
	int width;			/**< display width in characters */
	int height;			/**< display height in characters */
	/* low-level hardware-related stuff */
	int contrast;			/**< current contrast */
	int brightness;			/**< current brightness (for backlight on) */
	int offbrightness;		/**< current brightness (for backlight off) */
	int last_output_state;		/**< cache last value to output() */
	int backlightstate;		/**< state of backlight currently used */
	struct glcdHwFcns *glcd_functions;	/**< pointers to low-level functions */
	void *ct_data;			/**< Connection type specific data */
	/* Renderer settings */
	void *render_config;		/**< Settings for the font renderer */
	char use_ft2;			/**< (Not) use FreeType if available */
	/* Keypad related settings */
	char *keyMap[GLCD_KEYPAD_MAX];	/**< Maps scancode number to key strings */
	char *pressed_key;		/**< The last key string */
	struct timeval *key_wait_time;	/**< Time until key auto repeat */
	int key_repeat_delay;		/**< Time until first key repeat */
	int key_repeat_interval;	/**< Time between auto repeated keys */
} PrivateData;

/** Structure holding pointers to display specific functions */
struct glcdHwFcns {
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

	/* Poll for key presses */
	unsigned char (*poll_keys)(PrivateData *p);

	/* Close the interface on shutdown */
	void (*close)(PrivateData *p);
};

/* ================== Framebuffer functions and macros =================== */

#define FB_BLACK 1
#define FB_WHITE 0

/**
 * Draw one pixel into the framebuffer using 1bpp (black and white). This
 * function actually decides about the format of the framebuffer. Using this
 * implementation (0,0) is top left and bytes contain pixels from left to right.
 *
 * \param fb     Pointer to framebuffer
 * \param x      X-position
 * \param y      Y-position
 * \param color  Pixel color: 1 = set (black), 0 = not set (blank/white)
 */
static inline void
fb_draw_pixel(struct glcd_framebuf *fb, int x, int y, int color)
{
	unsigned int pos;	/* Byte within the framebuffer */
	unsigned char bit;	/* Bit within the framebuffer byte */

	if (x < 0 || x >= fb->px_width || y < 0 || y >= fb->px_height)
		return;

	if (fb->layout == FB_TYPE_LINEAR) {
		pos = y * fb->bytesPerLine + (x / 8);
		bit = 0x80 >> (x % 8);
	}
	else {
		pos = (y / 8) * fb->px_width + x;
		bit = 1 << (y % 8);
	}

	if (color == FB_BLACK)
		fb->data[pos] |= bit;
	else
		fb->data[pos] &= ~bit;
}


/**
 * Get color value of one pixel from the framebuffer.
 *
 * \param fb  Pointer to framebuffer
 * \param x   X-position
 * \param y   Y-position
 * \return  Pixel color: 1 = set (black), 0 = not set (blank/white)
 */
static inline int
fb_get_pixel(struct glcd_framebuf *fb, int x, int y)
{
	unsigned int pos;
	unsigned char bit;

	if (x < 0 || x >= fb->px_width || y < 0 || y >= fb->px_height)
		return FB_WHITE;

	if (fb->layout == FB_TYPE_LINEAR) {
		pos = y * fb->bytesPerLine + (x / 8);
		bit = 0x80 >> (x % 8);
	}
	else {
		pos = (y / 8) * fb->px_width + x;
		bit = 0x01 << (y % 8);
	}

	if (fb->data[pos] & bit)
		return FB_BLACK;
	else
		return FB_WHITE;
}
#endif
