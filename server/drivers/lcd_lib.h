#ifndef LCD_LIB_H
#define LCD_LIB_H

#ifndef LCD_H
#include "lcd.h"
#endif

int new_framebuf (struct lcd_logical_driver *driver, char *oldbuf);

#endif

