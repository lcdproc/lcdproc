#ifndef EYEBOXONE_H
#define EYEBOXONE_H

MODULE_EXPORT int  EyeboxOne_init (Driver *drvthis);
MODULE_EXPORT void EyeboxOne_close (Driver *drvthis);
MODULE_EXPORT int  EyeboxOne_width (Driver *drvthis);
MODULE_EXPORT int  EyeboxOne_height (Driver *drvthis);
MODULE_EXPORT void EyeboxOne_clear (Driver *drvthis);
MODULE_EXPORT void EyeboxOne_flush (Driver *drvthis);
MODULE_EXPORT void EyeboxOne_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void EyeboxOne_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void EyeboxOne_set_char (Driver *drvthis, int n, char *dat);

MODULE_EXPORT void EyeboxOne_backlight (Driver *drvthis, int on);

MODULE_EXPORT const char * EyeboxOne_get_key (Driver *drvthis);
MODULE_EXPORT const char * EyeboxOne_get_info (Driver *drvthis);

#define DEFAULT_DEVICE		"/dev/ttyS1"
#define DEFAULT_SPEED		19200
#define DEFAULT_CURSOR		0

#endif

