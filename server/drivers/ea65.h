#ifndef EA65_H
#define EA65_H

#define DEFAULT_BRIGHTNESS 500
#define DEFAULT_OFFBRIGHTNESS 0

MODULE_EXPORT int  EA65_init (Driver *drvthis);
MODULE_EXPORT void EA65_close (Driver *drvthis);
MODULE_EXPORT int  EA65_width (Driver *drvthis);
MODULE_EXPORT int  EA65_height (Driver *drvthis);
MODULE_EXPORT void EA65_clear (Driver *drvthis);
MODULE_EXPORT void EA65_flush (Driver *drvthis);
MODULE_EXPORT void EA65_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void EA65_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void EA65_backlight (Driver *drvthis, int on);
MODULE_EXPORT void EA65_output (Driver *drvthis, int on);

#endif
