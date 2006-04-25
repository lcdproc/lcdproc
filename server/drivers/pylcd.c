/* pylcd.c */

/*
 This is the LCDproc driver for the "pyramid" LCD device from Pyramid.

 Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>
 Copyright (C) 2006 coresystems GmbH <info@coresystems.de>
 
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

 */

/* 
 * This driver controls the programmable LC-Display from 
 * Pyramid Computer GmbH. For more information see
 * http://www.pyramid.de/e/produkte/server/pyramid-lcd.php
 * 
 * Contact Thomas Riewe <thomas.riewe@pyramid.de> for further
 * information on the LCD.
 *
 * Changes:
 *  2006-02-20 Stefan Reinauer <stepan@coresystems.de>
 *   - add support for onboard LEDs via "output" command
 *  2006-04-12 Stefan Reinauer <stepan@coresystems.de>
 *   - add support for user defined characters (set_char/get_free_chars)
 *   - add support for vbars/hbars, get_info, reorganize pyramid_init()
 *   - fixed a timing bug and implemented escaping in (real_)send_tele()
 *  2006-04-18 Stefan Reinauer <stepan@coresystems.de>
 *   - support combined key events
 *   - add more custom characters
 *   - fix german umlauts
 *   - fix cursor handling
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "lcd.h"
#include "pylcd.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd_lib.h"
#include "report.h"

#define min(a, b) ((a)<(b) ? (a) : (b))
#define True 1
#define False 0


#define MICROTIMEOUT 50000
#define NOKEY "00000"


/* variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "pyramid_";

/* Prototypes: */

int send_ACK(PrivateData *);

/* local functions for pylcd.c */

/* Performs an select on the device used. Returns if no data is available at the
 moment. Used to keep reading-operations from blocking the driver.
 */
int
data_ready(PrivateData *p)
{
    FD_ZERO(&p->rdfs);
    FD_SET(p->FD, &p->rdfs);
    return select(p->FD+1, &p->rdfs, NULL, NULL, &p->timeout)>0;
}

/* Reads one telegramm, stores the telegramm without ETX/STX in buffer
 returns True on successful detection of a telegram, False if nothing was read
 or the telegramm didn't match its CC or if MAXCOUNT was exeeded without
 reading a complete telegramm.
 */
int
read_tele(PrivateData *p, char *buffer)
{
    char zeichen=0;
    int len=0;
    char cc=0x00;

    while (data_ready(p)
           && (read(p->FD, &zeichen, 1)>0)
           && (zeichen!=0x02)
           && (len<MAXCOUNT))
        len++;

    if (zeichen!=0x02)
    {
        memset(buffer, MAXCOUNT, 0);
        return False;
    }

    cc ^= zeichen;
    len=0;

    while (data_ready(p)
           && (read(p->FD, &zeichen, 1)>0)
           && (len<MAXCOUNT+1))
    {
        buffer[len]=zeichen;
        cc ^= zeichen;
        if (zeichen==0x03) break;
        len++;
    }

    if (data_ready(p)
        && (read(p->FD, &zeichen, 1)>0)
        && (buffer[len]==0x03)
        && (zeichen==cc))
    {
        buffer[len]=0x00;
	/*debug(RPT_DEBUG, "%s: read %s", __FUNCTION__, buffer)*/;
        return True;
    }
    else
    {
        memset(buffer, MAXCOUNT, 0);
        return False;
    }
}

/* Wrapper for reading an acknowledge telegramm.
 */
int
read_ACK(PrivateData *p)
{
    char buffer[6];
    int retval=read_tele(p, buffer);

    return (retval && buffer[0]=='Q');
}

/* Send the input as telegramm. 
 * 
 * The telegram buffer just contains the raw telegram data
 *  - It shall not contain <STX> and <ETX> marks
 *  - It may contain bytes below 0x20, they are automatically escaped by
 *    real_sent_tele
 *    
 * NOTE: This function does not wait for any ACKs.
 */
