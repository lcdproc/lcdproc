#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

MODULE_EXPORT int irmanin_init (Driver *drvthis);
MODULE_EXPORT void irmanin_close (Driver *drvthis);
MODULE_EXPORT char irmanin_getkey (Driver *drvthis);

#endif
