/** \file server/drivers/CFontz633io.c
 * I/O routines for the \c CFontzPacket driver. Currently the CFA-631,
 * CFA-533, CFA-633 and CFA-635 LCDs use this type of protocol.
 *
 * \todo  As no reporting using report.h is possible here (does not has access
 *        to drvthis) make the send_#_message functions return some error code
 *        if send failed (or an error response is received).
 * \todo  Make the content of a response packet available to the driver.
 * \note  Modifying send_#_functions makes sound only if the CFontz633 driver
 *        has been removed.
 */

/*-
  ===========================================================================
  633 Test code for linux.
  Copyright 2002, David GLAUDE
  Partial copyright
  2001 Crystalfontz America, Inc.
  brent@crystalfontz.com
  Written by Brent A. Crosby
  www.crystalfontz.com
  ===========================================================================

  Partial copyright (C) 2005-2006 Peter Marschall

  Most of the code in this file is based on the translation from the windows
  code.

  Intensive code analysis performed by Peter Marschall showed that,
  at the time of the release of the GPL'ed interfacing code for the
  CF631/633 from CrystalFontz by Brent A. Crosby in 2004, the code
  in this file was either written by David Glaude for LCDproc or
  identical to the GPL'ed code or so close to it that the differences
  are negligible.

  For detailed information about the code analysis, please
  contact the LCDproc maintainer, Peter Marschall <peter@adpm.de>

  This file is hereby released under the terms below:

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#if defined(HAVE_SYS_SELECT_H)
# include <sys/select.h>
#else
# include <sys/time.h>
# include <sys/types.h>
#endif

#include "CFontz633io.h"

/* Return values for the check_for_packet() */
#define TRY_AGAIN 0
#define GOOD_MSG 1
#define GIVE_UP 2

/* define CFONTZ633_WRITE_DELAY to use select when waiting for packet acknowledgement */
#if !defined(CFONTZ633_WRITE_DELAY)
# define CFONTZ633_WRITE_DELAY 250
#endif


/* static local functions */
static void send_packet(int fd, COMMAND_PACKET *out, COMMAND_PACKET *in);
static int  get_crc(unsigned char *buf, int len, int seed);
static int  test_packet(int fd, unsigned char response, COMMAND_PACKET *in);
static int  check_for_packet(int fd, COMMAND_PACKET *in, unsigned char expected_length);
#ifdef DEBUG
static void print_packet(COMMAND_PACKET *packet);
#endif


/** \addtogroup CFA_KeyRing
 *
 * KeyRing handling functions.
 * This separates the producer from the consumer.
 * It is just a small fifo of unsigned char.
 * @{
 */

/* Global variable */
KeyRing keyring;

/**
 * Initialize/empty key ring by resetting its read & write pointers.
 * \param kr  Pointer to KeyRing.
 */
void EmptyKeyRing(KeyRing *kr)
{
	kr->head = kr->tail = 0;
}


/**
 * Add byte to key ring.
 * \param kr   Pointer to KeyRing.
 * \param key  Key byte to add.
 * \retval 1  Success (byte added).
 * \retval 0  Failure (key ring is full).
 */
int AddKeyToKeyRing(KeyRing *kr, unsigned char key)
{
	if (((kr->head + 1) % KEYRINGSIZE) != (kr->tail % KEYRINGSIZE)) {
		kr->contents[kr->head % KEYRINGSIZE] = key;
		kr->head = (kr->head + 1) % KEYRINGSIZE;
		return 1;
	}

	/* KeyRing overflow: do not accept extra key */
	return 0;
}


/**
 * Get byte from key ring.
 * \param kr  Pointer to KeyRing.
 * \retval retval  Byte from KeyRing.
 * \retval '\0'    Failure (key ring is empty).
 */
unsigned char GetKeyFromKeyRing(KeyRing *kr)
{
	unsigned char retval = '\0';

	kr->tail %= KEYRINGSIZE;

	if ((kr->head % KEYRINGSIZE) != kr->tail) {
		retval = kr->contents[kr->tail];
	        kr->tail = (kr->tail + 1) % KEYRINGSIZE;
	}

	return retval;
}
/** @} */



