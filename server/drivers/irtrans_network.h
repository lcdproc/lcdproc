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

#define IRTRANS_PROTOCOL_VERSION	208

#define COMMAND_SEND		1
#define COMMAND_LRNREM		2
#define COMMAND_LRNTIM		3
#define COMMAND_LRNCOM		4
#define COMMAND_CLOSE		5
#define COMMAND_STATUS		6
#define COMMAND_RESEND		7
#define COMMAND_LRNRAW		8
#define COMMAND_LRNRPT		9
#define COMMAND_LRNTOG		10
#define COMMAND_SETSTAT		11
#define COMMAND_LRNLONG		12
#define COMMAND_LRNRAWRPT	13
#define COMMAND_RELOAD		14
#define COMMAND_LCD		15
#define COMMAND_LEARNSTAT	16
#define COMMAND_TEMP		17
#define COMMAND_GETREMOTES	18
#define COMMAND_GETCOMMANDS	19
#define COMMAND_STORETRANS	20
#define COMMAND_LOADTRANS	21
#define COMMAND_SAVETRANS	22
#define COMMAND_FLASHTRANS	23
#define	COMMAND_FUNCTIONS	24
#define	COMMAND_TESTCOM		25
#define	COMMAND_LONGSEND	26
#define	COMMAND_SHUTDOWN	27
#define COMMAND_SENDCCF		28
#define COMMAND_LCDINIT		29
#define COMMAND_SETSWITCH	30
#define COMMAND_STATUSEX	31
#define COMMAND_RESET		32
#define	STATUS_MESSAGE		1
#define	STATUS_TIMING		2
#define	STATUS_DEVICEMODE	3
#define	STATUS_RECEIVE		4
#define STATUS_LEARN		5
#define STATUS_REMOTELIST	6
#define STATUS_COMMANDLIST	7
#define STATUS_TRANSLATE	8
#define STATUS_FUNCTION		9
#define	STATUS_DEVICEMODEEX	10

#pragma pack(1)
typedef struct {
    uint8_t mode;
    uint8_t time_cnt;
    uint8_t ir_repeat;
    uint8_t repeat_pause;
    uint16_t pause_len[TIME_LEN];
    uint16_t pulse_len[TIME_LEN];
    uint8_t data[CODE_LEN];
} TIMINGDATA;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint16_t timeout;
    int32_t adress;
    int8_t remote[80];
    int8_t command[20];
    uint8_t trasmit_freq;
} OLD_NETWORKCOMMAND;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint8_t lcdcommand;
    uint8_t timeout;
    int32_t adress;
    uint8_t wid;
    uint8_t hgt;
    int8_t framebuffer[200];
} OLD_LCDCOMMAND;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint16_t timeout;
    int32_t adress;
    int32_t protocol_version;
    int8_t remote[80];
    int8_t command[20];
    uint8_t trasmit_freq;
} NETWORKCOMMAND;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint16_t timeout;
    int32_t adress;
    int32_t protocol_version;
    uint16_t sendmask[32];
    uint16_t pronto_data[256];
} CCFCOMMAND;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint8_t lcdcommand;
    uint8_t timeout;
    int32_t adress;
    int32_t protocol_version;
    uint8_t wid;
    uint8_t hgt;
    int8_t framebuffer[200];
} LCDCOMMAND;

typedef struct {
    uint8_t netcommand;
    uint8_t mode;
    uint16_t timeout;
    int32_t adress;
    int32_t protocol_version;
    int32_t number;
    uint8_t setup;
    uint8_t type;
    uint8_t accelerator_timeout;
    uint8_t accelerator_repeat;
    uint16_t wait_timeout;
    uint8_t remote_num;
    uint8_t group_num;
    int8_t remote[80];
    int8_t command[20];
    int32_t source_mask;
    int32_t target_mask;
} TRANSLATECOMMAND;

typedef struct {
    int32_t send_mask;
    uint8_t device_mode;
    uint8_t extended_mode;
    uint16_t switch_mode;
    uint16_t features;
    int8_t version[10];
    int8_t remote[80];
    int8_t command[20];
} MODELINE;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint8_t align[2];
    int8_t data[16384];
} STATUSBUFFER;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int32_t serno;
    int32_t functions;
} FUNCTIONBUFFER;

typedef struct {
    int32_t target_mask;
    int32_t source_mask;
    int8_t name[80];
} REMOTELINE;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t offset;
    int16_t count_buffer;
    int16_t count_total;
    int16_t count_remaining;
    REMOTELINE remotes[40];
} REMOTEBUFFER;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t offset;
    int16_t count_buffer;
    int16_t count_total;
    int16_t count_remaining;
    int8_t commands[200][20];
} COMMANDBUFFER;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t offset;
    int16_t count_buffer;
    int16_t count_total;
    int16_t count_remaining;
    TRANSLATECOMMAND trdata[30];
} TRANSLATEBUFFER;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint8_t align[2];
    MODELINE stat[16];
} NETWORKMODE;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint8_t align;
    uint8_t count;
    uint8_t dev_adr[8];
    MODELINE stat[8][16];
} NETWORKMODEEX;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint16_t netstatus;
    uint16_t statuslevel;
    uint8_t align[2];
    int8_t message[256];
} NETWORKSTATUS;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint8_t align[2];
    TIMINGDATA timing;
} NETWORKTIMING;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    uint16_t command_num;
    int8_t remote[80];
    int8_t command[20];
    int8_t data[200];
} NETWORKRECV;

typedef struct {
    uint32_t clientid;
    int16_t statuslen;
    int16_t statustype;
    int16_t adress;
    int16_t learnok;
    int8_t remote[80];
    int16_t num_timings;
    int16_t num_commands;
    int8_t received[CODE_LEN];
} NETWORKLEARNSTAT;

typedef struct {
    SOCKET fd;
    int32_t type;
    uint32_t clientid;
    uint32_t callno;
    int8_t ip[20];
    WSAEVENT event;
    FILE *fp;
    int32_t timing;
    uint8_t resend_load;
    IRDATA ird;
    NETWORKLEARNSTAT learnstatus;
    int8_t restdata[sizeof(LCDCOMMAND)];
    int32_t restlen;
    int32_t restread;
}
NETWORKCLIENT;

#define TCP_PORT	21000
#define LIRC_PORT	8765
#define UDP_PORT	6510
#define WEB_PORT	80
#define ALTERNATE_WEB	8080
#define CLIENT_COUNT	64
