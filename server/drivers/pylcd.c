/* pylcd.c */

/*
 This is the LCDproc driver for the "pylcd" device from Pyramid.

 Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>

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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "lcd.h"
#include "pylcd.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define min(a, b) ((a)<(b) ? (a) : (b))
#define True 1
#define False 0

#define DEBUG 0

#define MICROTIMEOUT 50000
#define NOKEY "00000"

/* Prototypes: */

int send_ACK(pylcd_private_data *);

/* local functions for pylcd.c */

/* Performs an select on the device used. Returns if no data is available at the
 moment. Used to keep reading-operations from blocking the driver.
 */
int data_ready(pylcd_private_data *status)
{
    FD_ZERO(&status->rdfs);
    FD_SET(status->FD, &status->rdfs);
    return select(status->FD+1, &status->rdfs, NULL, NULL, &status->timeout)>0;
}

/* Reads one telegramm, stores the telegramm without ETX/STX in buffer
 returns True on successful detection of a telegram, False if nothing was read
 or the telegramm didn't match its CC or if MAXCOUNT was exeeded without
 reading a complete telegramm.
 */
int read_tele(pylcd_private_data *status, char *buffer)
{
    char zeichen=0;
    int len=0;
    char cc=0x00;

    while (data_ready(status)
           && (read(status->FD, &zeichen, 1)>0)
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

    while (data_ready(status)
           && (read(status->FD, &zeichen, 1)>0)
           && (len<MAXCOUNT+1))
    {
        buffer[len]=zeichen;
        cc ^= zeichen;
        if (zeichen==0x03) break;
        len++;
    }

    if (data_ready(status)
        && (read(status->FD, &zeichen, 1)>0)
        && (buffer[len]==0x03)
        && (zeichen==cc))
    {
        buffer[len]=0x00;
        if (DEBUG) printf("DEBUG: read %s\n", buffer);
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
int read_ACK(pylcd_private_data *status)
{
    char buffer[6];
    int retval=read_tele(status, buffer);

    return (retval && buffer[0]=='Q');
}

/* Send the input as telegramm. Depending on value for wfack
 this routine waits for an ACK and retransmitts till an ACK is received or
 doesn't expect an ACK.
 */
int send_tele(pylcd_private_data *status, char *buffer)
{
    char cc=0x00;
    int i, len;
    char buffer2[255];

    sprintf(buffer2,"\2%s\3", buffer);
    len=strlen(buffer2);
    for (i=0; i<len; i++)
        cc ^= buffer2[i];

    buffer2[len]=cc;

    write(status->FD, buffer2, len+1);
    /* tcflush (status->FD, TCIFLUSH); */
    if (DEBUG) printf("DEBUG: sent: %s\n", buffer);

    return 0;
}

/* Wrapper for sending ACKs via real_send_tele
 */
int send_ACK(pylcd_private_data *status)
{
    return send_tele(status, "Q");
}

unsigned long long timestamp(pylcd_private_data *status)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec*1000000+(unsigned long long)tv.tv_usec;
}


/* Sets the serial device, used for communication with the LCD, into raw mode
 */
int initTTY(int FD)
{
    struct termios tty_mode;

    if (tcgetattr(FD, &tty_mode) == 0) {
        cfmakeraw(&tty_mode);
        tty_mode.c_cflag |= CBAUDEX;
        cfsetospeed(&tty_mode, B115200);
        cfsetispeed(&tty_mode, 0);
        tty_mode.c_cc[VMIN] = 1;
        tty_mode.c_cc[VTIME] = 1;

        if (tcsetattr(FD, TCSANOW, &tty_mode) != 0) {
            perror("Setting TTY failed");
            return -1;
        }
    } else {
        perror("Reading TTY faild");
        return -1;
    }

    return 0;
}

/* Driver functions according to current API-Version */

/* Basic functions */

MODULE_EXPORT int  pylcd_init (Driver *drvthis, char *args)
{
    char buffer[6]="";
    int i;
    pylcd_private_data *status;

    status=(pylcd_private_data *) malloc(sizeof(pylcd_private_data));
    if ((status==NULL) || (drvthis->store_private_ptr(drvthis, status)<0))
    {
        printf("Error allocating memory for modules private data\n");
        return -1;
    }

    /* Read config file: */

    /* Which serial device should be used? */
    strncpy(status->devicename, drvthis->config_get_string ( drvthis->name , "Device" , 0 , "/dev/lcd"),sizeof(status->devicename));
    status->devicename[sizeof(status->devicename)-1]=0;
    printf("pylcd: Using device: %s\n", status->devicename);

    /* open and initialize serial device */
    status->FD=open(status->devicename, O_RDWR);

    if (status->FD==-1)
    {
        perror("Opening device failed");
        return -1;
    }
    if (initTTY(status->FD)!=0)
        return -1;

    status->timeout.tv_sec=0;
    status->timeout.tv_usec=MICROTIMEOUT;
    status->width=WIDTH;
    status->height=HEIGHT;
    status->LEDtoggle=0;
    strcpy(status->last_key_pressed, NOKEY);
    status->last_key_time = timestamp(status);
    status->last_buf_time = timestamp(status);


    /* Acknowledge all telegramms, the device may yet be sending.
       (Reset doesn't clear telegramms, darn protocol ... )
       */

    tcflush (status->FD, TCIFLUSH); /* clear everything */
    while (1)
    {
        i=read_tele(status, buffer);
        if (i==True)
            send_ACK(status);
        else
            break;
        usleep(600000);
    }

    /* Now Reset and clear */

    send_tele(status, "R");
    send_tele(status, "C0101");
    send_tele(status, "D                                ");
    send_tele(status, "C0101");
    send_tele(status, "M3");
    strcpy(status->framebuffer, "D                                ");
    status->FB_modified=1;

    return 0;
};

MODULE_EXPORT void pylcd_close (Driver *drvthis)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    close(status->FD);
};

MODULE_EXPORT int  pylcd_width (Driver *drvthis)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    return status->width;
};

