#ifndef LCD_JOY_H
#define LCD_JOY_H

extern lcd_logical_driver *joy;

int joy_init (struct lcd_logical_driver *driver, char *args);
void joy_close ();
char joy_getkey ();

#endif