/**
 * Send message with arguments to the given handle.
 * \param fd    File handle to write to.
 * \param msg   Command byte to write.
 * \param len   Length (in bytes) of data following.
 * \param data  Pointer to command argument data.
 */
void send_bytes_message(int fd, unsigned char msg, int len, unsigned char *data)
{
	COMMAND_PACKET out;
	COMMAND_PACKET in;

	out.command = msg;
	out.data_length = (unsigned char) ((len > MAX_DATA_LENGTH) ? MAX_DATA_LENGTH : len);
	memcpy(out.data, data, out.data_length);

	/* send message & calc CRC */
	send_packet(fd, &out, &in);
}


/**
 * Send message with one byte argument to the given handle.
 * \param fd     File handle to write to.
 * \param msg    Command byte to write.
 * \param value  Command argument.
 */
void send_onebyte_message(int fd, unsigned char msg, unsigned char value)
{
	COMMAND_PACKET out;
	COMMAND_PACKET in;

	out.command = msg;
	out.data_length = 1;
	out.data[0] = value;

	/* send message & calc CRC */
	send_packet(fd, &out, &in);
}


/**
 * Send message without arguments to the given handle.
 * \param fd    File handle to write to.
 * \param msg   Command byte to write.
 */
void send_zerobyte_message(int fd, unsigned char msg)
{
	COMMAND_PACKET out;
	COMMAND_PACKET in;

	out.command = msg;
	out.data_length = 0;

	/* send message & calc CRC */
	send_packet(fd, &out, &in);
}


/**
 * Send out to the given handle; calc & send CRC when doing so.
 * \param fd    File handle to write to.
 * \param out   Pointer to COMMAND_PACKET structure to write.
 * \param in    Pointer to COMMAND_PACKET structure to read after write.
 */
static void
send_packet(int fd, COMMAND_PACKET *out, COMMAND_PACKET *in)
{
	unsigned char CRC[2];

	write(fd, &out->command, 1);
	write(fd, &out->data_length, 1);
	if (out->data_length > 0)
		write(fd, out->data, out->data_length);

	/* calculate & send the CRC: convert to bytes manually to avoid endianess issues */
	out->crc = get_crc((unsigned char *) out, out->data_length + 2, 0xFFFF);
	CRC[0] = out->crc & 0xFF;
	CRC[1] = (out->crc >> 8) & 0xFF;
	write(fd, CRC, 2);

	/**** TEST STUFF ****/
	//print_packet(out);

	/* Every time we send a message, we also check for an incoming one. */
	test_packet(fd, 0x40 | out->command, in);
}


/**
 * Calculate CRC over given buffer with given length.
 * \param buf   Byte buffer.
 * \param len   Length of byte buffer.
 * \param seed  CRC seed value.
 * \return  CRC calulated.
 */
/* According to the "Painless Guide to CRC error dectectin algorithmsi
 * (http://www.repairfaq.org/filipg/LINK/F_crc_v3.html) this is the table driven
 * implementation of a CRC with the following parameters:
 * - WIDTH:  16		(16 bit CRC)
 * - POLY:   0x1021	(generating polynomial)
 * - INIT:   0xFFFF	(seed value for the register when starting the algorithm)
 * - REFIN:  TRUE	(reflect [i.e. bit-swap] each input byte before being processed)
 * - REFOUT: TRUE	(reflect [i.e. bit-swap] the result before the XOROUT phase)
 * - XOROUT: 0xFFFF	(value to xor the result before returning it as crc)
 */
