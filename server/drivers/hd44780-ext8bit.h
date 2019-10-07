#ifndef HD_EXT8BIT_H
#define HD_EXT8BIT_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_ext8bit(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
