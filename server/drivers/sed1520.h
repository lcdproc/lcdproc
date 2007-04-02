#ifndef SED1520_H
#define SED1520_H

#include "lcd.h"

MODULE_EXPORT int sed1520_init (Driver *drvthis);
MODULE_EXPORT void sed1520_close (Driver *drvthis);
MODULE_EXPORT int sed1520_width (Driver *drvthis);
MODULE_EXPORT int sed1520_height (Driver *drvthis);
MODULE_EXPORT int sed1520_cellwidth (Driver *drvthis);
MODULE_EXPORT int sed1520_cellheight (Driver *drvthis);
MODULE_EXPORT void sed1520_clear (Driver *drvthis);
MODULE_EXPORT void sed1520_flush (Driver *drvthis);
MODULE_EXPORT void sed1520_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void sed1520_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void sed1520_old_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void sed1520_old_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void sed1520_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  sed1520_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void sed1520_set_char (Driver *drvthis, int n, char *dat);

#endif
