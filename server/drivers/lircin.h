#ifndef LCD_LIRCIN_H
#define LCD_LIRCIN_H


extern lcd_logical_driver *lircin;

int lircin_init (struct lcd_logical_driver *driver, char *args);
void lircin_close ();
char lircin_getkey ();


#endif
