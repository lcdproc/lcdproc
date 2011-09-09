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

#define BYTES_PER_LINE (p->px_width / 8)

/** private data for the \c glcd driver */
typedef struct glcd_private_data {
	unsigned char *framebuf;	/**< frame buffer */
	int px_width;			/**< display width in dots */
	int px_height;			/**< display height in dots */
	int cellwidth;			/**< character cell width */
	int cellheight;			/**< character cell height */
	int width;			/**< display width in characters */
	int height;			/**< display height in characters */
	struct hwDependentFns *glcd_functions;	/**< pointers to low-level functions */
	void *ct_data;			/**< Connection type specific data */
} PrivateData;

/** Structure holding pointers to display specific functions */
typedef struct hwDependentFns {
	/* report and debug helper. Set by global glcd init */
	void (*drv_report)(const int level, const char *format,... /* args */ );
	void (*drv_debug)(const int level, const char *format,... /* args */ );

	/* Transfer the framebuffer to the display */
	void (*blit)(PrivateData *p);

	/* Switch the backlight on or off */
	/* void (*set_backlight)(PrivateData *p, char state); */

	/* Set the contrast (value from 0-1000) */
	/* void (*set_contrast)(PrivateData *p, unsigned int value); */

	/* Output "data" to output latch if there is one */
	/* void (*output)(PrivateData *p, int data); */

	/* Close the interface on shutdown */
	void (*close)(PrivateData *p);
} GLCD_functions;

#endif
