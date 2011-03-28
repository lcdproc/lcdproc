/*	wirz-sli.h -- Header file for LCDproc Wirz SLI driver
	Copyright (C) 1999 Horizon Technologies-http://horizon.pair.com/
	Written by Bryan Rittmeyer <bryanr@pair.com> - Released under GPL

        LCD info: http://www.wirz.com/sli/                               */

#ifndef SLI_H
#define SLI_H

MODULE_EXPORT int  sli_init (Driver *drvthis);
MODULE_EXPORT void sli_close (Driver *drvthis);
MODULE_EXPORT int  sli_width (Driver *drvthis);
MODULE_EXPORT int  sli_height (Driver *drvthis);
MODULE_EXPORT int  sli_cellwidth (Driver *drvthis);
MODULE_EXPORT int  sli_cellheight (Driver *drvthis);
MODULE_EXPORT void sli_clear (Driver *drvthis);
MODULE_EXPORT void sli_flush (Driver *drvthis);
MODULE_EXPORT void sli_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void sli_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void sli_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void sli_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  sli_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void sli_set_char (Driver *drvthis, int n, unsigned char *dat);

#endif
