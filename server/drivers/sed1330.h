/*
 * Driver for SED1330 graphical displays
 * Header file
 */

#ifndef SED1330_H
#define SED1330_H

#include "lcd.h"

MODULE_EXPORT int sed1330_init( Driver * drvthis, char *args );
MODULE_EXPORT void sed1330_close( Driver * drvthis );
MODULE_EXPORT int sed1330_width( Driver * drvthis );
MODULE_EXPORT int sed1330_height( Driver * drvthis );
MODULE_EXPORT void sed1330_clear( Driver * drvthis );
MODULE_EXPORT void sed1330_flush( Driver * drvthis );
MODULE_EXPORT void sed1330_string( Driver * drvthis, int x, int y, char lcd[] );
MODULE_EXPORT void sed1330_chr( Driver * drvthis, int x, int y, char c );

MODULE_EXPORT void sed1330_vbar( Driver * drvthis, int x, int y, int len, int promille, int pattern );
MODULE_EXPORT void sed1330_hbar( Driver * drvthis, int x, int y, int len, int promille, int pattern );
MODULE_EXPORT void sed1330_num( Driver * drvthis, int x, int num );
MODULE_EXPORT void sed1330_heartbeat( Driver * drvthis, int type );
MODULE_EXPORT void sed1330_cursor( Driver * drvthis, int x, int y, char state );

MODULE_EXPORT void sed1330_backlight( Driver * drvthis, int on );

#endif
