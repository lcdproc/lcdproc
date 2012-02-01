/** \file server/drivers/sdeclcd.c
 * This is the LCDproc driver for SDEC LCD Devices.
 * They are found in the Watchguard FireBox firewall appliances.
 * They are interfaced through the parallel port.
 *
 * The code is based on the spec file LMC-S2D20-01.pdf,
 * a technical hardware and programming guide for this LCD.
 */

/*-
 * Copyright(C) 2011, Francois Mertz <fireboxled AT gmail.com>
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
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "port.h"
#include "lpt-port.h"
#include "timing.h"
#include "lcd.h"
#include "report.h"
#include "adv_bignum.h"
#include "lcd_lib.h"

#include "sdeclcd.h"

/*
 * The following definitions are specific for the SDEC LCD device,
 * and the way it is wired in the Watchguard Firebox. There is a
 * mapping between the parallel port Control register and the
 * LCD control lines, as follows: (see lpt-port.h for defs)
 */
#define SDEC_BACKLIGHT	STRB	/* Strobe, bit 0			*/
#define SDEC_ENABLE	LF	/* Linefeed, bit			*/
#define SDEC_CONTRAST	INIT	/* Init, bit 2 <--FIXME, Untested	*/
#define SDEC_REG_SEL	SEL	/* Select Printer, bit 3		*/

/*
 * The following definitions are specific for the SDEC LCD device
 */
#define SDEC_DISP_W	20	/* Display Width			*/
#define SDEC_DISP_H	2	/* Display Height			*/
#define SDEC_ADD_LINE1	0x80	/* Address of beginning of Line 1	*/
#define SDEC_ADD_LINE2	0xC0	/* Address of beginning of Line 2	*/
#define SDEC_CELL_W	5	/* Character cell Width			*/
#define SDEC_CELL_H	8	/* Character cell Height		*/
#define SDEC_NUM_CC	8	/* Number of Custom Characters		*/
#define SDEC_IDENT	"SDEC LCD Driver with Keyboard, Version " VERSION

/*
 * Instruction Set for the SDEC LCD device
 */
#define SDEC_FN_CLEAR_DISPLAY	0x01	/* Clears the LCD		*/
#define SDEC_FN_RETURN_HOME	0x02	/* Returns cursor home		*/
#define SDEC_FN_SET_ENTRY_MODE	0x04	/* Sets Entry Mode		*/
#define SDEC_OPT_INCREMENT	0x02	/* Cursor moves right		*/
#define SDEC_FN_DISPLAY		0x08	/* Display:			*/
#define SDEC_OPT_DISPLAY_ON	0x04	/* Display ON option		*/
#define SDEC_OPT_CURSOR_ON	0x02	/* Cursor ON option		*/
#define SDEC_OPT_BLINK_ON	0x01	/* Blinking Cursor option	*/
#define SDEC_FN_CURSOR_R	0x14	/* Move cursor right 1 position	*/
#define SDEC_FN_CURSOR_L	0x10	/* Move cursor left 1 position	*/
#define SDEC_FN_FUNCTION_SET	0x20	/* Function Set:		*/
#define SDEC_OPT_8_BIT		0x10	/* 8bit option (vs 4 bit)	*/
#define SDEC_OPT_2_LINES	0x08	/* 2 Lines (vs 1 Line)		*/
#define SDEC_OPT_5X10		0x04	/* Each char is 5x10 (vs 5x7)	*/
#define SDEC_FN_CG_ADD		0x40	/* Set Custom Char address	*/
#define SDEC_FN_DD_ADD		0x80	/* Set Display address (cursor) */
#define SDEC_DATA 	SDEC_REG_SEL	/* For data, RS is ON		*/
#define SDEC_EXEC		0x00	/* For exec, RS is OFF		*/
#define SDEC_HOLD		40	/* Min exec time in micro sec	*/
#define SDEC_HOLD_ENABLE	1	/* Min time to hold ENABLE in micro
					 * sec, should be 450 nano sec	*/

/*
 * Basic PC parallel port constants. At this point, the driver only supports a
 * SDEC LCD device on port 0, as all Fireboxes seem to be wired that way.
 *
 *	http://en.wikipedia.org/wiki/Parallel_port
 */
#define LPT_PORT_0		0x0378
#define LPT_PORT_1		0x0278
#define LPT_PORT_2		0x03BC

