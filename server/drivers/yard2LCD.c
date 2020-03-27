/***************************************************************************
 *   Copyright (C) 2009 by M. Feser                                        *
 *   Copyright (C) 2013 by R. Geigenberger (Update for YARD2)              *
 *                         yard2lcdproc (.A.T.) yard2usb.de                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "lcd.h"
//#define DEBUG
#include "shared/report.h"
#include "yard2LCD.h"
#include "lcd_lib.h"

/*
//Type of characters currently stored in CGRAM. from lcd.h as info
typedef enum {
	standard,		// one char is used for heartbeat
	vbar,			// vertical bars
	hbar,			// horizontal bars
	icons,			// standard icons
	custom,			// custom settings
	bignum,			// big numbers
} CGmode;
*/

typedef enum {
	Dmode_standard,
 	Dmode_graphical
} Dmode;


/* Driver private data */
typedef struct driver_private_data {
	int fd;
	int width, height;
	int gwidth, gheight;
	int hspace, vspace;
	int cellwidth, cellheight;
	int bigcellwidth, bigcellheight;
	unsigned char *framebuf;
	int on_brightness;
	int off_brightness;
	char hw_brightness;
	CGmode ccmode;
	Dmode dispmode;
	char info[255];
	char LCDtype;
} PrivateData;

/* Prototypes of internal functions */
static int yard_hwWrite(Driver *drvthis, unsigned char *yardData, unsigned char Datalen);
static int yard_hwClearLCD(Driver *drvthis);
//static int yard_hwEnterGmode(Driver *drvthis);
static int yard_hwGotoXY(Driver *drvthis, unsigned char x, unsigned char y);
//static int yard_hwPrintChar(Driver *drvthis, char c);
static int yard_hwPrintCharArray(Driver *drvthis, unsigned char *str, unsigned char len);
static int yard_hwSetBrightness(Driver *drvthis, unsigned char brightVal);
static int yard_hwWriteCGRam(Driver *drvthis, unsigned char numChar, unsigned char *data);


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "yard_";

unsigned char yardCmd[64];

/*
 * Hardware function: Write YARD command to KeyLcd via socket
 */
static int 
yard_hwWrite(Driver *drvthis, unsigned char *yardData, unsigned char Datalen)
{
	debug(RPT_DEBUG, "%s: Event 01 - Enter yard_hwWrite: %d - %s",drvthis->name,Datalen,yardData);
	PrivateData *p = drvthis->private_data;
	unsigned char byteCnt;

	// Send data to YARD server
	if ( (Datalen > 0) && (Datalen <= MAX_YARDDATA_SIZE)) 
	{
		byteCnt = write(p->fd, yardData, Datalen); //(char *)&yardCMD
		//byteCnt = write(p->fd, "WTEST",5);
		if (byteCnt < 0) 
		{
			report(RPT_WARNING, "%s: Can't send data to YARD2 LCDserver !", drvthis->name);
			debug(RPT_DEBUG, "%s: Event 01.1 - Can't send data to YARD2 LCDserver !", drvthis->name);
			return -1;
		}
	}
	else
	{
		debug(RPT_DEBUG, "%s: Event 01.1 - %s: Too much Data for YARD Server: %d !", drvthis->name,Datalen);
		report(RPT_WARNING, "%s: Too much Data for YARD Server: %d !", drvthis->name,Datalen);
		return -1;
	}

	
	// Just wait for YARD server response; No matter of Data received, but > 2 bytes, normally ROK is send
	byteCnt = read(p->fd, (char *)&yardData, sizeof(yardData) );
	if (byteCnt < 0) 
	{
		debug(RPT_DEBUG, "%s: Cannot receive ROK from YARD2 server !", drvthis->name);
		report(RPT_ERR, "%s: Cannot receive ROK from YARD2 server !", drvthis->name);
		return -1;
	}

	return 0;
}


/*
 * Hardware function: Clears screen
 */
static int 
yard_hwClearLCD(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 02 - Enter yard_hwClearLCD",drvthis->name);
	unsigned char cmd;
	
	cmd = 'C'; // = Clear cmd for YARD
	return yard_hwWrite(drvthis, &cmd, 1);
}

/*
 * Hardware function: Shifts cursor to position (x,y), left upper corner equals (1,1)
 */
