/** \file server/drivers/imonlcd.c
 * Driver for SoundGraph iMON OEM LCD Modules
 *
 * Requires the lirc_imon module included with lirc >= 0.8.4a
 */

/*-
 * Copyright (c) 2004, Venky Raju <dev (at) venky (dot) ws>
 *               2007, Dean Harding <dean (at) codeka dotcom>
 *               2007, Christian Leuschen <christian (dot) leuschen (at) gmx (dot) de>
 *               2009, Jonathan Kyler <jkyler (at) users (dot) sourceforge (dot) net>
 *               2009, Eric Pooch < epooch (at) cox (dot) net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#include <math.h>

#include "lcd.h"
#include "lcd_lib.h"
#include "report.h"

/* Defines a 6x8 font based on ISO 8859-15 */
#define LCD_DEFAULT_CELL_WIDTH	6
#define LCD_DEFAULT_CELL_HEIGHT   8
#include "imonlcd_font.h"

#include "imonlcd.h"

#define IMONLCD_PACKET_DATA_SIZE 7

#define DEFAULT_DEVICE       "/dev/lcd0"
#define DEFAULT_SIZE         "96x16"	/**< This is the size in "pixels" (colXrow) */
#define DEFAULT_CONTRAST     200
#define DEFAULT_BACKLIGHT    1	/**< turn backlight on */
#define DEFAULT_DISCMODE     0	/**< spin the "slim" disc */
#define DEFAULT_ON_EXIT      1	/**< show the big clock */
#define DEFAULT_PROTOCOL     0	/**< protocol for 15c2:ffdc device */


#define ON_EXIT_SHOWMSG      0	/**< Do nothing - just leave the "shutdown"
				 * message there */
#define ON_EXIT_SHOWCLOCK    1	/**< Show the big clock */
#define ON_EXIT_BLANKSCREEN  2	/**< Blank the device completely */

#define PROTOCOL_FFDC        0	/**< protocol ID for 15c2:ffdc device */
#define PROTOCOL_0038        1	/**< protocol ID for 15c2:0038 device */

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "imonlcd_";

/** Private data for the \c imonlcd driver */
typedef struct imonlcd_private_data {
	char info[255];
	int imon_fd;

	/* send buffer for one command or display data */
	unsigned char tx_buf[8];

	/* framebuffer and backingstore for current contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	int bytesperline;

	int width, height;
	int cellwidth, cellheight;

	int on_exit;
	int contrast;		/* 0 = lowest contrast, 1000 = highest */
	int backlightOn;	/* stores the backlight state */

	/*
	 * 0 = two disc-segments spinning as default, 1 = their complement
	 * spinning
	 */
	int discMode;

	/*
	 * 0 = protocol for 15c2:ffdc device, 1 = protocol for 15c2:0038
	 * device
	 */
	int protocol;

	/* store commands appropriate for the version of the iMON LCD */
	uint64_t command_display;
	uint64_t command_shutdown;
	uint64_t command_display_on;
	uint64_t command_clear_alarm;

	/* last "state" of the CD icon so that we can animate it */
	int last_cd_state;

	/* save the last output state so we don't needlessly reset the icons */
	int last_output_state;
	int last_output_bar_state;
} PrivateData;

/*
 * Just for convenience and to have the commands at one place.
 */
#define COMMANDS_SET_ICONS	(uint64_t) 0x0100000000000000LL
#define COMMANDS_SET_CONTRAST	(uint64_t) 0x0300000000000000LL
#define COMMANDS_DISPLAY	(uint64_t) 0x0000000000000000LL  // must be or'd with the applicable COMMANDS_DISPLAY_BYTE
#define COMMANDS_SHUTDOWN	(uint64_t) 0x0000000000000008LL  // must be or'd with the applicable COMMANDS_DISPLAY_BYTE
#define COMMANDS_DISPLAY_ON	(uint64_t) 0x0000000000000040LL  // must be or'd with the applicable COMMANDS_DISPLAY_BYTE
#define COMMANDS_CLEAR_ALARM	(uint64_t) 0x0000000000000000LL  // must be or'd with the applicable COMMANDS_ALARM_BYTE
#define COMMANDS_SET_LINES0	(uint64_t) 0x1000000000000000LL
#define COMMANDS_SET_LINES1	(uint64_t) 0x1100000000000000LL
#define COMMANDS_SET_LINES2	(uint64_t) 0x1200000000000000LL
#define COMMANDS_INIT		(uint64_t) 0x0200000000000000LL  //not exactly sure what this does, but it's needed
#define COMMANDS_LOW_CONTRAST	(uint64_t) (COMMANDS_SET_CONTRAST + (uint64_t) 0x00FFFFFF00580A00LL)
#define COMMAND_MASK		(uint64_t) 0xFF00000000000000LL

/* Allow for variations in Soundgraphs numerous protocols */
/* 15c2:0038 SoundGraph iMON */
#define COMMANDS_DISPLAY_BYTE_0038	(uint64_t) 0x8800000000000000LL
#define COMMANDS_ALARM_BYTE_0038	(uint64_t) 0x8a00000000000000LL

/* 15c2:ffdc SoundGraph iMON */
#define COMMANDS_DISPLAY_BYTE_FFDC	(uint64_t) 0x5000000000000000LL
#define COMMANDS_ALARM_BYTE_FFDC	(uint64_t) 0x5100000000000000LL


/*
 * These are used with the imon_output function to determine which icons to
 * turn on/off. Because we only get a 32-bit integer to play, some of the
 * icons are grouped into "sets" from which you can only select to turn one
 * on at a time.
 */