int
real_send_tele(PrivateData *p, char *buffer, int len)
{
    char cc=0x00;
    int i, j;
    char buffer2[255];

    i=0; j=0; 
    buffer2[j++]=2; // emit <STX>

    /* copy the whole telegram package and escape
     * characters below 0x20.
     * ie. 0x8 --> <ESC> 0x28
     */
    
    while (len--) {
        if (buffer[i]<0x20) {
            buffer2[j++]=0x1b;
            buffer2[j++]=buffer[i++]+0x20;
        } else {
            buffer2[j++]=buffer[i++];
        }
    }
    buffer2[j++]=3; // emit <ETX> 
    len=j;	    // new package length
    
    /* calculate <BCC> over all bytes */
    for (i=0; i<len; i++)
        cc ^= buffer2[i];

    buffer2[len++]=cc;

    write(p->FD, buffer2, len);
    /* tcflush (p->FD, TCIFLUSH); */

    /* Take a little nap. This works as a pacemaker */
    usleep(50);
    
    /*debug(RPT_DEBUG, "%s: sent %s", __FUNCTION__, buffer);*/

    return 0;
}

/* If we know what our telegrams look like, we may use
 * send_tele instead of real_send_tele.
 * NOTE: This only works when there are no NULL bytes
 * in the telegram
 */

int
send_tele(PrivateData *p, char *buffer)
{
    return real_send_tele(p, buffer, strlen(buffer));
}

/* Wrapper for sending ACKs via real_send_tele
 */
int
send_ACK(PrivateData *p)
{
    return send_tele(p, "Q");
}

unsigned long long
timestamp(PrivateData *p)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec*1000000+(unsigned long long)tv.tv_usec;
}


/* Sets the serial device, used for communication with the LCD, into raw mode
 */
int
initTTY(Driver *drvthis, int FD)
{
    struct termios tty_mode;

    if (tcgetattr(FD, &tty_mode) == 0) {
        cfmakeraw(&tty_mode);
#ifdef CBAUDEX /* CBAUDEX not defined in FreeBSD */
        tty_mode.c_cflag |= CBAUDEX;
#endif
        cfsetospeed(&tty_mode, B115200);
        cfsetispeed(&tty_mode, 0);
        tty_mode.c_cc[VMIN] = 1;
        tty_mode.c_cc[VTIME] = 1;

        if (tcsetattr(FD, TCSANOW, &tty_mode) != 0) {
            report(RPT_ERR, "%s: setting TTY failed: %s", drvthis->name, strerror(errno));
            return -1;
        }
    } else {
        report(RPT_ERR, "%s: reading TTY failed: %s", drvthis->name, strerror(errno));
        return -1;
    }

    return 0;
}

/* This function sets all LEDs according to the private data structure */

int
set_leds(PrivateData *p)
{
    int i;
    char tele[3]="L00";

    for (i=0; i<7; i++) {
      tele[1]=i+'1';
      tele[2]=p->led[i]?'1':'0';
      send_tele(p, tele);
    }

    return 0;
}

/* Driver functions according to current API-Version */

/* Basic functions */

MODULE_EXPORT int  
pyramid_init (Driver *drvthis, char *args)
{
    char buffer[6]="";
    int i;
    PrivateData *p;

    /* get memory for private data */
    
    p = (PrivateData *) malloc(sizeof(PrivateData));
    if ((p == NULL) || (drvthis->store_private_ptr(drvthis, p) < 0))
    {
        report(RPT_ERR, "%s: error allocating memory for modules private data", drvthis->name);
        return -1;
    }

    /* fill static elements of private data */
    p->width = WIDTH;
    p->height = HEIGHT;
    p->customchars=CUSTOMCHARS;
    p->cellwidth=CELLWIDTH;
    p->cellheight=CELLHEIGHT;
    p->custom = normal;
    
    strcpy(p->last_key_pressed, NOKEY);
    p->last_key_time = timestamp(p);
    p->last_buf_time = timestamp(p);

    p->timeout.tv_sec = 0;
    p->timeout.tv_usec = MICROTIMEOUT;
    
    strcpy(p->framebuffer, "D                                ");
    p->FB_modified = 1;

    /* read config file, fill configuration 
     * dependent elements of private data
     */

    /* Which serial device should be used? */
    strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, "/dev/lcd"), sizeof(p->device));
    p->device[sizeof(p->device)-1] = '\0';
    report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

    /* Initialize connection to the LCD  */
    
    /* open and initialize serial device */
    p->FD = open(p->device, O_RDWR);

    if (p->FD == -1) {
        report(RPT_ERR, "%s: open(%s) failed: %s", drvthis->name, p->device, strerror(errno));
        return -1;
    }
    
    if (initTTY(drvthis, p->FD) != 0)
        return -1;
    
    /* Acknowledge all telegramms, the device may yet be sending.
     * (Reset doesn't clear telegramms, darn protocol ... )
     */

    tcflush(p->FD, TCIFLUSH); /* clear everything */
    
    while (1) {
        i = read_tele(p, buffer);
        if (i == True)
            send_ACK(p);
        else
            break;
        usleep(600000);
    }

    /* Initialize the display hardware: reset, clear and set cursor shape */

    send_tele(p, "R");
    send_tele(p, "C0101");
    send_tele(p, "D                                ");
    send_tele(p, "C0101");
    send_tele(p, "M3");

    /* hardware selftest + clear all LEDs */
    for (i = 0; i < 7; i++) {
    	p->led[i?i-1:0] = 0;
	p->led[i] = 1;
    	set_leds(p);
	usleep(10000);
    }
    for (i = 6; i >= 0; i--) {
    	p->led[i+1] = 0;
	p->led[i] = 1;
    	set_leds(p);
	usleep(10000);
    }
    for (i = 0; i < 7; i++) {
    	p->led[i] = 0;
    }
    set_leds(p);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
};

