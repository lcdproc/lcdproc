#ifndef LCD_SVGALIB_H
#define LCD_SVGALIB_H

MODULE_EXPORT int  svgalib_drv_init (Driver *drvthis, char *args);
MODULE_EXPORT void svgalib_drv_close (Driver *drvthis);
MODULE_EXPORT int  svgalib_drv_width (Driver *drvthis);
MODULE_EXPORT int  svgalib_drv_height (Driver *drvthis);
MODULE_EXPORT void svgalib_drv_clear (Driver *drvthis);
MODULE_EXPORT void svgalib_drv_flush (Driver *drvthis);
MODULE_EXPORT void svgalib_drv_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void svgalib_drv_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void svgalib_drv_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void svgalib_drv_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void svgalib_drv_num (Driver *drvthis, int x, int num);

MODULE_EXPORT char * svgalib_drv_get_key (Driver *drvthis);

#endif
