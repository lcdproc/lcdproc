/** \file server/drivers/mx5000.c
 * LCDd \c mx5000 driver for the Logitech MX5000 keyboard.
 */

/*
    Author: Christian Jodar (tian@gcstar.org)

    Copyright (C) 2008 Christian Jodar

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */

/*  Based on mx5000 tools http://home.gna.org/mx5000tools/ */

/*
 * Driver status
 * 02/01/2008: First test version
 *
 *
 *
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <libmx5000/mx5000.h>
#include <libmx5000/mx5000screencontent.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "mx5000.h"
#include "shared/report.h"


/* Internal functions */

/* Get optional input FD for the menu buttons */
static int
mx5000_get_input_fd (void)
{
    struct input_id input_id;
    char devname[PATH_MAX];
    struct dirent *dirent;
    DIR *dir;
    int err, fd;

    dir = opendir("/dev/input");
    if (!dir)
        return -1;

    while((dirent = readdir(dir)) != NULL) {
        if (dirent->d_type != DT_CHR ||
            strncmp(dirent->d_name, "event", 5))
                continue;

        strcpy(devname, "/dev/input/");
        strcat(devname, dirent->d_name);

        fd = open(devname, O_RDONLY | O_NONBLOCK);
        if (fd == -1)
            continue;

        err = ioctl(fd, EVIOCGID, &input_id);
        if (err == 0 && input_id.vendor == 0x046d && input_id.product == 0xb305)
        {
            report(RPT_DEBUG, "mx5000: Reading input events from %s", devname);
            return fd;
        }

        close(fd);
    }

    return -1;
}

/*
 * Prepare a static screen
 */
MODULE_EXPORT int
mx5000_init (Driver *drvthis, char *args)
{
    PrivateData *p;

    /* Allocate and store private data */
    p = (PrivateData *) calloc(1, sizeof(PrivateData));
    if (p == NULL)
        return -1;
    if (drvthis->store_private_ptr(drvthis, p))
        return -1;

    /* initialize private data */
    p->fd = -1;
    p->sc = NULL;

    debug(RPT_INFO, "mx5000: init(%p,%s)", drvthis, args);

    /* Read config file */
    /* Which hiddev device should be used */
    strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
    p->device[sizeof(p->device)-1] = '\0';
    debug(RPT_INFO,"%s: using Device %s", drvthis->name, p->device);

    p->wait = drvthis->config_get_int(drvthis->name, "WaitAfterRefresh", 0, DEFAULT_WAIT);

    p->fd = mx5000_open_path(p->device);
    if (p->fd == -1)
    {
        report(RPT_ERR, "%s: open(%s) failed (%s)",
                drvthis->name, p->device, strerror(errno));
        return -1;
    }
    p->sc = mx5000_sc_new_static();
    if (! p->sc)
    {
        report(RPT_ERR, "%s: creation of screen failed (%s)",
                drvthis->name, strerror(errno));
        return -1;
    }

    p->input_fd = mx5000_get_input_fd();

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
}


/*
 * Clean-up
 */
MODULE_EXPORT void
mx5000_close (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (p != NULL)
    {
        // Just to be sure we flushed current command
        sleep(1);
        mx5000_reset(p->fd);
        if (p->sc)
	{
	    mx5000_sc_free(p->sc);
	}
        if (p->fd >= 0)
	{
	    close(p->fd);
	}
	free(p);
    }
    drvthis->store_private_ptr(drvthis, NULL);
}


/*
 * Returns the display width
 */
MODULE_EXPORT int
mx5000_width (Driver *drvthis)
{
    return SCREEN_WIDTH;
}


/*
 * Returns the display height
 */
MODULE_EXPORT int
mx5000_height (Driver *drvthis)
{
    return SCREEN_HEIGHT;
}


/*
 * Returns the display's character cell width
 */
