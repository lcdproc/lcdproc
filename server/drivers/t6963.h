/*-
 * Base driver module for Toshiba T6963 based LCD displays ver 2.2
 *    ( ver 2.* is not at all compatible to 1.* versions!)
 *
 * Parts of this file are based on the kernel driver by
 * Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
 */

#ifndef T6963_H
#define T6963_H

#define DEFAULT_CELL_WIDTH 6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_SIZE "20x6"
#define DEFAULT_PORT 0x378

/** Configure LPT port for bidirectional input */
#define T6963_DATAIN(p)  port_out(T6963_CONTROL_PORT(p), port_in(T6963_CONTROL_PORT(p)) | 0x20)
/** Configure LPT port for bidirectional output */
#define T6963_DATAOUT(p) port_out(T6963_CONTROL_PORT(p), port_in(T6963_CONTROL_PORT(p)) & 0xdf)

/* RAM layout base addresses (for 64 KB RAM) */
/*
 * FIXME: Those addresses are fine for 64 KB RAM. If the display has less,
 * these values need to be adjusted by the user. It should do no harm setting
 * these for 8 KB RAM.
 */
#define TEXT_BASE	0x0000
#define ATTRIB_BASE	0x7000
#define CHARGEN_BASE	0xF000

/* 'Registers setting' commands */
#define SET_CURSOR_POINTER	0x21
#define SET_OFFSET_REGISTER	0x22
#define SET_ADDRESS_POINTER	0x24

/* 'Set control word' commands */
#define SET_TEXT_HOME_ADDRESS	0x40
#define SET_TEXT_AREA		0x41
#define SET_GRAPHIC_HOME_ADDRESS	0x42
#define SET_GRAPHIC_AREA	0x43

/* 'Mode set' command and options*/
#define SET_MODE	0x80
#define OR_MODE		0x00
#define EXOR_MODE	0x01
#define AND_MODE	0x03
#define ATTRIBUTE_MODE	0x04
#define INTERNAL_CG	0x00
#define EXTERNAL_CG	0x08

/* 'Display mode' command and options */
#define SET_DISPLAY_MODE	0x90
#define BLINK_ON	0x01
#define CURSOR_ON	0x02
#define TEXT_ON		0x04
#define GRAPHIC_ON	0x08

/* 'Cursor pattern select' command */
#define SET_CURSOR_PATTERN	0xa0

/* 'Data read/write' commands */
#define DATA_WRITE_INC	0xc0
#define DATA_READ_INC	0xc1
#define DATA_WRITE_DEC	0xc2
#define DATA_READ_DEC	0xc3
#define DATA_WRITE	0xc4
#define DATA_READ	0xc5

/* 'Data auto read/write' commands */
#define AUTO_WRITE	0xb0
#define AUTO_READ	0xb1
#define AUTO_RESET	0xb


/* Macros to make port usage more readable */
#define T6963_DATA_PORT(p) (p)
#define T6963_STATUS_PORT(p) ((p)+1)
#define T6963_CONTROL_PORT(p) ((p)+2)


/* Data types */
typedef unsigned short u16;
typedef unsigned char u8;


/* API functions */
MODULE_EXPORT int  t6963_init (Driver *drvthis);
MODULE_EXPORT void t6963_close (Driver *drvthis);
MODULE_EXPORT int  t6963_width (Driver *drvthis);
MODULE_EXPORT int  t6963_height (Driver *drvthis);
MODULE_EXPORT void t6963_clear (Driver *drvthis);
MODULE_EXPORT void t6963_flush (Driver *drvthis);
MODULE_EXPORT void t6963_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void t6963_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void t6963_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void t6963_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  t6963_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void t6963_set_char (Driver *drvthis, int n, char *dat);


/* Internal functions */
void t6963_graphic_clear (Driver *drvthis, int x1, int y1, int x2, int y2);
void t6963_set_nchar (Driver *drvthis, int n, unsigned char *dat, int num);

void t6963_low_set_control(Driver *drvthis, char wr, char ce, char cd, char rd);
void t6963_low_dsp_ready(Driver *drvthis);

void t6963_low_data(Driver *drvthis, u8 byte);
void t6963_low_command (Driver *drvthis, u8 byte);

void t6963_low_command_byte(Driver *drvthis, u8 cmd, u8 byte);
void t6963_low_command_2_bytes(Driver *drvthis, u8 cmd, u8 byte1, u8 byte2);
void t6963_low_command_word(Driver *drvthis, u8 cmd, u16 word);

void t6963_low_enable_mode (Driver *drvthis, u8 mode);
void t6963_low_disable_mode (Driver *drvthis, u8 mode);

void t6963_swap_buffers(Driver *drvthis);

#endif
