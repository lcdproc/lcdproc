#ifndef MTXORB_H
#define MTXORB_H

#include "lcd.h"

#define DEFAULT_CONTRAST	480
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		19200
#define DEFAULT_LINEWRAP	1
#define DEFAULT_AUTOSCROLL	1
#define DEFAULT_CURSORBLINK	0


MODULE_EXPORT int  MtxOrb_init (Driver *drvthis);
MODULE_EXPORT void MtxOrb_close (Driver *drvthis);
MODULE_EXPORT int  MtxOrb_width (Driver *drvthis);
MODULE_EXPORT int  MtxOrb_height (Driver *drvthis);
MODULE_EXPORT void MtxOrb_clear (Driver *drvthis);
MODULE_EXPORT void MtxOrb_flush (Driver *drvthis);
MODULE_EXPORT void MtxOrb_string (Driver *drvthis, int x, int y, char *string);
MODULE_EXPORT void MtxOrb_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void MtxOrb_set_char (Driver *drvthis, int n, char *dat);

MODULE_EXPORT int  MtxOrb_get_contrast (Driver *drvthis);
MODULE_EXPORT void MtxOrb_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void MtxOrb_backlight (Driver *drvthis, int on);
MODULE_EXPORT void MtxOrb_output (Driver *drvthis, int on);

MODULE_EXPORT const char * MtxOrb_get_key (Driver *drvthis);
MODULE_EXPORT const char * MtxOrb_get_info (Driver *drvthis);

MODULE_EXPORT void MtxOrb_num (Driver *drvthis, int x, int num);

MODULE_EXPORT int  MtxOrb_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void MtxOrb_vbar (Driver * drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void MtxOrb_hbar (Driver * drvthis, int x, int y, int len, int promille, int options);

#endif

