/* 	This is the LCDproc driver for various serial VFD Devices

	Copyright (C) 2006 Stefan Herdler

	This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
	driver.
	It may contain parts of other drivers of this package too.

	2006-02-13 Version 0.2: everything should work (not all hardware tested!)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307



	The serialVFD-driver should work with all NEC FIPC8367 based VFDs:
	NEC FC20X2JA
	NEC FM20X2KB-AB
	NEC FC20X1SA-AB/AA
	...
	Tested only on: NEC FM20X2KB-AB
	The driver also works (tested) on "KD Rev 2.1" (an ATMEL AT90S....
	based FM20X2KB-AB replacement).
	Note: Above char. 128 the FIPC-displays and KD's charactersets
	are different.

	The driver may work or will be easy to get working on the following
	Displays / Controllers (not tested!).
	Display:		Controller:
	FUTABA M204SD01AA	FUTABA 5P00A016
	Noritake CU20026SCPB-T	(microcontroller)
	Noritake CU20045SCPB-T28A	?
	... or compatible.
	(most Futaba/Noritake 7x5 dot VFDs with serial(rs232) interface
	(including  serial(rs232)/parallel types))
	Maybe there are little changes in the commands to do to get all
	functions to work correctly. But because of the similarity of the
	protocols, it wont be much work.

	If you want to add a new device to the driver add a new section
	to the displaytype-switch-case in the init-function and fill it
	with the corrrect commands for the display.
	(Try wich displaytype works best with your display, copy and modify
	it's section that is the easiest way I guess.)


        On this page I found pictures and datasheets of the VFD's:
	http://www.maltepoeggel.de/html/vfd/index.html




	To enable the bignumbers on 2-line displays (only this driver supports
	this yet) you have to edit "/clients/lcdproc/chrono.c" before compiling.
	In function "big_clock_screen" you have to replace the "4" by a "2" in
	this line: "if (lcd_hgt < 4)".



	This driver uses the serial mode of the displays.
	It is NOT possible to connect most of the displays directly to the
	serialport. The signal has to be inverted. I use the following
	circuit to do that job.

	Computer                                          Display
	                                                  (signal)
	HDD Powerconnector
	color(Voltage)

	red(+5V)    --------------------------------o----- +5V
	                                            |
	black(GND)  --------------------o           |
	                                |           R
	                                |           R 10k
	Serial(SUB-D 9Pin female)       V*          R
	pin(signal)                                 |
	                                            |
	3(TxD)  --RRRR--o--o      o-----------------o----- RxD
	          10k   |  |      |
	                |  |     C|
	                |  |   -----  BC547c (or similar NPN)
	                R  | B|  /  |
	            10k R  o--|-|   |
	                R     |  \  |
	                |      --->-    A*
	                |         E|    |
	                |          |    |
	5(GND)  --------o----------o----o----------------- GND

	Shield  ------------------------------------------
	optional

	*connect near display


	The pins on the different displays vary.



	FM20X2KB-AB:

	CN1:
	Pin 33 <--- RxD
	(Testmode: connect pins 25 & 26 then power up)

	CN2:
	Pin 1  <--- +5V
	Pin 2  ---- GND



	KD Rev 2.1:

	blue connector (6pin in a row) (the important one!):
	--------------------------------
	| +5V  +5V  RxD  GND  GND  GND |
	--------------------------------
	NOTE: hold the display in that position where you can read
	the "KD Rev 2.1" marking normally!

	gray connector (10pin 2 rows):
	Do not use. (the ATMEL ISP connector I guess)

	The two jumpers next to the gray connector:
	Normally not used.
	You can activate two different testmodes with them.





List of driver entry point:

init		Implemented.
close		Implemented.
width		Implemented.
height		Implemented.
clear		Implemented by space filling no custom char info.
flush		Calling draw_frame only.
string		Implemented.
chr		Implemented.
vbar		Implemented.
hbar		Implemented.
num		Implemented.
heartbeat	Implemented.
icon		Implemented.
cursor		NOT IMPLEMENTED: Is it really used?
set_char	Implemented.
get_free_chars	Implemented.
cellwidth	Implemented.
cellheight	Implemented.
get_contrast	Not implemented, no software control.
set_contrast	Not implemented, no software control.
get_brightness	Implemented.
set_brightness	Implemented.
backlight	Not implemented, no software control.
output		Not implemented.
get_key		Not implemented, no keys.
get_info	Implemented.

*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "serialVFD.h"

#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#define pos1_cursor	4 //moves cursor to top left character.
#define mv_cursor	5 //moves cursor to position specified by the next byte.
#define reset		6 //reset
#define init_cmds	7 //commands needed to initialize the display.
#define set_user_char	8 //set user character.
#define hor_tab		9 //moves cursor 1 chr right

typedef struct driver_private_data {
	char device[200];
	int fd;
	int speed;
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;
	/* defineable characters */
	int ccmode;
	int brightness;
	int customchars;
	int ISO_8859_1;
	unsigned int refresh_timer;
	unsigned char charmap[128];
	int display_type;
	char custom_char[31][7]; 	// used for "KD Rev 2.1" only
	char custom_char_changed[32]; 	// used only if need_refresh==1
	int need_refresh; 		// 1 if displays need refresh after changing custonchars
	char hw_cmd[10][4]; 		// hardwarespecific commands
	int usr_chr_dot_assignment[57];	// how to setup usercharacters
	unsigned int usr_chr_mapping[31];// where to place the usercharacters (0..30) in the asciicode
	int hbar_cc_offset;		// character offset of the bars
	int vbar_cc_offset;		// character offset of the bars
	char info[255];
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "serialVFD_";

