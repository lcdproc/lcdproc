/*
 * Base driver module for Toshiba T6963 based LCD displays. ver 1.0
 *
 * Parts of this file are based on the kernel driver by Alexander Frink <Alexander.Frink@Uni-Mainz.DE> 
 *
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <asm/io.h>
#include <sys/perm.h>

#include "shared/str.h"
#include "shared/debug.h"

#include "lcd.h"
#include "t6963.h"
#include "t6963_font.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

lcd_logical_driver *t6963;

static u16 t6963_out_port;
static u16 t6963_display_mode;
static u8  *t6963_display_buffer1;
static u8  *t6963_display_buffer2;
static u8  t6963_hbar_len[6];
static u8  t6963_graph_line[6];

int
t6963_init (struct lcd_logical_driver *driver, char *args)
{
        char *argv[64];
        int argc;
        int i;
        int tmp;

	debug ("Doing initialization\n");

	t6963_out_port = 0x378;
	t6963_display_mode = 0;
	t6963_graph_line[0] = 0x20;
	t6963_graph_line[1] = 0x30;
	t6963_graph_line[2] = 0x38;
	t6963_graph_line[3] = 0x3C;
	t6963_graph_line[4] = 0x3E;
	t6963_graph_line[5] = 0x3F;

	t6963 = driver;

	debug ("Reading arguments...\n");
        argc = get_args (argv, args, 64);

        
     /*     for(i=0; i<argc; i++)
           {
           printf("Arg(%i): %s\n", i, argv[i]);
           } */
        

        for (i = 0; i < argc; i++) {
                //printf("Arg(%i): %s\n", i, argv[i]);
                if (0 == strcmp (argv[i], "-p") || 0 == strcmp (argv[i], "--port")) {
                        if (i + 1 > argc) {
                                fprintf (stderr, "T6963_init: %s requires an argument\n", argv[i]);
                                return -1;
                        }
                        t6963_out_port = atoi (argv[++i]);
               } else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
                        printf ("LCDproc Toshiba T6963 LCD driver\n" "\t-p\t--port\tSelect the output port to use [0x378]\n" "\t-t\t--type\t\tSelect the LCD type (size) [20x6]\n" "\t-h\t--help\t\tShow this help information\n");
                        return -1;
               } else {
                        printf ("Invalid parameter: %s\n", argv[i]);
                }

        }

	debug ("Getting permissions to %i...\n", t6963_out_port);

	if(ioperm(t6963_out_port, 3, 1)) {
		fprintf (stderr, "T6963_init: failed (%s)\n", strerror (errno));
                return -1;
        }

	if(ioperm(0x80, 1, 1)) {
		fprintf (stderr, "T6963_init: failed (%s)\n", strerror (errno));
                return -1;
        }

	debug ("   cool, got 'em!\nSetting width and height\n");
	
	driver->wid = 20;
	driver->hgt = 6;

	debug ("done\nAllocating memory: %i x %i bytes = %i\n", driver->wid, driver->hgt, driver->wid * driver->hgt);

	// You must use driver->framebuf here, but may use lcd.framebuf later.
	if (!driver->framebuf)
		driver->framebuf = malloc (driver->wid * driver->hgt);

	if (!driver->framebuf) {
		t6963_close ();
		return -1;
	}

	t6963_display_buffer1 = malloc (driver->wid * 6);
	t6963_display_buffer2 = malloc (driver->wid * 6);
// Debugging...
	if(t6963_display_buffer1) memset(t6963_display_buffer1, ' ', driver->wid * 6);
	if(t6963_display_buffer2) memset(t6963_display_buffer1, ' ', driver->wid * 6);

	driver->cellwid = 6;
	driver->cellhgt = 8;
	t6963->cellwid = 6;
	t6963->cellhgt = 8;

	driver->clear = t6963_clear;
	driver->string = t6963_string;
	driver->chr = t6963_chr;
	driver->vbar = t6963_vbar;
	driver->init_vbar = NULL;
	driver->hbar = t6963_hbar;
	driver->init_hbar = NULL;
	driver->num = t6963_num;
	driver->init_num = t6963_init_num;

	driver->init = t6963_init;
	driver->close = t6963_close;
	driver->flush = t6963_flush;
	driver->flush_box = NULL;
	driver->contrast = t6963_contrast;
	driver->backlight = t6963_backlight;
	driver->set_char = t6963_set_char;
	driver->icon = t6963_icon;
	driver->draw_frame = t6963_draw_frame;

	driver->getkey = t6963_getkey;

	T6963_CEHI;  // disable chip
	T6963_RDHI;  // disable reading from LCD
	T6963_WRHI;  // disable writing to LCD
	T6963_CDHI;  // command/status mode
	T6963_DATAOUT; // make 8-bit parallel port an output port

        t6963_low_command_word (SET_GRAPHIC_HOME_ADDRESS, ATTRIB_BASE);
        t6963_low_command_word (SET_GRAPHIC_AREA,         driver->wid);
        t6963_low_command_word (SET_TEXT_HOME_ADDRESS,    TEXT_BASE);
        t6963_low_command_word (SET_TEXT_AREA,            driver->wid);

        t6963_low_command         (SET_MODE | OR_MODE | EXTERNAL_CG);
        t6963_low_command_2_bytes (SET_OFFSET_REGISTER, CHARGEN_BASE>>11, 0);
        t6963_low_command         (SET_CURSOR_PATTERN | 7); // cursor is 8 lines high
        t6963_low_command_2_bytes (SET_CURSOR_POINTER, 0, 0);

	t6963_set_nchar (0, fontdata_6x8, 255);

	t6963_low_enable_mode (TEXT_ON);
	t6963_low_enable_mode (GRAPHIC_ON);
	t6963_low_disable_mode (CURSOR_ON);
	t6963_low_disable_mode (BLINK_ON);

	t6963_clear ();
	t6963_graphic_clear (0, 0, driver->wid, driver->cellhgt * 6);
	t6963_flush();
	debug ("Initialization done!\n");

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

