#ifndef LCD_TEXT_H
#define LCD_TEXT_H

#include "lcd.h"

              int  text_init (Driver * drvthis, char *args);
MODULE_EXPORT void text_close (Driver *drvthis);
MODULE_EXPORT int  text_width (Driver *drvthis);
MODULE_EXPORT int  text_height (Driver *drvthis);
MODULE_EXPORT void text_clear (Driver *drvthis);
MODULE_EXPORT void text_flush (Driver *drvthis);
MODULE_EXPORT void text_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void text_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void text_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void text_backlight (Driver *drvthis, int on);
//MODULE_EXPORT void text_init_vbar (Driver *drvthis);
//MODULE_EXPORT void text_init_hbar (Driver *drvthis);
//MODULE_EXPORT void text_init_num (Driver *drvthis);
MODULE_EXPORT void text_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void text_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void text_num (Driver *drvthis, int x, int num);
//MODULE_EXPORT void text_set_char (Driver *drvthis, int n, char *dat);


#endif
