#ifndef HD_GPIO_H
#define HD_GPIO_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

/* initialize this particular driver */
int hd_init_gpio(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
