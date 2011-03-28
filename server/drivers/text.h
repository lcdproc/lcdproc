#ifndef LCD_TEXT_H
#define LCD_TEXT_H

MODULE_EXPORT int  text_init (Driver *drvthis);
MODULE_EXPORT void text_close (Driver *drvthis);
MODULE_EXPORT int  text_width (Driver *drvthis);
MODULE_EXPORT int  text_height (Driver *drvthis);
MODULE_EXPORT void text_clear (Driver *drvthis);
MODULE_EXPORT void text_flush (Driver *drvthis);
MODULE_EXPORT void text_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void text_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void text_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void text_backlight (Driver *drvthis, int on);
MODULE_EXPORT const char * text_get_info (Driver *drvthis);

#define TEXTDRV_DEFAULT_SIZE "20x4"

#endif
