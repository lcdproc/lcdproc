#ifndef LCD_JOY_H
#define LCD_JOY_H

#include "lcd.h"

MODULE_EXPORT int  joy_init (Driver *drvthis, char *args);
MODULE_EXPORT void joy_close (Driver *drvthis);

MODULE_EXPORT char joy_getkey (Driver *drvthis);

#endif
