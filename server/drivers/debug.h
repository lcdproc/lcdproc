#ifndef LCD_DEBUG_H
#define LCD_DEBUG_H

MODULE_EXPORT int  debug_init (Driver *drvthis);
MODULE_EXPORT void debug_close (Driver *drvthis);
MODULE_EXPORT int  debug_width (Driver *drvthis);
MODULE_EXPORT int  debug_height (Driver *drvthis);
MODULE_EXPORT int  debug_cellwidth (Driver *drvthis);
MODULE_EXPORT int  debug_cellheight (Driver *drvthis);
MODULE_EXPORT void debug_clear (Driver *drvthis);
MODULE_EXPORT void debug_flush (Driver *drvthis);
MODULE_EXPORT void debug_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void debug_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *debug_get_key (Driver *drvthis);

MODULE_EXPORT void debug_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void debug_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void debug_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  debug_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void debug_cursor(Driver *drvthis, int x, int y, int type);
MODULE_EXPORT void debug_set_char (Driver *drvthis, int n, char *dat);
MODULE_EXPORT int  debug_get_free_chars(Driver *drvthis);
/* debug_heartbeat not implemented, use server's default */

MODULE_EXPORT int  debug_get_contrast (Driver *drvthis);
MODULE_EXPORT void debug_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  debug_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void debug_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void debug_backlight (Driver *drvthis, int promille);
MODULE_EXPORT void debug_output(Driver *drvthis, int value);

MODULE_EXPORT const char *debug_get_info (Driver *drvthis);

#endif
