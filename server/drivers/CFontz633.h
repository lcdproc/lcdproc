#ifndef CFONTZ633_H
#define CFONTZ633_H

#include "lcd.h"

#define DEFAULT_CELL_WIDTH 6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_CONTRAST 560
#define DEFAULT_DEVICE "/dev/lcd"
#define DEFAULT_SPEED B19200
#define DEFAULT_BRIGHTNESS 100
#define DEFAULT_OFFBRIGHTNESS 0
#define DEFAULT_SIZE "16x2"

MODULE_EXPORT int  CFontz633_init (Driver * drvthis, char *device);
MODULE_EXPORT void CFontz633_close (Driver * drvthis);
MODULE_EXPORT int  CFontz633_width (Driver * drvthis);
MODULE_EXPORT int  CFontz633_height (Driver * drvthis);
MODULE_EXPORT void CFontz633_clear (Driver * drvthis);
MODULE_EXPORT void CFontz633_flush (Driver * drvthis);
MODULE_EXPORT void CFontz633_string (Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void CFontz633_chr (Driver * drvthis, int x, int y, char c);

MODULE_EXPORT void CFontz633_vbar (Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontz633_hbar (Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontz633_num (Driver * drvthis, int x, int num);
MODULE_EXPORT int  CFontz633_icon(Driver * drvthis, int x, int y, int icon);

MODULE_EXPORT void CFontz633_set_char (Driver * drvthis, int n, char *dat);

MODULE_EXPORT int  CFontz633_get_contrast (Driver * drvthis);
MODULE_EXPORT void CFontz633_set_contrast (Driver * drvthis, int contrast);
MODULE_EXPORT void CFontz633_backlight (Driver * drvthis, int on);

#endif