/* Internal functions */
void serialVFD_init_vbar (Driver *drvthis);
void serialVFD_init_hbar (Driver *drvthis);
void serialVFD_draw_frame (Driver *drvthis, char *dat);
void serialVFD_put_brightness (Driver *drvthis);
void serialVFD_put_char (Driver *drvthis, int n);

// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
serialVFD_init (Driver *drvthis)
{
	struct termios portset;
	int tmp, w, h, brightness;
	char size[200] = DEFAULT_SIZE;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;
	p->ccmode = CCMODE_STANDARD;
	p->ISO_8859_1 = 1;
	p->refresh_timer = 0;

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis );
	
	/* Read config file */
	/* Which device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot parse Size: %s; using default %s",
			drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	brightness = tmp;


	/* ISO 8859 1 */
	p->ISO_8859_1 = drvthis->config_get_bool(drvthis->name, "ISO_8859_1", 0, 1);

	/* Which displaytype */
	tmp = drvthis->config_get_int(drvthis->name, "Type", 0, DEFAULT_DISPLAYTYPE);
	if ((tmp < 0) || (tmp > 3)) {
		report(RPT_WARNING, "%s: Type must be between 0 and 3; using default %d",
			drvthis->name, DEFAULT_DISPLAYTYPE);
		tmp = DEFAULT_DISPLAYTYPE;
	}
	p->display_type = tmp;


	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	if ((tmp != 1200) && (tmp != 2400) && (tmp != 9600) && (tmp != 19200) && (tmp != 115200)) {
		report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200; using default %d",
			drvthis->name, DEFAULT_SPEED);
		tmp = DEFAULT_SPEED;
	}
	if (tmp == 1200) p->speed = B1200;
	else if (tmp == 2400) p->speed = B2400;
	else if (tmp == 9600) p->speed = B9600;
	else if (tmp == 19200) p->speed = B19200;
	else if (tmp == 115200) p->speed = B115200;



	/* Set up io port correctly, and open it...*/
	debug(RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw(&portset);
#else
		// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	// Set port speed
	cfsetospeed(&portset, p->speed);
	cfsetispeed (&portset, B0);

	// Do it...
	tcsetattr(p->fd, TCSANOW, &portset);

	/* make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* make sure the framebuffer backing store is there... */
	p->backingstore = (unsigned char *) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->width * p->height);

