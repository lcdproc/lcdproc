/** \file server/drivers/t6963_low.c
 * Low level output routines for Toshiba T6963 based LCD displays connected
 * to a parallel port.
 *
 * Wiring (no pins for the display given. Check with your datasheet!)
 *
 *\verbatim
 *  Parallel:              LCD:
 *  1 (Strobe) ----------- /WR
 *  2-9 (Data) ----------- DB0-DB7
 *  14 (Autofeed) -------- /CE
 *  16 (Init) ------------ C/D
 *  17 (Slct) ------------ /RD
 *                + 5V --- FS (6x8 font, used for t6963 driver)
 *                 GND --- FS (8x8 font, used for glcd connection type)
 *\endverbatim
 *
 */

/*-
 * Parts of this file are based on the kernel driver by
 * Alexander Frink <Alexander.Frink@Uni-Mainz.DE>
 *
 * Copyright (c) 2001 Manuel Stahl <mythos@xmythos.de>
 *               2011 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <stdio.h>

#include "port.h"
#include "lpt-port.h"
#include "timing.h"
#include "t6963_low.h"

/* Define the wiring */
#define nWR	nSTRB
#define nRD	nSEL
#define nCE	nLF
#define T_CMD	INIT
#define T_DATA	0x00		/* ~INIT didn't work here */


/**
 * Acquires access to parallel port and initializes timing. The parallel port
 * must be an I/O-address between 0x200 and 0x400.
 * \param p        Pointer to port configuration.
 * \return  0 on success, -1 if an error occured.
 */
int
t6963_low_init(T6963_port *p) {
	if ((p->port < 0x200) || (p->port > 0x400))
		return -1;

	if (port_access_multiple(p->port, 3))
		return -1;

	if (timing_init() == -1)
		return -1;

	return 0;
}


/**
 * Releases access to the parallel port.
 * \param p        Pointer to port configuration.
 */
void
t6963_low_close(T6963_port *p) {
	if ((p->port >= 0x200) && (p->port <= 0x400))
		port_deny_multiple(p->port, 3);
}

/**
 * Send one data byte to the display.
 * \param p        Pointer to port configuration.
 * \param byte     Data byte.
 */
void
t6963_low_data(T6963_port *p, u8 byte)
{
	t6963_low_dsp_ready(p, STA0|STA1);
	t6963_low_send(p, T_DATA, byte);
}

/**
 * Send one command byte to the display.
 * \param p        Pointer to port configuration.
 * \param byte     Command byte.
 */
void
t6963_low_command(T6963_port *p, u8 byte)
{
	t6963_low_dsp_ready(p, STA0|STA1);
	t6963_low_send(p, T_CMD, byte);
}

/**
 * Write one byte of data to display in AUTO mode (needs a different ready
 * check).
 * \param p        Pointer to port configuration.
 * \param byte     Data byte.
 */
void
t6963_low_auto_write(T6963_port *p, u8 byte)
{
	t6963_low_dsp_ready(p, STA3);
	t6963_low_send(p, T_DATA, byte);
}

/**
 * Send one byte of data followed by one command byte to the display.
 * \param p        Pointer to port configuration.
 * \param cmd      Command byte.
 * \param byte     Data value.
 */
void
t6963_low_command_byte(T6963_port *p, u8 cmd, u8 byte)
{
	t6963_low_data(p, byte);
	t6963_low_command(p, cmd);
}

/**
 * Send one word (two bytes) of data followed by one command byte to the
 * display. Low byte is output first.
 * \param p        Pointer to port configuration.
 * \param cmd      Command byte.
 * \param word     Data value (2 bytes)
 */
void
t6963_low_command_word(T6963_port *p, u8 cmd, u16 word)
{
	t6963_low_data(p, word & 0xFF);
	t6963_low_data(p, (word >> 8) & 0xFF);
	t6963_low_command(p, cmd);
}

/**
 * Check display status.
 * \param p        Pointer to port configuration.
 * \param sta      Bitmap of expected STA flags
 * \return  0 on success, -1 if ready could not be read
 */
int
t6963_low_dsp_ready(T6963_port *p, u8 sta)
{
	int portcontrol = 0;

	if (p->bidirectLPT == 1) {
		int val;
		int loop = 0;

		do {
			portcontrol = T_CMD | nWR | nRD | nCE;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			/* lower nRD, nCE, set bi-directional mode */
			portcontrol = T_CMD | nWR | ENBI;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			/* possible wait required here: tACC = 150 ns max */
			if (p->delayBus)
				timing_uPause(1);
			val = port_in(T6963_DATA_PORT(p->port));
			portcontrol = T_CMD | nWR | nRD | nCE;
			port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
			loop++;
			if (loop == 100)
				return -1;
		} while ((val & sta) != sta);
	}
	else {
		portcontrol = T_CMD | nWR | nRD | nCE;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
		portcontrol = T_CMD | nWR;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
		timing_uPause(150);
		portcontrol = T_CMD | nWR | nRD | nCE;
		port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	}

	return 0;
}

/**
 * Write a single command / or byte to the parallel port.
 * \param p        Pointer to port configuration.
 * \param type     Command or Data
 * \param byte     Data byte.
 */
inline void
t6963_low_send(T6963_port *p, u8 type, u8 byte)
{
	int portcontrol = 0;

	portcontrol = type | nWR | nRD | nCE;
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	port_out(T6963_DATA_PORT(p->port), byte);
	portcontrol = type | nRD;	/* lower nWR, nCE */
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
	/* possible wait required here: tWR */
	if (p->delayBus)
		timing_uPause(1);
	portcontrol = type | nWR | nRD | nCE;
	port_out(T6963_CONTROL_PORT(p->port), portcontrol ^ OUTMASK);
}
