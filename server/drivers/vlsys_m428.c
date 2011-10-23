/** \file server/drivers/vlsys_m428.c
 * LCDd \c vlsys_m428 driver for the VLSystem VFD/IR combination in the
 * computer case "Moneual MonCaso 320". The precise name of the VFD is
 * unknown, but it reports itself as "M428".
 *
 * The command set and its parameters have been discovered by
 * monitoring the serial port communication of the Windows driver.
 */

/*-
 * Copyright (c) 2011 Wolfgang Hauck <wolfgang.hauck@gmx.de>.
 * Derived from imon-module.
 *
 * This source code is being released under the GPL.
 * Please see the file COPYING in this package for details.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>

#include "lcd.h"
#include "report.h"
#include "vlsys_m428.h"


/** Default device path. */
#define DEFAULT_DEVICE	"/dev/ttyUSB0"
/** Default serial baud rate. */
#define DEFAULT_BAUD_RATE B57600
/** Display width in characters. */
#define DEFAULT_WIDTH	14
/** Display height in characters. */
#define DEFAULT_HEIGHT	1
/** Frame buffer size. */
#define DEFAULT_SIZE    (DEFAULT_WIDTH * DEFAULT_HEIGHT)

/** Maximal size of device path. */
#define PATH_SIZE 256

/* Command set and parameters */
/** Initialise display for IR receive. Unused by this driver. */
#define CMD_INIT     0x96
/** Enable icon on top of display. */
#define CMD_ICON     0x97
/** Print a character sequence starting from a specified display position */
#define CMD_PRINT    0x93
/** Complete print action */
#define CMD_COMPLETE 0x09
/** Switch power LED. */
#define CMD_LED      0x9B

/** Disable icon; parameter to command CMD_ICON. */
#define PAR_ICON_DISABLE 0x01
/** Enable icon; parameter to command CMD_ICON. */
#define PAR_ICON_ENABLE  0x02
/** Switch LED off; parameter to command CMD_LED. */
#define PAR_LED_OFF      0x02
/** Switch LED on; parameter to command CMD_LED. */
#define PAR_LED_ON       0x01


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "vlsys_m428_";

/** Private data for the \c vlsystem_m428 driver */
typedef struct vlsystem_m428_private_data {
	int fd;				/**< File descriptor of serial port */
	char framebuf[DEFAULT_SIZE];	/**< Frame buffer */
} PrivateData;

/* Internal functions */
static int dev_print_chr(PrivateData *p, char x, const char *s, unsigned int n);
static int dev_send_bytes(PrivateData *p, const char *s, unsigned int n);
static int initialise_display(PrivateData *p);
static int initialise_port(int fd);
static char map(char c);


/**
 * Sends an array of bytes to the display.
 * \param p    Pointer to private data.
 * \param s    Pointer to array of bytes.
 * \param n    Number of bytes to be sent.
 * \retval 0   Success.
 * \retval <0  Error (errno is set).
 */
static int
dev_send_bytes(PrivateData *p, const char *s, unsigned int n)
{
	int l;

	while (n > 0) {
		l = write(p->fd, s, n);
		if (l < 0) {
			return (-1);
		}
		s += l;
		n -= l;
	}

	return 0;
}


/**
 * Prints an array of characters on the display.
 * \param p    Pointer to private data.
 * \param x    Column position (1, 2, 3, ..., DEFAULT_WIDTH).
 * \param s    Pointer to array of characters.
 * \param n    Number of characters to be printed.
 * \retval 0   Success.
 * \retval <0  Error (errno is set).
 */
static int
dev_print_chr(PrivateData *p, char x, const char *s, unsigned int n)
{
	char c;

	c = CMD_PRINT;
	if (dev_send_bytes(p, &c, 1) < 0) {
		return (-1);
	}
	if (dev_send_bytes(p, &x, 1) < 0) {
		return (-1);
	}
	if (dev_send_bytes(p, s, n) < 0) {
		return (-1);
	}
	c = CMD_COMPLETE;
	if (dev_send_bytes(p, &c, 1) < 0) {
		return (-1);
	}

	return 0;
}