//setup displayspecific data
	switch (p->display_type) {
//nec_fipc
		case 0: 		//nec_fipc
			p->need_refresh = 0; 	// 1 if displays need refresh after changing custonchars
			p->customchars = 1;	// number of customchaaracters the display provides
			p->vbar_cc_offset = 5;	// character offset of the bars
			p->hbar_cc_offset = 12;	// character offset of the bars

			// hardwarespecific commands:
			//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
			//					.....
			//				 {commandlength , command N}}
			const char hw_cmd_0[10][4]=	{{1	,0x04},  	// dark
						{1      ,0x03},
						{1	,0x02},
						{1	,0x01},  	// bright
						{1	,0x0D},  	// pos1
						{1	,0x1B},  	// move cursor
						{1	,0x0C},  	// reset
						{2	,0x14, 0x11},  	// init
						{1	,0x1A}, 	// set user char
						{1	,0x09}}; 	// tab
			for (tmp = 0; tmp < 10; tmp++)
				for (w = 0; w < 4  ;w++)
					p->hw_cmd[tmp][w] = hw_cmd_0[tmp][w];

			// Translates ISO 8859-1 to display charset.
			const unsigned char charmap_0[] = {
				/* #128  = 0x80 */
				128, 129, 130, 131, 132, 133, 134, 135,
				136, 137, 138, 139, 140, 141, 142, 143,
				144, 145, 146, 147, 148, 149, 150, 151,
				152, 153, 154, 155, 156, 157, 158, 159,
				/* #160 = 0xA0 */
				160,   '!', 0xF7, 0xF8,  '?',  '?', 0x7C, 0xF9,
				'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
				0x8B, 0xF3, 0x89, 0x8A, 0x27, 0x98,  '?',  '.',
				',',   '?',  '?',  '?',  '?', 0x8C,  '?', 0xAA,
				/* #192 = 0xC0 */
				'A',   'A',  'A',  'A', 0xA2, 0xA8,  'A',  'C',
				'E',   'E',  'E', 0xB6,  'I',  'I',  'I',  'I',
				'D',   0xA9, 'O',  'O',  'O',  'O', 0xA3, 0x8D,
				'0',   'U',  'U',  'U', 0xA4,  'Y',  'p', 0x91,
				/* #224 = 0xE0 */
				'a',   'a',  'a',  'a', 0xA5,  'a',  'a',  'c',
				'e',   'e',  'e',  'e',  'i',  'i',  'i',  'i',
				'o',   'n',  'o',  'o',  'o',  'o', 0xA6, 0x8E,
				'0',   'u',  'u',  'u', 0xA7,  'y',  'p',  'y'
				};
			for (tmp = 0; tmp < 128 ; tmp++)
				p->charmap[tmp] = charmap_0[tmp];

			//{bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
			const int usr_chr_dot_assignment_0[57]={7,	 1, 2, 3, 4, 5, 0, 0, 0,
								6, 7, 8, 9,10, 0, 0, 0,
								11,12,13,14,15, 0, 0, 0,
								16,17,18,19,20, 0, 0, 0,
								21,22,23,24,25, 0, 0, 0,
								26,27,28,29,30, 0, 0, 0,
								31,32,33,34,35, 0, 0, 0};
			for (tmp = 0; tmp < 57 ;tmp++)
				p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment_0[tmp];

			//Where to place the usercharacters (0..30) in the asciicode.
			//Also used to map standardcharacters in the usercharacterspace(0..30)
			//(useful for displays with less then 30 usercharacters and predefined bars)
			const unsigned int usr_chr_mapping_0[31]=
			{0xAF,0,0,0,0,0, 0x5F, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0, 0x5F, 0xE1, 0xE3, 0xE4};
			for (tmp = 0; tmp < 31; tmp++)
				p->usr_chr_mapping[tmp] = usr_chr_mapping_0[tmp];
			break;

//KD Rev2.1
		case 1: 		//KD Rev2.1
			p->need_refresh=1; 	// 1 if displays need refresh after changing custonchars
			p->customchars=31;	// number of customchaaracters the display provides
			p->vbar_cc_offset=0;	// character offset of the bars
			p->hbar_cc_offset=0;	// character offset of the bars

			// hardwarespecific commands:
			//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
			//					.....
			//				 {commandlength , command N}}
			const char hw_cmd_1[10][4]=	{{1	,0x04},  	// dark
						{1      ,0x03},
						{1	,0x02},
						{1	,0x01},  	// bright
						{1	,0x0D},  	// pos1
						{1	,0x1B},  	// move cursor
						{1	,0x0C},  	// reset
						{2	,0x14, 0x11},  	// init
						{1	,0x1A}, 	// set user char
						{1	,0x09}}; 	// tab
			for (tmp = 0; tmp < 10; tmp++)
				for (w = 0; w < 4; w++)
					p->hw_cmd[tmp][w] = hw_cmd_1[tmp][w];

			const unsigned char charmap_1[] = {
				/* #128  = 0x80 */
				128, 129, 130, 131, 132, 133, 134, 135,
				136, 137, 138, 139, 140, 141, 142, 143,
				144, 145, 146, 147, 148, 149, 150, 151,
				152, 153, 154, 155, 156, 157, 158, 159,
				/* #160 = 0xA0 */
				160,   '!', 0xF7, 0xF8,  '?',  '?', 0x7C, 0xF9,
				'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
				0xA9, 0xBA,  '?',  '?', 0x27,  '?',  '?',  '.',
				',',   '?',  '?',  '?',  '?',  '?',  '?',  '?',
				/* #192 = 0xC0 */
				0xB2,  'A',  'A',  'A', 0xA2, 0xA1,  'A',  'C',
				'E',   'E', 0xA8, 0xB6, 0xAB, 0xAA, 0xAC, 0xA9,
				'D',  0xAE, 0xB1, 0xB0, 0xBF,  'O', 0xA3,  'x',
				0xAF, 0xB7, 0xB6, 0xB8, 0xA4,  'Y',  'p', 0x91,
				/* #224 = 0xE0 */
				0xD2, 0xC2, 0xC4,  'a', 0xA5, 0xC1, 0xC5, 0xC3,
				0xC7, 0xC6, 0xC8, 0xD4, 0xCC, 0xCB, 0xCD, 0xCA,
				'o',  0xCF, 0xD1, 0xD0, 0xCE,  'o', 0xA6, 0xBB,
				0xD0, 0xD7, 0xD6, 0xD8, 0xA7,  'y',  'p',  'y'
				};
			for (tmp = 0; tmp < 128; tmp++)
				p->charmap[tmp] = charmap_1[tmp];


			//{bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
			const int usr_chr_dot_assignment_1[57]={7,	 1, 2, 3, 4, 5, 0, 0, 0,
								6, 7, 8, 9,10, 0, 0, 0,
								11,12,13,14,15, 0, 0, 0,
								16,17,18,19,20, 0, 0, 0,
								21,22,23,24,25, 0, 0, 0,
								26,27,28,29,30, 0, 0, 0,
								31,32,33,34,35, 0, 0, 0};
			for (tmp = 0; tmp < 57; tmp++)
				p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment_1[tmp];

			//Where to place the usercharacters (0..30) in the asciicode.
			//Also used to map standardcharacters in the usercharacterspace(0..30)
			//(useful for displays with less then 30 usercharacters and predefined bars)
			const unsigned int usr_chr_mapping_1[31]=
			{0xAF};
			for (tmp = 0; tmp < 31; tmp++)
				p->usr_chr_mapping[tmp] = usr_chr_mapping_1[tmp];
			break;

//Noritake
		case 2: 		//Noritake
			p->need_refresh=0; 	// 1 if displays need refresh after changing custonchars
			p->customchars=16;	// number of customchaaracters the display provides
			p->vbar_cc_offset=0;	// character offset of the bars
			p->hbar_cc_offset=0;	// character offset of the bars

			// hardwarespecific commands:
			//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
			//					.....
			//				 {commandlength , command N}}
			const char hw_cmd_2[10][4]=	{{1	,0x1B, 0x4C, 0x00},  // dark
						{1      ,0x1B, 0x4C, 0x50},
						{2	,0x1B, 0x4C, 0x90},
						{1	,0x1B, 0x4C, 0xFF},  // bright
						{1	,0x0C},		// pos1
						{2	,0x1B, 0x48},	// move cursor
						{2	,0x1B, 0x49},	// reset
						{2	,0x14, 0x11},  	// init
						{2	,0x1B, 0x43}, 	// set user char
						{1	,0x09}}; 	// tab
			for (tmp = 0; tmp < 10; tmp++)
				for (w = 0; w < 4; w++)
					p->hw_cmd[tmp][w] = hw_cmd_2[tmp][w];

			// no charmap needed
			for (tmp = 128; tmp <= 255; tmp++)
				p->charmap[tmp]=tmp;

			//{bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
			const int usr_chr_dot_assignment_2[57]={5,	 1, 2, 3, 4, 5, 6, 7, 8,
								 9,10,11,12,13,14,15,16,
								17,18,19,20,21,22,23,24,
								25,26,27,28,29,30,31,32,
								33,34,35, 0, 0, 0, 0, 0};
			for (tmp=0;tmp < 57 ;tmp++)
				p->usr_chr_dot_assignment[tmp]=usr_chr_dot_assignment_2[tmp];

			//Where to place the usercharacters (0..30) in the asciicode.
			//Also used to map standardcharacters in the usercharacterspace(0..30)
			//(useful for displays with less then 30 usercharacters and predefined bars)
			const unsigned int usr_chr_mapping_2[31]=
			{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,\
 	 		0x0F, 0x10, 0x13 , 0x14, 0x1C, 0x1D, 0x1E, 0x1F};
			for (tmp=0;tmp < 31 ;tmp++)
				p->usr_chr_mapping[tmp]=usr_chr_mapping_2[tmp];
			break;
//Futaba
		case 3: 		//Futaba
			p->need_refresh=0; 	// 1 if displays need refresh after changing custonchars
			p->customchars=3;	// number of customchaaracters the display provides
			p->vbar_cc_offset=0x30;	// character offset of the bars
			p->hbar_cc_offset=0x30;	// character offset of the bars

			// hardwarespecific commands:
			//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
			//					.....
			//				 {commandlength , command N}}
			const char hw_cmd_3[10][4]=  {{1      ,0x04, 0x20},	// dark
						{1      ,0x04, 0x40},
						{2	,0x04, 0x60},
						{1	,0x04, 0xFF},	// bright
						{2	,0x10, 0x00},	// pos1
						{1	,0x10,},	// move cursor
						{1	,0x1F},		// reset
						{1	,0x11},  	// init
						{1	,0x03}, 	// set user char
						{1	,0x09}}; 	// tab
			for (tmp=0;tmp < (10) ;tmp++)
				for (w=0;w < (4) ;w++)
					p->hw_cmd[tmp][w]=hw_cmd_3[tmp][w];

			// Translates ISO 8859-1 to display charset.
			const unsigned char charmap_3[] = {
				/* #128  = 0x80 */
				128, 129, 130, 131, 132, 133, 134, 135,
				136, 137, 138, 139, 140, 141, 142, 143,
				144, 145, 146, 147, 148, 149, 150, 151,
				152, 153, 154, 155, 156, 157, 158, 159,
				/* #160 = 0xA0 */
				160,  0xAD, 0x9B, 0x9C, 0xC8, 0x9D, 0x7C, 0xC0,
				'"',   '?', 0xA6, 0xAE, 0xAA,  '-',  '?',  '?',
				0xEF, 0xCA, 0xC6, 0xC7, 0x27, 0xB8,  '?',  '.',
				',',   '?', 0xA7, 0xAF, 0xAC, 0xAB,  '?', 0xA8,
				/* #192 = 0xC0 */
				0xD0,  'A', 0xD5,  'A', 0x8E, 0x8F, 0x92, 0x80,
				0xD1, 0x90, 0xD6, 0xD3, 'I',   'I', 0xD7, 0xD4,
				'D',  0xA5,  'O',  'O', 0xD8,  'O', 0x99,  'x',
				'0',  0xD2,  'U', 0xD9, 0x9A,  'Y',  'p', 0xB1,
				/* #224 = 0xE0 */
				0x85, 0xA0, 0x83,  'a', 0x84, 0x86, 0x91, 0x87,
				0x8A, 0x82, 0x88, 0x89, 0x8D, 0xA1, 0x8C, 0x8B,
				'o',  0xA4, 0x95, 0xA9, 0x93,  'o', 0x94,  '/',
				'0',  0x97, 0xA3, 0x96, 0x81,  'y',  'p', 0x89
				};
			for (tmp = 0; tmp < 128; tmp++)
				p->charmap[tmp] = charmap_3[tmp];

			//{bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
			const int usr_chr_dot_assignment_3[57]={5,	 8, 7, 6, 5, 4, 3, 2, 1,
								16,15,14,13,12,11,10, 9,
								24,23,22,21,20,19,18,17,
								32,31,30,29,28,27,26,25,
								0, 0, 0, 0, 0,35,34,33};
			for (tmp=0;tmp < 57 ;tmp++)
				p->usr_chr_dot_assignment[tmp]=usr_chr_dot_assignment_3[tmp];

			//Where to place the usercharacters (0..30) in the asciicode.
			//Also used to map standardcharacters in the usercharacterspace(0..30)
			//(useful for displays with less then 30 usercharacters and predefined bars)
			const unsigned int usr_chr_mapping_3[31]=
			{0xCD, 0xCE, 0xCF};
			for (tmp = 0; tmp < 31; tmp++)
				p->usr_chr_mapping[tmp] = usr_chr_mapping_3[tmp];
			break;
		}

	//initialise display
	write(p->fd, &p->hw_cmd[reset][1],p->hw_cmd[reset][0]);
	write(p->fd, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);
	serialVFD_set_brightness(drvthis, 1, brightness);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;

}


