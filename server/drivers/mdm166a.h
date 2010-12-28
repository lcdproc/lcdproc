#ifndef MDM166A_H
#define MDM166A_H

MODULE_EXPORT int mdm166a_init (Driver *drvthis);
MODULE_EXPORT void mdm166a_close (Driver *drvthis);
MODULE_EXPORT int mdm166a_width (Driver *drvthis);
MODULE_EXPORT int mdm166a_height (Driver *drvthis);
MODULE_EXPORT int mdm166a_cellwidth (Driver *drvthis);
MODULE_EXPORT int mdm166a_cellheight (Driver *drvthis);
MODULE_EXPORT void mdm166a_clear (Driver *drvthis);
MODULE_EXPORT void mdm166a_flush (Driver *drvthis);
MODULE_EXPORT void mdm166a_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void mdm166a_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void mdm166a_vbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void mdm166a_hbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT int  mdm166a_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void mdm166a_set_char (Driver *drvthis, int n, char *dat);
MODULE_EXPORT void mdm166a_backlight (Driver *drvthis, int on);
MODULE_EXPORT const char * mdm166a_get_info (Driver *drvthis);
MODULE_EXPORT void mdm166a_output (Driver *drvthis, int on);

#endif
