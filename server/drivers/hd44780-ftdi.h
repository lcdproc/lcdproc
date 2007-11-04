#ifndef HD_FTDI_H
#define HD_FTDI_H

#include "lcd.h"		/* for Driver */
#include "hd44780-low.h"

// initialise this particular driver
int hd_init_ftdi(Driver *drvthis);

#endif