/////////////////////////////////////////////////////////////////
// Changes screen brightness (0 min ... 255 max ... 1000 also max)
//
MODULE_EXPORT void
serialVFD_set_brightness (Driver *drvthis, int state, int brightness)
{	// set p->brightness
	PrivateData *p = drvthis->private_data;

	if (brightness > 0) {
		if (brightness > 255)
			brightness = 255;
		if (p->brightness != brightness) {
			p->brightness = brightness;
			serialVFD_put_brightness(drvthis);
		}
	}
}

void
serialVFD_put_brightness (Driver *drvthis)
{	// set hardware brightness
	PrivateData *p = drvthis->private_data;
	int realbrightness = (int) (p->brightness / 64);

	//(4 steps 0-64, 65-128, 129-192, 193-1000)
	write(p->fd, &p->hw_cmd[realbrightness][1],\
		p->hw_cmd[realbrightness][0]);
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
serialVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	serialVFD_init_vbar(drvthis);
	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, p->vbar_cc_offset);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//

MODULE_EXPORT void
serialVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	serialVFD_init_hbar(drvthis);
	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, p->hbar_cc_offset);

}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, value 1 mean "on" and 0 is "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
serialVFD_set_char (Driver *drvthis, int n, char *dat)
{	//set char in p->custom_char
	PrivateData *p = drvthis->private_data;
	unsigned int byte, bit;

	if ((n < 0) || (n > p->customchars-1))
		return;
	if (!dat)
		return;

	for (byte = 0; byte < p->usr_chr_dot_assignment[0]; byte++) {
		int letter = 0;

		for (bit = 0; bit < 8; bit++) {
			if ((int) p->usr_chr_dot_assignment[bit+8*byte+1] != 0)
				letter |= (dat[(int)p->usr_chr_dot_assignment[bit+8*byte+1]-1] << bit);
		}
		p->custom_char[n][byte] = letter;
	}

	if (p->display_type != 1) //not KD Rev 2.1
		serialVFD_put_char(drvthis, n);

	if (p->need_refresh == 1)
		p->custom_char_changed[n] = 1;
}

