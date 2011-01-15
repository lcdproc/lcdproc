/** \file server/drivers/sed1330.h
 * Driver for SED1330/1335 graphical displays.
 */

#ifndef SED1330_H
#define SED1330_H

MODULE_EXPORT int sed1330_init(Driver *drvthis);
MODULE_EXPORT void sed1330_close(Driver *drvthis);
MODULE_EXPORT int sed1330_width(Driver *drvthis);
MODULE_EXPORT int sed1330_height(Driver *drvthis);
MODULE_EXPORT int sed1330_cellheight(Driver *drvthis);
MODULE_EXPORT int sed1330_cellwidth(Driver *drvthis);
MODULE_EXPORT void sed1330_clear(Driver *drvthis);
MODULE_EXPORT void sed1330_flush(Driver *drvthis);
MODULE_EXPORT void sed1330_string(Driver *drvthis, int x, int y, char lcd[]);
MODULE_EXPORT void sed1330_chr(Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void sed1330_vbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void sed1330_hbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void sed1330_num(Driver *drvthis, int x, int y, int num);
MODULE_EXPORT void sed1330_heartbeat(Driver *drvthis, int type);
MODULE_EXPORT int sed1330_icon(Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT const char *sed1330_get_key(Driver *drvthis);

#endif
