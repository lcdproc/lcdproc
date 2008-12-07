#ifndef LCD_SVGALIB_H
#define LCD_SVGALIB_H

#define DEFAULT_MODESTR		"G320x200x256"
#define DEFAULT_SIZE		"20x4"
#define DEFAULT_CONTRAST	500
#define DEFAULT_BRIGHTNESS	1000
#define DEFAULT_OFFBRIGHTNESS	500
#define DEFAULT_BACKLIGHT	0

#define CELLWIDTH	6
#define CELLHEIGHT	8

/** private data for the \c svga driver */
typedef struct svga_private_data {
	int mode;

	int width, height;
	int cellwidth, cellheight;
	int xoffs, yoffs;

	void *font;

	int contrast;
	int brightness;
	int offbrightness;
} PrivateData;


MODULE_EXPORT int  svga_init (Driver *drvthis);
MODULE_EXPORT void svga_close (Driver *drvthis);
MODULE_EXPORT int  svga_width (Driver *drvthis);
MODULE_EXPORT int  svga_height (Driver *drvthis);
MODULE_EXPORT int  svga_cellwidth (Driver *drvthis);
MODULE_EXPORT int  svga_cellheight (Driver *drvthis);
MODULE_EXPORT void svga_clear (Driver *drvthis);
MODULE_EXPORT void svga_flush (Driver *drvthis);
MODULE_EXPORT void svga_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void svga_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void svga_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void svga_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void svga_num (Driver *drvthis, int x, int num);

MODULE_EXPORT const char * svga_get_key (Driver *drvthis);

MODULE_EXPORT int  svga_get_contrast (Driver *drvthis);
MODULE_EXPORT void svga_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  svga_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void svga_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void svga_backlight (Driver *drvthis, int on);

#endif
