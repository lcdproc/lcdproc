#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

extern lcd_logical_driver *joy;

int irmanin_init (struct lcd_logical_driver *driver, char *args);
void irmanin_close ();
char irmanin_getkey ();

#endif
