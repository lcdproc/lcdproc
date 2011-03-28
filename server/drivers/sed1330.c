/** \file server/drivers/sed1330.c
 * LCDd \c sed1330 driver for SED1330/1335 graphical displays.
 *
 * EPSON has changed the the chip-labeling. SED1330 now is S1D13300 and
 * SED1335 is S1D13305.
 *
 * This driver drives the LCD in text mode and uses graphic mode to draw
 * the heartbeart and bars.
 */

/*-
 * Driver for SED1330/1335 graphical displays
 *
 * Copyright (c) 2001-2003, Joris Robijn <joris@robijn.net>
 * 		 2003, Michael Rohde <Micha.R@online.de>
 * 		 2006, Benjamin Wiedmann <benjamin.wiedmann@gmx.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/*-
 * Changelog:
 *
 * November 2001, Joris Robijn:
 * - Created the driver
 * - Parts copied from HD44780 driver
 * December 2001, Joris Robijn:
 * - Adapted to v0.5 API
 * December 2001, Guillaume Filion:
 * - Moved the delay timing code by Charles Steinkuehler to timing.h.
 * June 2002, Joris Robijn:
 * - Modified init things to support multiple font sizes
 * - More calculations v.s. static init data
 * - Finished keypad stuff
 * May 2003, Micha_R
 * - removing all the cursor visible/invisible stuff, because it produce much flicker
 *   (software rendering is now used (Joris))
 * - added support for displays with 192x192 resulution (type 4 = Seiko G191D, but beware,
 *   this display has no controller. You need an SED1330/1335 based interface circuit).
 * - fixed problem with clear display function. With displays for which the formula
 *   "display-hight / character hight" don't give a integer result, unusable lines at the
 *   bottom of the display wasn't cleared.
 * - verified with SED1335. The only difference between the two chips is the minimum
 *   reset-raise-time (SED1330 = 1 ms, SED1335 = 0.2 ms)
 * - added function sed1330_icon to get correct display of full blocks
 * May 2003, Joris Robijn
 * - Made bars have spaces between them (I hope people like this)
 * December 2006, Benjamin Wiedmann
 * - added support for Hitachi SP14Q002 gLCD (320x240) with ccfl inverter
 * - built for parport version of this interface: Wallbraun Electronics lcdinterface
 *   (specifications here: http://wallbraun-electronics.de/produkte/lcdinterface/index.html)
 * - wiring scheme used: "bitshaker" (called "yasedw" in serdisplib)
 *   --> classic   wiring: wr=16; a0=17; rd=01; cs=14
 *   --> bitshaker wiring: wr=01; a0=14; rd=16; cs=17
 * December 2006, Benjamin Wiedmann (additional changes, fixup)
 * - wiring scheme can now be changed at run time using "ConnectionType" config parameter
 *   in sed1330 driver section
 *   Usage of ConnectionType in LCDd.conf:
 *     ConnectionType=<classic|bitshaker>
 * - if no ConnectionType is set it defaults to "classic" wiring
 * January 2011, Markus Dolze:
 * - Add support for HG25504 (bye L. Lagendijk).
 * - Remove instructions that have been transfered to the User's Guide.
 * - Convert function comments to Doxygen.
 * - Remove the unused line drawing function.
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "sed1330.h"
#include "port.h"
#include "lpt-port.h"
#include "report.h"
#include "timing.h"

/* Use uPause from timing.h */
#define uPause timing_uPause

/* Keypad settings */
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 8

/* Command definitions */
#define CMD_SYSTEM_SET	0x40
#define CMD_SLEEP_IN	0x53
#define CMD_DISP_DIS	0x58
#define CMD_DISP_EN	0x59
#define CMD_SCROLL	0x44
#define CMD_CSR_FORM	0x5D
#define CMD_CGRAM_ADR	0x5C
#define CMD_CSR_DIR_R	0x4C
#define CMD_CSR_DIR_L	0x4D
#define CMD_CSR_DIR_U	0x4E
#define CMD_CSR_DIR_D	0x4F
#define CMD_HDOT_SCR	0x5A
#define CMD_OVLAY	0x5B
#define CMD_CSRW	0x46
#define CMD_CSRR	0x47
#define CMD_MWRITE	0x42
#define CMD_MREAD	0x43