static int
get_crc(unsigned char *buf, int len, int seed)
{

	/* CRC lookup table to avoid bit-shifting loops. */
	static const word crcLookupTable[256] = {
		0x00000, 0x01189, 0x02312, 0x0329B, 0x04624, 0x057AD, 0x06536, 0x074BF,
		0x08C48, 0x09DC1, 0x0AF5A, 0x0BED3, 0x0CA6C, 0x0DBE5, 0x0E97E, 0x0F8F7,
		0x01081, 0x00108, 0x03393, 0x0221A, 0x056A5, 0x0472C, 0x075B7, 0x0643E,
		0x09CC9, 0x08D40, 0x0BFDB, 0x0AE52, 0x0DAED, 0x0CB64, 0x0F9FF, 0x0E876,
		0x02102, 0x0308B, 0x00210, 0x01399, 0x06726, 0x076AF, 0x04434, 0x055BD,
		0x0AD4A, 0x0BCC3, 0x08E58, 0x09FD1, 0x0EB6E, 0x0FAE7, 0x0C87C, 0x0D9F5,
		0x03183, 0x0200A, 0x01291, 0x00318, 0x077A7, 0x0662E, 0x054B5, 0x0453C,
		0x0BDCB, 0x0AC42, 0x09ED9, 0x08F50, 0x0FBEF, 0x0EA66, 0x0D8FD, 0x0C974,
		0x04204, 0x0538D, 0x06116, 0x0709F, 0x00420, 0x015A9, 0x02732, 0x036BB,
		0x0CE4C, 0x0DFC5, 0x0ED5E, 0x0FCD7, 0x08868, 0x099E1, 0x0AB7A, 0x0BAF3,
		0x05285, 0x0430C, 0x07197, 0x0601E, 0x014A1, 0x00528, 0x037B3, 0x0263A,
		0x0DECD, 0x0CF44, 0x0FDDF, 0x0EC56, 0x098E9, 0x08960, 0x0BBFB, 0x0AA72,
		0x06306, 0x0728F, 0x04014, 0x0519D, 0x02522, 0x034AB, 0x00630, 0x017B9,
		0x0EF4E, 0x0FEC7, 0x0CC5C, 0x0DDD5, 0x0A96A, 0x0B8E3, 0x08A78, 0x09BF1,
		0x07387, 0x0620E, 0x05095, 0x0411C, 0x035A3, 0x0242A, 0x016B1, 0x00738,
		0x0FFCF, 0x0EE46, 0x0DCDD, 0x0CD54, 0x0B9EB, 0x0A862, 0x09AF9, 0x08B70,
		0x08408, 0x09581, 0x0A71A, 0x0B693, 0x0C22C, 0x0D3A5, 0x0E13E, 0x0F0B7,
		0x00840, 0x019C9, 0x02B52, 0x03ADB, 0x04E64, 0x05FED, 0x06D76, 0x07CFF,
		0x09489, 0x08500, 0x0B79B, 0x0A612, 0x0D2AD, 0x0C324, 0x0F1BF, 0x0E036,
		0x018C1, 0x00948, 0x03BD3, 0x02A5A, 0x05EE5, 0x04F6C, 0x07DF7, 0x06C7E,
		0x0A50A, 0x0B483, 0x08618, 0x09791, 0x0E32E, 0x0F2A7, 0x0C03C, 0x0D1B5,
		0x02942, 0x038CB, 0x00A50, 0x01BD9, 0x06F66, 0x07EEF, 0x04C74, 0x05DFD,
		0x0B58B, 0x0A402, 0x09699, 0x08710, 0x0F3AF, 0x0E226, 0x0D0BD, 0x0C134,
		0x039C3, 0x0284A, 0x01AD1, 0x00B58, 0x07FE7, 0x06E6E, 0x05CF5, 0x04D7C,
		0x0C60C, 0x0D785, 0x0E51E, 0x0F497, 0x08028, 0x091A1, 0x0A33A, 0x0B2B3,
		0x04A44, 0x05BCD, 0x06956, 0x078DF, 0x00C60, 0x01DE9, 0x02F72, 0x03EFB,
		0x0D68D, 0x0C704, 0x0F59F, 0x0E416, 0x090A9, 0x08120, 0x0B3BB, 0x0A232,
		0x05AC5, 0x04B4C, 0x079D7, 0x0685E, 0x01CE1, 0x00D68, 0x03FF3, 0x02E7A,
		0x0E70E, 0x0F687, 0x0C41C, 0x0D595, 0x0A12A, 0x0B0A3, 0x08238, 0x093B1,
		0x06B46, 0x07ACF, 0x04854, 0x059DD, 0x02D62, 0x03CEB, 0x00E70, 0x01FF9,
		0x0F78F, 0x0E606, 0x0D49D, 0x0C514, 0x0B1AB, 0x0A022, 0x092B9, 0x08330,
		0x07BC7, 0x06A4E, 0x058D5, 0x0495C, 0x03DE3, 0x02C6A, 0x01EF1, 0x00F78
	};

    	/* initialize the new crc with the given seed */
	int newCrc = seed;

	/* This algorithm is based on the IrDA LAP example */
	while (len-- > 0)
		newCrc = (newCrc >> 8) ^ crcLookupTable[(newCrc ^ *buf++) & 0xFF];

	/* Make this crc match the one's complement that is sent in the packet */
	return ((~newCrc) & 0xFFFF);
}


