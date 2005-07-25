/*
 * Base driver module for Toshiba T6963 based LCD displays ver 2.2
 *    ( ver 2.* is not at all compatible to 1.* versions!)
 *
 * Parts of this file are based on the kernel driver by Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
 *
 * Wiring:
 *  Parallel:              LCD:
 *  1 (Strobe) ----------- 5 (WR)
 *  2-9 (Data) ----------- 11-18 (Data)
 *  14 (Autofeed) -------- 7 (CE)
 *  16 (Init) ------------ 8 (C/D)
 *  17 (Slct) ------------ 6 (RD)
 *  18 (GND) ------------- 3 (GND)
 *  +5V ------------------ 3(LCD +)
 *
 */

#ifndef T6963_H
#define T6963_H

#include "lcd.h"

#define DEFAULT_CELL_WIDTH 6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_SIZE "20x6"
#define DEFAULT_PORT 0x378
#define DEFAULT_WRHI 0x04
#define DEFAULT_WRLO 0xfb
#define DEFAULT_CEHI 0xfe
#define DEFAULT_CELO 0x01
#define DEFAULT_CDHI 0xf7
#define DEFAULT_CDLO 0x08
#define DEFAULT_RDHI 0xfd
#define DEFAULT_RDLO 0x02

#define SM_UP (1)
#define SM_DOWN (2)
#define CM_ERASE (2)
#define CUR_LOWER_THIRD 3
#define CUR_LOWER_HALF 4
#define CUR_TWO_THIRDS 5
#define CUR_BLOCK 6
#define CUR_NONE 1

// 8bit Data input
#define T6963_DATAIN  port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x20)
// 8bit Data output
#define T6963_DATAOUT port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xdf)

#define TEXT_BASE 0x0000
#define ATTRIB_BASE 0x7000
#define CHARGEN_BASE 0xF000

#define SET_CURSOR_POINTER 0x21
#define SET_OFFSET_REGISTER 0x22
#define SET_ADDRESS_POINTER 0x24

#define SET_TEXT_HOME_ADDRESS 0x40
#define SET_TEXT_AREA 0x41
#define SET_GRAPHIC_HOME_ADDRESS 0x42
#define SET_GRAPHIC_AREA 0x43

#define SET_MODE 0x80
#define OR_MODE 0x00
#define EXOR_MODE 0x01
#define AND_MODE 0x03
#define ATTRIBUTE_MODE 0x04
#define INTERNAL_CG 0x00
#define EXTERNAL_CG 0x08

#define SET_DISPLAY_MODE 0x90
#define BLINK_ON 0x01
#define CURSOR_ON 0x02
#define TEXT_ON 0x04
#define GRAPHIC_ON 0x08

#define SET_CURSOR_PATTERN 0xa0

#define DATA_WRITE_INC 0xc0
#define DATA_READ_INC 0xc1
#define DATA_WRITE_DEC 0xc2
#define DATA_READ_DEC 0xc3
#define DATA_WRITE 0xc4
#define DATA_READ 0xc5

#define AUTO_WRITE 0xb0
#define AUTO_READ 0xb1
#define AUTO_RESET 0xb

#define POSITION(x,y)  ((y)*lcd.wid + (x))

#define T6963_DATA_PORT (t6963_out_port)
#define T6963_STATUS_PORT (t6963_out_port+1)
#define T6963_CONTROL_PORT (t6963_out_port+2)


// ****************************************************************************************
// *                  V A R I A B L E S                                                   *
// ****************************************************************************************

typedef unsigned short u16;
typedef unsigned char u8;

// ****************************************************************************************
// *                  F U N C T I O N S                                                   *
// ****************************************************************************************


MODULE_EXPORT int  t6963_init (Driver *drvthis);
MODULE_EXPORT void t6963_close (Driver *drvthis);
MODULE_EXPORT int  t6963_width (Driver *drvthis);
MODULE_EXPORT int  t6963_height (Driver *drvthis);
MODULE_EXPORT void t6963_clear (Driver *drvthis);
MODULE_EXPORT void t6963_flush (Driver *drvthis);
MODULE_EXPORT void t6963_string (Driver *drvthis, int x, int y, char string[]);
MODULE_EXPORT void t6963_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void t6963_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  t6963_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void t6963_set_char (Driver *drvthis, int n, char *dat);


void t6963_graphic_clear (Driver *drvthis, int x1, int y1, int x2, int y2);
void t6963_set_nchar (Driver *drvthis, int n, char *dat, int num);

void t6963_low_set_control(char wr, char ce, char cd, char rd);
void t6963_low_dsp_ready(void);

void t6963_low_data(u8 byte);
void t6963_low_command (u8 byte);

void t6963_low_command_byte(u8 cmd, u8 byte);
void t6963_low_command_2_bytes(u8 cmd, u8 byte1, u8 byte2);
void t6963_low_command_word(u8 cmd, u16 word);

void t6963_low_enable_mode (u8 mode);
void t6963_low_disable_mode (u8 mode);

void t6963_swap_buffers();

#endif