// Below here, you may use either lcd.framebuf or driver->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

void
t6963_close ()
{
	debug ("Shutting down!\n");
	t6963_low_disable_mode (BLINK_ON);

	ioperm(t6963_out_port, 3, 0);
	if (t6963->framebuf != NULL)
		free (t6963->framebuf);
	if (t6963_display_buffer1 != NULL) free (t6963_display_buffer1);
	if (t6963_display_buffer2 != NULL) free (t6963_display_buffer2);

	t6963->framebuf = NULL;
	t6963_display_buffer1 = NULL;
	t6963_display_buffer2 = NULL;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
t6963_clear ()
{
	int i;

	debug ("Clearing Display of size %i x %i\n", t6963->wid, 6);

	memset (t6963_display_buffer1, ' ', t6963->wid * 6);
//	for (i=0; i < 6; i++)
//		if (t6963_hbar_len[i]==0) t6963_graphic_clear(0, i*t6963->cellhgt, t6963->wid, (i+1)*t6963->cellhgt);

	debug ("Done\n");
}

void
t6963_graphic_clear (int x1, int y1, int x2, int y2)
{
	int x; 

	debug ("Clearing Graphic %i bytes\n", (x2-x1)*(y2-y1));

	for (;y1 < y2; y1++)
	{
		t6963_low_command_word(SET_ADDRESS_POINTER, ATTRIB_BASE + y1 * t6963->wid + x1);
		for (x = x1; x < x2; x++)
			t6963_low_command_byte(DATA_WRITE_INC, 0);
	}
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
t6963_flush ()
{
	int i;
	debug ("Flushing %i x %i\n", t6963->wid, t6963->hgt);

	for (i = 0; i < (t6963->wid * 6); i++)
	{
		debug ("%c%c|", t6963_display_buffer1[i], t6963_display_buffer2[i]);
		if (t6963_display_buffer1[i] != t6963_display_buffer2[i])
		{
			t6963_low_command_word(SET_ADDRESS_POINTER, TEXT_BASE + i);
			t6963_low_command_byte(DATA_WRITE, t6963_display_buffer1[i]);
		}
	}
	debug ("\n");
	//t6963->draw_frame(t6963->framebuf);
	t6963_swap_buffers();
	t6963_clear();
}

//////////////////////////////////////////////////////////////////////
// Send a rectangular area to the display.
//
// I've just called drv_base_flush() because there's not much point yet
// in flushing less than the entire framebuffer.
//
void
t6963_flush_box (int lft, int top, int rgt, int bot)
{
	debug ("flush_box\n");
	t6963_flush();
	//drv_base_flush();

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,6).
//
void
t6963_string (int x, int y, char string[])
{
	int i;

	debug ("String out\n");

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

//	t6963_low_command_word(SET_ADDRESS_POINTER,TEXT_BASE+POSITION(x,y));
	if(y * t6963->wid + x + strlen(string) <= t6963->wid * 6);
		memcpy(&t6963_display_buffer1[y * t6963->wid + x], string, strlen(string));

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
t6963_chr (int x, int y, char c)
{
	debug ("Char out\n");
	y--;
	x--;
	if ((y * t6963->wid) + x <= (t6963->wid * 6))
		t6963_display_buffer1[(y * t6963->wid) + x] = c;
}

//////////////////////////////////////////////////////////////////////
// Sets the contrast of the display.  Value is 0-255, where 140 is
// what I consider "just right".
//
int
t6963_contrast (int contrast)
{
//  printf("Contrast: %i\n", contrast);
	return -1;
}

//////////////////////////////////////////////////////////////////////
// Turns the lcd backlight on or off...
//
void
t6963_backlight (int on)
{
	/*
	   if(on)
	   {
	   printf("Backlight ON\n");
	   }
	   else
	   {
	   printf("Backlight OFF\n");
	   }
	 */
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for big numbers, if possible.
//
void
t6963_init_num ()
{
//  printf("Big Numbers.\n");
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
void
t6963_num (int x, int num)
{
//  printf("BigNum(%i, %i)\n", x, num);
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
void
t6963_set_nchar (int n, char *dat, int num)
{
	int row, col;
	char tmp;
	char letter;

	debug ("Setting char %i", n);

        if (!dat || n+num > 256)
                return;

	t6963_low_command_word(SET_ADDRESS_POINTER, CHARGEN_BASE + n*8);
        for (row = 0; row < t6963->cellhgt * num; row++) {
                letter = 0;
                for (col = 0; col < t6963->cellwid; col++) {
                        letter <<= 1;
                        letter |= (dat[(row * t6963->cellwid) + col] > 0);
                }

		t6963_low_command_byte(DATA_WRITE_INC, letter);
        }
}

void
t6963_set_char (int n, char *dat)
{
	t6963_set_nchar (n, dat, 1);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
void
t6963_vbar (int x, int len)
{
	int y;

	for (y = 0; y < len/t6963->cellhgt; y++)
		t6963_chr (x, 6-y, 219);

	if (len % t6963->cellhgt) 
		t6963_chr (x, 6-y, 211 + (len % t6963->cellhgt));

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
t6963_hbar (int x, int y, int len)
{
	int stop = x + len/t6963->cellwid;
	for (; x <= stop; x++)
		t6963_chr (x, y, 219);

	if (len % t6963->cellwid)
		t6963_chr (x, y, 225 - (len % t6963->cellwid));


}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
t6963_icon (int which, char dest)
{
//  printf("Char %i set to icon %i\n", dest, which);
}

void
t6963_draw_frame (char *dat)
{
	int x, y;

	if(!dat) return;

	for(x=1; x<=t6963->wid; x++)
	{
   		t6963_chr (x, 1, '-');
   		t6963_chr (x, 6, '-');
	}
  
	for(y=0; y<t6963->hgt; y++)
	{
		for(x=0; x<t6963->wid; x++)
			t6963_chr(x+1, y+1, dat[x+(y*t6963->wid)]);
	}
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
t6963_getkey ()
{
	return 0;
}

void
t6963_low_data (u8 byte)
{
 //   lcd_wait_until_ready();

    T6963_CDLO;
    T6963_WRLO;                       // activate LCD's write mode
    outb_p(byte, T6963_DATA_PORT);     // write value to data port
    T6963_CELO;                       // pulse enable LOW > 80 ns (hah!)
    T6963_CEHI;                       // return enable HIGH
    T6963_WRHI;                       // restore Write mode to inactive
}

void
t6963_low_command (u8 byte)
{
  //  lcd_wait_until_ready();

    outb_p(byte, T6963_DATA_PORT);  // present data to LCD on PC's port pins

    T6963_CDHI;                      // control/status mode
    T6963_RDHI;                      // make sure LCD read mode is off
    T6963_WRLO;                      // activate LCD write mode
    T6963_CELO;                      // pulse ChipEnable LOW, > 80 ns, enables LCD I/O
    T6963_CEHI;                      // disable LCD I/O
    T6963_WRHI;                      // deactivate write mode
}

void
t6963_low_command_byte(u8 cmd, u8 byte)
{
    t6963_low_data(byte);
    t6963_low_command(cmd);
}

void
t6963_low_command_2_bytes(u8 cmd, u8 byte1, u8 byte2)
{
    t6963_low_data(byte1);
    t6963_low_data(byte2);
    t6963_low_command(cmd);
}

void
t6963_low_command_word(u8 cmd, u16 word)
{
    t6963_low_data(word%256);
    t6963_low_data(word>>8);
    t6963_low_command(cmd);
}

void
t6963_low_enable_mode (u8 mode)
{
	t6963_display_mode |= mode;
	t6963_low_command(SET_DISPLAY_MODE | t6963_display_mode);
}

void
t6963_low_disable_mode (u8 mode)
{
	t6963_display_mode &= ~mode;
	t6963_low_command(SET_DISPLAY_MODE | t6963_display_mode);
}

void
t6963_swap_buffers ()
{
	u8 *tmp_buffer;
	tmp_buffer = t6963_display_buffer1;
	t6963_display_buffer1 = t6963_display_buffer2;
	t6963_display_buffer2 = tmp_buffer;
	tmp_buffer = NULL;
}
