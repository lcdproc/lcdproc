/**
 * Driver for VL System VFD and IR receiver M428 in Moneual Moncaso 320.
 */

/*-
 * Copyright (c) 2011 Wolfgang Hauck <wolfgang.hauck@gmx.de>.
 * Derived from imon-module.
 *
 * This source code is being released under the GPL.
 * Please see the file COPYING in this package for details.
 */

#ifndef VLSYS_M428_H
#define VLSYS_M428_H

MODULE_EXPORT int  vlsys_m428_init(Driver *drvthis);
MODULE_EXPORT void vlsys_m428_close(Driver *drvthis);

MODULE_EXPORT int  vlsys_m428_width(Driver *drvthis);
MODULE_EXPORT int  vlsys_m428_height(Driver *drvthis);
MODULE_EXPORT void vlsys_m428_clear(Driver *drvthis);
MODULE_EXPORT void vlsys_m428_flush(Driver *drvthis);
MODULE_EXPORT void vlsys_m428_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void vlsys_m428_chr(Driver *drvthis, int x, int y, char c);

MODULE_EXPORT const char * vlsys_get_info(Driver *drvthis);

#endif