MODULE_EXPORT void 
pyramid_close (Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    close(p->FD);
};

MODULE_EXPORT int  
pyramid_width (Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    return p->width;
};

MODULE_EXPORT int  
pyramid_height (Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    return p->height;
};

MODULE_EXPORT void 
pyramid_clear (Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->FB_modified=1;
    strcpy(p->framebuffer, "D                                ");
};

/* flush the content of the framebuffer to the display.
 * p->FB_modified tells if the content was modified. If not,
 * that means, nothing has to be done
 */
MODULE_EXPORT void 
pyramid_flush (Driver *drvthis)
{
    static char mesg[33];
    PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned long long current_time=timestamp(p);
    int i;

    if ((p->FB_modified==True) && (current_time>(p->last_buf_time+40000)))
    {
	memcpy(mesg, p->framebuffer, 33);

	// we got the japanese HD44780U, so convert the german umlauts and
	// other stuff
	for (i=1; i<33; i++) {
		switch ((unsigned char)mesg[i]) {
		// assume input is iso_8859-1
		case 0xe4: mesg[i]=0xe1; break; // ä
		case 0xf6: mesg[i]=0xef; break; // ö
		case 0xfc: mesg[i]=0xf5; break; // ü
		// This makes the display show nothing
		// though it is correct according the HD44780U DS
		//case 0xdf: mesg[i]=0xe2; break; // ß
		case 0xb7: mesg[i]=0xa5; break; // ·
		case 0xb0: mesg[i]=0xdf; break; // °
		}
	}
	
        send_tele(p, "C0101");
        real_send_tele(p, mesg, 33); /* We do not wait for the ACK here*/
        p->FB_modified=False;
        p->last_buf_time=current_time;
        sprintf(mesg, "C%02d%02d", p->C_x, p->C_y);
        real_send_tele(p, mesg,5);
        sprintf(mesg, "M%d", p->C_state);
        real_send_tele(p, mesg,2);
    }
}

MODULE_EXPORT void 
pyramid_string (Driver *drvthis, int x, int y, char *args)
{
    int offset;
    int len;
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->FB_modified = True;
    x = min(p->width, x);
    y = min(p->height, y);
    offset = (x)+p->width*(y-1);
    len = min(strlen(args), p->width*p->height-offset+1);
    memcpy(&p->framebuffer[offset], args, len);
};

MODULE_EXPORT void 
pyramid_chr (Driver *drvthis, int x, int y, char c)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->FB_modified = True;
    x = min(p->width, x);
    y = min(p->height, y);
    p->framebuffer[x+p->width*(y-1)]=c;
};


/* User defined characters */

