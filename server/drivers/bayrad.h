/* Header file for BayRAD driver
 * for LCDproc LCD software
 * by Nathan Yawn, yawn@emacinc.com
 * 3/24/01
 */


#ifndef _BAYRAD_H
#define _BAYRAD_H

#include "lcd.h"

MODULE_EXPORT int  bayrad_init(Driver * drvthis, char *args);
MODULE_EXPORT void bayrad_close(Driver * drvthis);
MODULE_EXPORT int  bayrad_width(Driver * drvthis);
MODULE_EXPORT int  bayrad_height(Driver * drvthis);
MODULE_EXPORT void bayrad_clear(Driver * drvthis);
MODULE_EXPORT void bayrad_flush(Driver * drvthis);
MODULE_EXPORT void bayrad_string(Driver * drvthis, int x, int y, char string[]);
MODULE_EXPORT void bayrad_chr(Driver * drvthis, int x, int y, char c);

MODULE_EXPORT void bayrad_vbar(Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void bayrad_hbar(Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  bayrad_icon(Driver * drvthis, int x, int y, int icon);

MODULE_EXPORT void bayrad_set_char(Driver * drvthis, int n, char *dat);

MODULE_EXPORT void bayrad_backlight(Driver * drvthis, int promille);

MODULE_EXPORT char *bayrad_get_key(Driver * drvthis);

MODULE_EXPORT void bayrad_init_vbar(Driver * drvthis);
MODULE_EXPORT void bayrad_init_hbar(Driver * drvthis);

#endif
