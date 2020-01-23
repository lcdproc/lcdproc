#ifndef HD_LIS2_H
#define HD_LIS2_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_lis2(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
