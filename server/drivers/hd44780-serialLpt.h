#ifndef HD_SERIALLPT_H
#define HD_SERIALLPT_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_serialLpt(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
