#ifndef HD_FTDI_H
#define HD_FTDI_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_ftdi(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
