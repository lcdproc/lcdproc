#ifndef LCD_DEBUG_H
#define LCD_DEBUG_H

#include "lcd.h"

              int  debug_init (Driver *drvthis, char *args);
MODULE_EXPORT void debug_close (Driver *drvthis);
MODULE_EXPORT int  debug_width (Driver *drvthis);
MODULE_EXPORT int  debug_height (Driver *drvthis);
MODULE_EXPORT void debug_clear (Driver *drvthis);
MODULE_EXPORT void debug_flush (Driver *drvthis);
MODULE_EXPORT void debug_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void debug_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void debug_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void debug_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void debug_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void debug_icon (Driver *drvthis, int which, char dest);

MODULE_EXPORT void debug_set_char (Driver *drvthis, int n, char *dat);

MODULE_EXPORT void debug_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void debug_backlight (Driver *drvthis, int promille);

MODULE_EXPORT char debug_getkey (Driver *drvthis);

MODULE_EXPORT void debug_init_vbar (Driver *drvthis);
MODULE_EXPORT void debug_init_hbar (Driver *drvthis);
MODULE_EXPORT void debug_init_num (Driver *drvthis);

#endif
