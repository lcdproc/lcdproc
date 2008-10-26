/*
 * irtrans driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 * Copyright (C) 1998-2007 <info@irtrans.de>
 *
 * Copyright (C) 2007 Phant0m <phantom@netkeke.com>
 * porting the LCDproc 0.4.3 code to LCDproc 0.5.1
 *
 * Inspired by:
 *  TextMode driver (LCDproc authors)
 *  irtrans driver (Irtrans)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

typedef unsigned char byte;
typedef unsigned short word;

#define CONTROL_LED	4

#define USB_RXREADY	0
#define USB_TXENABLE	1
#define USB_WRITE	6
#define	USB_READ	7

#define	IR_LED		0
#define	SBUS_DATA	1
#define SBUS_INPUT	2
#define	SBUS_TERM	3
#define IR_INPUT	7


#ifndef AVR
#pragma pack(1)
#define CODE_LEN	176     // 2 Byte mehr für String-Ende 0
#else
#define CODE_LEN	174

#define	USB

#endif

#define TIME_LEN	6
#define RAW_EXTRA	TIME_LEN * 4 + 2
#define CODE_LENRAW	(CODE_LEN + RAW_EXTRA - 2)



typedef struct {
    byte len;
    byte checksumme;
    byte command;

    byte address;
    word target_mask;

    byte ir_length;
    byte transmit_freq;
    byte mode;

    word pause_len[TIME_LEN];
    word pulse_len[TIME_LEN];
    byte time_cnt;
    byte ir_repeat;
    byte repeat_pause;

    byte data[CODE_LEN];
} IRDATA;


typedef struct {
    byte len;
    byte checksumme;
    byte command;

    byte address;
    word target_mask;

    byte ir_length;
    byte transmit_freq;
    byte mode;

    byte data[CODE_LENRAW];
} IRRAW;



#define DEVMODE_PC		0
#define DEVMODE_SEND		1
#define DEVMODE_IR		2
#define DEVMODE_SBUS		4
#define DEVMODE_IRCODE		8
#define DEVMODE_SBUSCODE	16

#define DEVMODE_MASK		31

#define DEVMODE_STATUS		128


// IR Commands
#define SBUS_REPEAT		1
#define HOST_VERSION		2
#define HOST_NETWORK_STATUS	3

#define SBUS_SEND		4
#define SBUS_RESEND		5
#define HOST_SEND		6
#define HOST_RESEND		7


#define SBUS_LEARN		16
#define HOST_LEARNIR		18

#define SBUS_QUICKPARM		48
#define HOST_LEARNIRQUICK	50

#define SBUS_RAWMODE		80
#define HOST_LEARNIRRAW		82

#define SBUS_REPEATMODE		144
#define HOST_LEARNIRREPEAT	146

#define SBUS_RAWREPEATMODE	208
#define HOST_LEARNIRRAWREPEAT	210

#define SBUS_RESET		192
#define SBUS_PING		193
#define SBUS_PONG		194
#define SBUS_PARAMETER		196
#define HOST_SETMODE		197


#define ADRESS_MASK		15
#define ADRESS_LOCAL		16
#define ADRESS_ALL		32


#define	START_BIT		1
#define REPEAT_START		2
#define START_MASK		3
#define RC5_DATA		4
#define	RC6_DATA		8
#define	RAW_DATA		16

#define LCD_DATA		32
#define LCD_BACKLIGHT		1
#define LCD_TEXT		2
#define DEFAULT_FREQ		39
#define RAW_FREQ		38
#define IR_CORRECT		8       // Entspricht 64 µs Korrektur
#define RCX_TOLERANCE		19      // Entspricht 152 µs Toleranz
#define IR_TOLERANCE		15      // Entspricht 120 µs Toleranz
#define RAW_TOLERANCE		20

typedef struct {
    byte sbus_len;
    byte sbus_checksumme;
    byte sbus_command;
    byte sbus_address;
    byte mode;
    word target_mask;
    byte hotcode_len;
    byte hotcode[100];
} MODE_BUFFER;

typedef struct {
    byte sbus_len;
    byte sbus_checksumme;
    byte sbus_command;
    byte sbus_address;
    byte device_mode;
    word send_mask;
    byte version[10];
} STATUS_LINE;

typedef struct {
    byte my_adress;
    STATUS_LINE stat[16];
} STATUS_BUFFER;