#define LPT_DEFAULT 		LPT_PORT_0
#define LPT_STATUS		(LPT_DEFAULT+1)
#define LPT_CONTROL		(LPT_DEFAULT+2)
/* Control port hardware inversion */
#define LPT_CTRL_MASK		OUTMASK
/* Bits actually used by Status port */
#define LPT_STUS_MASK		(SELIN|PAPEREND|ACK|BUSY|FAULT)

/*
 * Driver implementation choices.
 *
 * Heart beat location on screen, and corresponding address, char,
 * and backlight timeout
 */
#define SDEC_HB_LOC		(SDEC_DISP_W-1)
#define SDEC_HB_ADD		(SDEC_ADD_LINE1+SDEC_HB_LOC)
#define SDEC_HB_CHAR		':'
#define SDEC_BKLT_DFT		30

/**
 * This structure will hold data private to this Driver
 * As the driver is implemented as a Shared Library, persistent data needs to
 * be put in a dynamically allocated struct. Otherwise, all instances of the
 * library would share the same values.
 */
typedef struct driver_private_data {
	CGmode ccmode;		/* Current custom character mode	*/
	unsigned char bklgt;	/* Backlight status			*/
	unsigned char lastkbd;	/* Last keyboard status			*/
	unsigned char hb_stus;	/* Heart Beat status			*/
	unsigned bklgt_timer;	/* Seconds to keep the backlight on	*/
	time_t bklgt_lasttime;	/* When back light was turned on	*/
	time_t hrbt_lasttime;	/* When heart beat was output		*/
	char *framebuf;		/* Frame buffer				*/
	char *framelcd;		/* Frame buffer on the LCD		*/
	char *vbar_cg;		/* Vertical Bar bitmaps			*/
	char *hbar_cg;		/* Horizontal Bar bitmaps		*/
	char *bignum_cg;	/* Big Numbers bitmaps			*/
} PrivateData;

/* Variables assisting the driver loader */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;	/* Foreground is not required */
MODULE_EXPORT int supports_multiple = 0;	/* No support for mult. LCDs  */
MODULE_EXPORT char *symbol_prefix = "sdeclcd_";	/* Prefix for API entries     */

/**
 * Main LCD control routine
 * \param register_select	What to set the Register Select line to
 * \param backlight		What to set the Backlight line to
 * \param data			Data to send to LCD (char or instruction)
 * \param usec			Wait time in micro seconds
 *
 * This is the basic routine that controls the LCD. It interfaces with the
 * parallel ports and contains the logic for flipping and holding the lines
 * as required. It tries and be comprehensive enough to avoid spreading
 * LPT port logic all over the code.
 *
 * The Register Select line determines if an output is to be understood as an
 * instruction, or as data to be displayed.
 * The Backlight line controls the backlight in a direct manner.
 *
 * The sequence is:
 *		Bring Enable up
 *		Output the data (command or character)
 *		Hold Enable for 450 nano sec (.45 micro sec)
 *		Bring Enable down
 *		Wait for the LCD to execute the command, typically 40 micro sec
 *
 * Take into account the hardware inversion of some control port bits.
 */
static inline void
_sdec_control_wait(unsigned char register_select, unsigned char backlight,
		   unsigned char data, int usec)
{
	port_out(LPT_CONTROL, (register_select | backlight | SDEC_ENABLE) ^ LPT_CTRL_MASK);
	port_out(LPT_DEFAULT, data);
	timing_uPause(SDEC_HOLD_ENABLE);
	port_out(LPT_CONTROL, (register_select | backlight) ^ LPT_CTRL_MASK);
	timing_uPause(usec);
}

/**
 * These macros are meant to be used by the driver code. They are wrappers
 * around the low level routine, and capture the Register Select logic.
 */
#define sdec_write(a,b) _sdec_control_wait(SDEC_DATA, b, a, SDEC_HOLD)
#define sdec_exec(a,b) _sdec_control_wait(SDEC_EXEC, b, a, SDEC_HOLD)
#define sdec_exec_wait(a,b) _sdec_control_wait(SDEC_EXEC, 0, a,b)

/**
 * This is the mandatory initialization sequence for the LCD, as per the SDEC
 * programming guide. Note the required execution times.
 */