MODULE_EXPORT void pyramid_set_char (Driver *drvthis, int n, char *dat)
{
	char tele[10] = "G@ABCDEFGH";
	int row, column, pixels;
	
	PrivateData *p = (PrivateData *) drvthis->private_data;
	

	if (n<0 && n>15) {
		debug(RPT_DEBUG, "only characters 0-15 can be changed.\n");
		return;
	}
	
	if (!dat) {
		debug(RPT_DEBUG, "no character data\n");
		return;
	}

	// which character?
	tele[1]=n+0x40;
	
	for (row = 0; row < p->cellheight; row++) {
		pixels=0; 
		for (column = 0; column < p->cellwidth; column++) {
			pixels <<= 1;
			pixels |= (dat[(row * p->cellwidth) + column] != 0);
		}
		pixels |= 0x40; // pixel information is transferred with
				// an offset of 40h
		
		tele[row+2]=pixels;
	}
        real_send_tele(p, tele, 10);
};

MODULE_EXPORT int  pyramid_get_free_chars (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return (p->customchars);
};


MODULE_EXPORT int  pyramid_cellwidth (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return (p->cellwidth);
};

MODULE_EXPORT int  pyramid_cellheight (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return (p->cellheight);
};



// Sets up for vertical bars.  Call before pyramid->vbar()

static void
pyramid_init_vbar (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
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
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->custom != vbar) {
		pyramid_set_char (drvthis, 1, a);
		pyramid_set_char (drvthis, 2, b);
		pyramid_set_char (drvthis, 3, c);
		pyramid_set_char (drvthis, 4, d);
		pyramid_set_char (drvthis, 5, e);
		pyramid_set_char (drvthis, 6, f);
		pyramid_set_char (drvthis, 7, g);
		p->custom = vbar;
	}
}