/** \addtogroup CFA_ReceiveBuffer
 *
 * ReceiveBuffer handling functions.
 * The receive buffer is a circular buffer of bytes with the possibility to
 * 'peek' into received bytes.
 *
 *\verbatim
 * |                 v peek                           |
 * |--------------------------------------------------|
 * |          ^ tail              ^ head              |
 *\endverbatim
 * @{
 */

/* Global variable */
ReceiveBuffer receivebuffer;

/**
 * Initialize/empty receive buffer by resetting its pointers.
 * \param rb  Pointer to ReceiveBuffer structure.
 */
void EmptyReceiveBuffer(ReceiveBuffer *rb)
{
	rb->head = rb->tail = rb->peek = 0;
}


/**
 * Read given number of bytes from given file handle into receive buffer.
 * \param rb      Pointer to ReceiveBuffer structure.
 * \param fd      File handle to read from.
 * \param number  Max. number of bytes to read from file handle.
 */
void SyncReceiveBuffer(ReceiveBuffer *rb, int fd, unsigned int number)
{
	unsigned char buffer[MAX_DATA_LENGTH];
	int BytesRead;

#if defined(HAVE_SELECT) && defined(CFONTZ633_WRITE_DELAY) && (CFONTZ633_WRITE_DELAY > 0)
	fd_set rfds;
	struct timeval timeout;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = CFONTZ633_WRITE_DELAY;
	retval = select(fd + 1, &rfds, NULL, NULL, &timeout);

	if (!retval)
		return;
#endif

	if (number > MAX_DATA_LENGTH)
		number = MAX_DATA_LENGTH;

	BytesRead = read(fd, buffer, number);

	if (BytesRead > 0) {
		int i;

		/* wrap write pointer to the receive buffer */
		rb->head %= RECEIVEBUFFERSIZE;

		/* store the bytes read wrapping at the buffer end */
		for (i = 0; i < BytesRead; i++) {
			rb->contents[rb->head] = buffer[i];
			rb->head = (rb->head + 1) % RECEIVEBUFFERSIZE;
		}
	}
}


/**
 * Get number of bytes available for reading in receive buffer.
 * \param rb  Pointer to ReceiveBuffer structure.
 * \return  Number of bytes available in receive buffer.
 */
int BytesAvail(ReceiveBuffer *rb)
{
	int avail_bytes = rb->head - rb->tail;

	if (avail_bytes < 0)
		avail_bytes += RECEIVEBUFFERSIZE;

	return(avail_bytes % RECEIVEBUFFERSIZE);
}


/**
 * Get next byte from receive buffer.
 * \param rb  Pointer to ReceiveBuffer structure.
 * \retval return_byte  Next byte in receive buffer.
 * \retval '\0'         Failure (receive buffer is empty).
 */
