/*
 * Serial LPT driver module for Hitachi HD44780 based LCD displays by
 * Andrew McMeikan. The LCD is operated in it's 4 bit-mode through a
 * 4094 shift register and supports a keypad.
 *
 * Copyright (c)  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  modular driver 1999 Benjamin Tse <blt@Comports.com> 
 *
 * Full connection details at http://members.xoom.com/andrewmuck/LCD.htm
 *
 * printer port	  4094/LCD
 * D2 (4)	  EN  (6 - LCD)
 * D3 (5)	  D   (2 - 4094)
 * D4 (6)	  CLK (3 - 4094)
 * +Vcc		  OE, STR (15, 1 - 4094)
 * D7 (9)	  EN2 (6 - LCD2) (optional)
 *
 * 4094		  LCD
 * Q1 (4)	  D4 (11)
 * Q2 (5)	  D5 (12)
 * Q3 (6)	  D6 (13)
 * Q4 (7)	  D7 (14)
 * Q6 (13)	  RS (4)
 * Gnd		  nRW (5)
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-serialLpt.h"
#include "port.h"

// Hardware specific functions
void lcdserLpt_HD44780_senddata(unsigned char displayID, 
				unsigned char flags, 
				unsigned char ch);

static char lcdserLpt_HD44780_getkey(void);
void rawshift(unsigned char r);
void shiftreg(unsigned char displayID, unsigned char r);

#define RS	16
#define LCDDATA 8
#define LCDCLOCK 16
#define EN1	4
#define EN2	32


static unsigned int lp;
static char lastkey=0;


// Initialisation
int hd_init_serialLpt(HD44780_functions *hd44780_functions, 
		      lcd_logical_driver *driver, 
		      char *args, 
		      unsigned int port)
{
  int displayID = EN1 | EN2;

  lp = port;

  hd44780_functions->senddata   = lcdserLpt_HD44780_senddata;

  // TODO: enable keypad only if specified on command line
  driver->getkey = lcdserLpt_HD44780_getkey;

  // setup the lcd in 4 bit mode
  shiftreg(displayID,3);
  hd44780_functions->uPause(4100);

  shiftreg(displayID,3);
  hd44780_functions->uPause(100);

  shiftreg(displayID,3);
  hd44780_functions->uPause(40);

  shiftreg(displayID,2);
  hd44780_functions->uPause(40);

  // set display type functions
  lcdserLpt_HD44780_senddata(0, 
			     RS_INSTR, 
			     FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
  // clear
  lcdserLpt_HD44780_senddata(0, RS_INSTR, CLEAR);
  hd44780_functions->uPause(1600);
    // Now turn on the display
  lcdserLpt_HD44780_senddata(displayID, 
			     RS_INSTR, 
			     ONOFFCTRL | DISPON | CURSOROFF | CURSORNOBLINK);
  hd44780_functions->uPause(1600);

  return 0;
}


void lcdserLpt_HD44780_senddata(unsigned char displayID, 
				unsigned char flags, 
				unsigned char ch)
{
  unsigned char dispID = 0, 
    portControl = 0,
    h = ch >> 4,
    l = ch & 15;

  if (displayID == 1)
    dispID |= EN1;
  else if (displayID == 2)
    dispID |= EN2;
  else
    dispID |= EN1 | EN2;

  if (flags == RS_DATA)
    portControl = RS;
  else
    portControl = 0;

  shiftreg(displayID,flags | portControl | h); 
  shiftreg(displayID,flags | portControl | l); 

  /* TODO: instead of delay read keys here */
}

char lcdserLpt_HD44780_getkey()
{
  // TODO: a keypad scan that does not use shift register
  // define a key table 
  char keytr[]="ABCDEFGH";
  int n;

  rawshift(0);			//send all line on shift register low
  hd44780_functions->uPause(1);
  if ((port_in(lp+1)&32)==0)	//test if line back is low - if not return(0)
  {	//else 
    //start walking a single zero across the eight lines 
    for(n=7;n>=0;n--)
    {
      rawshift(255 - (1<<n));
      hd44780_functions->uPause(1);
		
      if ((port_in(lp+1)&32)==0)	// check if line back is low if yes debounce 
      {
	if (lastkey==keytr[n])
	  return(0);
	//	printf("key is %c %d\n",keytr[n],n);
	lastkey=keytr[n];
	return(keytr[n]);	//return correct key code.
      }
    }
  }	//else fall out and return(0) [too transient a keypress]
  lastkey=0;
  return(0);
}

/* this function sends r out onto the shift register */
void rawshift (unsigned char r)
{
  int i;			
  for (i=7;i>=0;i--)		/* MSB first	*/
  {
    port_out(lp,((r>>i)&1)*LCDDATA);		/*set up data	*/
    port_out(lp,(((r>>i)&1)*LCDDATA)|LCDCLOCK);/*rising edge of clock	*/
  }
}

// displayID = value on parallel port to toggle the correct display
void shiftreg (unsigned char displayID, unsigned char r)
{				
  rawshift(r);
  port_out(lp,displayID);	//latch it, to correct display
  port_out(lp,0);
}
