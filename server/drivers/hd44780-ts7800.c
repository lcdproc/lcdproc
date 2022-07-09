/** \file server/drivers/hd44780-ts7800.c
 * \c 8bit connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 8 bit-mode to be connected to LCD header on TS7800 ARM SBC board.
 */
/* Copyright (c)  1999, 1995 Benjamin Tse <blt@Comports.com>
 *                2001 Joris Robijn <joris@robijn.net>
 *                2012,2022 Jozsef Czompo <czo@czo.hu>
 *
 *
 * Created modular driver Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Based on the code in the lcdtime package which uses the LCD
 * controller in its 8-bit mode.
 *
 * Based on hd44879-8bit driver code by Benjamin Tse, Joris Robijn
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-ts7800.h"
#include "hd44780-low.h"
#include "report.h"

#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>

#define DIOBASE       0xE8000000
#define RW_MASK       0x00200000
#define ENABLE_MASK   0x00100000
#define BASELINE_MASK 0x00080000
#define RS_MASK       0x00040000
#define BUSY_MASK     0x20000000
#define RW_REG(ptr) *(ptr + 0x08/sizeof(unsigned int))

volatile unsigned int *lcd;
#define MIRROR_PINS

inline int getLCDbit(int n) {
#ifdef MIRROR_PINS
  switch (n) {
  case 3: n =4 ; break;
  case 4:  n =3 ; break;
  case 5: n =6 ; break;
  case 6: n =5 ; break;
  case 7: n =8 ; break;
  case 8: n =7 ; break;
  case 9: n =10 ; break;
  case 10: n =9 ; break;
  case 11: n =12 ; break;
  case 12: n =11 ; break;
  case 13: n =14 ; break;
  case 14: n =13 ; break;
 }
#endif
  return (*lcd >> (16+n-1)) & 1;
}

//inline
void putLCDbit(int n,int val) {
   volatile int i;
   unsigned int tmp;

#ifdef MIRROR_PINS
  switch (n) {
  case 3: n =4 ; break;
  case 4:  n =3 ; break;
  case 5: n =6 ; break;
  case 6: n =5 ; break;
  case 7: n =8 ; break;
  case 8: n =7 ; break;
  case 9: n =10 ; break;
  case 10: n =9 ; break;
  case 11: n =12 ; break;
  case 12: n =11 ; break;
  case 13: n =14 ; break;
  case 14: n =13 ; break;
 }
#endif
  if (val) {
    tmp = *(lcd+1);
    tmp |= (1 << (16+n-1));
    *(lcd+1) = tmp;
  } else {
    tmp = *(lcd+1);
    tmp &= ~(1 << (16+n-1));
    *(lcd+1) = tmp;
  }
  i = *(lcd+1); // force bus cycle to flush write now
  if (i != tmp) printf("no lcd?"); // don't really need this
}

inline void lcd_instr(int i) {
  putLCDbit(4,!i);
}

inline void lcd_data_out(int data) {
  int i;
  for (i=0;i<8;i++) {
    putLCDbit(i+7,data & 1);
    data >>= 1;
  }
}

inline void lcd_write(int w) {
  if (!w) {
    lcd_data_out(0xFF);
  }
  putLCDbit(5,!w);
}


inline void lcd_enable(int e) {
  putLCDbit(6,e);
}

inline void lcd_cmd(int cmd) {
  lcd_write(1);
  lcd_data_out(cmd);
  lcd_instr(1);

}

inline void lcd_data(int data) {
  lcd_data_out(data);
  lcd_instr(0);
  lcd_write(1);
}

void ts7800_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_ts7800(Driver *drvthis)
{
  volatile unsigned int *dioptr;
  int fd = open("/dev/mem", O_RDWR|O_SYNC);

	dioptr = (unsigned int *)mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, DIOBASE);
	RW_REG(dioptr) &= ~BASELINE_MASK; /* baseline should always be zero */
	lcd = dioptr + 1;


	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;
	hd44780_functions->senddata = ts7800_HD44780_senddata;

	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 4100);
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT);
	hd44780_functions->uPause(p, 100);
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_8BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause(p, 40);
	common_init (p, IF_8BIT);
	return 0;
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void ts7800_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	if ( flags == RS_INSTR ) lcd_cmd(ch);
	if ( flags == RS_DATA ) lcd_data(ch);

	lcd_enable(1);
	if (p->delayBus) p->hd44780_functions->uPause(p, 1);
	lcd_enable(0);
}