#define IMON_OUTPUT_CD_MASK		0x00000001
#define IMON_OUTPUT_TOPROW_MASK		0x0000000E
#define IMON_OUTPUT_SPEAKER_MASK	0x00000030
#define IMON_OUTPUT_SPDIF_MASK		0x00000040
#define IMON_OUTPUT_SRC_MASK		0x00000080
#define IMON_OUTPUT_FIT_MASK		0x00000100
#define IMON_OUTPUT_TV_MASK		0x00000200
#define IMON_OUTPUT_HDTV_MASK		0x00000400
#define IMON_OUTPUT_SCR1_MASK		0x00000800
#define IMON_OUTPUT_SCR2_MASK		0x00001000
#define IMON_OUTPUT_BRICONS_MASK	0x0000E000
#define IMON_OUTPUT_BMICONS_MASK	0x00070000
#define IMON_OUTPUT_BLICONS_MASK	0x00380000
#define IMON_OUTPUT_VOL_MASK		0x00400000
#define IMON_OUTPUT_TIME_MASK		0x00800000
#define IMON_OUTPUT_ALARM_MASK		0x01000000
#define IMON_OUTPUT_REC_MASK		0x02000000
#define IMON_OUTPUT_REP_MASK		0x04000000
#define IMON_OUTPUT_SFL_MASK		0x08000000

#define IMON_OUTPUT_PBARS_MASK		0x10000000
#define IMON_OUTPUT_DISK_IN_MASK	0x20000000

#define IMON_ICON_ALL		(uint64_t) 0x00FFFFFFFFFFFFFFLL

/* Byte 6 */
#define IMON_ICON_DISK_OFF	(uint64_t) 0x7F7000FFFFFFFFFFLL
#define IMON_ICON_DISK_ON	(uint64_t) 0x0080FF0000000000LL

#define IMON_ICON_DISK_IN	(uint64_t) 0x0080000000000000LL
#define IMON_ICON_CD_IN		(uint64_t) 0x00806B0000000000LL
#define IMON_ICON_DVD_IN	(uint64_t) 0x0080550000000000LL

/* Byte 5 */
#define IMON_ICON_WMA2	((uint64_t) 0x1 << 39)
#define IMON_ICON_WAV	((uint64_t) 0x1 << 38)
#define IMON_ICON_REP	((uint64_t) 0x1 << 37)
#define IMON_ICON_SFL	((uint64_t) 0x1 << 36)
#define IMON_ICON_ALARM	((uint64_t) 0x1 << 35)
#define IMON_ICON_REC	((uint64_t) 0x1 << 34)
#define IMON_ICON_VOL	((uint64_t) 0x1 << 33)
#define IMON_ICON_TIME	((uint64_t) 0x1 << 32)

/* Byte 4 */
#define IMON_ICON_XVID	((uint64_t) 0x1 << 31)
#define IMON_ICON_WMV	((uint64_t) 0x1 << 30)
#define IMON_ICON_MPG2	((uint64_t) 0x1 << 29)
#define IMON_ICON_AC3	((uint64_t) 0x1 << 28)
#define IMON_ICON_DTS	((uint64_t) 0x1 << 27)
#define IMON_ICON_WMA	((uint64_t) 0x1 << 26)
#define IMON_ICON_MP3	((uint64_t) 0x1 << 25)
#define IMON_ICON_OGG	((uint64_t) 0x1 << 24)

/* Byte 3 */
#define IMON_ICON_SRC	((uint64_t) 0x1 << 23)
#define IMON_ICON_FIT	((uint64_t) 0x1 << 22)
#define IMON_ICON_TV_2	((uint64_t) 0x1 << 21)
#define IMON_ICON_HDTV	((uint64_t) 0x1 << 20)
#define IMON_ICON_SCR1	((uint64_t) 0x1 << 19)
#define IMON_ICON_SCR2	((uint64_t) 0x1 << 18)
#define IMON_ICON_MPG	((uint64_t) 0x1 << 17)
#define IMON_ICON_DIVX	((uint64_t) 0x1 << 16)

/* Byte 2 */
#define IMON_SPKR_FC	((uint64_t) 0x1 << 15)
#define IMON_SPKR_FR	((uint64_t) 0x1 << 14)
#define IMON_SPKR_SL	((uint64_t) 0x1 << 13)
#define IMON_SPKR_LFE	((uint64_t) 0x1 << 12)
#define IMON_SPKR_SR	((uint64_t) 0x1 << 11)
#define IMON_SPKR_RL	((uint64_t) 0x1 << 10)
#define IMON_SPKR_SPDIF	((uint64_t) 0x1 << 9)
#define IMON_SPKR_RR	((uint64_t) 0x1 << 8)

/* Byte 1 */
#define IMON_ICON_MUSIC	((uint64_t) 0x1 << 7)
#define IMON_ICON_MOVIE	((uint64_t) 0x1 << 6)
#define IMON_ICON_PHOTO	((uint64_t) 0x1 << 5)
#define IMON_ICON_CD_DVD  ((uint64_t) 0x1 << 4)
#define IMON_ICON_TV	((uint64_t) 0x1 << 3)
#define IMON_ICON_WEBCAST  ((uint64_t) 0x1 << 2)
#define IMON_ICON_NEWS	((uint64_t) 0x1 << 1)
#define IMON_SPKR_FL	((uint64_t) 0x1)


/* prototypes for driver internal functions */
static void imonlcd_display_init(Driver *drvthis);
static void draw_bigchar(imon_bigfont *font, int ch, int x, int y, PrivateData *p);
static void setLineLength(int topLine, int botLine, int topProgress, int botProgress, PrivateData *p);
static void setBuiltinProgressBars(int topLine, int botLine, int topProgress, int botProgress, PrivateData *p);
static int lengthToPixels(int length);
static void send_command_data(uint64_t commandData, PrivateData *p);
static void send_packet(PrivateData *p);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0	   Success.
 * \retval <0	  Error.
 */
