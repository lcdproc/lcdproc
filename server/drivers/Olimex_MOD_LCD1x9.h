#ifndef OLIMEX_LCD1x9_H
#define OLIMEX_LCD1x9_H

MODULE_EXPORT int  olimex1x9_init (Driver *drvthis);
MODULE_EXPORT void olimex1x9_close (Driver *drvthis);
MODULE_EXPORT int  olimex1x9_width (Driver *drvthis);
MODULE_EXPORT int  olimex1x9_height (Driver *drvthis);
MODULE_EXPORT void olimex1x9_clear (Driver *drvthis);
MODULE_EXPORT void olimex1x9_flush (Driver *drvthis);
MODULE_EXPORT void olimex1x9_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void olimex1x9_chr (Driver *drvthis, int x, int y, char c);

#endif