static void
sdec_init()
{
	/* Mandatory reset sequence */
	sdec_exec_wait(SDEC_FN_FUNCTION_SET | SDEC_OPT_8_BIT, 15000);
	sdec_exec_wait(SDEC_FN_FUNCTION_SET | SDEC_OPT_8_BIT, 4100);
	sdec_exec_wait(SDEC_FN_FUNCTION_SET | SDEC_OPT_8_BIT, 100);
	sdec_exec_wait(SDEC_FN_FUNCTION_SET | SDEC_OPT_8_BIT | SDEC_OPT_2_LINES, 100);
	sdec_exec_wait(SDEC_FN_DISPLAY, 40);
	sdec_exec_wait(SDEC_FN_CLEAR_DISPLAY, 1640);
	sdec_exec_wait(SDEC_FN_SET_ENTRY_MODE | SDEC_OPT_INCREMENT, 40);

	/* Additional command */
	sdec_exec(SDEC_FN_DISPLAY | SDEC_OPT_DISPLAY_ON, 0);
}

/**
 * API: Initialize the driver.
 */
MODULE_EXPORT int
sdeclcd_init(Driver *drvthis)
{
	PrivateData *p;
	int i, j;
	/*
	 * Compact version of big num character bitmaps. Stack each line
	 * separately to visualize each char.
	 */
	static char bignum_bitmaps[] = {
		b___XXXX, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___,
		b__XXXX_, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX,
		b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b___XXXX,
		b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b__XXXX_,
		b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX,
		b__XXXXX, b_______, b_______, b_______, b_______, b_______, b_______, b__XXXXX,
		b__XXXX_, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b_____XX, b__XXXX_,
		b___XXXX, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b__XX___, b___XXXX
	};

	/* Allocate and store private data */
	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize private data and read config */
	p->ccmode = standard;
	p->bklgt = BACKLIGHT_ON;
	p->bklgt_timer = SDEC_BKLT_DFT;
	p->bklgt_lasttime = time(NULL);
	p->hrbt_lasttime = time(NULL);
	p->hb_stus = HEARTBEAT_OFF;
	p->framebuf = (char *)malloc(SDEC_DISP_W * SDEC_DISP_H);
	p->framelcd = (char *)malloc(SDEC_DISP_W * SDEC_DISP_H);
	p->vbar_cg = (char *)malloc(SDEC_CELL_H * SDEC_NUM_CC);
	p->hbar_cg = (char *)malloc(SDEC_CELL_H * SDEC_NUM_CC);
	p->bignum_cg = (char *)bignum_bitmaps;

	if (NULL == p->framebuf || NULL == p->framelcd ||
	    NULL == p->vbar_cg || NULL == p->hbar_cg) {
		report(RPT_ERR, "%s: unable to allocate framebuffer",
		       drvthis->name);
		return -1;
	}

	memset(p->framebuf, ' ', SDEC_DISP_W * SDEC_DISP_H);
	memset(p->framelcd, ' ', SDEC_DISP_W * SDEC_DISP_H);

	/* Prepare custom character bitmaps:vbar, hbar */
	for (i = 0; i < SDEC_NUM_CC; i++) {
		for (j = 0; j < SDEC_CELL_H; j++) {
			/* Visualize this: */
			p->vbar_cg[i * SDEC_CELL_H + SDEC_CELL_H - j - 1] = (j <= i) ? 0xFF : 0;
			p->hbar_cg[i * SDEC_CELL_H + j] = (~(0x0F >> i)) & 0x1F;
		}
	}

	p->bignum_cg = bignum_bitmaps;

	/* Initializes the scheduler, see timing.h */
	timing_init();

	/* Initialize access to the ports */
	if (port_access_multiple(LPT_DEFAULT, 3)) {
		report(RPT_ERR, "%s: cannot get IO-permission for 0x%03X! Are we root?",
		       drvthis->name, LPT_DEFAULT);
		return -1;
	}
	sdec_init();
	return 0;
}

/**
 * API: Close the driver (do necessary clean-up).
 */