MODULE_EXPORT int
imonlcd_init(Driver *drvthis)
{
	PrivateData *p = NULL;
	char buf[256];
	int graph_width, graph_height;	/* width and height of screen, in pixels */
	int tmp;

	/* Allocate, initialize and store private p */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL) {
		report(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}
	if (drvthis->store_private_ptr(drvthis, p)) {
		report(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		return -1;
	}

	p->imon_fd = -1;

	graph_width = 0;
	graph_height = 0;

	p->width = 0;		/* Display width, in characters */
	p->height = 0;		/* Display height, in characters */
	p->cellwidth = LCD_DEFAULT_CELL_WIDTH;	 /* width of a character, in pixels */
	p->cellheight = LCD_DEFAULT_CELL_HEIGHT; /* height of a character, in pixels */

	p->last_cd_state = 0;
	p->last_output_state = 0x0;	/* no icons turned on at startup */
	p->last_output_bar_state = 0x0;	/* no bars turned on at startup */
	p->discMode = 0;

	/* Get settings from config file */

	/* Get device */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, buf);

	/* Open device for writing */
	if ((p->imon_fd = open(buf, O_WRONLY)) < 0) {
		report(RPT_ERR, "%s: ERROR opening %s (%s).", drvthis->name, buf, strerror(errno));
		report(RPT_ERR, "%s: Did you load the iMON kernel module?", drvthis->name);
		return -1;
	}
	/* Get protocol version */
	tmp = drvthis->config_get_int(drvthis->name, "Protocol", 0, DEFAULT_PROTOCOL);
	if ((tmp < 0) || (tmp > 1)) {
		report(RPT_WARNING, "%s: Protocol must be between 0 and 1; using default %d",
		       drvthis->name, DEFAULT_PROTOCOL);
		tmp = DEFAULT_PROTOCOL;
	}
	p->protocol = tmp;

	/* Set commands based on protocol version */
	if (p->protocol == PROTOCOL_FFDC) {
		p->command_display = (uint64_t) (COMMANDS_DISPLAY | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_shutdown = (uint64_t) (COMMANDS_SHUTDOWN | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_display_on = (uint64_t) (COMMANDS_DISPLAY_ON | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_clear_alarm = (uint64_t) (COMMANDS_CLEAR_ALARM | COMMANDS_ALARM_BYTE_FFDC);
	} else if (p->protocol == PROTOCOL_0038) {
		p->command_display = (uint64_t) (COMMANDS_DISPLAY | COMMANDS_DISPLAY_BYTE_0038);
		p->command_shutdown = (uint64_t) (COMMANDS_SHUTDOWN | COMMANDS_DISPLAY_BYTE_0038);
		p->command_display_on = (uint64_t) (COMMANDS_DISPLAY_ON | COMMANDS_DISPLAY_BYTE_0038);
		p->command_clear_alarm = (uint64_t) (COMMANDS_CLEAR_ALARM | COMMANDS_ALARM_BYTE_0038);
	} else {		/* default - use FFDC */
		report(RPT_WARNING, "%s: Protocol version not recognized: %d; using default %d",
		       drvthis->name, p->protocol, PROTOCOL_FFDC);
		p->command_display = (uint64_t) (COMMANDS_DISPLAY | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_shutdown = (uint64_t) (COMMANDS_SHUTDOWN | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_display_on = (uint64_t) (COMMANDS_DISPLAY_ON | COMMANDS_DISPLAY_BYTE_FFDC);
		p->command_clear_alarm = (uint64_t) (COMMANDS_CLEAR_ALARM | COMMANDS_ALARM_BYTE_FFDC);
	}

	/* Get size settings */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	if ((sscanf(buf, "%dx%d", &graph_width, &graph_height) != 2)
	    || (graph_width <= 0) || (graph_height <= 0)
	    || (graph_height < LCD_DEFAULT_CELL_HEIGHT)
	    || ((graph_width / p->cellwidth) > LCD_MAX_WIDTH)
	    || ((graph_height / p->cellheight) > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
		       drvthis->name, buf, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &graph_width, &graph_height);
	}
	/* Convert dimension in pixels to characters. */
	p->width = (graph_width / p->cellwidth);
	p->height = (graph_height / p->cellheight);

	/* Store this to make working with the frame buffer memory easier. */
	p->bytesperline = graph_width;

	/*
	 * Get the "on exit" setting so we know what to do when we shut the
	 * device down
	 */
	tmp = drvthis->config_get_int(drvthis->name, "OnExit", 0, DEFAULT_ON_EXIT);
	if ((tmp < 0) || (tmp > 3)) {
		report(RPT_WARNING, "%s: OnExit must be between 0 and 3; using default %d",
		       drvthis->name, DEFAULT_ON_EXIT);
		tmp = DEFAULT_ON_EXIT;
	}
	p->on_exit = tmp;

	/* Get the "contrast" setting */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast must be between 0 and 1000; using default %d",
		       drvthis->name, DEFAULT_CONTRAST);
		tmp = DEFAULT_CONTRAST;
	}
	p->contrast = tmp;

	/* Get the "backlight" setting */
	p->backlightOn = drvthis->config_get_bool(drvthis->name, "Backlight", 0, DEFAULT_BACKLIGHT);

	/* Get the "disc-mode" setting */
	p->discMode = drvthis->config_get_bool(drvthis->name, "DiscMode", 0, DEFAULT_DISCMODE);

	/*
	 * We need a little bit of extra memory in the frame buffer so that
	 * all of the last 7-bit-long packet data will be within the frame
	 * buffer. See imonlcd_flush() for where we take advantage of this.
	 */
	tmp = 0;
	while (((p->bytesperline * p->height) + tmp) % IMONLCD_PACKET_DATA_SIZE) {
		tmp++;
	}

	/* Make sure the frame buffer is there... */
	p->framebuf = (unsigned char *)malloc((p->bytesperline * p->height) + tmp);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		return -1;
	}

	/* Clear the framebuffer. */
	imonlcd_clear(drvthis);

	/*
	 * This is the extra memory that will fill out the last packet's
	 * data.
	 */
	memset(p->framebuf + (p->bytesperline * p->height), 0xFF, tmp);

	/* Make sure the framebuffer backing store is there... */
	p->backingstore = (unsigned char *)malloc(p->bytesperline * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->bytesperline * p->height);

	imonlcd_display_init(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

/**
 * Send the "initialize" commands to the screen.
 * \param drvthis  Pointer to driver structure.
 */
static void
imonlcd_display_init(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->backlightOn)
		send_command_data(p->command_display_on, p);
	else
		send_command_data(p->command_shutdown, p);

	send_command_data(p->command_clear_alarm, p);
	imonlcd_set_contrast(drvthis, p->contrast);
	send_command_data(COMMANDS_INIT, p);	/* unknown, required init command */
	send_command_data(COMMANDS_SET_ICONS, p);
	/* clear the progress-bars on top and bottom of the display */
	send_command_data(COMMANDS_SET_LINES0, p);
	send_command_data(COMMANDS_SET_LINES1, p);
	send_command_data(COMMANDS_SET_LINES2, p);
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imonlcd_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	time_t tt;
	struct tm *t;
	uint64_t data;

	if (p != NULL) {
		if (p->imon_fd >= 0) {
			if (p->on_exit == ON_EXIT_SHOWMSG) {
				/*
				 * "show message" means "do nothing" - the
				 * message is there already
				 */
				report(RPT_INFO, "%s: closing, leaving \"goodbye\" message.", drvthis->name);
			} else if (p->on_exit == ON_EXIT_BLANKSCREEN) {
				/*
				 * turning backlight off (confirmed for my
				 * Silverstone LCD) (as "cybrmage" at
				 * mediaportal pointed out, his LCD is an
				 * Antec built-in one and turns completely
				 * off with this command)
				 */
				report(RPT_INFO, "%s: closing, turning backlight off.", drvthis->name);
				send_command_data(p->command_shutdown, p);
				send_command_data(p->command_clear_alarm, p);
			} else {
				/*
				 * by default, show the big clock. We need to
				 * set it to the current time, then it just
				 * keeps counting automatically.
				 */
				report(RPT_INFO, "%s: closing, showing clock.", drvthis->name);

				tt = time(NULL);
				t = localtime(&tt);

				data = p->command_display;
				data += ((uint64_t) t->tm_sec << 48);
				data += ((uint64_t) t->tm_min << 40);
				data += ((uint64_t) t->tm_hour << 32);
				data += ((uint64_t) t->tm_mday << 24);
				data += ((uint64_t) t->tm_mon << 16);
				data += (((uint64_t) t->tm_year) << 8);
				data += 0x80;
				send_command_data(data, p);
				send_command_data(p->command_clear_alarm, p);
			}

			close(p->imon_fd);
		}
		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
imonlcd_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	strcpy(p->info, "SoundGraph iMON LCD driver v0.6 : 15c2:ffdc and 15c2:0038 ");
	return p->info;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imonlcd_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, 0x00, p->bytesperline * p->height);
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
imonlcd_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	unsigned char msb;
	int offset = 0;

	/*
	 * The display only provides for a complete screen refresh. If
	 * nothing has changed, don't refresh.
	 */
	if (memcmp(p->backingstore, p->framebuf, (p->bytesperline * p->height)) == 0)
		return;

	for (msb = 0x20; msb < 0x3c; msb++) {
		/* Copy the packet data from the frame buffer. */
		memcpy(p->tx_buf, p->framebuf + offset, IMONLCD_PACKET_DATA_SIZE);

		/* Add the memory register byte to the packet data. */
		p->tx_buf[IMONLCD_PACKET_DATA_SIZE] = msb;

		send_packet(p);

		offset += IMONLCD_PACKET_DATA_SIZE;
	}

	/* Update the backing store. */
	memcpy(p->backingstore, p->framebuf, (p->bytesperline * p->height));
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
imonlcd_string(Driver *drvthis, int x, int y, const char string[])
{
	int i;

	for (i = 0; string[i] != '\0'; i++)
		imonlcd_chr(drvthis, x + i, y, string[i]);
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param ch       Character that gets written.  (ISO 8859-1 + special characters)
 */
MODULE_EXPORT void
imonlcd_chr(Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = drvthis->private_data;
	imon_font *defn;
	int col;

	if ((x < 1) || (y < 1) || (x > p->width) || (y > p->height))
		return;

	x--; y--;
	/* Convert from characters to pixels. */
	x *= p->cellwidth;
	y *= p->bytesperline;

	defn = &font[(unsigned char) ch];

	/* Copy character from font into frame buffer */
	for (col = 0; col < p->cellwidth; col++) {
		p->framebuf[x + y] = defn->pixels[col];
		x++;
	}
}

/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
imonlcd_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	/*
	 * use the imonlcd_font narrow vbar characters and let lcd_lib do the
	 * rest
	 */
	lib_vbar_static(drvthis, x, y, len, promille, options,
			p->cellheight, IMONLCD_FONT_START_VBAR_NARROW-1);
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille (i.e. from 0 to 1000).
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
imonlcd_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	/*
	 * I don't like the reliance on SEAMLESS_HBARS, so the
	 * lib_hbar_static code is replicated here.  (./configure
	 * --enable-seamless hbars would be required for the hbar display to
	 * look right if lib_hbar_static() were used.)
	 */

	int total_pixels = ((long)2 * len * p->cellwidth + 1) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos++) {

		int pixels = total_pixels - p->cellwidth * pos;

		if (pixels >= p->cellwidth) {
			/* write a "full" block to the screen... */
			imonlcd_chr(drvthis, x + pos, y, p->cellwidth + IMONLCD_FONT_START_HBAR_NARROW - 1);
		} else if (pixels > 0) {
			/* write a partial block... */
			imonlcd_chr(drvthis, x + pos, y, pixels + IMONLCD_FONT_START_HBAR_NARROW - 1);
			break;
		} else {
			;	/* write nothing (not even a space) */
		}
	}

}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0   Icon has been successfully defined/written.
 * \retval <0  Server core shall define/write the icon.
 */
MODULE_EXPORT int
imonlcd_icon(Driver *drvthis, int x, int y, int icon)
{
	char ch1 = '?';
	char ch2 = '\0';

	switch (icon) {
	case ICON_BLOCK_FILLED:
		ch1 = IMONLCD_FONT_FULL_BLOCK;
		break;
	case ICON_HEART_OPEN:
		ch1 = IMONLCD_FONT_SMALL_HEART;
		break;
	case ICON_HEART_FILLED:
		ch1 = IMONLCD_FONT_FULL_HEART;
		break;
	case ICON_ARROW_UP:
		ch1 = IMONLCD_FONT_UP_ARROW;
		break;
	case ICON_ARROW_DOWN:
		ch1 = IMONLCD_FONT_DOWN_ARROW;
		break;
	case ICON_ARROW_LEFT:
		ch1 = IMONLCD_FONT_LEFT_ARROW;
		break;
	case ICON_ARROW_RIGHT:
		ch1 = IMONLCD_FONT_RIGHT_ARROW;
		break;
	case ICON_STOP:
		ch1 = IMONLCD_FONT_STOP;
		ch2 = ' ';
		break;
	case ICON_PAUSE:
		ch1 = IMONLCD_FONT_PAUSE;
		ch2 = ' ';
		break;
	case ICON_PLAY:
		ch1 = IMONLCD_FONT_PLAY;
		ch2 = ' ';
		break;
	case ICON_PLAYR:
		ch1 = IMONLCD_FONT_REVERSE;
		ch2 = ' ';
		break;
	case ICON_FF:
		ch1 = IMONLCD_FONT_PLAY;
		ch2 = IMONLCD_FONT_PLAY;
		break;
	case ICON_FR:
		ch1 = IMONLCD_FONT_REVERSE;
		ch2 = IMONLCD_FONT_REVERSE;
		break;
	case ICON_NEXT:
		ch1 = IMONLCD_FONT_PLAY;
		ch2 = IMONLCD_FONT_FWD;
		break;
	case ICON_PREV:
		ch1 = IMONLCD_FONT_RWD;
		ch2 = IMONLCD_FONT_REVERSE;
		break;
	case ICON_REC:
		ch1 = IMONLCD_FONT_RECORD;
		ch2 = ' ';
		break;
	default:
		/* let the server core do the rest */
		return -1;
	}

	imonlcd_chr(drvthis, x, y, ch1);
	if (ch2 != '\0')
		imonlcd_chr(drvthis, x + 1, y, ch2);

	return 0;
}

/**
 * Draws a "big" number at the specified x-coordinate.
 *
 * Normally, the number that is displayed is "meant" to be 3x4 characters, but
 * because we have a bit more flexibility, I've drawn the numbers as just being
 * 12x16 pixels. That means that while the client will pass x-values between 0
 * and 16, we need to scale it and make sure the numbers remain centered.
 *
 * \param drvthis  A pointer to the Driver structure.
 * \param x        The x-coordinate to display the character at.
 * \param num      The number to display ("10" is the colon).
 */
MODULE_EXPORT void
imonlcd_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;

	/*
	 * This isn't that great, it makes assumptions about the coordinates
	 * the client passes to us. However, it works for MythTV... and looks
	 * pretty cool, too.
	 */
	if (num < 10)
		x = 12 + (int)(((x - 1) * p->cellwidth) * 0.75);
	else
		x = 12 + (int)(((x - 1) * p->cellwidth) * 0.72);

	draw_bigchar(bigfont, (num >= 10 ? ':' : (num + '0')), x, 0, p);
}

/**
 * Sets the "output state" for the device. We use this to control the icons
 * around the outside the display. The bits in \c state correspond to the
 * icons as follows:
 *
 * \verbatim
 * bit 0       : disc icon (0=off, 1='spin') , if Toprow==4, use CD-animation,
 *               else use "HDD-recording-animation"
 * bit 1,2,3   : top row (0=none, 1=music, 2=movie, 3=photo, 4=CD/DVD, 5=TV, 6=Web,
 *               7=News/Weather)
 * bit 4,5     : 'speaker' icons (0=off, 1=LR, 2=5.1ch, 3=7.1ch)
 * bit 6       : S/PDIF icon
 * bit 7       : 'SRC'
 * bit 8       : 'FIT'
 * bit 9       : 'TV'
 * bit 10      : 'HDTV'
 * bit 11      : 'SRC1'
 * bit 12      : 'SRC2'
 * bit 13,14,15: bottom-right icons (0=off, 1=MP3, 2=OGG, 3=WMA, 4=WAV)
 * bit 16,17,18: bottom-middle icons (0=off, 1=MPG, 2=AC3, 3=DTS, 4=WMA)
 * bit 19,20,21: bottom-left icons (0=off, 1=MPG, 2=DIVX, 3=XVID, 4=WMV)
 * bit 22      : 'VOL' (volume)
 * bit 23      : 'TIME'
 * bit 24      : 'ALARM'
 * bit 25      : 'REC' (recording)
 * bit 26      : 'REP' (repeat)
 * bit 27      : 'SFL' (shuffle)
 * bit 28      : Abuse this for progress bars (if set to 1), lower bits represent
 *               the length (6 bits each: P|6xTP|6xTL|6xBL|6xBP with P = bit 28,
 *               TP=Top Progress, TL = Top Line, BL = Bottom Line, BP = Bottom Progress).
 *               If bit 28 is set to 1, lower bits are interpreted as
 *               lengths; otherwise setting the symbols as usual.
 *               0 <= length <= 32, bars extend from left to right.
 *               length > 32, bars extend from right to left, length is counted
 *               from 32 up (i.e. 35 means a length of 3).
 *
 *     Remember: There are two kinds of calls!
 *               With bit 28 set to 1: Set all bars (leaving the symbols as is),
 *               with bit 28 set to 0: Set the symbols (leaving the bars as is).
 *     Beware:   TODO: May become a race condition, if both calls are executed
 *                     before the display gets updated. Keep this in mind in your
 *                     client-code.
 * bit 29      : 'disc-in icon' - half ellipsoid under the disc symbols (0=off, 1=on)
 * \endverbatim
 *
 * \param drvthis  Pointer to driver structure.
 * \param state    This symbols or bars to display.
 */
MODULE_EXPORT void
imonlcd_output(Driver *drvthis, int state)
{

	PrivateData *p = drvthis->private_data;
	uint64_t icon = 0x0;

	/* bit 28 : Abuse this for progress bars. See above for usage. */
	if ( state & IMON_OUTPUT_PBARS_MASK || state == 0 ) {
		if (state != p->last_output_bar_state) {
			/* extract the bar-values for each bar separately */
			int topProgress = (state & 63);
			int topLine = (state & (63 << 6)) >> 6;
			int botProgress = (state & (63 << 12)) >> 12;
			int botLine = (state & (63 << 18)) >> 18;

			botProgress = botProgress > 32 ? -(botProgress - 32) : botProgress;
			topProgress = topProgress > 32 ? -(topProgress - 32) : topProgress;
			botLine = botLine > 32 ? -(botLine - 32) : botLine;
			topLine = topLine > 32 ? -(topLine - 32) : topLine;

			setLineLength(topLine, botLine, topProgress, botProgress, p);

			p->last_output_bar_state = state;
		}

		/*
		 * If the current output state is 'all on' (-1) or 'all off' (0), update
		 * the icons with that state. Otherwise, update the icons with the last
		 * icon output state (this is only used to keep the CD spinning).
		 */
		if (state != 0 && state != -1 ) {
			state = p->last_output_state;
		}
	}

	/* Don't update if no icons need to be changed. */
	if (state == p->last_output_state && !(state != -1 && (state & IMON_OUTPUT_CD_MASK))) {
		return;
	}
	p->last_output_state = state;

	if (state == -1) {	/* the value for "on" in the lcdproc-protocol */
		icon = (uint64_t) IMON_ICON_ALL;
		send_command_data(COMMANDS_SET_ICONS | icon, p);
		return;
	}

	/* bit 0 : disc icon (0=off, 1='spin') */
	if (state & IMON_OUTPUT_CD_MASK) {
		/* Each icon bit represents a section of the cd,
		 * starting at the top as msb, and going counter-clockwise.
		 * Start with the top-right & bottom-left on.
		 */
		unsigned char tmp_cd_bitmap = (0x01 | (0x01 << 4));

		if (p->last_cd_state >= 3 )
			p->last_cd_state = 0;
		else
			p->last_cd_state++;

		/* Shift the bits to the left, and the cd moves clock-wise. */
		tmp_cd_bitmap <<= p->last_cd_state;

		if (p->discMode == 1)
			tmp_cd_bitmap = ~tmp_cd_bitmap;

		icon |= ((uint64_t)tmp_cd_bitmap) << 40;
	}
	/*
	 * bit 1,2,3 : top row
	 * (0=off, 1=MUSIC, 2=MOVIE, 3=PHOTO, 4=CD/DVD, 5=TV, 6=WEB, 7=NEWS/WEATHER)
	 */
	if (state & IMON_OUTPUT_TOPROW_MASK) {
		switch (((state & IMON_OUTPUT_TOPROW_MASK) >> 1)) {
		case 1:
			icon |= IMON_ICON_MUSIC;
			break;
		case 2:
			icon |= IMON_ICON_MOVIE;
			break;
		case 3:
			icon |= IMON_ICON_PHOTO;
			break;
		case 4:
			icon |= IMON_ICON_CD_DVD;
			break;
		case 5:
			icon |= IMON_ICON_TV;
			break;
		case 6:
			icon |= IMON_ICON_WEBCAST;
			break;
		case 7:
			icon |= IMON_ICON_NEWS;
			break;
		default:
			break;
		}
	}
	/* bit 4,5 : 'speaker' icons (0=off, 1=L+R, 2=5.1ch, 3=7.1ch) */
	if (state & IMON_OUTPUT_SPEAKER_MASK) {
		switch (((state & IMON_OUTPUT_SPEAKER_MASK) >> 4)) {
		case 3:
			icon |= (IMON_SPKR_SL | IMON_SPKR_SR);
		case 2:
			icon |= (IMON_SPKR_FC | IMON_SPKR_RL | IMON_SPKR_RR);
		case 1:
			icon |= (IMON_SPKR_FL | IMON_SPKR_FR);
		default:
			break;
		}
	}
	/* bit 6 : S/PDIF icon */
	if (state & IMON_OUTPUT_SPDIF_MASK)	icon |= IMON_SPKR_SPDIF;
	/* bit 7 : 'SRC' */
	if (state & IMON_OUTPUT_SRC_MASK)	icon |= IMON_ICON_SRC;
	/* bit 8 : 'FIT' */
	if (state & IMON_OUTPUT_FIT_MASK)	icon |= IMON_ICON_FIT;
	/* bit 9 : 'TV' */
	if (state & IMON_OUTPUT_TV_MASK)	icon |= IMON_ICON_TV_2;
	/* bit 10 : 'HDTV' */
	if (state & IMON_OUTPUT_HDTV_MASK)	icon |= IMON_ICON_HDTV;
	/* bit 11 : 'SRC1' */
	if (state & IMON_OUTPUT_SCR1_MASK)	icon |= IMON_ICON_SCR1;
	/* bit 12 : 'SRC2' */
	if (state & IMON_OUTPUT_SCR2_MASK)	icon |= IMON_ICON_SCR2;
	/*
	 * bit 13,14,15: bottom-right icons
	 * (0=off, 1=MP3, 2=OGG, 3=WMA, 4=WAV)
	 */
	if (state & IMON_OUTPUT_BRICONS_MASK) {
		switch (((state & IMON_OUTPUT_BRICONS_MASK) >> 13)) {
		case 1:
			icon |= IMON_ICON_MP3;
			break;
		case 2:
			icon |= IMON_ICON_OGG;
			break;
		case 3:
			icon |= IMON_ICON_WMA2;
			break;
		case 4:
			icon |= IMON_ICON_WAV;
			break;
		default:
			break;
		}
	}
	/*
	 * bit 16,17,18: bottom-middle icons
	 * (0=off, 1=MPG, 2=AC3, 3=DTS, 4=WMA)
	 */
	if (state & IMON_OUTPUT_BMICONS_MASK) {
		switch (((state & IMON_OUTPUT_BMICONS_MASK) >> 16)) {
		case 1:
			icon |= IMON_ICON_MPG2;
			break;
		case 2:
			icon |= IMON_ICON_AC3;
			break;
		case 3:
			icon |= IMON_ICON_DTS;
			break;
		case 4:
			icon |= IMON_ICON_WMA;
			break;
		default:
			break;
		}
	}
	/*
	 * bit 19,20,21: bottom-left icons
	 * (0=off, 1=MPG, 2=DIVX, 3=XVID, 4=WMV)
	 */
	if (state & IMON_OUTPUT_BLICONS_MASK) {
		switch (((state & IMON_OUTPUT_BLICONS_MASK) >> 19)) {
		case 1:
			icon |= IMON_ICON_MPG;
			break;
		case 2:
			icon |= IMON_ICON_DIVX;
			break;
		case 3:
			icon |= IMON_ICON_XVID;
			break;
		case 4:
			icon |= IMON_ICON_WMV;
			break;
		default:
			break;
		}
	}
	/* bit 22 : 'VOL' (volume) */
	if (state & IMON_OUTPUT_VOL_MASK)	icon |= IMON_ICON_VOL;
	/* bit 23 : 'TIME' */
	if (state & IMON_OUTPUT_TIME_MASK)	icon |= IMON_ICON_TIME;
	/* bit 24 : 'ALARM' */
	if (state & IMON_OUTPUT_ALARM_MASK)	icon |= IMON_ICON_ALARM;
	/* bit 25 : 'REC' (recording) */
	if (state & IMON_OUTPUT_REC_MASK)	icon |= IMON_ICON_REC;
	/* bit 26 : 'REP' (repeat) */
	if (state & IMON_OUTPUT_REP_MASK)	icon |= IMON_ICON_REP;
	/* bit 27 : 'SFL' (shuffle) */
	if (state & IMON_OUTPUT_SFL_MASK)	icon |= IMON_ICON_SFL;
	/* bit 29 : 'disc-in' */
	if (state & IMON_OUTPUT_DISK_IN_MASK)	icon |= IMON_ICON_DISK_IN;

	send_command_data(COMMANDS_SET_ICONS | icon, p);
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
imonlcd_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
imonlcd_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
imonlcd_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
imonlcd_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Sends a command to the screen. The kernel module expects data to be
 * sent in 8 byte chunks, so for simplicity, we allow you to define
 * the data as a 64-bit integer.
 * However, we have to reverse the bytes to the order the display requires.
 *
 * \param value  The data to send. Must be in a format that is recognized by
 *               the device. The kernel module doesn't actually do validation.
 * \param p      The private data containing the file descriptor to write to.
 */
static void
send_command_data(uint64_t value, PrivateData *p)
{
	int i;

	/* Fill the send buffer. */
	for (i = 0; i < sizeof(p->tx_buf); i++) {
		p->tx_buf[i] = (unsigned char)((value >> (i * 8)) & 0xFF);
	}

	send_packet(p);
}

/**
 * Sends data to the screen.
 *
 * \param p The private data structure containing a tx_buf with the data to send.
 */
static void
send_packet(PrivateData *p)
{
	int err;
	err = write(p->imon_fd, p->tx_buf, sizeof(p->tx_buf));

	if (err <= 0)
		printf("%s: error writing to file descriptor: %d", "imon", err);
}


/**
 * Sets the contrast of the display.
 *
 * \param drvthis   Pointer to driver structure.
 * \param promille  The value the contrast is set to in promille (0 = lowest
 *                  contrast; 1000 = highest contrast).
 * \return 0 on failure, >0 on success.
 */
MODULE_EXPORT
void
imonlcd_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;

	if (promille < 0) {
		promille = 0;
	} else if (promille > 1000) {
		promille = 1000;
	}
	p->contrast = promille;

	/*
	 * Send contrast normalized to the hardware-understandable-value (0
	 * to 40). 0 is the lowest and 40 is the highest. The actual
	 * perceived contrast varies depending on the type of display.
	 */
	send_command_data(COMMANDS_LOW_CONTRAST + (uint64_t) (p->contrast / 25), p);
}

/**
 * Gets the current contrast of the display.
 *
 * \param drvthis Pointer to driver structure.
 * \return The current contrast in promille (0 = lowest contrast;
 *         1000 = highest contrast).
 */
MODULE_EXPORT int
imonlcd_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->contrast;
}

/**
 * Sets the backlight state of the display.
 *
 * \param drvthis Pointer to driver structure.
 * \param on The backlight state boolean-like: 0 = off; >0 = on.
 */
MODULE_EXPORT void
imonlcd_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	/* To prevent superfluous (and erroneous) communication */
	if (p->backlightOn == on)
		return;
	else
		p->backlightOn = on;

	if (on)
		send_command_data(p->command_display_on, p);
	else
		send_command_data(p->command_shutdown, p);
}


