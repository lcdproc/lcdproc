/**
 * \file server/drivers/serialPOS_aedex.c
 *
 * LCDd Helper functions for driving AEDEX displays through
 * \c serialPOS
 *
 * See \file server/drivers/serialPOS_common.h for information on
 * functions.
 */

/*-
 * These are the helper functions for driving Point Of Sale ("POS") devices
 * using the LCDproc serialPOS driver through the AEDEX protocol
 *
 * Copyright (C) 2006, 2007 Eric Pooch
 * Copyright (C) 2018, Shenghao Yang
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

#include "serialPOS_common.h"

/**
 * Maximum size of the stream of characters
 * that can be sent to an AEDEX display,
 * when updating the display in either of the three update modes, in bytes.
 */
static const int AEDEX_MAX_CHARACTER_STREAM_LENGTH = 40;

/**
 * Maximum height of an AEDEX display - protocol limitation
 */
static const int AEDEX_MAX_HEIGHT = 2;

/*
 * AEDEX command sequences - smart compiler should
 * optimize these all out, if possible.
 */

/**
 * AEDEX command sequence to change attention code
 *
 * The \code # character seems to confuse some displays, so we change
 * the attention code to \code SPACE and \code SPACE
 */
static const uint8_t AEDEX_CHANGE_ATTN_CODE_TO_NUL_SOH[] = {0x21, 0x23, 0x38,
							    0x20, 0x20, 0x0d};

/**
 * AEDEX command sequence to indicate a stream of characters
 * destined for both lines.
 *
 * Consists of the header for the AEDEX write single double line command.
 *
 * Might not operate successfully if the screen is not 20xHEIGHT - if the
 * screen is not 20xHEIGHT, we write data to the top and bottom rows
 * separately when updating. This is no issue when clearing the display.
 */
static const uint8_t AEDEX_BOTH_LINE_START[] = {0x20, 0x20, 0x39};

/**
 * AEDEX command sequence to indicate a stream of characters
 * destined for the upper line.
 *
 * Consists of the header for the AEDEX write single line (top) command.
 */
static const uint8_t AEDEX_UPPER_LINE_START[] = {0x20, 0x20, 0x31};

/**
 * AEDEX command sequence to indicate a stream of characters
 * destined for the lower line
 *
 * Consists of the header for the AEDEX write single line (bottom)
 * command.
 */
static const uint8_t AEDEX_LOWER_LINE_START[] = {0x20, 0x20, 0x32};

/**
 * AEDEX command sequence to terminate a stream of characters to
 * all three line destinations.
 *
 * Consists of the end byte for all the static AEDEX write line(s)
 * commands.
 */
static const uint8_t AEDEX_LINE_END[] = {0x0d};

/*
 * For documentation on what these functions do, see
 * serialPOS_common.h
 */

static int command_buffer_sz(PrivateData* data);

static int init(PrivateData* data, unsigned char* buffer);

static int flush(PrivateData* data, unsigned char* buffer);

/**
 * serialPOS operations function pointer structure for the AEDEX protocol
 *
 * Supported operations:
 * - \c command_buffer_sz
 * - \c init
 * - \c flush
 */
const ops serialPOS_aedex_ops = {
    command_buffer_sz, init, flush, NULL
};

static int
command_buffer_sz(PrivateData* data)
{
	/*
	 * In the worst case situation, with the display having a width of
	 * AEDEX_MAX_CHARACTER_STREAM_LENGTH and two lines,
	 * we need to update each line separately, sending
	 * all AEDEX_MAX_CHARACTER_STREAM_LENGTH characters at once for each
	 * line.
	 *
	 * The size of the command buffer returned is the amount of space
	 * both these commands will take to send.
	 */
	return (sizeof(AEDEX_LOWER_LINE_START) + sizeof(AEDEX_UPPER_LINE_START)
		+ (2 * sizeof(AEDEX_LINE_END))
		+ (2 * AEDEX_MAX_CHARACTER_STREAM_LENGTH));
}

