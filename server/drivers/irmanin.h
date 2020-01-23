#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

#include <elektra.h>

MODULE_EXPORT int irmanin_init (Driver *drvthis, Elektra * elektra);
MODULE_EXPORT void irmanin_close (Driver *drvthis);
MODULE_EXPORT const char *irmanin_get_key (Driver *drvthis);

#endif