/**
 * Maps a character encoded in ISO 8859-1 to display encoding.
 * Note: Encoding is not complete; some characters
 *       are approximated by similar ones.
 * - Approximates for 0xA6, 0xAD, 0xB4, 0xB7, 0xCD, 0xFA.
 * - No character for 0xA8, 0xA9, 0xAC, 0xAE, 0xAF, 0xB8.
 * \param c  Character (ISO 8859-1).
 * \return   Character (display encoding or an approximation),
 *           zero if map value is unknown.
 */
static char
map(char c)
{
	static const char display[] = {
		/* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
		0x20, 0xAD, 0xBD, 0x9C, 0xCF, 0xBE, 0x7C, 0xF5, 0x00, 0x00, 0xA6, 0xAE, 0x00, 0x2D, 0x00, 0x00,	/* 0xA? */
		0xF8, 0xF1, 0xFD, 0xFC, 0x27, 0x16, 0xF4, 0x2E, 0x00, 0xFB, 0xA7, 0xAF, 0xAC, 0xAB, 0xF3, 0xA8,	/* 0xB? */
		0xB7, 0xB5, 0xB6, 0xC7, 0x8E, 0x8F, 0x92, 0x80, 0xD4, 0x90, 0xD2, 0xD3, 0xD6, 0x49, 0xD7, 0xD8,	/* 0xC? */
		0xD1, 0xA5, 0xE3, 0xE0, 0xE2, 0xE5, 0x99, 0x9E, 0xBF, 0xEB, 0xE9, 0xEA, 0x9A, 0xED, 0xE7, 0xE1,	/* 0xD? */
		0x85, 0xA0, 0x83, 0xC6, 0x84, 0x86, 0x91, 0x87, 0x8A, 0x82, 0x88, 0x89, 0x8D, 0xA1, 0x8C, 0x8B,	/* 0xE? */
		0xD0, 0xA4, 0x95, 0xA2, 0x93, 0xE4, 0x94, 0xF6, 0xCB, 0x97, 0x75, 0x96, 0x81, 0xEC, 0xE8, 0x98	/* 0xF? */
	};
	unsigned char d = c;

	if (d >= 0x20 && d <= 0x7F) {
		return d;	/* standard ASCII */
	}
	else if (d >= 0xA0) {
		return display[d - 0xA0];	/* ISO 8859-1 extension */
	}
	else {
		return 0;
	}
}


/**
 * Initialises the serial port.
 * \param fd   File descriptor of serial port.
 * \retval 0   Success.
 * \retval <0  Error.
 */
static int
initialise_port(int fd)
{
	struct termios options;

	if (tcgetattr(fd, &options) < 0) {
		return -1;
	}
#ifdef HAVE_CFMAKERAW
	cfmakeraw(&options);
#else
	options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
			     | INLCR | IGNCR | ICRNL | IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	options.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	options.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	cfsetispeed(&options, DEFAULT_BAUD_RATE);
	cfsetospeed(&options, DEFAULT_BAUD_RATE);
	if (tcsetattr(fd, TCSAFLUSH, &options) < 0) {
		return -1;
	}

	return 0;
}


/**
 * Initialises the display.
 * Switches red LED on and off such that a short flash is visible;
 * useful for LED test and debugging purposes.
 * Note: All commands sent do not trigger a response on the serial line,
 * therefore other read-only drivers are not disturbed.
 * \param p    Pointer to private data.
 * \retval 0   Success.
 * \retval <0  Error.
 */
static int
initialise_display(PrivateData *p)
{
	const char on[] = {CMD_LED, PAR_LED_ON};
	const char off[] = {CMD_LED, PAR_LED_OFF};
	int i;			/* icon index */

	/* Switch red LED on */
	if (dev_send_bytes(p, on, sizeof(on))) {
		return -1;
	}
	/* Send output initialisation sequence (non-intrusive, no response) */
	for (i = 1; i < DEFAULT_SIZE; i++) {
		char icon[] = {CMD_ICON, 0, PAR_ICON_DISABLE};
		icon[1] = i;
		if (dev_send_bytes(p, icon, sizeof(icon))) {
			return -1;
		}
	}
	/* Switch red LED off */
	if (dev_send_bytes(p, off, sizeof(off))) {
		return -1;
	}

	return 0;
}