void
serialVFD_put_char (Driver *drvthis, int n)
{	// put char in display
	PrivateData *p = drvthis->private_data;

	write(p->fd, &p->hw_cmd[set_user_char][1],
		p->hw_cmd[set_user_char][0]);// substitute and select Character to overwrite
	write(p->fd, &p->usr_chr_mapping[n], 1);
	write(p->fd, &p->custom_char[n][0], p->usr_chr_dot_assignment[0]);// overwrite selected Character
}



/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized serialVFD->width*serialVFD->height
//
void
serialVFD_draw_frame (Driver *drvthis, char *dat)
{
	PrivateData *p = drvthis->private_data;
	int i, j, last_chr = -10;

	if (!dat)
		return;
	if (p->refresh_timer > 500) { // Do a full refresh every 500 refreshs.
	// With this it is possible to switch display on and off while lcdproc is running
		write(p->fd, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);
		serialVFD_put_brightness(drvthis); // restore brightness

		for (i = 0; i < (p->height * p->width); i++)
			p->backingstore[i] = '\0'; // clear Backing-store

		if (p->display_type != 1) {//not KD Rev 2.1
			for (i = 0; i < p->customchars; i++) // refresh all customcharacters
				serialVFD_put_char(drvthis, i);
/*				{
					write(p->fd, &p->hw_cmd[set_user_char][1],\
					p->hw_cmd[set_user_char][0]);// substitute and select character to overwrite
					write(p->fd, &p->usr_chr_mapping[i], 1);
					write(p->fd, &p->custom_char[(int)dat[i]][0], p->usr_chr_dot_assignment[0]);// overwrite selected Character
				}*/
		}
		p->refresh_timer = 0;
	}
	p->refresh_timer++;

	for (i = 0; i < (p->height * p->width); i++) {
		/* Backing-store implementation.  If it's already
		 * on the screen, don't put it there again
		 */

		if (dat[i] != p->backingstore[i] || (dat[i] <=30 && p->custom_char_changed[(int)dat[i]]))
		{
			if (last_chr < i-1) { // if not last char written cursor has to be moved.
				if (last_chr < i-2-p->hw_cmd[mv_cursor][0]) {
					write(p->fd, &p->hw_cmd[mv_cursor][1],
						p->hw_cmd[mv_cursor][0]);
					write(p->fd, &i, 1);
				}
				else {
					for (j = last_chr; j < (i-1); j++) {
						write(p->fd, &p->hw_cmd[hor_tab][1], p->hw_cmd[hor_tab][0]);
					}
				}
			}

			if (dat[i] >= 0 && dat[i] <= 30) {// custom character
				if (p->display_type == 1) {// KD Rev 2.1 only
					write(p->fd, "\x1A\xDB", 2);		// substitute and select character to overwrite(237)
					write(p->fd, &p->custom_char[(int)dat[i]][0], 7);// overwrite selected character
					write(p->fd, "\xDB", 1);			// write character
				}
				else {	// all other displays
					write(p->fd, &p->usr_chr_mapping[(int)dat[i]], 1);
				}
			}
			else if (dat[i] < 0 && (p->ISO_8859_1 != 0)) { // ISO_8859_1 translation for 129 ... 255
				write(p->fd, &p->charmap[dat[i] + 128], 1);
			}
			else {
				write(p->fd, &dat[i], 1);
			}

			last_chr = i;
		}
	}

	if (last_chr != -10) { // update backingstore if something changed
		int i;

		memcpy(p->backingstore, dat, p->height * p->width);

		for (i = 0; i < p->customchars; i++)
			p->custom_char_changed[i] = '\0';
	}
}


