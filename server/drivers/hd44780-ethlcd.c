/** \file hd44780-ethlcd.c
 * \c ethlcd connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 */

/*
 * ethlcd ethernet driver module for Hitachi HD44780 based LCD displays
 * ethlcd is an opensource project based on atmel's atmega microcontroller
 * and enc28j60 ethernet device. The device is connected via ethernet, has own
 * IP address and is available via TCP protocol. More info at project homepage:
 * http://manio.skyboo.net/ethlcd/
 *
 * Copyright (c) 2008  Mariusz Bialonczyk <manio@skyboo.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 */

#include "hd44780-ethlcd.h"
#include "hd44780-low.h"
#include "../../shared/sockets.h"
#include "report.h"
 
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>


void ethlcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
unsigned char ethlcd_HD44780_scankeypad(PrivateData *p);
void ethlcd_HD44780_backlight(PrivateData *p, unsigned char state);
void ethlcd_HD44780_close(PrivateData *p);
// fake pause function (pausing is handled by ethlcd device itself)
void ethlcd_HD44780_uPause(PrivateData *p, int usecs) {}


// initialisation function
int hd_init_ethlcd(Driver *drvthis)
{
	char hostname[_POSIX_HOST_NAME_MAX];
	long flags;

	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	hd44780_functions->senddata = ethlcd_HD44780_senddata;
	hd44780_functions->backlight = ethlcd_HD44780_backlight;
	hd44780_functions->scankeypad = ethlcd_HD44780_scankeypad;
	hd44780_functions->uPause = ethlcd_HD44780_uPause;
	hd44780_functions->close = ethlcd_HD44780_close;

	//reading configuration file
	strncpy(hostname, drvthis->config_get_string(drvthis->name, "Device", 0, "ethlcd"), sizeof(hostname));

	p->sock = sock_connect(hostname, DEFAULT_ETHLCD_PORT);
	if (p->sock < 0) {
		report(RPT_ERR, "Connecting to %s:%d failed", hostname, DEFAULT_ETHLCD_PORT);
		return -1;
	}

	//we need to have a blocking read back again:
	if (fcntl(p->sock, F_GETFL, &flags) < 0)
	{
		report(RPT_ERR, "Cannot set the socket to blocking mode");
		return -1;
	}
	flags &= ~O_NONBLOCK;
	fcntl(p->sock, F_SETFL, flags);

	// Set up two-line, small character (5x8) mode
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);

	common_init(p, IF_4BIT);

	if (p->have_keypad) {
		// Remember which input lines are stuck
		p->stuckinputs = 0;
	}

	return 0;
}


// ethlcd_HD44780_senddata
void ethlcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	static char buff[2];

	if (flags == RS_INSTR)
		buff[0] = ETHLCD_SEND_INSTR;
	else //if (flags == RS_DATA)
		buff[0] = ETHLCD_SEND_DATA;
	buff[1] = ch;

	sock_send(p->sock, buff, 2);	//send data to device
	sock_recv(p->sock, &buff, 1);	//wait for reply
}


unsigned char ethlcd_HD44780_scankeypad(PrivateData *p)
{
	unsigned char readval;
	static char buff[2];

	buff[0] = ETHLCD_GET_BUTTONS;

	sock_send(p->sock, buff, 1);	//send data to device
	sock_recv(p->sock, &buff, 2);	//wait for answer

	if (buff[0] != ETHLCD_GET_BUTTONS)  //check if this is true reply for our packet
		return 0;
	//answer should be in second byte on bits 0-6 in negative logic:
	readval = buff[1];

	readval = ~readval & 0x3F;	//reverse logic
	switch (readval)
	{
		case 1:  return(0x34);	//C
		case 2:  return(0x24);	//B
		case 4:  return(0x14);	//A
		case 8:  return(0x25);  //F
		case 16: return(0x15);	//E
		case 32: return(0x44);	//D
		default: return('\0');
	}
}


void ethlcd_HD44780_backlight(PrivateData *p, unsigned char state)
{
	static char buff[2];

	buff[0] = ETHLCD_SET_BACKLIGHT;

	if (state == BACKLIGHT_ON)
	{
		if (p->brightness >= 500)
			buff[1] = ETHLCD_BACKLIGHT_ON;
		else
			buff[1] = ETHLCD_BACKLIGHT_HALF;
	}
	else
		buff[1] = ETHLCD_BACKLIGHT_OFF;

	sock_send(p->sock, buff, 2);	//send data to device
	sock_recv(p->sock, &buff, 1);	//wait for reply
}


void ethlcd_HD44780_close(PrivateData *p)
{
	sock_close(p->sock);
}