MODULE_EXPORT int  pylcd_height (Driver *drvthis)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    return status->height;
};

MODULE_EXPORT void pylcd_clear (Driver *drvthis)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    status->FB_modified=1;
    strcpy(status->framebuffer, "D                                ");
};

/* flushed the content of the framebuffer to the display.
 status->FB_modified tells if the content was modified. If not,
 that means, nothing has to be done
 */
MODULE_EXPORT void pylcd_flush (Driver *drvthis)
{
    static char mesg[16];
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;
    unsigned long long current_time=timestamp(status);

    if ((status->FB_modified==True) && (current_time>(status->last_buf_time+40000)))
    {
        send_tele(status, "C0101");
        send_tele(status, status->framebuffer); /* We do not wait for the ACK here*/
        status->FB_modified=False;
        status->last_buf_time=current_time;
        sprintf(mesg, "C%02d%02d", status->C_x, status->C_y);
        send_tele(status, mesg);
        sprintf(mesg, "M%d", status->C_state);
        send_tele(status, mesg);
    }
}

MODULE_EXPORT void pylcd_string (Driver *drvthis, int x, int y, char *args)
{
    int offset;
    int len;
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    status->FB_modified=True;
    x=min(status->width, x);
    y=min(status->height, y);
    offset=(x)+status->width*(y-1);
    len=min(strlen(args), status->width*status->height-offset+1);
    memcpy(&status->framebuffer[offset], args, len);
};

MODULE_EXPORT void pylcd_chr (Driver *drvthis, int x, int y, char c)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    status->FB_modified=True;
    x=min(status->width, x);
    y=min(status->height, y);
    status->framebuffer[x+status->width*(y-1)]=c;

};


/* Extended functions */

