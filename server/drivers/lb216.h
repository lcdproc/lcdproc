#ifndef LB216_H
#define LB216_H

#include "lcd.h"

MODULE_EXPORT int  LB216_init(Driver * drvthis, char *device);
MODULE_EXPORT void LB216_close(Driver * drvthis);
MODULE_EXPORT int  LB216_width (Driver *drvthis);
MODULE_EXPORT int  LB216_height (Driver *drvthis);
MODULE_EXPORT void LB216_clear (Driver * drvthis);
MODULE_EXPORT void LB216_flush(Driver * drvthis);
MODULE_EXPORT void LB216_string (Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void LB216_chr(Driver * drvthis, int x, int y, char c) ;

MODULE_EXPORT void LB216_vbar(Driver * drvthis, int x, int len);
MODULE_EXPORT void LB216_hbar(Driver * drvthis, int x, int y, int len);
MODULE_EXPORT void LB216_num(Driver * drvthis, int x, int num);
MODULE_EXPORT void LB216_icon(Driver * drvthis, int which, char dest);

MODULE_EXPORT void LB216_set_char(Driver * drvthis, int n, char *dat);

MODULE_EXPORT void LB216_backlight(Driver * drvthis, int on);

MODULE_EXPORT void LB216_init_vbar(Driver * drvthis);
MODULE_EXPORT void LB216_init_hbar(Driver * drvthis);
MODULE_EXPORT void LB216_init_num(Driver * drvthis);

#endif
