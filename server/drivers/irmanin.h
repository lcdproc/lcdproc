#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

extern lcd_logical_driver *joy;

MODULE_EXPORT int irmanin_init (struct lcd_logical_driver *driver, char *args);
MODULE_EXPORT void irmanin_close ();
MODULE_EXPORT char irmanin_getkey ();

#endif
