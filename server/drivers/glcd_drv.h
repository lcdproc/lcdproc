#ifndef GLCD_DRV_H
#define GLCD_DRV_H

MODULE_EXPORT int  glcd_init(Driver *drvthis);
MODULE_EXPORT void glcd_close(Driver *drvthis);
MODULE_EXPORT int  glcd_width(Driver *drvthis);
MODULE_EXPORT int  glcd_height(Driver *drvthis);
MODULE_EXPORT int  glcd_cellwidth(Driver *drvthis);
MODULE_EXPORT int  glcd_cellheight(Driver *drvthis);
MODULE_EXPORT void glcd_clear(Driver *drvthis);
MODULE_EXPORT void glcd_flush(Driver *drvthis);
MODULE_EXPORT void glcd_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void glcd_chr(Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void glcd_num(Driver *drvthis, int x, int num);
MODULE_EXPORT void glcd_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void glcd_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  glcd_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT const char *glcd_get_info(Driver *drvthis);

MODULE_EXPORT int  glcd_get_contrast (Driver *drvthis);
MODULE_EXPORT void glcd_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  glcd_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void glcd_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void glcd_backlight (Driver *drvthis, int promille);
MODULE_EXPORT void glcd_output(Driver *drvthis, int value);

MODULE_EXPORT const char *glcd_get_key(Driver *drvthis);
#endif