MODULE_EXPORT int
mx5000_cellwidth(Driver *drvthis)
{
    return CELL_WIDTH;
}


/*
 * Returns the display's character cell height
 */
MODULE_EXPORT int
mx5000_cellheight(Driver *drvthis)
{
    return CELL_HEIGHT;
}


/*
 * Flushes all output to the lcd...
 */
MODULE_EXPORT void
mx5000_flush (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    /*
      We could only update the needed part. But it would be hard to really
      figure out what has been changed
    */

    if (p->changed)
    {
        mx5000_sc_send(p->sc, p->fd);
        p->changed = 0;
        // The screen seems to be unable to react quickly
        usleep(1000 * p->wait);
    }
}

/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,4).
 */
MODULE_EXPORT void
mx5000_chr (Driver *drvthis, int x, int y, char c)
{
    PrivateData *p = drvthis->private_data;
    char str[2];
    // The position should be given in pixels
    int px, py;

    if ((x <= 0) || (y <= 0) || (x > SCREEN_WIDTH) || (y > SCREEN_HEIGHT))
    {
        return;
    }

    px = (x - 1) * CELL_WIDTH;
    py = y * CELL_HEIGHT;

    str[0] = c;
    str[1] = 0;
    mx5000_sc_add_text(p->sc, str, -1, STATIC, py, px);
    p->changed = 1;
}

/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,4).
 */
MODULE_EXPORT void
mx5000_string (Driver *drvthis, int x, int y, const char string[])
{
    PrivateData *p = drvthis->private_data;
    int actual_len;
    // The position should be given in pixels
    int px, py;

    if ((x <= 0) || (y <= 0) || (x > SCREEN_WIDTH) || (y > SCREEN_HEIGHT))
    {
        return;
    }

    actual_len = strlen(string);
    // If the string is too long, it is cut
    if ((actual_len + x - 1) > SCREEN_WIDTH)
    {
        actual_len = SCREEN_WIDTH - x + 1;
    }

    px = (x - 1) * CELL_WIDTH;
    py = y * CELL_HEIGHT;

    mx5000_sc_add_text(p->sc, string, actual_len, STATIC, py, px);
    p->changed = 1;
    //printf("Added %s on %d,%d\n",string,x,y);
}

/*
 * Sets the backlight on or off.
 * Not supported
 */
MODULE_EXPORT void
mx5000_backlight (Driver *drvthis, int on)
{
}

/*
 * Draws a progressbar.
 * x and y are the start position of the bar.
 * The bar by default grows in the 'right' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */
MODULE_EXPORT void
mx5000_pbar (Driver *drvthis, int x, int y, int len, int promille)
{
    PrivateData *p = drvthis->private_data;
    // The position should be given in pixels
    int px, py;
    //printf("Bar : %d, %d, %d, %d, %d\n", x, y, len, promille, options);
    px = (x - 1) * CELL_WIDTH;
    py = y * CELL_HEIGHT;

    mx5000_sc_add_progress_bar(p->sc,
			       (promille * len + 500) / 1000,
			       len,
			       STATIC,
			       py,
			       px);
    p->changed = 1;
}

MODULE_EXPORT void
mx5000_num (Driver *drvthis, int x, int num)
{
    PrivateData *p = drvthis->private_data;
    int px, py;
    char text[12];
    px = (x - 1) * CELL_WIDTH;
    py = 33;

    if (num < 10)
    {
        sprintf(text, "%d", num);
    }
    else
    {
        sprintf(text, ":");
	px -= CELL_WIDTH;
    }

    //printf("Display %s on %d, %d\n", text, px, py);

    mx5000_sc_add_text(p->sc, text, -1, STATICHUGE, py, px);
}

