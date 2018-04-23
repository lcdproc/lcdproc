#ifndef SERPOS_H
#define SERPOS_H

MODULE_EXPORT int serialPOS_init(Driver* drvthis);
MODULE_EXPORT void serialPOS_close(Driver* drvthis);
MODULE_EXPORT int serialPOS_width(Driver* drvthis);
MODULE_EXPORT int serialPOS_height(Driver* drvthis);
MODULE_EXPORT int serialPOS_cellwidth(Driver* drvthis);
MODULE_EXPORT int serialPOS_cellheight(Driver* drvthis);
MODULE_EXPORT void serialPOS_clear(Driver* drvthis);
MODULE_EXPORT void serialPOS_flush(Driver* drvthis);
MODULE_EXPORT void serialPOS_string(Driver* drvthis, int x, int y,
				    const char string[]);
MODULE_EXPORT void serialPOS_chr(Driver* drvthis, int x, int y, char c);
MODULE_EXPORT const char* serialPOS_get_key(Driver* drvthis);

MODULE_EXPORT void serialPOS_vbar(Driver* drvthis, int x, int y, int len,
				  int promille, int options);
MODULE_EXPORT void serialPOS_hbar(Driver* drvthis, int x, int y, int len,
				  int promille, int options);
MODULE_EXPORT void serialPOS_num(Driver* drvthis, int x, int num);
MODULE_EXPORT void serialPOS_cursor(Driver* drvthis, int x, int y, int state);

MODULE_EXPORT int serialPOS_get_free_chars(Driver* drvthis);
// MODULE_EXPORT void serialPOS_set_char (Driver *drvthis, int n, unsigned char
// *dat);

MODULE_EXPORT int serialPOS_get_brightness(Driver* drvthis, int state);
MODULE_EXPORT void serialPOS_set_brightness(Driver* drvthis, int state,
					    int promille);
MODULE_EXPORT void serialPOS_backlight(Driver* drvthis, int state);

MODULE_EXPORT void serialPOS_output(Driver* drvthis, int state);
MODULE_EXPORT const char* serialPOS_get_info(Driver* drvthis);

#endif /* serialPOS_H */
