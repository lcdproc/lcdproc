#ifndef HD_WINAMP_H
#define HD_WINAMP_H

#include "lcd.h"					  /* for Driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_winamp(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
