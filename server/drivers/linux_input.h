#ifndef LINUX_INPUT_H
#define LINUX_INPUT_H

#include "lcd.h"

#include <elektra.h>

MODULE_EXPORT int  linuxInput_init (Driver *drvthis, Elektra * elektra);
MODULE_EXPORT void linuxInput_close (Driver *drvthis);

MODULE_EXPORT const char *linuxInput_get_key (Driver *drvthis);

#endif