/* LCD types known */
#define TYPE_G321D	1
#define TYPE_G121C	2
#define TYPE_G242C	3
#define TYPE_G191D	4
#define TYPE_G2446	5
#define TYPE_SP14Q002	6
#define TYPE_HG25504	7

/* Memory locations */
#define SCR1_L 0x00
#define SCR1_H 0x00
#define SCR2_L 0x00
#define SCR2_H 0x06


/** private data for the \c sed1330 driver */
typedef struct sed1330_private_data {
	int type;		/**< display type */

	/* wiring scheme variables to be set by sed1330_init() */
	int A0;
	int nRESET;
	int nWR;

	int port;		/**< LPT port to use */

	unsigned char *framebuf_text;
	unsigned char *lcd_contents_text;
	unsigned char *framebuf_graph;
	unsigned char *lcd_contents_graph;

	int width, height;
	int cellwidth, cellheight;
	int graph_width, graph_height;
	int bytesperline, textlines_in_memory;

	char have_keypad;

	/*
	 * keyMapDirect contains an array of the ascii-codes that should be
	 * generated when a directly connected key is pressed (not in
	 * matrix).
	 */
	char *keyMapDirect[KEYPAD_MAXX];

	/*
	 * keyMapMatrix contrains an array with arrays of the ascii-codes
	 * that should be generated when a key in the matrix is pressed.
	 */
	char *keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX];

	char *pressed_key;
	int pressed_key_repetitions;
	struct timeval pressed_key_time;

	int stuckinputs;
} PrivateData;

static char *defaultKeyMapDirect[KEYPAD_MAXX] = {"Enter", "Up", "Down", "Escape", "F1"};

static char *defaultKeyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
	{"1", "2", "3", "A", "E"},
	{"4", "5", "6", "B", "F"},
	{"7", "8", "9", "C", "G"},
	{"*", "0", "#", "D", "H"},
	{NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL}};

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "sed1330_";


/* Local functions */
void sed1330_command(PrivateData * p, char command, int datacount, unsigned char *data);
void sed1330_rect(PrivateData * p, int x1, int y1, int x2, int y2, char pattern);
inline void sed1330_set_pixel(PrivateData * p, int x, int y, int value);
unsigned char sed1330_scankeypad(PrivateData * p);
unsigned char sed1330_readkeypad(PrivateData * p, unsigned int YData);


/**
 * API: Init the driver and display
 */
