/* Driver module for Hitachi HD44780 based Optrex DMC-20481 LCD display 
 * The module is operated in it's 4 bit-mode to be connected to a single
 * 8 bit-port
 *
 * Copyright (c) 1998 Richard Rognlie       GNU Public License  
 *                    <rrognlie@gamerz.net>
 *
 * Large quantities of this code lifted (nearly verbatim) from
 * the lcd4.c module of lcdtext.  Copyright (C) 1997 Matthias Prinke
 * <m.prinke@trashcan.mcnet.de> and covered by GNU's GPL.
 * In particular, this program is free software and comes WITHOUT
 * ANY WARRANTY.
 *
 * Matthias stole (er, adapted) the code from the package lcdtime by
 * Benjamin Tse (blt@mundil.cs.mu.oz.au), August/October 1995
 * which uses the LCD-controller's 8 bit-mode.
 * References: port.h             by <damianf@wpi.edu>
 *             Data Sheet LTN211, Philips
 *             Various FAQs and TXTs about Hitachi's LCD Controller HD44780 -
 *                www.paranoia.com/~filipg is a good starting point  ???   
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/perm.h>

#include "../../shared/str.h"

#include "lcd.h"
#include "hd44780.h"
#include "drv_base.h"


lcd_logical_driver *HD44780;

#define EN 64
#define RW 32
#define RS 16

#ifndef LPTPORT
unsigned int port = 0x378;
#else
unsigned int port = LPTPORT;
#endif

static int lcd_x=0;
static int lcd_y=0;

static void HD44780_linewrap(int on);
static void HD44780_autoscroll(int on);

static int lp;

static void HD44780_senddata(unsigned char flags, unsigned char ch)
{
	unsigned char h = ch >> 4;
	unsigned char l = ch & 15;

	port_out(lp, 128 | EN | flags | h);	usleep(1); port_out(lp, 128 | flags | h);
	port_out(lp, 128 | EN | flags | l);	usleep(1); port_out(lp, 128 | flags | l);
}

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int HD44780_init(lcd_logical_driver *driver, char *args) 
{
   char *argv[64];
   int argc;
   int i;
   
   
   HD44780 = driver;
   

   argc = get_args(argv, args, 64);

   for(i=0; i<argc; i++)
   {
      //printf("Arg(%i): %s\n", i, argv[i]);
      if(0 == strcmp(argv[i], "-p")  ||
	 0 == strcmp(argv[i], "--port"))
      {
	 if(i + 1 > argc) {
	    fprintf(stderr, "HD44780_init: %s requires an argument\n",
		    argv[i]);
	    return -1;
	 }
	 printf("Sorry, runtime lpt-port switching not yet implemented...\n");
	 //strcpy(device, argv[++i]);
      }
      else if(0 == strcmp(argv[i], "-c")  ||
	 0 == strcmp(argv[i], "--contrast"))
      {
	 if(i + 1 > argc) {
	    fprintf(stderr, "HD44780_init: %s requires an argument\n",
		    argv[i]);
	    return -1;
	 }
	 printf("Sorry, contrast changing not yet implemented...\n");
	 //contrast = atoi(argv[++i]);
      }
      else if(0 == strcmp(argv[i], "-h")  ||
	 0 == strcmp(argv[i], "--help"))
      {
	 printf("LCDproc HD44780 driver\n"
		"\t-p\t--port\tSelect the output device to use [0x%x]\n"
		"\t-c\t--contrast\tSet the initial contrast [default]\n"
		"\t-h\t--help\t\tShow this help information\n",
		port);
	 return -1;
      }
      else
      {
	 printf("Invalid parameter: %s\n", argv[i]);
      }
      
   }
   
   
   
   // Set up io port correctly, and open it...
	if ((ioperm(port,1,1)) == -1) {
		fprintf(stderr, "HD44780_init: failed (%s)\n", strerror(errno)); 
		return -1;
	}

	lp = port;

	// init HD44780
	port_out(lp, 128 | EN | 3);	usleep(1); port_out(lp, 128 | 3);		usleep(5000);
	port_out(lp, 128 | EN | 3);	usleep(1); port_out(lp, 128 | 3);		usleep(150);
	port_out(lp, 128 | EN | 3);	usleep(1); port_out(lp, 128 | 3);
	// now in 8-bit mode...  set 4-bit mode
	port_out(lp, 128 | EN | 2);	usleep(1); port_out(lp, 128 | 2);
	// now in 4-bit mode...   set 2 line, small char mode
	HD44780_senddata(0, 32 | 8);

	//clear
	HD44780_senddata(0, 1);
	// set lcd on (4), cursor_on (2), and cursor_blink(1)
	HD44780_senddata(0, 8 | 4 | 0 | 0);

	// Set display-specific stuff..
	//HD44780_linewrap(1);
	//HD44780_autoscroll(1);

	// Make sure the frame buffer is there...  
	if (!HD44780->framebuf) 
		HD44780->framebuf = (unsigned char *) malloc(lcd.wid * lcd.hgt);

	if (!HD44780->framebuf) {
	   //HD44780_close();
		return -1;
	}


	// Set the functions the driver supports...

	driver->clear =      (void *)-1;
	driver->string =     (void *)-1;
	driver->chr =        (void *)-1;
	driver->vbar =       HD44780_vbar;
	driver->init_vbar =  HD44780_init_vbar;
	driver->hbar =       HD44780_hbar;
	driver->init_hbar =  HD44780_init_hbar;
	driver->num =        HD44780_num;
	driver->init_num =   HD44780_init_num;
	
	driver->init =       HD44780_init;
	driver->close =      (void *)-1;
	driver->flush =      HD44780_flush;
	driver->flush_box =  HD44780_flush_box;
	driver->contrast =   HD44780_contrast;
	driver->backlight =  HD44780_backlight;
	driver->set_char =   HD44780_set_char;
	driver->icon =       HD44780_icon;
	driver->draw_frame = HD44780_draw_frame;

	driver->getkey =     NULL;
	
	return lp;
}



/////////////////////////////////////////////////////////////////
// Clean-up
//
/*
void HD44780_close() 
{
  drv_base_close();
}
*/

