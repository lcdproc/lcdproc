#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

extern lcd_logical_driver *joy;

MODULE_EXPORT int irmanin_init (Driver *drvthis, char *args);
MODULE_EXPORT void irmanin_close ();
MODULE_EXPORT char irmanin_getkey ();

#endif
