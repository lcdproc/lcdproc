/** \file server/drivers/irtrans.c
 * LCDd \c irtrans driver for IRTrans VFD displays.
 */

/*
 * irtrans driver
 *
 * Copyright (C) 1998-2007 <info@irtrans.de>
 *
 * Copyright (C) 2007 Phant0m <phantom@netkeke.com>
 * porting the LCDproc 0.4.3 code to LCDproc 0.5.1
 *
 * Inspired by:
 *  TextMode driver (LCDproc authors)
 *  irtrans driver (Irtrans)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lcd.h"
#include "irtrans.h"
#include "irtrans_remote.h"
#include "irtrans_network.h"
#include "irtrans_errcode.h"
#include "report.h"

// Variables

/* This is necessary for Solaris <= Solaris 9 */
#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

/** private data for the \c irtrans driver */
typedef struct irtrans_private_data {
    int width;
    int height;
    int socket;
    int timeout;
    long last_time;
    byte backlight;
    int has_backlight;
    char hostname[256];
    char *framebuf;
    char *shadow_buf;
} PrivateData;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "irtrans_";

//////////////////////////////////////////////////////////////////////////
////////////////////// For Irtrans Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

int InitClientSocket(char host[], SOCKET *sock, unsigned long id);
int SendCommand(Driver *drvthis, LCDCOMMAND *com, STATUSBUFFER *stat);


