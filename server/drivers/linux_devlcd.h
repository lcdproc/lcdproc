#ifndef LINUX_DEVLCD_H
#define LINUX_DEVLCD_H

MODULE_EXPORT int  linuxDevLcd_init (Driver *drvthis);
MODULE_EXPORT void linuxDevLcd_close (Driver *drvthis);
MODULE_EXPORT int  linuxDevLcd_width (Driver *drvthis);
MODULE_EXPORT int  linuxDevLcd_height (Driver *drvthis);
MODULE_EXPORT void linuxDevLcd_clear (Driver *drvthis);
MODULE_EXPORT void linuxDevLcd_flush (Driver *drvthis);
MODULE_EXPORT void linuxDevLcd_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void linuxDevLcd_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void linuxDevLcd_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void linuxDevLcd_backlight (Driver *drvthis, int on);
MODULE_EXPORT const char * linuxDevLcd_get_info (Driver *drvthis);

#define DEFAULT_DEVICE	"-"
#define TEXTDRV_DEFAULT_SIZE "20x4"

#endif
