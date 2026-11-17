#ifndef LCD_ASUSTOR_H
#define LCD_ASUSTOR_H

#define ASUSTOR_DEFAULT_DEVICE "/dev/ttyS1"
#define ASUSTOR_DEFAULT_SIZE "16x2"
#define ASUSTOR_WIDTH 16
#define ASUSTOR_HEIGHT 2
#define ASUSTOR_CELLWIDTH 5
#define ASUSTOR_CELLHEIGHT 8

MODULE_EXPORT int asustor_init(Driver *drvthis);
MODULE_EXPORT void asustor_close(Driver *drvthis);
MODULE_EXPORT int asustor_width(Driver *drvthis);
MODULE_EXPORT int asustor_height(Driver *drvthis);
MODULE_EXPORT int asustor_cellwidth(Driver *drvthis);
MODULE_EXPORT int asustor_cellheight(Driver *drvthis);
MODULE_EXPORT void asustor_clear(Driver *drvthis);
MODULE_EXPORT void asustor_flush(Driver *drvthis);
MODULE_EXPORT void asustor_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void asustor_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *asustor_get_key(Driver *drvthis);
MODULE_EXPORT void asustor_backlight(Driver *drvthis, int on);
MODULE_EXPORT void asustor_output(Driver *drvthis, int on);
MODULE_EXPORT const char *asustor_get_info(Driver *drvthis);

#endif