MODULE_EXPORT int
mx5000_icon(Driver *drvthis, int x, int y, int icon)
{
    PrivateData *p = drvthis->private_data;
    int px, py;
    enum display_icon mx5000_icon = 0;

    px = (x - 1) * CELL_WIDTH;
    py = y * CELL_HEIGHT;

    switch (icon)
    {
        case ICON_ARROW_UP:
	  mx5000_icon = ARROW_UP;
	  break;
        case ICON_ARROW_DOWN:
	  mx5000_icon = ARROW_DOWN;
	  break;
        case ICON_ARROW_LEFT:
	  mx5000_icon = TRIANGLE_LEFT;
	  break;
        case ICON_ARROW_RIGHT:
	  mx5000_icon = TRIANGLE_RIGHT;
	  break;
        case ICON_PAUSE:
	  mx5000_icon = PAUSE1;
	  break;
        case ICON_PLAY:
	  mx5000_icon = TRIANGLE_RIGHT;
	  break;
        case ICON_NEXT:
	  mx5000_icon = NEXTTRACK1;
	  break;
        case ICON_REC:
	  mx5000_icon = CIRCLE;
	  break;
    }
    if (mx5000_icon)
    {
        mx5000_sc_add_icon(p->sc, mx5000_icon, STATIC, py, px);
        return 0;
    }
    else
    {

        return -1; /* Let the core do other icons */
    }
}

MODULE_EXPORT void
mx5000_heartbeat(Driver *drvthis, int state)
{
}

/*
 * Gets number of custom chars (always NUM_CCs)
 */
MODULE_EXPORT int
mx5000_get_free_chars(Driver *drvthis)
{
    return 0;
}

/*
 * Clears the LCD screen
 */
MODULE_EXPORT void
mx5000_clear (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    // No way yet (as of 0.1.1) to clear the screen in libmx5000
    if (p->sc)
    {
        mx5000_sc_free(p->sc);
    }
    p->sc = mx5000_sc_new_static();
    if (! p->sc)
    {
        report(RPT_ERR, "%s: creation of screen failed (%s)",
	       drvthis->name, strerror(errno));
    }
    p->changed = 1;
}


MODULE_EXPORT const char *
mx5000_get_info (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->info, '\0', sizeof(p->info));
    strcpy(p->info, "Logitech MX 5000 Driver");

    return p->info;
}

/*
 * Map scancodes for the buttons below the LCD to the default [menu]
 * config key strings for the example LCDd.conf .
 */
static const char *
mx5000_translate_scancode(int scan_code)
{
    const char *key_name = NULL;

    switch (scan_code) {
    case 0xc100c:
        /* Most left key below the LCD, we use this to enter/exit the menu */
	key_name = "Escape";
	break;
    case 0xc100d:
        /* Most right key below the LCD, we use this to select menu items */
	key_name = "Enter";
	break;
    case 0xc100e:
        /* Left button of the 2 middle buttons, marked with an arrow pointing up */
	key_name = "Up";
	break;
    case 0xc100f:
        /* Right button of the 2 middle buttons, marked with an arrow pointing down */
	key_name = "Down";
	break;
    }

    if (key_name)
        report(RPT_DEBUG, "mx5000_get_key detected %s key-press", key_name);

    return key_name;
}

/**
 * Read the next input event.
 * \param drvthis  Pointer to driver structure.
 * \retval         String representation of the key;
 *                 \c NULL for nothing available / error.
 */
MODULE_EXPORT const char *
mx5000_get_key (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;
    struct input_event event;
    int scan_code = 0;
    int value = 0;

    while (read(p->input_fd, &event, sizeof(event)) == sizeof(event)) {
	switch (event.type) {
	case EV_SYN:
	    /* If we got a keypress return the scancode translated to a string */
	    if (scan_code && value)
                return mx5000_translate_scancode(scan_code);
            /* Reset */
            scan_code = 0;
            value = 0;
            break;
        case EV_KEY:
            value = event.value;
            break;
        case EV_MSC:
            if (event.code == MSC_SCAN)
                scan_code = event.value;
            break;
	}
    }
    return NULL;
}