MODULE_EXPORT int irtrans_init(Driver *drvthis)
{
    PrivateData *p;
    char buf[256];

    /* Allocate and store private data */
    p = (PrivateData *) calloc(1, sizeof(PrivateData));
    if (p == NULL)
        return -1;
    if (drvthis->store_private_ptr(drvthis, p))
        return -1;

    /* initialize private data */
    p->has_backlight =
        drvthis->config_get_bool(drvthis->name, "Backlight", 0, 0);
    report(RPT_INFO, "%s: Backlight %d", drvthis->name, p->backlight);

    strncpy(p->hostname,
            drvthis->config_get_string(drvthis->name, "Hostname", 0,
                                       IRTRANS_DEFAULT_HOSTNAME),
            sizeof(p->hostname));
    p->hostname[sizeof(p->hostname) - 1] = '\0';
    report(RPT_INFO, "%s: Hostname is %s", drvthis->name, p->hostname);

    // Set display sizes
    if ((drvthis->request_display_width() > 0)
        && (drvthis->request_display_height() > 0)) {
        // Use size from primary driver
        p->width = drvthis->request_display_width();
        p->height = drvthis->request_display_height();
    } else {
        /* Use our own size from config file */
        strncpy(buf,
                drvthis->config_get_string(drvthis->name, "Size", 0,
                                           IRTRANS_DEFAULT_SIZE),
                sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';
        if ((sscanf(buf, "%dx%d", &p->width, &p->height) != 2)
            || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
            || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
            report(RPT_WARNING,
                   "%s: cannot read Size: %s; using default %s",
                   drvthis->name, buf, IRTRANS_DEFAULT_SIZE);
            sscanf(IRTRANS_DEFAULT_SIZE, "%dx%d", &p->width, &p->height);
        }
    }

    // Allocate the framebuffer and shadow buffer
    p->framebuf = malloc(p->width * p->height);
    p->shadow_buf = malloc(p->width * p->height);
    if (p->framebuf == NULL) {
        report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
        return -1;
    }
    if (p->shadow_buf == NULL) {
        report(RPT_ERR, "%s: unable to create shadow buffer",
               drvthis->name);
        return -1;
    }
    memset(p->framebuf, ' ', p->width * p->height);
    memset(p->shadow_buf, ' ', p->width * p->height);

    // InitClientSocket
    if (InitClientSocket(p->hostname, &p->socket, 0)) {
        report(RPT_ERR, "%s: unable to init client socket", drvthis->name);
        return -1;
    }

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
}

/////////////////////////////////////////////////////////////////
// Closes the device
//
MODULE_EXPORT void irtrans_close(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    irtrans_clear(drvthis);
    sleep(5);
    p->backlight = 0;
    irtrans_flush(drvthis);

    if (p != NULL) {
        if (p->framebuf != NULL)
            free(p->framebuf);
        free(p);
    }

    close(p->socket);

    drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int irtrans_width(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int irtrans_height(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    return p->height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void irtrans_clear(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->framebuf, ' ', p->width * p->height);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void irtrans_flush(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    LCDCOMMAND buf;
    STATUSBUFFER stat;

    if (!memcmp(p->shadow_buf, p->framebuf, p->width * p->height))
        return;

    if ((time(0) - p->last_time) < p->timeout)
        return;

    memcpy(buf.framebuffer, p->framebuf, p->width * p->height);
    buf.wid = p->width;
    buf.hgt = p->height;

    buf.netcommand = COMMAND_LCD;
    buf.adress = 'L';
    buf.lcdcommand = LCD_TEXT | p->backlight;
    buf.protocol_version = IRTRANS_PROTOCOL_VERSION;

    SendCommand(drvthis, &buf, &stat);  // Error Handling
    memcpy(p->shadow_buf, p->framebuf, p->width * p->height);
    p->last_time = time(0);

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
irtrans_string(Driver *drvthis, int x, int y, char string[])
{
    PrivateData *p = drvthis->private_data;
    int i;

    x--;
    y--;                        // Convert 1-based coords to 0-based...

    if ((y < 0) || (y >= p->height))
        return;

    for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
        if (x >= 0)             // no write left of left border
            p->framebuf[(y * p->width) + x] = string[i];
    }
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void irtrans_chr(Driver *drvthis, int x, int y, char c)
{
    PrivateData *p = drvthis->private_data;

    y--;
    x--;

    if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
        p->framebuf[(y * p->width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets the contrast
//
MODULE_EXPORT void irtrans_set_contrast(Driver *drvthis, int promille)
{
    //PrivateData *p = drvthis->private_data;

    debug(RPT_DEBUG, "Contrast: %d", promille);
}

/////////////////////////////////////////////////////////////////
// Sets the backlight brightness
//
MODULE_EXPORT void irtrans_backlight(Driver *drvthis, int on)
{
    PrivateData *p = drvthis->private_data;
    if (on && p->has_backlight)
        p->backlight = LCD_BACKLIGHT;
    else
        p->backlight = 0;

    debug(RPT_DEBUG, "Backlight %s", (on) ? "ON" : "OFF");
}

int SendCommand(Driver *drvthis, LCDCOMMAND *com, STATUSBUFFER *stat)
{
    PrivateData *p = drvthis->private_data;
    int res;

    res = send(p->socket, (char *) com, sizeof(LCDCOMMAND), 0);

    if (res != sizeof(LCDCOMMAND)) {
        close(p->socket);
        return (ERR_SEND);
    }

    memset(stat, 0, sizeof(STATUSBUFFER));

    do {
        res = recv(p->socket, (char *) stat, 8, 0);
        if (stat->statuslen > 8) {
            res =
                recv(p->socket, ((char *) stat) + 8, stat->statuslen - 8,
                     0);
        } else
            return (0);
    } while (stat->statustype == STATUS_RECEIVE);
    return (0);
}

int InitClientSocket(char host[], SOCKET *sock, unsigned long id)
{
    struct sockaddr_in serv_addr;
    unsigned long adr;
    struct hostent *he;
    struct in_addr addr;

    adr = inet_addr(host);
    if (adr == INADDR_NONE) {
        he = gethostbyname(host);
        if (he == NULL)
            return (ERR_FINDHOST);
        memcpy(&addr, he->h_addr_list[0], sizeof(struct in_addr));
        adr = addr.s_addr;
    }


    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock < 0)
        return (ERR_OPENSOCKET);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = adr;
    serv_addr.sin_port = htons(TCP_PORT);

    if (connect(*sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
        0)
        return (ERR_CONNECT);
    send(*sock, (char *) &id, 4, 0);
    return (0);
}