static int 
yard_hwGotoXY(Driver *drvthis, unsigned char x, unsigned char y)
{
	debug(RPT_DEBUG, "%s: Event 03 - Enter yard_hwGotoXY: X.%d - Y.%d",drvthis->name,x,y);
	PrivateData *p = (PrivateData *)drvthis->private_data;
	unsigned char cmdBuf[5];

	// Error check
	switch (p->dispmode)
	{
		case Dmode_standard: 
		{
			if ( (x > p->width) || (x < 1) || (y > p->height) || (y < 1) )
				return -1;
			
			// Setup and send YARD command
			cmdBuf[0] = 'G';
			cmdBuf[1] = x - 1;	// Zero index start ! lcdproc starts with 1,1, YARD 0,0
			cmdBuf[2] = y - 1;
			return yard_hwWrite(drvthis, cmdBuf, 3);
			
		}
		default: {
			return -1;
		}
	}
}


/*
 * Hardware function: Prints a single character not used now
 */
 /*
static int 
yard_hwPrintChar(Driver *drvthis, char c)
{
	debug(RPT_DEBUG, "%s: Event 04 - Enter yard_hwPrintChar: %s",drvthis->name,c);
	unsigned char cmdBuf[2];
	
	// Setup YARD command
	cmdBuf[0] = 'W';
	cmdBuf[1] = c;

	// Send command
	return yard_hwWrite(drvthis, cmdBuf, 2);
}
*/

/*
 * Hardware function: Prints a character array
 */
static int 
yard_hwPrintCharArray(Driver *drvthis, unsigned char *str, unsigned char len)
{
	debug(RPT_DEBUG, "%s: Event 05 - Enter yard_hwPrintCharArray: %d - %s",drvthis->name,len,str);
	unsigned char cmdBuf[MAX_YARDDATA_SIZE];
	
	// Error check
	if (len > MAX_YARDDATA_SIZE) 
	{
		report(RPT_WARNING, "%s: PrintCharArray parameter too large !", drvthis->name);
		return -1;
	}
	
	cmdBuf[0] = 'W';
	memcpy(&cmdBuf[1], str, len);
	
	// Send command
	return yard_hwWrite(drvthis, cmdBuf, len + 1);
}

/*
 * Hardware function: Sets brightness of the backlight
 */
static int 
yard_hwSetBrightness(Driver *drvthis, unsigned char brightVal)
{
	debug(RPT_DEBUG, "%s: Event 06 - Enter yard_hwSetBrightness: %d",drvthis->name,brightVal);
	unsigned char cmdBuf[3];
	
	// Setup YARD command
	cmdBuf[0] = 'B';
	cmdBuf[1] = brightVal;
	return yard_hwWrite(drvthis, cmdBuf, 2);
}

/*
 * Hardware function: Writes a character into the CGRAM
 */
static int 
yard_hwWriteCGRam(Driver *drvthis, unsigned char numChar, unsigned char *data)
{
	debug(RPT_DEBUG, "%s: Event 07 - Enter yard_hwWriteCGRam: %d - %02X",drvthis->name,numChar,data);
	unsigned char cmdBuf[11];
	
	// Setup YARD command
	cmdBuf[0] = 'I';
	cmdBuf[1] = numChar;
	memcpy(&cmdBuf[2], data, 8);
	return yard_hwWrite(drvthis, cmdBuf, 10);
}

/*
 * Inits settings and connection
 */