/*
MODULE_EXPORT void mydriver_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern){};
MODULE_EXPORT void mydriver_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern){};
MODULE_EXPORT void mydriver_num (Driver *drvthis, int x, int num){};
MODULE_EXPORT void mydriver_heartbeat (Driver *drvthis, int state){};
MODULE_EXPORT void mydriver_icon (Driver *drvthis, int x, int y, int icon){};
*/

MODULE_EXPORT void pylcd_cursor (Driver *drvthis, int x, int y, int state)
{
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    status->C_x=x;
    status->C_y=y;
    status->C_state=state;
};

/* Userdef characters, are those still supported ? */

/*
MODULE_EXPORT void mydriver_set_char (Driver *drvthis, int n, char *dat){};
MODULE_EXPORT int  mydriver_get_free_chars (Driver *drvthis){return 0;};
MODULE_EXPORT int  mydriver_cellwidth (Driver *drvthis){return 0;};
MODULE_EXPORT int  mydriver_cellheight (Driver *drvthis){return 0;};
*/

/* Hardware functions */

/*
MODULE_EXPORT int  mydriver_get_contrast (Driver *drvthis){return 0;};
MODULE_EXPORT void mydriver_set_contrast (Driver *drvthis, int promille){};
MODULE_EXPORT int  mydriver_get_brightness (Driver *drvthis, int state){return 0;};
MODULE_EXPORT void mydriver_set_brightness (Driver *drvthis, int state, int promille){};
MODULE_EXPORT void mydriver_backlight (Driver *drvthis, int on)
MODULE_EXPORT void mydriver_output (Driver *drvthis, int state){};
*/

/* Key functions */

MODULE_EXPORT char *pylcd_get_key (Driver *drvthis)
{
    /* supports only one key at a time */

    static char buffer[MAXCOUNT];
    unsigned long long current_time;
    int retval;
    pylcd_private_data *status = (pylcd_private_data *) drvthis->private_data;

    /* Now we read everything from the display and as long as we got ACKs, we ignore them. */
    while (1)
    {
        retval=read_tele(status, buffer);
        if ((retval==False) || (buffer[0]!='Q')) break;
    }
    if (retval==False)
        strcpy(buffer, status->last_key_pressed);
    else
        send_ACK(status);

    /* If a key wasn't released yet it may be released now. */
    if (buffer[0]=='K')
    {
        /* test if its a release event */
        if (   (strcmp(buffer, "K0003")==0)
            || (strcmp(buffer, "K0030")==0)
            || (strcmp(buffer, "K0300")==0)
            || (strcmp(buffer, "K3000")==0))
        {
            if (DEBUG) printf("DEBUG: Key released: %s\n", status->last_key_pressed);
            strcpy(status->last_key_pressed, NOKEY);
            return NULL;
        }
        else /* It must be a new key event */
        {
            strcpy(status->last_key_pressed, buffer);
            if (DEBUG) printf("DEBUG: Key pressed: %s\n", status->last_key_pressed);
        }
    }
    /* If no keys are pressed at this time, we are done. */
    if (status->last_key_pressed[0]==NOKEY[0])
        return NULL;

    current_time = timestamp(status);
    if (current_time>status->last_key_time+500000) /* (buffer[0]=='K' ? 500000 : 250000)) */
        status->last_key_time=current_time;
    else
        return NULL;

    if (strcmp(status->last_key_pressed, "K0001")==0) /* first from left */
        return "Up";
    if (strcmp(status->last_key_pressed, "K0010")==0) /* second from left */
        return "Down";
    if (strcmp(status->last_key_pressed, "K0100")==0) /* third from left */
        return "Enter";
    if (strcmp(status->last_key_pressed, "K1000")==0) /* last from left */
        return "Escape";

    return NULL;
};


/* Returns a string. Server cannot modify this string. */

/*
MODULE_EXPORT char *mydriver_get_info (){return NULL;};
*/


