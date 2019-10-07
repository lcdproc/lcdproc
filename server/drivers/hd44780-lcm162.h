#ifndef HD_LCM162_H
#define HD_LCM162_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_lcm162(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