MODULE_EXPORT void
sdeclcd_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p) {
		if (p->framebuf)
			free(p->framebuf);
		if (p->framelcd)
			free(p->framelcd);
		if (p->vbar_cg)
			free(p->vbar_cg);
		if (p->hbar_cg)
			free(p->hbar_cg);
		free(p);
	}

	if (!port_deny_multiple(LPT_DEFAULT, 3)) {
		report(RPT_WARNING, "%s: cannot release IO-permission for 0x%03X!",
		       drvthis->name, LPT_DEFAULT);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/**
 * API: Return the display width in characters.
 */
MODULE_EXPORT int
sdeclcd_width(Driver *drvthis)
{
	return SDEC_DISP_W;
}

/**
 * API: Return the display height in characters.
 */
MODULE_EXPORT int
sdeclcd_height(Driver *drvthis)
{
	return SDEC_DISP_H;
}

/**
 * API: Flush the contents of the framebuffer out to the LCD
 *
 * The idea is that we only write to the LCD what we must. Therefore we compare
 * the content of the framebuffer to the content of the LCD as we last wrote it
 * and write when there is a diff. Writing involves setting the cursor (at a
 * cost of 40 micro sec), and then write the framebuffer char (another 40 micro
 * sec). As the Entry Mode was set to auto-increment the cursor on every write,
 * we only set the cursor when we must.
 */
MODULE_EXPORT void
sdeclcd_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	int i, c;
	unsigned char addr;

	for (i = 0, c = -1; i < SDEC_DISP_H * SDEC_DISP_W; i++) {
		if (p->framelcd[i] == p->framebuf[i])
			continue;
		/* Found diff:	cursor ok ? */
		if (c != i) {
			/* Set cursor address to where the change is */
			addr = (i < SDEC_DISP_W) ?
				(SDEC_ADD_LINE1 + i) :
				(SDEC_ADD_LINE2 + i - SDEC_DISP_W);
			/* This costs 40 micro sec. */
			sdec_exec(SDEC_FN_DD_ADD | addr, p->bklgt);
			c = i;
		}
		/* Write to LCD, 40 micro sec */
		sdec_write(p->framebuf[i], p->bklgt);
		/* LCD Entry Mode auto increments cursor, so keep track */
		if (SDEC_DISP_W - 1 == c++)
			c = -1;
		/* Force cursor set */
		p->framelcd[i] = p->framebuf[i];
	}
}

/**
 * API: Print a string on the screen at position (x,y).
 */
MODULE_EXPORT void
sdeclcd_string(Driver *drvthis, int x, int y, char string[])
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int len;

	if (y < 1 || y > SDEC_DISP_H || x < 1 || x > SDEC_DISP_W)
		return;

	x--;			/* convert coordinates to 0-base */
	y--;

	len = strlen(string);
	memcpy(p->framebuf + x + y * SDEC_DISP_W, string,
	       (x + len > SDEC_DISP_W) ? SDEC_DISP_W - x : len);
}

/**
 * API: Print a character on the screen at position (x,y).
 */
MODULE_EXPORT void
sdeclcd_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (y < 1 || y > SDEC_DISP_H || x < 1 || x > SDEC_DISP_W)
		return;
	p->framebuf[(y - 1) * SDEC_DISP_W + x - 1] = c;
}

/**
 * API: Clear the screen.
 */
MODULE_EXPORT void
sdeclcd_clear(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	memset(p->framebuf, ' ', SDEC_DISP_W * SDEC_DISP_H);
}

/**
 * API: Return the width of a character in pixels.
 */
MODULE_EXPORT int
sdeclcd_cellwidth(Driver *drvthis)
{
	return SDEC_CELL_W;
}

/**
 * API: Return the height of a character in pixels.
 */
MODULE_EXPORT int
sdeclcd_cellheight(Driver *drvthis)
{
	return SDEC_CELL_H;
}

/**
 * API: Get total number of custom characters available.
 */
MODULE_EXPORT int
sdeclcd_get_free_char(Driver *drvthis)
{
	return SDEC_NUM_CC;
}

/**
 * API:
 * Implementation routine for Icons. The server asks for an icon by code, and
 * this routine does the job. Simply updates the frame buffer with a suitable
 * char based on the icon code requested. As the LCD has its own character
 * map with latin, chinese, greek and extra symbols, it is only a matter of
 * mapping to some character on the LCD that (loosely) resembles the
 * requested code. This implementation does NOT use custom characters.
 */