MODULE_EXPORT void
serialVFD_num( Driver * drvthis, int x, int num )
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if (p->ccmode != CCMODE_BIGNUM) { // Are the customcharacters set up correctly? If not:
		do_init = 1;	// Lib_adv_bignum has to set the customcharacters.
		p->ccmode = CCMODE_BIGNUM; // Switch customcharactermode to bignum.
	}
	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, p->height, do_init, p->customchars);
}



/*
 * Places an icon on screen
 */
MODULE_EXPORT int
serialVFD_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char icons[2][5 * 7] = {
		{
		 1, 1, 1, 1, 1,			  // Empty Heart
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1,			  // Filled Heart
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 },
	};
	// Yes we know, this is a VERY BAD implementation :-)
	switch (icon) {
		case ICON_BLOCK_FILLED:
			serialVFD_chr(drvthis, x, y, 127);
			break;
		case ICON_HEART_FILLED:
		        p->ccmode = CCMODE_STANDARD;
			serialVFD_set_char(drvthis, 0, icons[1]);
			serialVFD_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
		        p->ccmode = CCMODE_STANDARD;
			serialVFD_set_char(drvthis, 0, icons[0]);
			serialVFD_chr(drvthis, x, y, 0);
			break;
		default:
			return -1; // Let the core do other icons
	}
	return 0;
}



