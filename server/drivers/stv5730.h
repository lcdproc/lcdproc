#ifndef STV5730_H
#define STV5730_H

#include "lcd.h"

MODULE_EXPORT int stv5730_init (Driver *drvthis, char *args);
MODULE_EXPORT void stv5730_close (Driver *drvthis);
MODULE_EXPORT int stv5730_width (Driver *drvthis);
MODULE_EXPORT int stv5730_height (Driver *drvthis);
MODULE_EXPORT int stv5730_cellwidth (Driver *drvthis);
MODULE_EXPORT int stv5730_cellheight (Driver *drvthis);
MODULE_EXPORT void stv5730_clear (Driver *drvthis);
MODULE_EXPORT void stv5730_flush (Driver *drvthis);
MODULE_EXPORT void stv5730_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void stv5730_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void stv5730_vbar (Driver *drvthis, int x, int len);
MODULE_EXPORT void stv5730_hbar (Driver *drvthis, int x, int y, int len);
MODULE_EXPORT void stv5730_num (Driver *drvthis, int x, int num);
MODULE_EXPORT void stv5730_icon (Driver *drvthis, int which, char dest);

#endif
