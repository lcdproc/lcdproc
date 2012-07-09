/** \file server/drivers/t6963_low.h
 * Command set for Toshiba T6963 based LCD displays.
 *
 * This header file defines the T6963 command set and various controller
 * parameters like memory addresses.
 */

/*-
 * Base driver module for Toshiba T6963 based LCD displays.
 *
 * Parts of this file are based on the kernel driver by
 * Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * Copyright (c) 2001 Manuel Stahl <mythos@xmythos.de>
 *               2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifndef T6963_IO_H
#define T6963_IO_H

/*
 * These are the maximum values the controller supports in single-scan
 * configuration with FontSelector (FS) = 8x8. Dual-scan configuration is
 * not supported by this driver.
 */
#define T6963_MAX_WIDTH  640
#define T6963_MAX_HEIGHT 128

/* RAM layout base addresses (suitable for 8 KB RAM) */
#define TEXT_BASE	0x0000	/* 1K, fine for 40x16 or 80x8 */
#define GRAPHIC_BASE	0x0400	/* 5K, fine for 240x128 */
#define CHARGEN_BASE	0x1800	/* 2K, required for 256 CC */

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
#define AUTO_RESET	0xb2

/* 'Status read' answer codes */
#define STA0		0x01
#define STA1		0x02
#define STA3		0x08

/* Macros to make port usage more readable */
#define T6963_DATA_PORT(p) (p)
#define T6963_STATUS_PORT(p) ((p)+1)
#define T6963_CONTROL_PORT(p) ((p)+2)

/* Shorthand data types */
typedef unsigned short u16;
typedef unsigned char u8;

/** Configuration of parallel port */
typedef struct t6963_port_config {
	unsigned int port;
	short bidirectLPT;
	short delayBus;
} T6963_port;

/* External usable functions */
int t6963_low_init(T6963_port *p);
void t6963_low_close(T6963_port *p);
void t6963_low_data(T6963_port *p, u8 byte);
void t6963_low_auto_write(T6963_port *p, u8 byte);
void t6963_low_command(T6963_port *p, u8 byte);
void t6963_low_command_byte(T6963_port *p, u8 cmd, u8 byte);
void t6963_low_command_word(T6963_port *p, u8 cmd, u16 word);
int t6963_low_dsp_ready(T6963_port *p, u8 sta);
inline void t6963_low_send(T6963_port *p, u8 type, u8 byte);

#endif
