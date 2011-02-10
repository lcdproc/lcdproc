#ifndef LCD_XOSDLIB_H
#define LCD_XOSDLIB_H

#include <xosd.h>

#define DEFAULT_FONT		"fixed"
#define DEFAULT_SIZE		"20x4"
#define DEFAULT_OFFSET		"0x0"
#define DEFAULT_CONTRAST	500
#define DEFAULT_BRIGHTNESS	1000
#define DEFAULT_OFFBRIGHTNESS	500
#define DEFAULT_BACKLIGHT	0

#define CELLWIDTH	6
#define CELLHEIGHT	8

/** private data for the \c xosd driver */
typedef struct xosd_private_data {
	xosd *osd;		/**< xosd handle */

	char font[256];		/**< xosd font properties */

	/* dimensions */
	int width, height;
	int xoffs, yoffs;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	int contrast;
	int brightness;
	int offbrightness;
} PrivateData;


MODULE_EXPORT int  xosdlib_drv_init (Driver *drvthis);
MODULE_EXPORT void xosdlib_drv_close (Driver *drvthis);
MODULE_EXPORT int  xosdlib_drv_width (Driver *drvthis);
MODULE_EXPORT int  xosdlib_drv_height (Driver *drvthis);
MODULE_EXPORT void xosdlib_drv_clear (Driver *drvthis);
MODULE_EXPORT void xosdlib_drv_flush (Driver *drvthis);
MODULE_EXPORT void xosdlib_drv_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void xosdlib_drv_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void xosdlib_drv_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void xosdlib_drv_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void xosdlib_drv_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  xosdlib_drv_get_free_chars (Driver *drvthis);
MODULE_EXPORT int  xosdlib_drv_get_contrast (Driver *drvthis);
MODULE_EXPORT void xosdlib_drv_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  xosdlib_drv_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void xosdlib_drv_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void xosdlib_drv_backlight (Driver *drvthis, int on);

#endif /* XOSDLIB_H */
