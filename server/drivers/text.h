#ifndef LCD_TEXT_H
#define LCD_TEXT_H

extern lcd_logical_driver *text;

int text_init (struct lcd_logical_driver *driver, char *args);

#endif