// Inits horizontal bars...
static void
pyramid_init_hbar (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	char a[] = {
		1, 0, 0, 0, 0,
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
		1, 1, 1, 1, 0,
	};
	char e[] = {
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (p->custom != hbar) {
		pyramid_set_char (drvthis, 1, a);
		pyramid_set_char (drvthis, 2, b);
		pyramid_set_char (drvthis, 3, c);
		pyramid_set_char (drvthis, 4, d);
		pyramid_set_char (drvthis, 5, e);
		p->custom = hbar;
	}
}

static void
pyramid_init_custom1 (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
	};

	char b[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 0,
		1, 1, 1, 0, 0,
		1, 1, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	
	char c[] = {
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	};

	char d[] = {
		1, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	};
	
	if (p->custom != custom1) {
		pyramid_set_char (drvthis, 1, a);
		pyramid_set_char (drvthis, 2, b);
		pyramid_set_char (drvthis, 3, c);
		pyramid_set_char (drvthis, 4, d);
		p->custom = custom1;
	}
}



// Draws a vertical bar...

MODULE_EXPORT void
pyramid_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	pyramid_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

// Draws a horizontal bar to the right.

MODULE_EXPORT void
pyramid_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	pyramid_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


MODULE_EXPORT int
pyramid_icon (Driver *drvthis, int x, int y, int icon)
{
	char icons[8][5 * 8] = {
		{
		 1, 1, 1, 1, 1,			  // Empty Heart
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1,			  // Filled Heart
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },
		
		{
                 0, 0, 1, 0, 0, 		  // Arrow up 
                 0, 1, 1, 1, 0,
                 1, 0, 1, 0, 1,
                 0, 0, 1, 0, 0,
                 0, 0, 1, 0, 0,
                 0, 0, 1, 0, 0,
                 0, 0, 1, 0, 0,
                 0, 0, 0, 0, 0,
                 },
		
                {
                 0, 0, 1, 0, 0, 		  // Arrow Down
                 0, 0, 1, 0, 0,
                 0, 0, 1, 0, 0,
                 0, 0, 1, 0, 0,
                 1, 0, 1, 0, 1,
                 0, 1, 1, 1, 0,
                 0, 0, 1, 0, 0,
                 0, 0, 0, 0, 0,
                 },
		
                {
                 0, 0, 0, 0, 0,			  // Checkbox off 
                 0, 0, 0, 0, 0,
                 1, 1, 1, 1, 1,
                 1, 0, 0, 0, 1,
                 1, 0, 0, 0, 1,
                 1, 0, 0, 0, 1,
                 1, 1, 1, 1, 1,
                 0, 0, 0, 0, 0,
                 },
		
                {
                 0, 0, 1, 0, 0, 		  // Checkbox on
                 0, 0, 1, 0, 0,
                 1, 1, 1, 0, 1,
                 1, 0, 1, 1, 0,
                 1, 0, 1, 0, 1,
                 1, 0, 0, 0, 1,
                 1, 1, 1, 1, 1,
                 0, 0, 0, 0, 0,
                 },
		
                {
                 0, 0, 0, 0, 0,			  // Checkbox gray 
                 0, 0, 0, 0, 0,
                 1, 1, 1, 1, 1,
                 1, 0, 1, 0, 1,
                 1, 1, 0, 1, 1,
                 1, 0, 1, 0, 1,
                 1, 1, 1, 1, 1,
                 0, 0, 0, 0, 0,
                 },
		
		{
		 0, 0, 0, 0, 0,			  // Ellipsis
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1,
		 },
	};

	PrivateData *p = (PrivateData *) drvthis->private_data;
#if 0

	// notify the others that we messed up their character set.
	// Unused for heartbeats as we should always leave that icon alone.
	// 
	// Leaving this in the code as notification for other similar cases
	if (p->custom == bign) {
		printf("Switching to beat\n");
		p->custom = beat;
	}
#endif

	switch( icon ) {
		case ICON_BLOCK_FILLED:
			pyramid_chr( drvthis, x, y, 255 );
			break;
			
		case ICON_HEART_OPEN:
			pyramid_set_char( drvthis, 0, icons[0] );
			pyramid_chr( drvthis, x, y, 0 );
			break;

		case ICON_HEART_FILLED:
			pyramid_set_char( drvthis, 0, icons[1] );
			pyramid_chr( drvthis, x, y, 0 );
			break;
			
		case ICON_ARROW_UP:
			pyramid_set_char( drvthis, 2, icons[2] );
			pyramid_chr( drvthis, x, y, 2 );
			p->custom = icon;
			break;
			
		case ICON_ARROW_DOWN:
			pyramid_set_char( drvthis, 3, icons[3] );
			pyramid_chr( drvthis, x, y, 3 );
			p->custom = icon;
			break;

		case ICON_ARROW_LEFT:
			pyramid_chr( drvthis, x, y, '\177' );
			break;
			
		case ICON_ARROW_RIGHT:
			pyramid_chr( drvthis, x, y, '\176' );
			break;

		case ICON_CHECKBOX_ON:
			pyramid_set_char( drvthis, 10, icons[4] );
			pyramid_chr( drvthis, x, y, 10 );
			break;

		case ICON_CHECKBOX_OFF:
			pyramid_set_char( drvthis, 11, icons[5] );
			pyramid_chr( drvthis, x, y, 11 );
			break;
			
		case ICON_CHECKBOX_GRAY:
			pyramid_set_char( drvthis, 12, icons[6] );
			pyramid_chr( drvthis, x, y, 12 );
			break;
			
		case ICON_ELLIPSIS:
			pyramid_set_char( drvthis, 13, icons[7] );
			pyramid_chr( drvthis, x, y, 13 );
			break;

		default:
			printf("x=%d, y=%d, icon=%x\n", x,y,icon);
			return -1;
	}
	return 0;
}

/*
MODULE_EXPORT void 
pyramid_num (Driver *drvthis, int x, int num){};
MODULE_EXPORT void 
pyramid_heartbeat (Driver *drvthis, int state){};
*/

MODULE_EXPORT void 
pyramid_cursor (Driver *drvthis, int x, int y, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->C_x = x;
    p->C_y = y;
    switch (state) {
    case CURSOR_OFF:
	state=0;
	break;
    case CURSOR_DEFAULT_ON:
	state=3;
	break;
    case CURSOR_UNDER:
	state=1;
	break;
    case CURSOR_BLOCK:
	state=2;
	break;
    }
    p->C_state = state;
};

/* Hardware functions */

#if 0
// all of these are not supported by the display (data sheet)
MODULE_EXPORT int  
pyramid_get_contrast (Driver *drvthis){return 0;};
MODULE_EXPORT void 
pyramid_set_contrast (Driver *drvthis, int promille){};
MODULE_EXPORT int  
pyramid_get_brightness (Driver *drvthis, int state){return 0;};
MODULE_EXPORT void 
pyramid_set_brightness (Driver *drvthis, int state, int promille){};
MODULE_EXPORT void 
pyramid_backlight (Driver *drvthis, int on)
#endif

MODULE_EXPORT void 
pyramid_output (Driver *drvthis, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    int i;

    for (i = 0; i < 7; i++) 
        p->led[i] = state & (1 << i);
    
    set_leds(p);

    if(state & (1 << 8)) {
	    pyramid_init_custom1(drvthis);
    }
};

/* Key functions */

MODULE_EXPORT const char *
pyramid_get_key (Driver *drvthis)
{
    /* supports only one key at a time */

    static char buffer[MAXCOUNT];
    unsigned long long current_time;
    int retval;
    PrivateData *p = (PrivateData *) drvthis->private_data;

    /* Now we read everything from the display,
     * and as long as we got ACKs, we ignore them. 
     * (eat up all pending ACKs)
     */
    
    while (1) {
        retval = read_tele(p, buffer);
        if ((retval == False) || (buffer[0] != 'Q')) break;
    }
    if (retval == False)
        strcpy(buffer, p->last_key_pressed);
    else
        send_ACK(p);

    /* If a key wasn't released yet it may be released now. */
    if (buffer[0] == 'K')
    {
        /* test if its a release event */
        if (   (strcmp(buffer, "K0003") == 0)
            || (strcmp(buffer, "K0030") == 0)
            || (strcmp(buffer, "K0300") == 0)
            || (strcmp(buffer, "K3000") == 0))
        {
            debug(RPT_DEBUG, "%s: Key released: %s", __FUNCTION__, p->last_key_pressed);
            strcpy(p->last_key_pressed, NOKEY);
            return NULL;
        }
        else /* It must be a new key event */
        {
            strcpy(p->last_key_pressed, buffer);
            debug(RPT_DEBUG, "%s: Key pressed: %s", __FUNCTION__, p->last_key_pressed);
        }
    }
    /* If no keys are pressed at this time, we are done. */
    if (p->last_key_pressed[0] == NOKEY[0])
        return NULL;

    current_time = timestamp(p);
    if (current_time > p->last_key_time + 500000) /* (buffer[0]=='K' ? 500000 : 250000)) */
        p->last_key_time = current_time;
    else
        return NULL;

    if (strcmp(p->last_key_pressed, "K0001") == 0) /* first from left */
        return "Up";
    if (strcmp(p->last_key_pressed, "K0010") == 0) /* second from left */
        return "Down";
    if (strcmp(p->last_key_pressed, "K0100") == 0) /* third from left */
        return "Enter";
    if (strcmp(p->last_key_pressed, "K1000") == 0) /* last from left */
        return "Escape";
    
#ifdef PYRAMID_DECODE_COMBINED_KEYPRESSES
    /* Do we really want to type that much */
    if (strcmp(p->last_key_pressed, "K0012") == 0) /* A+B */
        return "Up+Down";
    if (strcmp(p->last_key_pressed, "K0021") == 0) /* B+A */
        return "Down+Up";

    if (strcmp(p->last_key_pressed, "K0102") == 0) /* A+C */
        return "Up+Enter";
    if (strcmp(p->last_key_pressed, "K0201") == 0) /* C+A */
        return "Enter+Up";
    if (strcmp(p->last_key_pressed, "K1002") == 0) /* A+D */
        return "Up+Escape";
    if (strcmp(p->last_key_pressed, "K2001") == 0) /* D+A */
        return "Escape+Up";
    
    if (strcmp(p->last_key_pressed, "K0120") == 0) /* B+C */
        return "Down+Enter";
    if (strcmp(p->last_key_pressed, "K0210") == 0) /* C+B */
        return "Enter+Down";
    if (strcmp(p->last_key_pressed, "K1020") == 0) /* B+D */
        return "Down+Escape";
    if (strcmp(p->last_key_pressed, "K2010") == 0) /* D+B */
        return "Escape+Down";
 
    if (strcmp(p->last_key_pressed, "K0012") == 0) /* C+D */
        return "Enter+Escape";
    if (strcmp(p->last_key_pressed, "K0021") == 0) /* D+C */
        return "Escape+Enter";
#endif

    return NULL; // Ignore combined key events
};


/* Returns a string. Server cannot modify this string. */
MODULE_EXPORT const char *
pylcd_get_info (Driver *drvthis)
{
    static char *pylcd_info_string="Pyramid LCD driver";

    return pylcd_info_string;
};


