#ifndef LCD_LIB_H
#define LCD_LIB_H

#ifndef LCD_H
#include "lcd.h"
#endif

void lib_hbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellwidth, int cc_offset);
void lib_vbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellheight, int cc_offset);

#endif

