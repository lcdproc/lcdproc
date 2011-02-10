#ifndef GLK_H
#define GLK_H

MODULE_EXPORT int  glk_init(Driver *drvthis);
MODULE_EXPORT void glk_close(Driver *drvthis);
MODULE_EXPORT int  glk_width(Driver *drvthis);
MODULE_EXPORT int  glk_height(Driver *drvthis);
MODULE_EXPORT int  glk_cellwidth(Driver *drvthis);
MODULE_EXPORT int  glk_cellheight(Driver *drvthis);
MODULE_EXPORT void glk_clear(Driver *drvthis);
MODULE_EXPORT void glk_flush(Driver *drvthis);
MODULE_EXPORT void glk_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void glk_chr(Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void glk_num(Driver *drvthis, int x, int num);
MODULE_EXPORT void glk_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void glk_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  glk_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT int  glk_get_free_chars(Driver *drvthis);
MODULE_EXPORT void glk_set_char(Driver *drvthis, int n, char *dat);

MODULE_EXPORT int  glk_get_contrast(Driver *drvthis);
MODULE_EXPORT void glk_set_contrast(Driver *drvthis, int promille);
MODULE_EXPORT void glk_backlight(Driver *drvthis, int on);
MODULE_EXPORT void glk_output(Driver *drvthis, int on);

MODULE_EXPORT const char *glk_get_key(Driver *drvthis);

#endif
