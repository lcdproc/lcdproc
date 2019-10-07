#ifndef HD_USS720_H
#define HD_USS720_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

/* initialise this particular driver */
int hd_init_uss720(Driver *drvthis, const Hd44780DriverConfig * config);
#endif