/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before serialVFD->vbar()
//
void
serialVFD_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->ccmode != CCMODE_VBAR) {
		p->ccmode = CCMODE_VBAR;
		serialVFD_set_char(drvthis, 1, a);
		serialVFD_set_char(drvthis, 2, b);
		serialVFD_set_char(drvthis, 3, c);
		serialVFD_set_char(drvthis, 4, d);
		serialVFD_set_char(drvthis, 5, e);
		serialVFD_set_char(drvthis, 6, f);
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
serialVFD_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char a[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};

	if (p->ccmode != CCMODE_HBAR) {
		p->ccmode = CCMODE_HBAR;
		serialVFD_set_char(drvthis, 1, a);
		serialVFD_set_char(drvthis, 2, b);
		serialVFD_set_char(drvthis, 3, c);
		serialVFD_set_char(drvthis, 4, d);
	}
}





/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
serialVFD_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);

}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
serialVFD_string (Driver *drvthis, int x, int y, char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;
	for (i = 0; string[i] != '\0'; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
serialVFD_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		close(p->fd);

		if (p->framebuf)
			free(p->framebuf);

		if (p->backingstore)
			free(p->backingstore);
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
serialVFD_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
serialVFD_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Returns the maximum number of custom char slots (not how many
// are free at a moment, maybe this isn't even needed...
//
MODULE_EXPORT int
serialVFD_get_free_chars (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->customchars;
}
/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
serialVFD_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
serialVFD_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


MODULE_EXPORT void
serialVFD_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	
	serialVFD_draw_frame(drvthis, p->framebuf);
}



/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
serialVFD_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;
	p->framebuf[(y * p->width) + x ] = c;
}

/////////////////////////////////////////////////////////////////
//
// Retrieves brightness
//
MODULE_EXPORT int
serialVFD_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return p->brightness;
}

/////////////////////////////////////////////////////////////////
// provides some info about this driver
//
MODULE_EXPORT const char *
serialVFD_get_info (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	strcpy(p->info, "Driver for many serialVFDs from NEC(all FIPC based), Noritake, Futaba and the \"KD Rev2.1\"VFD.");
	return p->info;
}
