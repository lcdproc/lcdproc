#ifndef LCD_LIRCIN_H
#define LCD_LIRCIN_H

#include "lcd.h"

MODULE_EXPORT int lircin_init (Driver * drvthis, char *args);
MODULE_EXPORT void lircin_close (Driver * drvthis);
MODULE_EXPORT char lircin_getkey (Driver * drvthis);

#endif
