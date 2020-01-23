#ifndef HD_LCDSTAT_H
#define HD_LCDSTAT_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_4bit(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