/**
 * Draws a "big" character - that is, one that's twice as big as a
 * normal character - at the specified position on the screen.
 * \param font  font definition
 * \param ch    the character to display
 * \param x     horizontal position
 * \param y     vertical position
 * \param p     pointer to the PrivateData structure
 */
static void
draw_bigchar(imon_bigfont *font, int ch, int x, int y, PrivateData *p)
{
	imon_bigfont *defn = font;
	int i;
	int colBorder;

	while (defn->ch != ch && defn->ch != '\0') {
		defn++;
	}

	/*
	 * correction for the number flashing with the colon running
	 * "lcdproc K"
	 */
	colBorder = 12;
	if (ch == ':')
		colBorder = 6;

	for (i = 0; i < colBorder; i++) {
		p->framebuf[x + i + (y * colBorder)] = (defn->pixels[i] & 0xFF00) >> 8;
	}
	for (i = 0; i < colBorder; i++) {
		p->framebuf[x + i + (y * colBorder) + p->bytesperline] = (defn->pixels[i] & 0x00FF);
	}
}


/**
 * Sets the length of the built-in progress-bars and lines.
 * Values from -32 to 32 are allowed. Positive values indicate that bars extend
 * from left to right, negative values indicate that the run from right to left.
 * Conventient method to simplify setting the bars with "human understandable
 * values".
 *
 * \see setBuiltinProgressBars, lengthToPixels
 *
 * \param topLine      Length of the top line (-32 to 32)
 * \param botLine      Length of the bottom line (-32 to 32)
 * \param topProgress  Length of the top progress bar (-32 to 32)
 * \param botProgress  Length of the bottom progress bar (-32 to 32)
 * \param p The private data structure containing the file descriptor to write to.
 */