/**
 * Initialises the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
vlsys_m428_init(Driver *drvthis)
{
	PrivateData *p = NULL;
	char path[PATH_SIZE];	/* device path (serial port) */

	/* Allocate and initialise private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL) {
		report(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}
	p->fd = -1;		/* fail safe value */
	memset(p->framebuf, ' ', sizeof(p->framebuf));

	/* Store pointer to private data */
	if (drvthis->store_private_ptr(drvthis, p)) {
		report(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		vlsys_m428_close(drvthis);
		return -1;
	}

	/* Get device path from configuration file */
	strncpy(path,
		drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE),
		sizeof(path));
	path[sizeof(path) - 1] = '\0';
	report(RPT_INFO, "%s: using device %s", drvthis->name, path);

	/* Open device for writing (reading would interfere with LIRC) */
	if ((p->fd = open(path, O_WRONLY)) < 0) {
		report(RPT_ERR,
		       "%s: could not open %s (%s)",
		       drvthis->name, path, strerror(errno));
		vlsys_m428_close(drvthis);
		return -1;
	}
	/* Serial port configuration */
	else if (initialise_port(p->fd) < 0) {
		report(RPT_ERR, "%s: could not configure %s (%s)",
		       drvthis->name, path, strerror(errno));
		vlsys_m428_close(drvthis);
		return -1;
	}
	/* Initialise display for output */
	else if (initialise_display(p) < 0) {
		report(RPT_ERR, "%s: could not initialise display", drvthis->name);
		vlsys_m428_close(drvthis);
		return -1;
	}
	/* Erase display */
	else if (dev_print_chr(p, 1, p->framebuf, DEFAULT_SIZE) < 0) {
		report(RPT_ERR, "%s: could not erase display", drvthis->name);
		vlsys_m428_close(drvthis);
		return -1;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Closes the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
vlsys_m428_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	/* Clean up resources */
	if (p != NULL) {
		/* Close serial port */
		if (p->fd >= 0) {
			close(p->fd);
		}
		free(p);
	}
	/* Invalidate local driver structure */
	drvthis->store_private_ptr(drvthis, NULL);

	report(RPT_DEBUG, "%s: close() done", drvthis->name);
}


/**
 * Provides some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
vlsys_m428_get_info(Driver *drvthis)
{
	const char *info_string = "Driver for VLSystem VFD/IR combination in Moneual MonCaso 320";

	return info_string;
}


/**
 * Clears the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
vlsys_m428_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', DEFAULT_SIZE);
}


/**
 * Flushes data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
vlsys_m428_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	dev_print_chr(p, 1, p->framebuf, DEFAULT_SIZE);
}


/**
 * Prints a string on the screen at position (x,y).
 * The upper-left corner is (1,1),
 * the lower-right corner is (DEFAULT_WIDTH, DEFAULT_HEIGHT).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
vlsys_m428_string(Driver *drvthis, int x, int y, const char string[])
{
	int i;

	for (i = 0; string[i] != '\0'; i++)
		vlsys_m428_chr(drvthis, x + i, y, string[i]);
}


/**
 * Puts a character on the screen at position (x,y).
 * The upper-left corner is (1,1),
 * the lower-right corner is (DEFAULT_WIDTH, DEFAULT_HEIGHT).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
vlsys_m428_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	if ((x >= 1) && (y >= 1) && (x <= DEFAULT_WIDTH) && (y <= DEFAULT_HEIGHT)) {
		p->framebuf[x - 1] = map(c);
	}
}


/**
 * Returns the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
vlsys_m428_width(Driver *drvthis)
{
	return DEFAULT_WIDTH;
}


/**
 * Returns the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
vlsys_m428_height(Driver *drvthis)
{
	return DEFAULT_HEIGHT;
}
