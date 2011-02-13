#ifndef LB216_H
#define LB216_H

MODULE_EXPORT int  LB216_init(Driver *drvthis);
MODULE_EXPORT void LB216_close(Driver *drvthis);
MODULE_EXPORT int  LB216_width (Driver *drvthis);
MODULE_EXPORT int  LB216_height (Driver *drvthis);
MODULE_EXPORT int  LB216_cellwidth (Driver *drvthis);
MODULE_EXPORT int  LB216_cellheight (Driver *drvthis);
MODULE_EXPORT void LB216_clear (Driver *drvthis);
MODULE_EXPORT void LB216_flush(Driver *drvthis);
MODULE_EXPORT void LB216_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void LB216_chr(Driver *drvthis, int x, int y, char c) ;

MODULE_EXPORT void LB216_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void LB216_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void LB216_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  LB216_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void LB216_set_char(Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT void LB216_backlight(Driver *drvthis, int on);

#endif
