/** \file server/drivers/rawserial.c
 * LCDd \c rawserial driver.
 * This is primarily of use when you want to use LCDproc as a data-source for
 * an external MCU or other similar hardware, in a situation where the LCD
 * control characters normally emitted are actually obnoxious, rather then
 * beneficial.
 * As such, it simply dumps the ENTIRE (no deltas or such) framebuffer at a
 * configurable rate (default 1 Hz).
 */

/*-
 * Copyright (C) 2014 Connor Wolf <lcdproc@imaginaryindustries.com>
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
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <limits.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "lcd.h"
#include "rawserial.h"
#include "report.h"


/** private data for the \c rawserial driver */
typedef struct rawserial_private_data {
	int width;		/**< display width in characters */
	int height;		/**< display height in characters */
	char *framebuf;		/**< frame buffer */
	int fd;		/**< handle to the device */

	/** \name Event loop timing. refresh_time and refresh_delta form the
	 * event loop timing mechanism for configurable update rates.
	 *@{*/
	uint refresh_time;		/**< time at the last screen update */
	uint refresh_delta;		/**< time step to next screen update */
	/**@}*/
} PrivateData;

/* Local prototypes */
static uint get_millisecond_time(void);


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "rawserial_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
rawserial_init(Driver *drvthis)
{
	PrivateData *p;
	char buf[256];
	char device[200];

	int speed = DEFAULT_SPEED;
	int tmp;
	double tmpf;

	struct termios portset;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p) != 0)
		return -1;

	/* initialize private data */
	tmpf = drvthis->config_get_float(drvthis->name, "UpdateRate", 0, DEFAULT_UPDATE_RATE);
	if (tmpf < 0.0005 || tmpf > 10) {
		report(RPT_WARNING, "%s: UpdateRate out of range; using default %g",
		       drvthis->name, DEFAULT_UPDATE_RATE);
		tmpf = DEFAULT_UPDATE_RATE;
	}
	p->refresh_delta = (int) ((double) SECOND_GRANULARITY / tmpf);

	/* Subtract refresh delta so we update immediately on start */
	p->refresh_time = get_millisecond_time() - p->refresh_delta;
	report(RPT_INFO, "%s: start-up time: %u, refresh delta: %u ms per update", drvthis->name,
	       p->refresh_time, p->refresh_delta);

	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	switch (tmp) {
	case 1200:
		speed = B1200;
		break;
	case 2400:
		speed = B2400;
		break;
	case 9600:
		speed = B9600;
		break;
	case 19200:
		speed = B19200;
		break;
	case 115200:
		speed = B115200;
		break;
	default:
		report(RPT_WARNING,
		       "%s: Speed must be 1200, 2400, 9600, 19200 or 115200; using default %d",
		       drvthis->name, DEFAULT_SPEED);
		speed = DEFAULT_SPEED;
	}

	/* which serial device should be used */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE),
		sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "%s: using Device %s at baud rate: %d (configured = %d)",
	       drvthis->name, device, speed, tmp);

	/* Set display sizes */
	if ((drvthis->request_display_width() > 0)
	    && (drvthis->request_display_height() > 0)) {
		/* Use size from primary driver */
		p->width = drvthis->request_display_width();
		p->height = drvthis->request_display_height();
	}
	else {
		/* Use our own size from config file */
		strncpy(buf,
			drvthis->config_get_string(drvthis->name, "Size", 0, DISPLAY_DEFAULT_SIZE),
			sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';
		if ((sscanf(buf, "%dx%d", &p->width, &p->height) != 2)
		    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
		    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
			       drvthis->name, buf, DISPLAY_DEFAULT_SIZE);
			sscanf(DISPLAY_DEFAULT_SIZE, "%dx%d", &p->width, &p->height);
		}
	}
	report(RPT_INFO, "%s: using Size %dx%d", drvthis->name, p->width, p->height);

	/* Allocate the framebuffer */
	p->framebuf = malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		goto err_out;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* Set up I/O port correctly, and open it... */
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, device, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: device %s could not be opened", drvthis->name, device);
		goto err_out;
	}
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

	tcgetattr(p->fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw(&portset);
#else
	/* The hard way */
	portset.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	portset.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	/* Set timeouts */
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	return 0;

      err_out:
	rawserial_close(drvthis);
	return -1;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
rawserial_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);
		if (p->framebuf != NULL)
			free(p->framebuf);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
rawserial_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
rawserial_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
rawserial_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Flush data on screen to the display.
 *
 * Output is written at every \c refresh_delta milliseconds to the device.
 * However, updates do not occur more often than every 125 ms (8 Hz) which is
 * the default rendering rate in LCDd.
 *
 * This function implements its own update scheduler which is independent of
 * the main rendering loop to achieve a more constant output rate.
 *
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
rawserial_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];

	uint currentTime = get_millisecond_time();
	int t_delta = currentTime - p->refresh_time;

	/*
	 * Sanity checking in case of major time-shifts due to NTP updates or
	 * daylight savings time. (INT_MAX / 1e3) is 2147483 milliseconds, 2147
	 * seconds, or 35 minutes.
	 */
	if (((t_delta + 1) > (INT_MAX / 1e3)) || (t_delta < 0)) {
		report(RPT_WARNING,
		       "%s: Major time-delta between flush calls! Old time: %d, new time: %d",
		       drvthis->name, p->refresh_time, currentTime);
		p->refresh_time = currentTime;
	}

	if (currentTime > (p->refresh_time + p->refresh_delta)) {
		int dataEnd = p->height * p->width;

		/* Dump the contents of the framebuffer out the serial port.
		 * There is no processing and no control chars are emitted,
		 * just a plain-old newline at the end of the record. */
		memcpy(out, p->framebuf, dataEnd);
		write(p->fd, out, dataEnd);
		write(p->fd, "\n", 1);

		report(RPT_DEBUG,
		       "%s: flush exec time: %u, refresh delta: %u, current clock: %u, rendering loop overshoot: %d ms",
		       drvthis->name, p->refresh_time, p->refresh_delta, currentTime,
		       currentTime - p->refresh_time - p->refresh_delta);

		/* Update the event timer so we'll trigger after the next
		 * p->refresh_delta milliseconds */
		p->refresh_time += p->refresh_delta;
	}
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
rawserial_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;			/* Convert 1-based coords to 0-based */

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)	/* no write left of left border */
			p->framebuf[(y * p->width) + x] = string[i];
	}
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
rawserial_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
rawserial_get_info(Driver *drvthis)
{
	static char *info_string =
		"Raw Text mode driver for use as a source for sending data to external hardware";

	return info_string;
}


/**
 * Get current time in milliseconds, used for determining update event timing
 *
 * This WILL wrap fairly often, but we don't care, since we're just comparing
 * two values, both of which will wrap at the same time. Note that gettimeofday
 * is the wrong thing to use here, since it's not monotonic at all. We *really*
 * should use `clock_gettime(CLOCK_MONOTONIC, &ts);` since that's both
 * monotonic and immune to NTP updates, DST time changes, etc. However, that
 * would require linking librealtime.
 *
 * \return  int with current millisecond time.
 */
static uint
get_millisecond_time(void)
{
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return ((ts.tv_sec * SECOND_GRANULARITY) +
		(ts.tv_usec / (1000000.0 / SECOND_GRANULARITY))) + 0.5;
}
