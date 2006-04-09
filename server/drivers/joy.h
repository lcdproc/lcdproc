#ifndef LCD_JOY_H
#define LCD_JOY_H

#include "lcd.h"

MODULE_EXPORT int  joy_init (Driver *drvthis);
MODULE_EXPORT void joy_close (Driver *drvthis);

MODULE_EXPORT const char *joy_get_key (Driver *drvthis);

#endif