unsigned char GetByte(ReceiveBuffer *rb)
{
	unsigned char return_byte = '\0';

	/* wrap read pointer to the receive buffer */
	rb->tail %= RECEIVEBUFFERSIZE;

	/* See if there are any more bytes available. */
	if (rb->tail != (rb->head % RECEIVEBUFFERSIZE)) {
		/* There is at least one more byte. */
		return_byte = rb->contents[rb->tail];

		/* Increment read pointer (wrap if needed) */
		rb->tail = (rb->tail + 1) % RECEIVEBUFFERSIZE;
	}

	return(return_byte);
}


/**
 * Return number of bytes available for peeking in receive buffer.
 * \param rb  Pointer to ReceiveBuffer structure.
 * \return  Number of bytes available for peeking in receive buffer.
 */
int PeekBytesAvail(ReceiveBuffer *rb)
{
	int avail_bytes = rb->head - rb->peek;

	if (avail_bytes < 0)
		avail_bytes += RECEIVEBUFFERSIZE;

	return(avail_bytes % RECEIVEBUFFERSIZE);
}


/**
 * Sync peek pointer with read pointer.
 * \param rb  Pointer to ReceiveBuffer structure.
 */
void SyncPeekPointer(ReceiveBuffer *rb)
{
	rb->peek = rb->tail;
}


/**
 * Accept peeked data by syncing the read pointer to the peek pointer.
 * \param rb  Pointer to ReceiveBuffer structure.
 */
void AcceptPeekedData(ReceiveBuffer *rb)
{
	rb->tail = rb->peek;
}


/**
 * Peek next byte from receive buffer.
 * \param rb  Pointer to ReceiveBuffer structure.
 * \retval return_byte  Next byte in receive buffer.
 * \retval '\0'         Failure (receive buffer is empty).
 */
unsigned char PeekByte(ReceiveBuffer *rb)
{
	unsigned char return_byte = '\0';

	/* wrap peek pointer to the receive buffer */
	rb->peek %= RECEIVEBUFFERSIZE;

	/* See if there are any more bytes available. */
	if (rb->peek != (rb->head % RECEIVEBUFFERSIZE)) {
		/* There is at least one more byte. */
		return_byte = rb->contents[rb->peek];

		/* Increment the peek pointer (wrap if needed). */
		rb->peek = (rb->peek + 1) % RECEIVEBUFFERSIZE;
	}

	return(return_byte);
}
/** @} */


/**
 * Check for a response packet and try to identify it.
 * \param fd        File handle to read from.
 * \param response  Expected response command.
 * \param in        Pointer to COMMAND_PACKET structure to write the response to.
 * \retval 1  Expected response received.
 * \retval 0  Expected response not received.
 *
 * \todo check_for_packet is always called with MAX_DATA_LENGTH. This doesn't
 *       do any harm, but passing that parameter is useless then. Additionally
 *       one complete packet is MAX_DATA_LENGTH + 4 (command, length, CRC).
 */
static int
test_packet(int fd, unsigned char response, COMMAND_PACKET *in)
{
#if defined(HAVE_SELECT) && defined(CFONTZ633_WRITE_DELAY) && (CFONTZ633_WRITE_DELAY > 0)
	int response_received = 0;
	int loop;

	/* wait for answer packet but forever (LCDs should answer within max. 250ms) */
	for (loop = 250000/CFONTZ633_WRITE_DELAY; !response_received && loop > 0; loop--) {
		int is_msg = check_for_packet(fd, in, MAX_DATA_LENGTH);

		while (is_msg != GIVE_UP) {
			if (is_msg == GOOD_MSG) {
				/* key activity ? */
				if (in->command == 0x80)
					AddKeyToKeyRing(&keyring, in->data[0]);
				else if (in->command == response)
					response_received = 1;
			}

			is_msg = check_for_packet(fd, in, MAX_DATA_LENGTH);
		}
	}
	return(response_received);
#else
	int is_msg = check_for_packet(fd, in, MAX_DATA_LENGTH);
	while (is_msg != GIVE_UP) {
		if (is_msg == GOOD_MSG) {
			if (in->command == 0x80)
				AddKeyToKeyRing(&keyring, in->data[0]);
		}

		is_msg = check_for_packet(fd, in, MAX_DATA_LENGTH);
	}

	return 1;
#endif
}


