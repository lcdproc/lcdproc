#ifndef LCD_rawserial_H
#define LCD_rawserial_H

#define DEFAULT_DEVICE		"/dev/cuaU0"
#define DISPLAY_DEFAULT_SIZE 	"40x4"

/* Default 9600 baud, 1 hz update rate, time resolution of 1 ms */
#define DEFAULT_SPEED		B9600
#define DEFAULT_UPDATE_RATE	1.0
#define SECOND_GRANULARITY	1000

MODULE_EXPORT int rawserial_init(Driver *drvthis);
MODULE_EXPORT void rawserial_close(Driver *drvthis);
MODULE_EXPORT int rawserial_width(Driver *drvthis);
MODULE_EXPORT int rawserial_height(Driver *drvthis);
MODULE_EXPORT void rawserial_clear(Driver *drvthis);
MODULE_EXPORT void rawserial_flush(Driver *drvthis);
MODULE_EXPORT void rawserial_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void rawserial_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *rawserial_get_info(Driver *drvthis);

#endif
