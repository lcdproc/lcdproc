#ifndef HD_I2C_H
#define HD_I2C_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_i2c(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