MODULE_EXPORT int
sdeclcd_icon(Driver *drvthis, int x, int y, int icon_code)
{
	char icon_char;

	switch (icon_code) {
	    case ICON_BLOCK_FILLED:
		icon_char = 0xFF;
		break;
	    case ICON_ARROW_UP:
		icon_char = '|';
		break;
	    case ICON_ARROW_DOWN:
		icon_char = '|';
		break;
	    case ICON_ARROW_LEFT:
		icon_char = 0x7F;
		break;
	    case ICON_ARROW_RIGHT:
		icon_char = 0x7E;
		break;
	    case ICON_CHECKBOX_OFF:
		icon_char = 0xDB;
		break;
	    case ICON_CHECKBOX_ON:
		icon_char = 0xE8;
		break;
	    case ICON_CHECKBOX_GRAY:
		icon_char = 0xA5;
		break;
	    case ICON_SELECTOR_AT_LEFT:
		icon_char = 0x7E;
		break;
	    case ICON_SELECTOR_AT_RIGHT:
		icon_char = 0x7F;
		break;
	    case ICON_ELLIPSIS:
		icon_char = 0xD0;
		break;
	    default:
		return -1;
	}

	/* Update frame buffer */
	sdeclcd_chr(drvthis, x, y, icon_char);
	return 0;
}

/**
 * API:
 * Implements heart beat. This routine implements a visual cue that the whole
 * LCDproc server/driver and host are still alive. We could update the frame
 * buffer, but we chose to go straight to the hardware instead.
 * We do NOT rely on custom characters for this. Instead, we simply alternate
 * the frame buffer value with a special char in the corner. This way, we keep
 * the character under the heart beat somewhat visible. After all, this is only
 * 20x2. This is called by the server very often, so we pace ourselves, and
 * beat the heart on our schedule.
 * OPTIONAL
 */
MODULE_EXPORT void
sdeclcd_heartbeat(Driver *drvthis, int type)
{
	PrivateData *p = drvthis->private_data;

	if (time(NULL) <= p->hrbt_lasttime)
		return;
	/* Move cursor */
	sdec_exec(SDEC_FN_DD_ADD | SDEC_HB_ADD, p->bklgt);
	/* Write */
	if (HEARTBEAT_ON == type && HEARTBEAT_OFF == p->hb_stus)
		sdec_write(SDEC_HB_CHAR, p->bklgt);
	else
		sdec_write(p->framelcd[SDEC_HB_LOC], p->bklgt);
	/* Toggle status */
	p->hb_stus = (HEARTBEAT_ON == p->hb_stus) ?
		HEARTBEAT_OFF : HEARTBEAT_ON;
	/* Save time */
	p->hrbt_lasttime = time(NULL);
}

/**
 * API: Draw a horizontal bar to the right.
 */
MODULE_EXPORT void
sdeclcd_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int i, j;

	if (hbar != p->ccmode) {
		for (i = 0; i < SDEC_CELL_W; i++) {
			for (j = 0; j < SDEC_CELL_H; j++) {
				/* Set Custom Char Address */
				sdec_exec(SDEC_FN_CG_ADD | (i * SDEC_CELL_H + j), p->bklgt);
				/* Write bitmap at Address */
				sdec_write(p->hbar_cg[i * SDEC_CELL_H + j], p->bklgt);
			}
		}
		p->ccmode = hbar;
	}
	/* 1 bar is at char 0 or 8, so offset is 7 (avoiding offset of -1) */
	lib_hbar_static(drvthis, x, y, len, promille, options, SDEC_CELL_W, 7);
}

/**
 * API: Draw a vertical bar bottom-up.
 */
MODULE_EXPORT void
sdeclcd_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int i, j;

	if (vbar != p->ccmode) {
		for (i = 0; i < SDEC_NUM_CC; i++) {
			for (j = 0; j < SDEC_CELL_H; j++) {
				/* Set Custom Char Address */
				sdec_exec(SDEC_FN_CG_ADD | (i * SDEC_CELL_H + j), p->bklgt);
				/* Write bitmap at Address */
				sdec_write(p->vbar_cg[i * SDEC_CELL_H + j], p->bklgt);
			}
		}
		p->ccmode = vbar;
	}
	/* see note in hbar */
	lib_vbar_static(drvthis, x, y, len, promille, options, SDEC_CELL_H, 7);
}

/**
 * API: Write a big number to the screen.
 */
