#ifndef HD_PIPLATE_H
#define HD_PIPLATE_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

/* initialise this particular driver */
int hd_init_i2c_piplate(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