MODULE_EXPORT int 
yard_init(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 08 - Enter yard_init",drvthis->name);
	struct sockaddr_un srvAddr;
	int tmp, srvAddrLen;
	PrivateData *p;
	char sockpath[200] = DEFAULT_SOCK_PATH; 
	unsigned char byteCnt;
	char Recbuffer[10];
	

	// Allocate and store private data
	p = (PrivateData *)malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p) < 0)
		return -1;

	// Initialize cellsize, character spacing and graphical offset
	p->fd = -1;
	p->cellwidth = DEFAULT_CELLWIDTH;
	p->cellheight = DEFAULT_CELLHEIGHT;
	p->bigcellwidth = DEFAULT_CELLWIDTH_BIG;
	p->bigcellheight = DEFAULT_CELLHEIGHT_BIG;
	p->hspace = DEFAULT_HSPACE;
	p->vspace = DEFAULT_VSPACE;
	p->ccmode = standard;
	p->dispmode = Dmode_standard;
	p->LCDtype = 0; //only use 0=HD44780; not supported now 1=KS0066;2=T6963c;3=KS0108
	p->width  = DEFAULT_WIDTH;
	p->height = DEFAULT_HEIGHT;
	p->gwidth = DEFAULT_GWIDTH;
	p->gheight = DEFAULT_GHEIGHT;

	// Establish connection to YARD server
	bzero( (char *)&srvAddr, sizeof(srvAddr));
	srvAddr.sun_family = AF_UNIX;
	strcpy(srvAddr.sun_path, sockpath);
	srvAddrLen = strlen(srvAddr.sun_path) + sizeof(srvAddr.sun_family);
	
	p->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (p->fd < 0) {
		report(RPT_ERR, "%s: Can't create socket !", drvthis->name);
		return -1;
	}
	if (connect(p->fd, (struct sockaddr *)&srvAddr, srvAddrLen) < 0) {
		report(RPT_ERR, "%s: Can't connect to yardsrv !", drvthis->name);
		return -1;
	}
	
	//Get the config from yard2srvd. Config is not in LCDd.conf !
	sprintf(Recbuffer,"LCDPROC");
	byteCnt = write(p->fd,Recbuffer, strlen(Recbuffer)); 
	if (byteCnt < 0) 
	{
		report(RPT_ERR, "%s: Can't send config request to YARD2 LCDserver !", drvthis->name);
		return -1;
	}

	// Wait for YARD server response
	byteCnt = read(p->fd, Recbuffer, sizeof(Recbuffer) );
	if (byteCnt < 0) 
	{
		report(RPT_ERR, "%s: Cannot receive config from YARD server !", drvthis->name);
		return -1;
	}
	else if (byteCnt == 1) 
	{
		report(RPT_ERR, "%s: YARD communication timeout !", drvthis->name);
		return -1;
	}

	//Receiveconfig format: 0='C';1=sizeX;2=sizeY;3=LCDType(0=HD44780,1=KS0066,2=T6963,3=KS0108)
	if(Recbuffer[0] == 'C') //C=Config
	{
		p->width   = Recbuffer[1];
		p->height  = Recbuffer[2];
		p->LCDtype = Recbuffer[3];
	}
	else
	{
		report(RPT_ERR, "%s: YARD Config Receive error !", drvthis->name);
		return -1;
	}

	if(p->LCDtype > 1) //only char LCDs are currently supported
	{
		report(RPT_ERR, "%s: YARD LCD type %d not supported by this version or the driver !", drvthis->name,p->LCDtype);
		return -1;
	}
	
	// Allocate framebuf & Setup frame buffer x2 to be sure that the buffer is big enough
	p->framebuf = (unsigned char *) malloc((p->width * p->height)*2);
	if (p->framebuf == NULL) 
	{
		report(RPT_ERR, "%s: Can't create framebuffer !", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', (p->width * p->height)*2);

/* not done yet
	// Init size and graphical size
*/	

/*	
	// Init backlight-on brightness -> done by YARD2 normally in auto mode
*/
	p->on_brightness = DEFAULT_ON_BRIGHTNESS;
/*
	// Init backlight-off brightness -> done by YARD2 normally in auto mode
*/
	tmp = DEFAULT_OFF_BRIGHTNESS;
	p->off_brightness = tmp;


	report(RPT_DEBUG, "%s: Init done", drvthis->name);
	return 0;
}

/*
 * Closes down the driver
 */
MODULE_EXPORT void
yard_close(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 09 - Enter yard_close",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;
	
	if (p != NULL) {
		close(p->fd);
		if (p->framebuf)
			free(p->framebuf);
		free(p);
	}

	drvthis->store_private_ptr(drvthis, NULL);
}

/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
yard_width(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 10 - Enter yard_width",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	return p->width;
}

/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
yard_height(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 11 - Enter yard_height",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	return p->height;
}

/*
 * Clears the framebuffer
 */
MODULE_EXPORT void
yard_clear(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 12 - Enter yard_clear",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;
	
	// Reset custom char mode and clear frambuffer
	p->ccmode = standard;
	memset(p->framebuf, ' ', p->width * p->height);
}

/*
 * Flushes a single frame to the LCD
 */
MODULE_EXPORT void
yard_flush(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 13 - Enter yard_flush",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;
	int i;
	int width, height;

	switch (p->dispmode) {
		case Dmode_standard: 
		{
			// Set width and height
			width = p->width;
			height = p->height;
			break;
		}
		default: 
		{
			debug(RPT_DEBUG, "%s: Event 24 - Only Char. Mode supported: %d",drvthis->name,p->dispmode);
			return;
			break;
		}
	}
	
	// Send framebuffer content to LCD
	for (i = 0; i < height; i++) 
	{
		// Line feed
		yard_hwGotoXY(drvthis, 1, i+1);
			
		// Write single line
		yard_hwPrintCharArray(drvthis, &(p->framebuf[i*width]), width);
		debug(RPT_DEBUG, "YARD2: flushed line %d - chars %d", i,width);
	}


}

/*
 * Prints a string on the LCD
 */
MODULE_EXPORT void
yard_string(Driver *drvthis, int x, int y, const char string[])
{
	debug(RPT_DEBUG, "%s: Event 14 - Enter yard_string: X.%d - Y.%d - %s",drvthis->name,x,y,string);

	PrivateData *p = (PrivateData *)drvthis->private_data;
	int i;

	// Change display mode if necessary; Only char mode 0 is supported right now
	if (p->dispmode == Dmode_graphical) 
	{
		yard_hwClearLCD(drvthis);
		p->dispmode = Dmode_standard;
	}
	
	x--; //start with 0,0
	y--;

	if ((y < 0) || (y >= p->height)) return;

	

	for (i = 0; (string[i] != '\0') && (x < p->width); i++) //, x++) 
	{
		if (x >= 0) // no write left of left border
			p->framebuf[(y * p->width) + x + i] = string[i];
	}

/*
	for (i = 0; string[i] != '\0'; i++) 
	{
		// Check for buffer overflow
		if ( (y * p->width + x + i) >= (p->width * p->height) )
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
*/
}

/*
 * Prints a single character on the LCD
 */
MODULE_EXPORT void
yard_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	debug(RPT_DEBUG, "%s: Event 15 - Enter yard_chr",drvthis->name);
	
	// Error check
	if ( (x > p->width) || (y > p->height) )
		return;
	
	// Change display mode if necessary
	if (p->dispmode == Dmode_graphical) 
	{
		yard_hwClearLCD(drvthis);
		p->dispmode = 0;
	}
	
	y--; //start with 0,0
	x--;
	p->framebuf[(y * p->width) + x ] = c;
}