static void HD44780_position(int x, int y)
{
	int val = x + (y%2) * 0x40;
	if (y>=2) val += 20;

	HD44780_senddata(0,128|val);

	lcd_x = x;
	lcd_y = y;
}

void HD44780_flush()
{
  HD44780_draw_frame(lcd.framebuf);
}


void HD44780_flush_box(int lft, int top, int rgt, int bot)
{
	int x,y;
  
//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

	for (y=top; y<=bot; y++) {
		HD44780_position(lft,y);

		for (x=lft ; x<=rgt ; x++) {
			HD44780_senddata(RS,lcd.framebuf[(y*20)+x]);
		}
		//write(fd, lcd.framebuf[(y*20)+lft, rgt-lft+1]);
	}

}


/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
//
int HD44780_contrast(int contrast) 
{
  return 0;
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
void HD44780_backlight(int on)
{
}


/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void HD44780_linewrap(int on)
{
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void HD44780_autoscroll(int on)
{
	HD44780_senddata(0,4 | on * 2);
}


/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//
void HD44780_init_vbar() 
{
  char a[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
  };
  char b[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char c[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char d[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char e[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char f[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char g[] = {
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };

  HD44780_set_char(1,a);
  HD44780_set_char(2,b);
  HD44780_set_char(3,c);
  HD44780_set_char(4,d);
  HD44780_set_char(5,e);
  HD44780_set_char(6,f);
  HD44780_set_char(7,g);
  
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void HD44780_init_hbar() 
{

  char a[] = {
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
  };
  char b[] = {
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
  };
  char c[] = {
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
  };
  char d[] = {
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
  };

  HD44780_set_char(1,a);
  HD44780_set_char(2,b);
  HD44780_set_char(3,c);
  HD44780_set_char(4,d);

}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
void HD44780_vbar(int x, int len) 
{
	char map[9] = {32, 1, 2, 3, 4, 5, 6, 7, 255 };

	int y;
	for(y=lcd.hgt; y > 0 && len>0; y--) {
		if (len >= lcd.cellhgt)
			drv_base_chr(x, y, 255);
		else
			drv_base_chr(x, y, map[len]);

		len -= lcd.cellhgt;
	}
  
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void HD44780_hbar(int x, int y, int len) 
{
	char map[6] = { 32, 1, 2, 3, 4, 255  };

	for (; x<=lcd.wid && len>0; x++) {
		if (len >= lcd.cellwid)
			drv_base_chr(x,y,255);
		else
			drv_base_chr(x, y, map[len]);

		len -= lcd.cellwid;

	}
}


/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void HD44780_init_num() 
{
	char out[3];
	sprintf(out, "%cn", 254);
	//write(fd, out, 2);
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void HD44780_num(int x, int num) 
{
  char out[5];
  sprintf(out, "%c#%c%c", 254, x, num);
  //write(fd, out, 4);
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void HD44780_set_char(int n, char *dat)
{
	int row, col;
	int letter;

	if(n < 0 || n > 7) return;
	if(!dat) return;

	HD44780_senddata(0, 64 | n*8);

	for(row=0; row<lcd.cellhgt; row++) {
		letter = 0;
		for(col=0; col<lcd.cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row*lcd.cellwid) + col] > 0);
		}
		HD44780_senddata(RS, letter);
	}

}


void HD44780_icon(int which, char dest)
{
  char icons[3][5*8] = {
   {
     1,1,1,1,1,  // Empty Heart
     1,0,1,0,1,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,0,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },   

   {
     1,1,1,1,1,  // Filled Heart
     1,0,1,0,1,
     0,1,0,1,0,
     0,1,1,1,0,
     0,1,1,1,0,
     1,0,1,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },
   
   {
     0,0,0,0,0,  // Ellipsis
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,1,0,1,
   },
   
  };

   
  HD44780_set_char(dest, &icons[which][0]);
}


/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized lcd.wid*lcd.hgt
//
void HD44780_draw_frame(char *dat)
{
	int x,y;

	if (!dat) return;

	for (y=0; y<=3; y++) {
		HD44780_position(0,y);

		for (x=0 ; x<=19 ; x++) {
			HD44780_senddata(RS,dat[(y*lcd.wid)+x]);
		}
	}

}