static int
init(PrivateData* data, uint8_t* buffer)
{
	/*
	 * AEDEX displays have a maximum dimension of
	 * AEDEX_MAX_CHARACTER_STREAM_LENGTH x 2 (w x h),
	 * return error if display is not of the right size
	 */
	if ((data->width > AEDEX_MAX_CHARACTER_STREAM_LENGTH)
	    || (data->height > AEDEX_MAX_HEIGHT)) {
		return -1;
	}
	/*
	 * AEDEX protocol - no private data, it is nearly stateless.
	 *
	 * We accept default values for fields in PrivateData structure
	 */
	/* Change attention code and clear display */
	unsigned char* const start = buffer;
	buffer = bytecpy_advance_ptr(buffer, AEDEX_CHANGE_ATTN_CODE_TO_NUL_SOH,
				     sizeof(AEDEX_CHANGE_ATTN_CODE_TO_NUL_SOH));
	buffer = bytecpy_advance_ptr(buffer, AEDEX_BOTH_LINE_START,
				     sizeof(AEDEX_BOTH_LINE_START));
	*(buffer++) = ' ';
	buffer =
	    bytecpy_advance_ptr(buffer, AEDEX_LINE_END, sizeof(AEDEX_LINE_END));
	return (buffer - start);
}

static int
flush(PrivateData* data, uint8_t* buffer)
{
	/*
	 * Check which lines to flush
	 */
	uint8_t* const start   = buffer;
	uint32_t lines_flushed = serialPOS_lines_to_flush(data);
	switch (lines_flushed) {
	    case 0:
		/* No lines need flushing */
		break;
	    case 3:
		/* Both lines need flushing */
		if (data->width == 20) {
			/* Display width is 20 chars, we can use twin
			 * line update */
			buffer = bytecpy_advance_ptr(
			    buffer, AEDEX_BOTH_LINE_START,
			    sizeof(AEDEX_BOTH_LINE_START));
			buffer = bytecpy_advance_ptr(
			    buffer, data->framebuf,
			    data->width * data->height);
			buffer =
			    bytecpy_advance_ptr(buffer, AEDEX_LINE_END,
						sizeof(AEDEX_LINE_END));
		}
		else {
			/*
			 * Display width is not 20 chars, we must do two
			 * separate updates
			 */
			buffer = bytecpy_advance_ptr(
			    buffer, AEDEX_UPPER_LINE_START,
			    sizeof(AEDEX_UPPER_LINE_START));
			buffer = bytecpy_advance_ptr(
			    buffer, data->framebuf, data->width);
			buffer =
			    bytecpy_advance_ptr(buffer, AEDEX_LINE_END,
						sizeof(AEDEX_LINE_END));
			buffer = bytecpy_advance_ptr(
			    buffer, AEDEX_LOWER_LINE_START,
			    sizeof(AEDEX_LOWER_LINE_START));
			buffer = bytecpy_advance_ptr(
			    buffer, data->framebuf + data->width,
			    data->width);
			buffer =
			    bytecpy_advance_ptr(buffer, AEDEX_LINE_END,
						sizeof(AEDEX_LINE_END));
		}
		break;
	    case 1:
		/* Only first line needs flushing */
		buffer =
		    bytecpy_advance_ptr(buffer, AEDEX_UPPER_LINE_START,
					sizeof(AEDEX_UPPER_LINE_START));
		buffer = bytecpy_advance_ptr(buffer, data->framebuf,
					     data->width);
		buffer = bytecpy_advance_ptr(buffer, AEDEX_LINE_END,
					     sizeof(AEDEX_LINE_END));
		break;
	    case 2:
		/* Only second line needs flushing */
		buffer =
		    bytecpy_advance_ptr(buffer, AEDEX_LOWER_LINE_START,
					sizeof(AEDEX_LOWER_LINE_START));
		buffer = bytecpy_advance_ptr(
		    buffer, data->framebuf + data->width, data->width);
		buffer = bytecpy_advance_ptr(buffer, AEDEX_LINE_END,
					     sizeof(AEDEX_LINE_END));
		break;
	}

	return (buffer - start);
}
