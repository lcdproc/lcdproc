/***************************************************************************
 *   Copyright (C) 2013 by R. Geigenberger   *
 *      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef yard2LCD_H
#define yard2LCD_H

#define MAX_YARDDATA_SIZE		40
#define DEFAULT_SOCK_PATH        "/tmp/lcdserver"
#define DEFAULT_OFF_BRIGHTNESS	 100
#define DEFAULT_ON_BRIGHTNESS	 1000
#define DEFAULT_WIDTH		16
#define DEFAULT_HEIGHT		4
#define DEFAULT_GWIDTH		64
#define DEFAULT_GHEIGHT		32
#define DEFAULT_CELLWIDTH        5
#define DEFAULT_CELLHEIGHT       8
#define DEFAULT_CELLWIDTH_BIG	 10
#define DEFAULT_CELLHEIGHT_BIG	 14
#define DEFAULT_HSPACE           2
#define DEFAULT_VSPACE           1
#define NUM_CUSTOMCHARS          8


MODULE_EXPORT int  yard_init (Driver *drvthis);
MODULE_EXPORT void yard_close (Driver *drvthis);
MODULE_EXPORT int  yard_width (Driver *drvthis);
MODULE_EXPORT int  yard_height (Driver *drvthis);
MODULE_EXPORT void yard_clear (Driver *drvthis);
MODULE_EXPORT void yard_flush (Driver *drvthis);
MODULE_EXPORT void yard_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void yard_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void yard_num(Driver *drvthis, int x, int num);
MODULE_EXPORT void yard_set_char(Driver *drvthis, int n, unsigned char *dat);
MODULE_EXPORT int  yard_get_free_chars(Driver *drvthis);
MODULE_EXPORT int  yard_cellwidth (Driver *drvthis);
MODULE_EXPORT int  yard_cellheight (Driver *drvthis);
MODULE_EXPORT int  yard_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void yard_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void yard_backlight (Driver *drvthis, int on);
MODULE_EXPORT const char * yard_get_info( Driver *drvthis );


#endif