MODULE_EXPORT void
sdeclcd_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	int i, j;

	/* Big Nums are made of these chunks: */
	static char bignum_map[11][2][2] = {
		{{0, 1}, {2, 3}},	/* 0 */
		{{32, 4}, {32, 4}},	/* 1 */
		{{5, 6}, {7, 5}},
		{{5, 6}, {5, 6}},
		{{2, 3}, {32, 4}},
		{{7, 5}, {5, 6}},
		{{7, 5}, {7, 6}},
		{{0, 1}, {32, 4}},
		{{7, 6}, {7, 6}},
		{{7, 6}, {5, 6}},	/* 9 */
		{{0xA1, 32}, {0xDF, 32}}	/* : */
	};

	if (num < 0 || num > 10)
		return;

	/* Check custom char in LCD memory */
	if (bignum != p->ccmode) {
		for (i = 0; i < SDEC_NUM_CC; i++) {
			for (j = 0; j < SDEC_CELL_H; j++) {
				/* Set Custom Char Address */
				sdec_exec(SDEC_FN_CG_ADD | (i * SDEC_CELL_H + j), p->bklgt);
				/* Write bitmap at Address */
				sdec_write(p->bignum_cg[i * SDEC_CELL_H + j], p->bklgt);
			}
		}
		p->ccmode = bignum;
	}
	/* Update the frame buffer */
	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			if (bignum_map[num][j][i] != ' ')
				sdeclcd_chr(drvthis, x + i, j + 1, bignum_map[num][j][i]);
}

/**
 * API:
 * This routine implements the control of the back light. Here,
 * we set the back light status variable only, and let the next update take
 * care of the actual light control line.
 * Try and keep the light off, unless someone pushed a button not too long ago.
 * The light has a poor half life, so try and preserve it.
 */
MODULE_EXPORT void
sdeclcd_backlight(Driver *drvthis, int level)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (time(NULL) - p->bklgt_lasttime >= p->bklgt_timer || 0 == level)
		p->bklgt = BACKLIGHT_OFF;
	else
		p->bklgt = BACKLIGHT_ON;
}

/**
 * API: Get key from keypad. This routine implements the polling of the
 * keypad. This is not part of the SDEC LCD, but is wired through the
 * parallel port as well. It seems natural to include it in this driver.
 * Different boxes have different mapping to keys, but all codes are
 * fortunately unique. As this routine is called periodically by the server,
 * this is where we added the logic to turn off the backlight after some
 * time.
 */
MODULE_EXPORT const char *
sdeclcd_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *)drvthis->private_data;
	unsigned kbd;

	/*
	 * check backlight timer. It is important to make sure we turn the
	 * light off when there is no activity. With a half life of only
	 * 3,000 hours, we need to preserve it.
	 */
	if (time(NULL) - p->bklgt_lasttime >= p->bklgt_timer)
		p->bklgt = BACKLIGHT_OFF;
	else
		p->bklgt = BACKLIGHT_ON;

	/* read keyboard status */
	kbd = port_in(LPT_STATUS) & LPT_STUS_MASK;
	/* check if keyboard changed */
	if (kbd == p->lastkbd)
		return NULL;
	/* reset backlight counter */
	p->bklgt_lasttime = time(NULL);

	p->lastkbd = kbd;
	/*-
	 * Return key text according to status reg mapping:
	 * X-e 	 U:70 R:F8 D:68 L:58 Rel:78
	 * X-peak U:C8 R:E0 D:C0 L:E8 Rel: 88,80,A8,A0
	 */
	switch (kbd) {
	    case 0x70:
	    case 0xC8:
		return ("Up");
	    case 0xF8:
	    case 0xE0:
		return ("Right");
	    case 0x68:
	    case 0xC0:
		return ("Down");
	    case 0x58:
	    case 0xE8:
		return ("Left");
	    case 0x78:		/* button Releases */
	    case 0x88:
	    case 0x80:
	    case 0xA8:
	    case 0xA0:
		return (NULL);
	    default:		/* Code not mapped: */
		report(RPT_DEBUG, "LCDd sdeclcd.c/sdeclcd_get_key() %2x unmapped", kbd);
		return (NULL);
	}
}

/**
 * API: Return info string about this driver.
 */
MODULE_EXPORT const char *
sdeclcd_get_info(Driver *drvthis)
{
	return SDEC_IDENT;
}