/*
 * Returns the number of custom characters
 */
MODULE_EXPORT int
yard_get_free_chars(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 16 - Enter yard_get_free_chars",drvthis->name);
	return NUM_CUSTOMCHARS;
}

/*
 * Sets a custom character
 */
MODULE_EXPORT void
yard_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	debug(RPT_DEBUG, "%s: Event 17 - Enter yard_set_char: Nr.%d - %02X",drvthis->name,n,dat);

	if ( (n < 0) || (n >= NUM_CUSTOMCHARS) )
		return;
	if (!dat)
		return;

	yard_hwWriteCGRam(drvthis, n, dat);
}

/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
yard_cellwidth(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 18 - Enter yard_cellwidth",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	return p->cellwidth;
}

/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
yard_cellheight(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 19 - Enter yard_cellheight",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	return p->cellheight;
}

/*
 * Reads brightness (in promille)
 */
MODULE_EXPORT int
yard_get_brightness(Driver *drvthis, int state)
{
	debug(RPT_DEBUG, "%s: Event 20 - Enter yard_get_brightness: %d",drvthis->name,state);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->on_brightness : p->off_brightness;
}

/*
 * Sets brightness (in promille)
 */
MODULE_EXPORT void
yard_set_brightness(Driver *drvthis, int state, int promille)
{
	debug(RPT_DEBUG, "%s: Event 21 - Enter yard_set_brightness: %d - %d",drvthis->name,state,promille);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	// Error check
	if (promille < 0 || promille > 1000)
		return;

	// Store values
	if (state == BACKLIGHT_ON) {
		p->on_brightness = promille;
	}
	else {
		p->off_brightness = promille;
	}
}

/*
 * Sets the backlight on or off
 * The hardware supports any value between 0 and 255
 */
MODULE_EXPORT void
yard_backlight(Driver *drvthis, int on)
{
	debug(RPT_DEBUG, "%s: Event 22 - Enter yard_backlight: %d",drvthis->name,on);
	PrivateData *p = (PrivateData *)drvthis->private_data;

	int hardware_value = (on == BACKLIGHT_ON) ? p->on_brightness : p->off_brightness;

	// Map range to hardware [0, 1000] -> [0, 255]
	hardware_value /= 4;
	if (hardware_value != p->hw_brightness) 
	{
		// Send command and update private data
		yard_hwSetBrightness(drvthis, hardware_value);
		p->hw_brightness = (char)hardware_value;
	}
}

/*
 * Provides info about the driver
 */
MODULE_EXPORT const char *
yard_get_info(Driver *drvthis)
{
	debug(RPT_DEBUG, "%s: Event 23 - Enter yard_get_info",drvthis->name);
	PrivateData *p = (PrivateData *)drvthis->private_data;
	
	strcpy(p->info, "Socket-based driver for Y.A.R.D.2 USB LCD");
	return p->info;
}