static void
setLineLength(int topLine, int botLine, int topProgress, int botProgress, PrivateData *p)
{
	setBuiltinProgressBars(lengthToPixels(topLine),
			       lengthToPixels(botLine),
			       lengthToPixels(topProgress),
			       lengthToPixels(botProgress),
			       p
	);
}


/**
 * Sets the length of the built-in progress-bars and lines.
 * Values from -32 to 32 are allowed. Positive values indicate that bars extend
 * from left to right, negative values indicate that the run from right to left.
 *
 * \param topLine      Pitmap of the top line
 * \param botLine      Pitmap of the bottom line
 * \param topProgress  Pitmap of the top progress bar
 * \param botProgress  Pitmap of the bottom progress bar
 * \param p The private data structure containing the file descriptor to write to.
 */
static void
setBuiltinProgressBars(int topLine, int botLine,
		       int topProgress, int botProgress, PrivateData *p)
{
	/* Least sig. bit is on the right */
	uint64_t data;

	/* send bytes 1-4 of topLine and 1-3 of topProgress */
	data = (uint64_t) topLine & 0x00000000FFFFFFFFLL;
	data |= (((uint64_t) topProgress) << 8 * 4) & 0x00FFFFFF00000000LL;
	send_command_data(COMMANDS_SET_LINES0 | data, p);

	/* send byte 4 of topProgress, bytes 1-4 of botProgress and 1-2 of botLine */
	data = (((uint64_t) topProgress) >> 8 * 3) & 0x00000000000000FFLL;
	data |= (((uint64_t) botProgress) << 8) & 0x000000FFFFFFFF00LL;
	data |= (((uint64_t) botLine) << 8 * 5) & 0x00FFFF0000000000LL;
	send_command_data(COMMANDS_SET_LINES1 | data, p);

	/* send remaining bytes 3-4 of botLine */
	data = ((uint64_t) botLine) >> 8 * 2;
	send_command_data(COMMANDS_SET_LINES2 | data, p);
}

