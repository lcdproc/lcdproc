/*
 * Base driver module for Toshiba T6963 based LCD displays. ver 2.2
 *
 * Parts of this file are based on the kernel driver by Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Manuel Stahl <mythos@xmythos.de>
 *
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
//#include <asm/io.h>
//#include <sys/perm.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "t6963.h"
#include "t6963_font.h"

#include "shared/debug.h"
#include "report.h"
#include "lcd_lib.h"
#include "port.h"


#define DEBUG3 if(debug_level > 2) printf
#define DEBUG4 if(debug_level > 3) printf

#define debug_level 2

//extern int debug_level;

static u16 t6963_out_port;
static u16 t6963_display_mode;
static u8  *t6963_display_buffer1;
static u8  *t6963_display_buffer2;
static u8  t6963_graph_line[6];

static char *framebuf = NULL;
static int width;
static int height;
static int cellwidth;
static int cellheight;
static char bidirectLPT;
static char graphicON;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "t6963_";

MODULE_EXPORT int
t6963_init (Driver *drvthis, char *args)
{
	int w, h, p, i, ecp_input;

	char size[200] = DEFAULT_SIZE;

		debug(RPT_INFO, "T6963: init(%p,%s)", drvthis, args );

	t6963_display_mode = 0;
	t6963_graph_line[0] = 0x20;
	t6963_graph_line[1] = 0x30;
	t6963_graph_line[2] = 0x38;
	t6963_graph_line[3] = 0x3C;
	t6963_graph_line[4] = 0x3E;
	t6963_graph_line[5] = 0x3F;

		debug(RPT_DEBUG, "T6963: reading config file...");

   /* Read config file */

		/* -------------------------- Which size --------------------------------------*/
	strncpy(size, drvthis->config_get_string ( drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size, "%dx%d", &w, &h ) != 2
	  || (w <= 0) || (w > LCD_MAX_WIDTH)
	  || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
	  	report (RPT_WARNING, "T6963_init: Cannot read size: %s, Using default value.\n", size);
		sscanf( DEFAULT_SIZE, "%dx%d", &w, &h );
	} else {
		width = w;
		height = h;
	}

		/* --------------------------- Which port --------------------------------------*/
	p = drvthis->config_get_int ( drvthis->name, "Port", 0, DEFAULT_PORT);
	if(0x200 <= p && p <= 0x400) {
		t6963_out_port = p;
	} else {
		t6963_out_port = DEFAULT_PORT;
		report (RPT_WARNING, "T6963_init: Port value must be between 0x200 and 0x400. Using default value.\n");
	}
                /* ---------------------------- Is ECP mode on ----------------------------------*/
	bidirectLPT = drvthis->config_get_bool ( drvthis->name, "ECPlpt", 0, 0 );
                /* ---------------------------- Use graphic -------------------------------------*/
         graphicON = drvthis->config_get_bool ( drvthis->name, "graphic", 0, 0 );


  /* -- Get permission to parallel port --------------------------------------------*/

		debug (RPT_DEBUG, "T6963: Getting permission to parallel port %d...", t6963_out_port);

        if( port_access_full(t6963_out_port) ) {   //ioperm(t6963_out_port, 3, 1)) {
		report (RPT_ERR, "T6963_init: no permission to port %d: (%s)\n", t6963_out_port, strerror (errno));
               	return -1;
        }

        if( port_access(0x80) ) {	//ioperm(0x80, 1, 1)) {
	       report (RPT_ERR, "T6963_init: no permission to port 0x80: (%s)\n", strerror (errno));
                return -1;
        }
		debug (RPT_DEBUG, "T6963:   cool, got 'em!");


  /* -- Set charakter size --*/

	cellwidth = 6;
	cellheight = 8;

  /* -- Allocate framebuffer --*/

		debug (RPT_DEBUG, "T6963: Allocate framebuffer...");
	framebuf = malloc (width * height);

	if (!framebuf) {
		report (RPT_ERR, "T6963_init: No memory for framebuffer!");
		t6963_close (drvthis);
		return -1;
	}
		debug (RPT_DEBUG, "T6963:    done!");


  /* -- Allocate memory for double buffering --*/

		debug (RPT_DEBUG, "T6963: Allocating double buffering...");
	t6963_display_buffer1 = malloc (width * height);
	t6963_display_buffer2 = malloc (width * height);
   /* - Clear front and back buffer -*/
	if(t6963_display_buffer1 && t6963_display_buffer2) {
		memset(t6963_display_buffer1, ' ', width * height);
		memset(t6963_display_buffer1, ' ', width * height);
		debug (RPT_DEBUG, "T6963:     done!");
	} else {
		report (RPT_ERR, "T6963: No memory for double buffering!");
		t6963_close (drvthis);
		return -1;
	}

  /* ------------------- I N I T I A L I Z A T I O N ----------------------- */
		debug (RPT_DEBUG, "T6963: Sending init to display...");

        t6963_low_set_control(1, 1, 1, 1);
        T6963_DATAOUT; // make 8-bit parallel port an output port

    /* - Test ECP mode -*/
        if(bidirectLPT == 1) {
        		debug (RPT_DEBUG, "T6963: Testing ECP mode...");
             i=0; ecp_input=0;
	    T6963_DATAIN;
	    do {
	    	i++;
		t6963_low_set_control(1, 0, 1, -1);
		t6963_low_set_control(1, 0, 1, 0);
		ecp_input = port_in(T6963_DATA_PORT);
		t6963_low_set_control(1, 0, 1, -1);
	    } while (i < 50000 && (ecp_input & 3)!=3);
	    T6963_DATAOUT;
             if (i >= 50000) debug(RPT_WARNING, "T6963: ECP mode not working!\n -> is now disabled\n");
             else debug(RPT_DEBUG, "T6963: working!");
        }

		debug (RPT_DEBUG, "T6963:  set graphic/text home adress and area");

        t6963_low_command_word (SET_GRAPHIC_HOME_ADDRESS, ATTRIB_BASE);
        t6963_low_command_word (SET_GRAPHIC_AREA,         width);
        t6963_low_command_word (SET_TEXT_HOME_ADDRESS,    TEXT_BASE);
        t6963_low_command_word (SET_TEXT_AREA,            width);

        t6963_low_command         (SET_MODE | OR_MODE | EXTERNAL_CG);
        t6963_low_command_2_bytes (SET_OFFSET_REGISTER, CHARGEN_BASE>>11, 0);
        t6963_low_command         (SET_CURSOR_PATTERN | 7); // cursor is 8 lines high
        t6963_low_command_2_bytes (SET_CURSOR_POINTER, 0, 0);

        t6963_set_nchar (drvthis, 0, fontdata_6x8, 256);

        t6963_low_enable_mode (TEXT_ON);
        if (graphicON == 0) t6963_low_disable_mode (GRAPHIC_ON);
        else		   t6963_low_enable_mode (GRAPHIC_ON);
        t6963_low_disable_mode (CURSOR_ON);
        t6963_low_disable_mode (BLINK_ON);

        t6963_clear (drvthis);
        t6963_graphic_clear (drvthis, 0, 0, width, cellheight * height);
        t6963_flush(drvthis);

		debug (RPT_DEBUG, "T6963: Initialization done!");

	return 0;						  // 200 is arbitrary.  (must be 1 or more)
}

