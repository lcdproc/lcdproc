/*
 * Base driver module for Toshiba T6963 based LCD displays ver 1.2
 *
 * Parts of this file are based on the kernel driver by Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
 */

#ifndef T6963_H
#define T6963_H

#define SM_UP (1)
#define SM_DOWN (2)
#define CM_ERASE (2)
#define CUR_LOWER_THIRD 3
#define CUR_LOWER_HALF 4
#define CUR_TWO_THIRDS 5
#define CUR_BLOCK 6
#define CUR_NONE 1

// take PC 1 HI
#define T6963_CEHI    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xfe)
// take PC 1 LO
#define T6963_CELO    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x01)

// take PC 14 HI
#define T6963_RDHI    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xfd)
// take PC 14 LO
#define T6963_RDLO    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x02)

// take PC 16 HI
#define T6963_WRHI    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x04)
// take PC 16 LO
#define T6963_WRLO    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xfb)

// take PC 17 HI
#define T6963_CDHI    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xf7)
// take PC 17 LO
#define T6963_CDLO    port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x08)

// 8bit Data input
#define T6963_DATAIN  port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) | 0x20)
// 8bit Data output
#define T6963_DATAOUT port_out(T6963_CONTROL_PORT, port_in(T6963_CONTROL_PORT) & 0xdf)

#define CHARGEN_BASE 0x0000
#define TEXT_BASE 0x0800
#define ATTRIB_BASE 0x1000
#define ATTRIB_AREA 0x03BF

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

extern lcd_logical_driver *t6963;

int  t6963_init (struct lcd_logical_driver *driver, char *args);
void t6963_close ();
void t6963_clear ();
void t6963_graphic_clear ();
void t6963_flush ();
void t6963_string (int x, int y, char string[]);
void t6963_chr (int x, int y, char c);
void t6963_vbar (int x, int len);
void t6963_hbar (int x, int y, int len);
void t6963_num (int x, int num);
void t6963_set_nchar (int n, char *dat, int num);
void t6963_set_char (int n, char *dat);
void t6963_icon (int which, char dest);
void t6963_heartbeat (int type);
void t6963_flush_box (int lft, int top, int rgt, int bot);
void t6963_draw_frame (char *dat);
char t6963_getkey ();

void t6963_low_data(u8 byte);
void t6963_low_command (u8 byte);

void t6963_low_command_byte(u8 cmd, u8 byte);
void t6963_low_command_2_bytes(u8 cmd, u8 byte1, u8 byte2);
void t6963_low_command_word(u8 cmd, u16 word);

void t6963_low_enable_mode (u8 mode);
void t6963_low_disable_mode (u8 mode);

void t6963_swap_buffers();

#endif