/**
 * Maps values to corresponding pixmaps for the built-in progress bars.
 * Values from -32 to 32 are allowed. Positive values indicate that bars extend
 * from left to right, negative values indicate that they run from right to left.
 *
 * \param length The length of the bar.
 * \return The pixmap that represents the given length.
 */
static int
lengthToPixels(int length)
{
	int pixLen[] =
	{
		0x00, 0x00000080, 0x000000c0, 0x000000e0, 0x000000f0,
		0x000000f8, 0x000000fc, 0x000000fe, 0x000000ff,
		0x000080ff, 0x0000c0ff, 0x0000e0ff, 0x0000f0ff,
		0x0000f8ff, 0x0000fcff, 0x0000feff, 0x0000ffff,
		0x0080ffff, 0x00c0ffff, 0x00e0ffff, 0x00f0ffff,
		0x00f8ffff, 0x00fcffff, 0x00feffff, 0x00ffffff,
		0x80ffffff, 0xc0ffffff, 0xe0ffffff, 0xf0ffffff,
		0xf8ffffff, 0xfcffffff, 0xfeffffff, 0xffffffff
	};

	if (abs(length) > 32)
		return (0);

	if (length >= 0)
		return pixLen[length];
	else
		return (pixLen[32 + length] ^ 0xffffffff);
}

/* EOF */