/**
 * Check for a packet to read. If there is a valid packet in the input buffer
 * it will copy it into \c in and return GOOD_MSG. If there is no enough data
 * available for a valid packet it returns GIVE_UP.
 *
 * \param fd        File handle to read from.
 * \param in        Pointer to COMMAND_PACKET structure to write the response to.
 * \param expected_length  Expected response length.
 *
 * \retval GIVE_UP    No message and we should not retry until new input.
 * \retval TRY_AGAIN  No message but we should try again immediately.
 * \retval GOOD_MSG   Message correctly identified.
 */
static int
check_for_packet(int fd, COMMAND_PACKET *in, unsigned char expected_length)
{
	int i;
	int testcrc;

	SyncReceiveBuffer(&receivebuffer, fd, expected_length);

	/*
	 * There must be at least 4 bytes available in the input stream for
	 * there to be a valid command in it (command, length, no data, CRC).
	 */
	if (BytesAvail(&receivebuffer) < 4) {
		return(GIVE_UP);
	}

	/* Look into the buffer without removing the data. */
	SyncPeekPointer(&receivebuffer);

	/*
	 * Look at potential command byte. If it is not a valid command
	 * discard it and try again.
	 */
	in->command = PeekByte(&receivebuffer);
	if ((in->command & 0x3F) > MAX_COMMAND) {
		GetByte(&receivebuffer);
		return(TRY_AGAIN);
	}

	/*
	 * Get the data_length and check it is within valid range. If not
	 * discard it and try again.
	 */
	in->data_length = PeekByte(&receivebuffer);
	if (in->data_length > MAX_DATA_LENGTH) {
		GetByte(&receivebuffer);
		return(TRY_AGAIN);
	}

	/*
	 * Now there must be at least in->data_length + sizeof(CRC) bytes
	 * still available for us to continue. If not give up and try again
	 * if more bytes are available.
	 */
	if ((int) PeekBytesAvail(&receivebuffer) < (in->data_length + 2)) {
		return(GIVE_UP);
	}

	/* There is enough data to make a packet. Transfer over the data. */
	for (i = 0; i < in->data_length; i++)
		in->data[i] = PeekByte(&receivebuffer);

	/* Convert CRC bytes to CRC manually to avoid endianess issues */
	in->crc = PeekByte(&receivebuffer);
	in->crc |= PeekByte(&receivebuffer) << 8;

	/* Compute the expected checksum and compare it to the received one. */
	testcrc = get_crc((unsigned char *) in, in->data_length+2, 0xFFFF);
	if (in->crc == testcrc) {
		/* This is a good packet. Remove it from the serial buffer. */
		AcceptPeekedData(&receivebuffer);

		/**** TEST STUFF ****/
		//print_packet(in);

		return(GOOD_MSG);
	}

	/*
	 * The CRC did not match. Toss out one byte of garbage.  Next pass
	 * through should re-sync.
	 */
	GetByte(&receivebuffer);
	return(TRY_AGAIN);
}


#ifdef DEBUG
/*
 * This prints a hex dump of a packet to stderr.
 */
static void
print_packet(COMMAND_PACKET *packet)
{
	int i, cmd, top, len;

	top = (0xC0 & (packet->command)) >> 6;
	cmd = (0x3F & (packet->command));
	len = packet->data_length;

	fprintf(stderr, "Message (%d,%d) %d [", top, cmd, len);
	for (i = 0; i < packet->data_length; i++)
		fprintf(stderr, " %02x", packet->data[i]);
	fprintf(stderr, " ] %02x %02x .\n", packet->crc & 0xFF, (packet->crc >> 8) & 0xFF);
}
#endif