MODULE_EXPORT int
sed1330_init(Driver * drvthis)
{
	const char *s;
	PrivateData *p;
	unsigned char data[8];

	debug(RPT_DEBUG, "%s( %p )", __FUNCTION__, drvthis);

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize PrivateData */
	p->framebuf_text = NULL;
	p->lcd_contents_text = NULL;
	p->framebuf_graph = NULL;
	p->lcd_contents_graph = NULL;

	/* READ THE CONFIG FILE */

	/* Port. Default: 0x278 (second parallel port!) */
	p->port = drvthis->config_get_int(drvthis->name, "Port", 0, 0x278);

	/* Char size */
	s = drvthis->config_get_string(drvthis->name, "CellSize", 0, "6x10");
	if (sscanf(s, "%dx%d", &(p->cellwidth), &(p->cellheight)) != 2) {
		report(RPT_ERR, "%s: cannot interpret CellSize %s",
		       drvthis->name, s);
		return -1;
	}
	if ((p->cellwidth < 6) || (p->cellwidth > 8)
	    || (p->cellheight < 7) || (p->cellheight > 16)) {
		report(RPT_ERR, "%s: CellSize exceeds allowed range of 6x7 to 8x16",
		       drvthis->name);
		return -1;
	}

	/* Type */
	s = drvthis->config_get_string(drvthis->name, "Type", 0, NULL);
	if (s == NULL) {
		report(RPT_ERR, "%s: you need to specify the display type",
		       drvthis->name);
		return -1;
	}
	else if (strcmp(s, "G321D") == 0) {
		p->type = TYPE_G321D;
		p->graph_width = 320;
		p->graph_height = 200;
	}
	else if (strcmp(s, "G121C") == 0) {
		p->type = TYPE_G121C;
		p->graph_width = 128;
		p->graph_height = 128;
	}
	else if (strcmp(s, "G242C") == 0) {
		p->type = TYPE_G242C;
		p->graph_width = 240;
		p->graph_height = 128;
	}
	else if (strcmp(s, "G191D") == 0) {
		p->type = TYPE_G191D;
		p->graph_width = 192;
		p->graph_height = 192;
	}
	else if (strcmp(s, "G2446") == 0) {
		p->type = TYPE_G2446;
		p->graph_width = 240;
		p->graph_height = 64;
	}
	else if (strcmp(s, "SP14Q002") == 0) {
		p->type = TYPE_SP14Q002;
		p->graph_width = 320;
		p->graph_height = 240;
	}
	else if (strcmp(s, "HG25504") == 0) {
		p->type = TYPE_HG25504;
		p->graph_width = 256;
		p->graph_height = 128;
	}
	else {
		report(RPT_ERR, "%s: Unknown display type %s", drvthis->name, s);
		return -1;
	}
	report(RPT_INFO, "%s: Using LCD type %s", drvthis->name, s);

	/*
	 * Get ConnectionType, if no type is set, default to "classic" wiring
	 * so it even works with config files missing that ConnectionType entry
	 * in sed1330 driver section.
	 */
	s = drvthis->config_get_string(drvthis->name, "ConnectionType", 0, "classic");

	/* Set wiring initialization parameters based on ConnectionType */
	if (strcmp(s, "classic") == 0) {
		p->A0 = SEL;		/* port 17 */
		p->nRESET = STRB;	/* port 1 */
		p->nWR = INIT;		/* port 16 */
	}
	else if (strcmp(s, "bitshaker") == 0) {
		p->A0 = nLF;		/* port 14 */
		p->nRESET = INIT;	/* port 16 */
		p->nWR = STRB;		/* port 1 */
	}
	else {
		report(RPT_ERR, "%s: Unknown ConnectionType %s", drvthis->name, s);
		return -1;
	}
	report(RPT_INFO, "%s: Using ConnectionType %s", drvthis->name, s);

	/* Is a keypad attached? Default: no */
	p->have_keypad = drvthis->config_get_bool(drvthis->name, "keypad", 0, 0);

	/* Set up keymap */
	if (p->have_keypad) {
		int x, y;

		/* Set up direct keys first */
		for (x = 0; x < KEYPAD_MAXX; x++) {
			char buf[40];

			/* First fill with default value */
			p->keyMapDirect[x] = defaultKeyMapDirect[x];

			/* Read config value */
			sprintf(buf, "keydirect_%1d", x + 1);
			s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

			/* Was a key specified in the config file ? */
			if (s != NULL) {
				p->keyMapDirect[x] = strdup(s);
				report(RPT_INFO, "%s: Direct key %d: \"%s\"",
				       drvthis->name, x, s);
			}
		}

		/* Then set up matrix keys */
		for (x = 0; x < KEYPAD_MAXX; x++) {
			for (y = 0; y < KEYPAD_MAXY; y++) {
				char buf[40];

				/* First fill with default value */
				p->keyMapMatrix[y][x] = defaultKeyMapMatrix[y][x];

				/* Read config value */
				sprintf(buf, "keymatrix_%1d_%d", x + 1, y + 1);
				s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

				/* Was a key specified in the config file ? */
				if (s != NULL) {
					p->keyMapMatrix[y][x] = strdup(s);
					report(RPT_INFO, "%s: Matrix key %d,%d: \"%s\"",
					       drvthis->name, x, y, s);
				}
			}
		}
	}

	/* Calculate some sizes */
	p->width = p->graph_width / p->cellwidth;
	p->height = p->graph_height / p->cellheight;
	p->bytesperline = (p->graph_width - 1) / p->cellwidth + 1;
	p->textlines_in_memory = (p->graph_height - 1) / p->cellheight + 1;

	report(RPT_INFO, "%s: Text size: %dx%d", drvthis->name, p->width, p->height);
	report(RPT_INFO, "%s: Cell size: %dx%d", drvthis->name, p->cellwidth, p->cellheight);
	report(RPT_INFO, "%s: Graphical size: %dx%d", drvthis->name, p->graph_width, p->graph_height);

	/* Allocate and clear framebuffers for text and graphic */
	p->framebuf_text = (unsigned char *)malloc(p->bytesperline * p->textlines_in_memory);
	if (p->framebuf_text == NULL) {
		report(RPT_ERR, "%s: error allocating text framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf_text, ' ', p->bytesperline * p->textlines_in_memory);

	p->lcd_contents_text = (unsigned char *)malloc(p->bytesperline * p->textlines_in_memory);
	if (p->lcd_contents_text == NULL) {
		report(RPT_ERR, "%s: error allocating lcd_contents_text", drvthis->name);
		return -1;
	}
	memset(p->lcd_contents_text, 0, p->bytesperline * p->textlines_in_memory);

	p->framebuf_graph = (unsigned char *)malloc(p->bytesperline * p->graph_height);
	if (p->framebuf_graph == NULL) {
		report(RPT_ERR, "%s: error allocating graphical framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf_graph, 0, p->bytesperline * p->graph_height);

	p->lcd_contents_graph = (unsigned char *)malloc(p->bytesperline * p->graph_height);
	if (p->lcd_contents_graph == NULL) {
		report(RPT_ERR, "%s: error allocating lcd_contents_graph", drvthis->name);
		return -1;
	}
	memset(p->lcd_contents_graph, 0xFF, p->bytesperline * p->graph_height);

	/* Arrange for access to port */
	debug(RPT_DEBUG, "%s: getting port access", __FUNCTION__);
	if (port_access_multiple(p->port, 3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X: %s",
		       drvthis->name, p->port, strerror(errno));
		return -1;
	}

	if (timing_init() == -1) {
		report(RPT_ERR, "%s: timing_init() failed (%s)",
		       drvthis->name, strerror(errno));
		return -1;
	}

	/* INITIALIZE THE LCD */

	/* End reset-state */
	debug(RPT_DEBUG, "%s: initializing LCD", __FUNCTION__);
	port_out(p->port+2, (p->nWR) ^ OUTMASK);		/* raise ^WR */
	port_out(p->port+2, (p->nRESET | p->nWR) ^ OUTMASK);	/* lower RESET */
	uPause(200);
	port_out(p->port+2, (p->nWR) ^ OUTMASK);		/* raise RESET */
	uPause(200);
	port_out(p->port+2, (p->nRESET | p->nWR) ^ OUTMASK);	/* lower RESET */
	uPause(4000);

	switch (p->type) {
	    case TYPE_G321D:
		data[4] = 0x38;
		break;
	    case TYPE_SP14Q002:
		data[4] = 0x38;
		break;
	    case TYPE_G121C:
		data[4] = 0x7F;	/* ? FIXME: please confirm these numbers */
		break;
	    case TYPE_G242C:
		data[4] = 0x7F;	/* ? */
		break;
	    case TYPE_G2446:
		data[4] = 0x7F;	/* ? */
		break;
	    case TYPE_G191D:
		data[4] = 0x5c;	/* ? */
		break;
	    case TYPE_HG25504:
		data[4] = 0x7F;	/* Confirmed! */
		break;
	    default:
		return -1;
	}

	/* Prepare and send SYSTEM SET command */
	data[0] = 0x30;		/* Int. CGROM, no D6 cor., 8-pixel height, single-panel */
	data[1] = 0x80 + p->cellwidth - 1;	/* two-frame AC drive, cellwidth */
	data[2] = p->cellheight - 1;
	data[3] = p->width - 1;	/* C/R: address range by one display line */
	/* data[4] should be filled already */
	/* C/R = ((clock / (refresh * (L/F - 1))) - 1) / 9 */
	data[5] = p->graph_height - 1;	/* L/F: frame height */
	data[6] = p->bytesperline;	/* APL: horizontal address range low */
	data[7] = 0;			/* APH: horizontal address range high */
	sed1330_command(p, CMD_SYSTEM_SET, 8, data);

	/* Set screen1 and screen2 memory locations */
	/* FIXME: The memory locations need to be calculated! */
	sed1330_command(p, CMD_SCROLL, 6, ((unsigned char[6]) {SCR1_L, SCR1_H, 0xC7, SCR2_L, SCR2_H, 0xC7}));

	/* Prepare and send 'set cursor size' command */
	data[0] = p->cellwidth - 1;
	data[1] = 7;		/* underscore type, 8 pixels high */
	sed1330_command(p, CMD_CSR_FORM, 2, data);

	/* horizontal pixel shift=0 */
	sed1330_command(p, CMD_HDOT_SCR, 1, ((unsigned char[1]) {0x00}));
	/*
	 * XOR mode, set screen1 and screen3 as text (screen2 and screen4 are
	 * always graphic)
	 */
	sed1330_command(p, CMD_OVLAY, 1, ((unsigned char[1]) {0x01}));
	/* set display off, cursor off, screen1 on, screen2 on, screen3 off */
	sed1330_command(p, CMD_DISP_DIS, 1, ((unsigned char[1]) {0x14}));
	/* set cursor move right */
	sed1330_command(p, CMD_CSR_DIR_R, 0, NULL);

	sed1330_flush(drvthis);
	sed1330_command(p, CMD_DISP_EN, 0, NULL);	/* And display on */

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Send a command and optionally accompanying data.
 * \param p          Pointer to private data
 * \param command    Command byte
 * \param datacount  Number of data bytes to send
 * \param data       Pointer to data buffer
 */
void
sed1330_command(PrivateData * p, char command, int datacount, unsigned char *data)
{
	int i;
	int port = p->port;

	port_out(port+2, (p->nRESET | p->nWR | p->A0) ^ OUTMASK);	/* set A0 to indicate command */
	port_out(port, command);	/* set up command */
	port_out(port+2, (p->nRESET | p->A0) ^ OUTMASK);		/* lower ^WR */
	port_out(port+2, (p->nRESET | p->nWR | p->A0) ^ OUTMASK);	/* rise ^WR again */
	port_out(port+2, (p->nRESET | p->nWR) ^ OUTMASK);		/* clear A0 to indicate data */

	/* Optionally output data */
	for (i = 0; i < datacount; i++) {
		port_out(port, data[i]);	/* set up data */
		port_out(port + 2, (p->nRESET) ^ OUTMASK);		/* lower ^WR */
		port_out(port + 2, (p->nRESET | p->nWR) ^ OUTMASK);	/* rise ^WR again */
	}
}


/**
 * API: Close the display
 */
MODULE_EXPORT void
sed1330_close(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	if (p != NULL) {
		int i, j;

		for (i = 0; i < KEYPAD_MAXX; i++) {
			if (p->keyMapDirect[i] != NULL)
				free(p->keyMapDirect[i]);
			for (j = 0; j < KEYPAD_MAXY; j++) {
				if (p->keyMapMatrix[i][j] != NULL)
					free(p->keyMapMatrix[i][j]);
			}
		}

		if (p->framebuf_text != NULL)
			free(p->framebuf_text);
		if (p->lcd_contents_text != NULL)
			free(p->lcd_contents_text);
		if (p->framebuf_graph != NULL)
			free(p->framebuf_graph);
		if (p->lcd_contents_graph != NULL)
			free(p->lcd_contents_graph);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * API: Returns the display width
 */
MODULE_EXPORT int
sed1330_width(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_INFO, "%s()", __FUNCTION__);

	return p->width;
}


/**
 * API: Returns the display height
 */
MODULE_EXPORT int
sed1330_height(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_INFO, "%s()", __FUNCTION__);

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
sed1330_cellwidth(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: returning cellwidth", drvthis->name);

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
sed1330_cellheight(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: returning cellheight", drvthis->name);

	return p->cellheight;
}


/**
 * API: Clear the framebuffers
 */
MODULE_EXPORT void
sed1330_clear(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	memset(p->framebuf_text, ' ', p->bytesperline * p->textlines_in_memory);
	memset(p->framebuf_graph, '\0', p->bytesperline * p->graph_height);
}


/**
 * API: Place a string in the (text) framebuffer
 */
MODULE_EXPORT void
sed1330_string(Driver * drvthis, int x, int y, char *str)
{
	PrivateData *p = drvthis->private_data;
	unsigned char *dest;
	int offset, len;

	debug(RPT_DEBUG, "%s( x=%d, y=%d, str=\"%s\" )", __FUNCTION__, x, y, str);

	if ((y < 1) || (y > p->height)) {
		return;		/* outside framebuf */
	}

	/* Calculate offset and length to write */
	if (x < 1) {
		offset = (1 - x);
		x = 1;
	}
	else {
		offset = 0;
	}
	len = strlen(str) - offset;
	if (len > p->width - x + 1) {
		len = p->width - x + 1;
	}

	/* Calculate destination address */
	dest = p->framebuf_text + (y - 1) * p->bytesperline + (x - 1);

	/* And write */
	memcpy(dest, str, len);
}


/**
 * API: Place a single character in the text framebuffer
 */
MODULE_EXPORT void
sed1330_chr(Driver * drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s( x=%d, y=%d, c='%c' )", __FUNCTION__, x, y, c);

	if ((y < 1) || (y > p->height) || (x < 1) || (x > p->width)) {
		return;		/* outside framebuf */
	}
	p->framebuf_text[(y - 1) * p->bytesperline + (x - 1)] = c;
}


/**
 * API: Flush the framebuffer to the display. On call to this function
 * both framebuffers (text and graphic) are written to the display.
 */
MODULE_EXPORT void
sed1330_flush(Driver * drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned int pos, start_pos, nr_equal, fblen, len, cursor_pos;
	unsigned char csrloc[2];

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	fblen = p->bytesperline * p->textlines_in_memory;
	for (pos = 0; pos < fblen;) {
		start_pos = pos;
		for (nr_equal = 0; pos < fblen && nr_equal < 4; pos++) {
			if (p->lcd_contents_text[pos] == p->framebuf_text[pos]) {
				nr_equal++;
			}
			else {
				nr_equal = 0;
			}
		}
		len = pos - start_pos - nr_equal;
		if (len > 0) {
			cursor_pos = start_pos + 256 * SCR1_H + SCR1_L;
			csrloc[0] = cursor_pos % 256;
			csrloc[1] = cursor_pos / 256;
			sed1330_command(p, CMD_CSRW, 2, csrloc);
			sed1330_command(p, CMD_MWRITE, len, p->framebuf_text + start_pos);
			memcpy(p->lcd_contents_text + start_pos, p->framebuf_text + start_pos, len);
		}
	}

	fblen = p->bytesperline * p->graph_height;
	for (pos = 0; pos < fblen;) {
		start_pos = pos;
		for (nr_equal = 0; pos < fblen && nr_equal < 4; pos++) {
			if (p->lcd_contents_graph[pos] == p->framebuf_graph[pos]) {
				nr_equal++;
			}
			else {
				nr_equal = 0;
			}
		}
		len = pos - start_pos - nr_equal;
		if (len > 0) {
			cursor_pos = start_pos + 256 * SCR2_H + SCR2_L;
			csrloc[0] = cursor_pos % 256;
			csrloc[1] = cursor_pos / 256;
			sed1330_command(p, CMD_CSRW, 2, csrloc);
			sed1330_command(p, CMD_MWRITE, len, p->framebuf_graph + start_pos);
			memcpy(p->lcd_contents_graph + start_pos, p->framebuf_graph + start_pos, len);
		}
	}
}


/**
 * Draws a rectangle into the graphic framebuffer.
 * \param p        Pointer to driver's private data
 * \param x1       X-coordinate of start point
 * \param y1       Y-coordinate of start point
 * \param x2       X-coordinate of end point
 * \param y2       Y-coordinate of end point
 * \param pattern  0 = clear, 1 = set
 */
void
sed1330_rect(PrivateData * p, int x1, int y1, int x2, int y2, char pattern)
{
	int x, y;

	/* Swap coordinates if needed */
	if (x1 > x2) {
		int swap;
		swap = x1;
		x1 = x2;
		x2 = swap;
	}
	if (y1 > y2) {
		int swap;
		swap = y1;
		y1 = y2;
		y2 = swap;
	}
	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			sed1330_set_pixel(p, x, y, pattern);
		}
	}
}


/**
 * Set or clear a single pixel in the graphic framebuffer.
 * \param p      Pointer to driver's private data
 * \param x      LCD x coordinate (0-based)
 * \param y      LCD y coordinate (0-based)
 * \param value  0 = clear, 1 = set
 */
inline void
sed1330_set_pixel(PrivateData * p, int x, int y, int value)
{
	unsigned int bytepos;
	unsigned char bitmask;

	bytepos = y * p->bytesperline + x / p->cellwidth;
	bitmask = 0x80 >> (x % p->cellwidth);
	if (value) {
		p->framebuf_graph[bytepos] |= bitmask;	/* set it */
	}
	else {
		p->framebuf_graph[bytepos] &= ~bitmask;	/* clear it */
	}
}


/**
 * API: Draws a vertical bar at the bottom. Internally this in implemented by
 * drawing a filled rectangle to the graphic framebuffer.
 */
MODULE_EXPORT void
sed1330_vbar(Driver * drvthis, int x, int y, int len, int promille, int pattern)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", __FUNCTION__, x, y, len, promille, pattern);

	sed1330_rect(p, (x - 1) * p->cellwidth, y * p->cellheight,
		     x * p->cellwidth - 2, y * p->cellheight - (long)len * p->cellheight * promille / 1000 - 1, 1);
}


/**
 * API: Draws a horizontal bar to the right (len=pos) or to the left (len=neg).
 * Internally this in implemented by drawing a filled rectangle to the graphic
 * framebuffer.
 */
MODULE_EXPORT void
sed1330_hbar(Driver * drvthis, int x, int y, int len, int promille, int pattern)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s( x=%d, y=%d, len=%d, promille=%d, pattern=%d )", __FUNCTION__, x, y, len, promille, pattern);

	sed1330_rect(p, (x - 1) * p->cellwidth, (y - 1) * p->cellheight,
		     (x - 1) * p->cellwidth + (long)len * p->cellwidth * promille / 1000 - 1, y * p->cellheight - 3, 1);
}


/**
 * API: Writes a big number.
 * \note This is currently only a stub and prevents the alternate big
 * characters from core to be used.
 */
MODULE_EXPORT void
sed1330_num(Driver * drvthis, int x, int y, int num)
{
	//PrivateData * p = drvthis->private_data;

	debug(RPT_DEBUG, "%s( x=%d, y=%d, num=%d )", __FUNCTION__, x, y, num);
}


/**
 * API: Does the heartbeat.
 * Or in fact a bouncing ball :)
 */
MODULE_EXPORT void
sed1330_heartbeat(Driver * drvthis, int type)
{
	PrivateData *p = drvthis->private_data;
	static int timer = 0;	/* 'timer' for the bouncing ball */
	int pos;
	int n;

	static char bouncing_ball[8][8] = {
		{0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF},
		{0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF},
		{0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF},
		{0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF},
		{0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF},
		{0xFF, 0x87, 0x87, 0x87, 0xFF, 0xFF, 0xFF, 0xFF},
		{0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF},
		{0xFF, 0xFF, 0xCF, 0x87, 0x87, 0xCF, 0xFF, 0xFF},
	};

	debug(RPT_DEBUG, "%s( type=%d )", __FUNCTION__, type);

	/* Do nothing if heartbeat is disabled */
	if (type == HEARTBEAT_OFF)
		return;

	pos = p->width - 1;	/* Draw into top right corner */
	p->framebuf_text[pos] = ' ';	/* Clear the text framebuffer there */

	/* Draw the bouncing ball based on timer into graphic framebuffer */
	for (n = 0; n < p->cellheight; n++) {
		/* We only have data for 8 rows */
		if (n < 8) {
			p->framebuf_graph[pos] = bouncing_ball[timer][n];
		}
		else {
			p->framebuf_graph[pos] = 0;
		}
		pos += p->bytesperline;
	}

	timer++;
	timer %= 8;
}


/**
 * API: Place an icon on the screen. Currently no icons are supported except
 * ICON_BLOCK_FILLED.
 */
MODULE_EXPORT int
sed1330_icon(Driver * drvthis, int x, int y, int icon)
{
	switch (icon) {
	    case ICON_BLOCK_FILLED:
		sed1330_chr(drvthis, x, y, 255);
		break;
	    default:
		return -1;
	}
	return 0;
}


/**
 * API: Get a key from the keypad (if there is one)
 */
MODULE_EXPORT const char *
sed1330_get_key(Driver * drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char scancode;
	char *keystr = NULL;
	struct timeval curr_time, time_diff;

	if (!p->have_keypad)
		return NULL;

	gettimeofday(&curr_time, NULL);

	/* Get the key! */
	scancode = sed1330_scankeypad(p);
	if (scancode) {
		if (scancode & 0xF0) {
			keystr = p->keyMapMatrix[((scancode & 0xF0) >> 4) - 1][(scancode & 0x0F) - 1];
		}
		else {
			keystr = p->keyMapDirect[scancode - 1];
		}
	}

	/* Check if a new or repeated key press event is to be reported */
	if (keystr != NULL) {
		if (keystr == p->pressed_key) {
			timersub(&curr_time, &(p->pressed_key_time), &time_diff);
			if (((time_diff.tv_usec / 1000 + time_diff.tv_sec * 1000) - KEYPAD_AUTOREPEAT_DELAY) < 1000 * p->pressed_key_repetitions / KEYPAD_AUTOREPEAT_FREQ) {
				/*
				 * The key is already pressed quite some time
				 * but it's not yet time to return a repeated
				 * keypress
				 */
				return NULL;
			}
			/* Otherwise a keypress will be returned */
			p->pressed_key_repetitions++;
		}
		else {
			/* It's a new keypress */
			p->pressed_key_time = curr_time;
			p->pressed_key_repetitions = 0;
			report(RPT_INFO, "%s: Key pressed: %s (%d,%d)",
			       drvthis->name, keystr, scancode & 0x0F, (scancode & 0xF0) >> 4);
		}
	}

	/* Store the key for the next round */
	p->pressed_key = keystr;

	return keystr;
}


/**
 * Scan the keypad. This is identical in function to the scankeypad function
 * in hd44780.c (except it does not have connection type specific low-level
 * read function), so read there for a complete description of this algorithm.
 *
 * \param p  Pointer to PrivateData structure.
 * \return   Scancode of the key.
 */
unsigned char
sed1330_scankeypad(PrivateData * p)
{
	unsigned int keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned int Ypattern;
	unsigned int Yval;
	signed char exp;

	unsigned char scancode = 0;

	keybits = sed1330_readkeypad(p, 0);

	if (keybits) {
		shiftingbit = 1;
		for (shiftcount = 0; shiftcount < KEYPAD_MAXX && !scancode; shiftcount++) {
			if (keybits & shiftingbit) {
				scancode = shiftcount + 1;
			}
			shiftingbit <<= 1;
		}
	}
	else {
		Ypattern = (1 << KEYPAD_MAXY) - 1;

		if (sed1330_readkeypad(p, Ypattern)) {
			Ypattern = 0;
			Yval = 0;
			for (exp = 3; exp >= 0; exp--) {
				Ypattern = ((1 << (1 << exp)) - 1) << Yval;
				keybits = sed1330_readkeypad(p, Ypattern);
				if (!keybits) {
					Yval += (1 << exp);
				}
			}

			keybits = sed1330_readkeypad(p, 1 << Yval);
			shiftingbit = 1;
			for (shiftcount = 0; shiftcount < KEYPAD_MAXX && !scancode; shiftcount++) {
				if (keybits & shiftingbit) {
					scancode = (Yval + 1) << 4 | (shiftcount + 1);
				}
				shiftingbit <<= 1;
			}
		}
	}

	return scancode;
}


/**
 * Read keypress.
 * \param p      Pointer to driver's private data structure.
 * \param YData  Bitmap of rows / lines to enable.
 * \return       Bitmap of the pressed keys.
 */
unsigned char
sed1330_readkeypad(PrivateData * p, unsigned int YData)
{
	unsigned char readval;

	/*
	 * Output 8 bits. Convert the positive logic to the negative logic on
	 * the LPT port.
	 */
	port_out(p->port, ~YData & 0x00FF);

	/* Read inputs */
	readval = ~port_in(p->port + 1) ^ INMASK;

	/* And convert value back (MSB first). */
	return (((readval & FAULT) / FAULT << 4) |		/* pin 15 */
		((readval & SELIN) / SELIN << 3) |		/* pin 13 */
		((readval & PAPEREND) / PAPEREND << 2) |	/* pin 12 */
		((readval & BUSY) / BUSY << 1) |		/* pin 11 */
		((readval & ACK) / ACK)) & ~p->stuckinputs;	/* pin 10 */
}