// Below here, you may use either lcd.framebuf or driver->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

MODULE_EXPORT void
t6963_close (Driver *drvthis)
{
	debug (RPT_INFO, "Shutting down!\n");
	t6963_low_disable_mode (BLINK_ON);

	port_deny_full(t6963_out_port);

	if (framebuf != NULL)
		free (framebuf);
	if (t6963_display_buffer1 != NULL) free (t6963_display_buffer1);
	if (t6963_display_buffer2 != NULL) free (t6963_display_buffer2);

	framebuf = NULL;
	t6963_display_buffer1 = NULL;
	t6963_display_buffer2 = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
t6963_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
t6963_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
t6963_clear (Driver *drvthis)
{
		debug (RPT_DEBUG, "Clearing Display of size %i x %i\n", width, 6);
	memset (t6963_display_buffer1, ' ', width * 6);
		debug (RPT_DEBUG, "Done\n");
}

void
t6963_graphic_clear (Driver *drvthis, int x1, int y1, int x2, int y2)
{
	int x;
		debug (RPT_DEBUG, "Clearing Graphic %i bytes\n", (x2-x1)*(y2-y1));
	for (;y1 < y2; y1++)
	{
		t6963_low_command_word(SET_ADDRESS_POINTER, ATTRIB_BASE + y1 * width + x1);
		for (x = x1; x < x2; x++)
			t6963_low_command_byte(DATA_WRITE_INC, 0);
	}
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
t6963_flush (Driver *drvthis)
{
	int i;
	debug (RPT_DEBUG, "Flushing %i x %i\n", width, height);

	for (i = 0; i < (width * height); i++)
	{
		debug (RPT_DEBUG, "%i%i|", t6963_display_buffer1[i], t6963_display_buffer2[i]);
		if (t6963_display_buffer1[i] != t6963_display_buffer2[i] || bidirectLPT == 0)
		{
			t6963_low_command_word(SET_ADDRESS_POINTER, TEXT_BASE + i);
			t6963_low_command_byte(DATA_WRITE, t6963_display_buffer1[i]);
		}
	}
	debug (RPT_DEBUG, "Done\n");
	t6963_swap_buffers();
	t6963_clear(drvthis);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,6).
//
MODULE_EXPORT void
t6963_string (Driver *drvthis, int x, int y, char string[])
{
	debug (RPT_DEBUG, "String out\n");

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

//	t6963_low_command_word(SET_ADDRESS_POINTER,TEXT_BASE+POSITION(x,y));
	if(y * width + x + strlen(string) <= width * 6);
		memcpy(&t6963_display_buffer1[y * width + x], string, strlen(string));

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,6).
//
MODULE_EXPORT void
t6963_chr (Driver *drvthis, int x, int y, char c)
{
	debug (RPT_DEBUG, "Char out\n");
	y--;
	x--;
	if ((y * width) + x <= (width * 6))
		t6963_display_buffer1[(y * width) + x] = c;
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
MODULE_EXPORT void
t6963_num (Driver *drvthis, int x, int num)
{
//  printf("BigNum(%i, %i)\n", x, num);
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
void
t6963_set_nchar (Driver *drvthis, int n, char *dat, int num)
{
	int row, col;
	char letter;

	debug (RPT_DEBUG, "Setting char %i", n);

        if (!dat || n+num > 256)
                return;

	t6963_low_command_word(SET_ADDRESS_POINTER, CHARGEN_BASE + n*8);
        for (row = 0; row < cellheight * num; row++) {
                letter = 0;
                for (col = 0; col < cellwidth; col++) {
                        letter <<= 1;
                        letter |= (dat[(row * cellwidth) + col] > 0);
                }

		t6963_low_command_byte(DATA_WRITE_INC, letter);
        }
}

MODULE_EXPORT void
t6963_set_char (Driver *drvthis, int n, char *dat)
{
	t6963_set_nchar (drvthis, n, dat, 1);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
t6963_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 212);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
t6963_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 220);
}

/////////////////////////////////////////////////////////////////
// Sets an icon...
//
MODULE_EXPORT int
t6963_icon (Driver *drvthis, int x, int y, int icon)
{
	debug (RPT_DEBUG, "T6963: set icon %d", icon);
	switch( icon ) {
		case ICON_BLOCK_FILLED:
			t6963_chr( drvthis, x, y, 219 );
			break;
		case ICON_HEART_FILLED:
			t6963_chr( drvthis, x, y, 3 );
			break;
		case ICON_HEART_OPEN:
			t6963_chr( drvthis, x, y, 4 );
			break;
		default:
			return -1;
	}
	return 0;
}

/* ---------------------- internal functions ------------------------------------- */
void
t6963_low_set_control(char wr, char ce, char cd, char rd)
{
	unsigned char status = port_in(T6963_CONTROL_PORT); /* TODO: support multiple wirings! */
	if(wr == 1)  /* WR = HI */
		status &= 0xfe;
	else if(wr == 0)
		status |= 0x01;
	if(ce == 1)  /* CE = HI */
		status &= 0xfd;
	else if(ce == 0)
		status |= 0x02;
	if(cd == 0)  /* CD = HI */
		status &= 0xfb;
	else if(cd == 1)
		status |= 0x04;
	if(rd == 1)  /* CE = HI */
		status &= 0xf7;
	else if(rd == 0)
		status |= 0x08;
	port_out(T6963_CONTROL_PORT, status);

}

void
t6963_low_dsp_ready (void)
{
    int i = 0;
    int input;

    if(bidirectLPT == 1) {
	    T6963_DATAIN;
	    do {
	    	i++;
		t6963_low_set_control(1, 0, 1, -1);
		t6963_low_set_control(1, 0, 1, 0);
		input = port_in(T6963_DATA_PORT);
		t6963_low_set_control(1, 0, 1, -1);
	    } while (i < 50000 && (input & 3)!=3);
	    T6963_DATAOUT;
    } else {
    	for(i=0; i<3; i++)
		port_out(0x80, 0x00);  /* wait 1ms */
    }
}

void
t6963_low_data (u8 byte)
{
    t6963_low_dsp_ready();

    port_out(T6963_DATA_PORT, byte);     // write value to data port
    t6963_low_set_control(1, 1,  0, 1);
    t6963_low_set_control(0, 0,  0, 1);
    t6963_low_set_control(1, 1,  1, 1);
}

void
t6963_low_command (u8 byte)
{
    t6963_low_dsp_ready();

    port_out(T6963_DATA_PORT, byte);  // present data to LCD on PC's port pins

    t6963_low_set_control(1, 1, 1, 1);
    t6963_low_set_control(0, 0, 1, 1);
    t6963_low_set_control(1, 1, 0, 1);
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