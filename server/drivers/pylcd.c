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
	debug(RPT_DEBUG, "%s: read %s", __FUNCTION__, buffer);
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

/* Send the input as telegramm. Depending on value for wfack
 this routine waits for an ACK and retransmitts till an ACK is received or
 doesn't expect an ACK.
 */
int
send_tele(PrivateData *p, char *buffer)
{
    char cc=0x00;
    int i, len;
    char buffer2[255];

    sprintf(buffer2,"\2%s\3", buffer);
    len=strlen(buffer2);
    for (i=0; i<len; i++)
        cc ^= buffer2[i];

    buffer2[len]=cc;

    write(p->FD, buffer2, len+1);
    /* tcflush (p->FD, TCIFLUSH); */
    debug(RPT_DEBUG, "%s: sent %s", __FUNCTION__, buffer);

    return 0;
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

    p = (PrivateData *) malloc(sizeof(PrivateData));
    if ((p == NULL) || (drvthis->store_private_ptr(drvthis, p) < 0))
    {
        report(RPT_ERR, "%s: error allocating memory for modules private data", drvthis->name);
        return -1;
    }

    /* Read config file: */

    /* Which serial device should be used? */
    strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, "/dev/lcd"), sizeof(p->device));
    p->device[sizeof(p->device)-1] = '\0';
    report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

    /* open and initialize serial device */
    p->FD = open(p->device, O_RDWR);

    if (p->FD == -1) {
        report(RPT_ERR, "%s: open(%s) failed: %s", drvthis->name, p->device, strerror(errno));
        return -1;
    }
    if (initTTY(drvthis, p->FD) != 0)
        return -1;

    p->timeout.tv_sec = 0;
    p->timeout.tv_usec = MICROTIMEOUT;
    p->width = WIDTH;
    p->height = HEIGHT;
    p->LEDtoggle = 0;
    strcpy(p->last_key_pressed, NOKEY);
    p->last_key_time = timestamp(p);
    p->last_buf_time = timestamp(p);

    /* Acknowledge all telegramms, the device may yet be sending.
       (Reset doesn't clear telegramms, darn protocol ... )
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

    /* Now Reset and clear */

    send_tele(p, "R");
    send_tele(p, "C0101");
    send_tele(p, "D                                ");
    send_tele(p, "C0101");
    send_tele(p, "M3");
    strcpy(p->framebuffer, "D                                ");
    p->FB_modified = 1;

    for (i = 0; i < 7; i++)
    	p->led[i] = 0;
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

/* flushed the content of the framebuffer to the display.
 p->FB_modified tells if the content was modified. If not,
 that means, nothing has to be done
 */
MODULE_EXPORT void 
pyramid_flush (Driver *drvthis)
{
    static char mesg[16];
    PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned long long current_time=timestamp(p);

    if ((p->FB_modified==True) && (current_time>(p->last_buf_time+40000)))
    {
        send_tele(p, "C0101");
        send_tele(p, p->framebuffer); /* We do not wait for the ACK here*/
        p->FB_modified=False;
        p->last_buf_time=current_time;
        sprintf(mesg, "C%02d%02d", p->C_x, p->C_y);
        send_tele(p, mesg);
        sprintf(mesg, "M%d", p->C_state);
        send_tele(p, mesg);
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


/* Extended functions */

/*
MODULE_EXPORT void 
pyramid_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern){};
MODULE_EXPORT void 
pyramid_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern){};
MODULE_EXPORT void 
pyramid_num (Driver *drvthis, int x, int num){};
MODULE_EXPORT void 
pyramid_heartbeat (Driver *drvthis, int state){};
MODULE_EXPORT void 
pyramid_icon (Driver *drvthis, int x, int y, int icon){};
*/

MODULE_EXPORT void 
pyramid_cursor (Driver *drvthis, int x, int y, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->C_x = x;
    p->C_y = y;
    p->C_state = state;
};

/* Userdef characters, are those still supported ? */

/*
MODULE_EXPORT void 
pyramid_set_char (Driver *drvthis, int n, char *dat){};
MODULE_EXPORT int  
pyramid_get_free_chars (Driver *drvthis){return 0;};
MODULE_EXPORT int  
pyramid_cellwidth (Driver *drvthis){return 0;};
MODULE_EXPORT int  
pyramid_cellheight (Driver *drvthis){return 0;};
*/

/* Hardware functions */

/*
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
*/

MODULE_EXPORT void 
pyramid_output (Driver *drvthis, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    int i;

    for (i = 0; i < 7; i++) 
        p->led[i] = state & (1 << i);
    
    set_leds(p);
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

    /* Now we read everything from the display and as long as we got ACKs, we ignore them. */
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

    return NULL;
};


/* Returns a string. Server cannot modify this string. */

/*
MODULE_EXPORT const char *
pyramid_get_info (){return NULL;};
*/


