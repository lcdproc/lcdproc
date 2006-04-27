/* interfacing routines for the CrystalFontz CFA-631, CFA-633 and CFA-635 LCDs

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307

 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#if defined(HAVE_SYS_SELECT_H)
# include <sys/select.h>
#else
# include <sys/time.h>
# include <sys/types.h>
#endif /* defined(HAVE_SYS_SELECT_H) */

#include "CFontz633io.h"


#define TRY_AGAIN 0
#define GOOD_MSG 1
#define GIVE_UP 2


/* static local functions */
static void send_packet(int fd, COMMAND_PACKET *out, COMMAND_PACKET *in);
static int  get_crc(unsigned char *buf, int len, int seed);
static int  test_packet(int fd, unsigned char response, COMMAND_PACKET *in);
static int  check_for_packet(int fd, COMMAND_PACKET *in, unsigned char expected_length);
static void print_packet(COMMAND_PACKET *packet);


/* global variables */
KeyRing keyring;
ReceiveBuffer receivebuffer;



/*
 * KeyRing handling functions.
 * This separates the producer from the consumer.
 * It is just a small fifo of unsigned char.
 */

/** initialize/empty key ring by resetting its read & write pointers */
void EmptyKeyRing(KeyRing *kr)
{
	kr->head = kr->tail = 0;
}


/** add byte to key ring; return success (byte added) / failure (key ring is full) */
int AddKeyToKeyRing(KeyRing *kr, unsigned char key)
{
	if (((kr->head + 1) % KEYRINGSIZE) != (kr->tail % KEYRINGSIZE)) {
		/* fprintf(stderr, "We add key: %d\n", key); */

	        kr->contents[kr->head % KEYRINGSIZE] = key;
  		kr->head = (kr->head + 1) % KEYRINGSIZE;
		return 1;
	}

	/* KeyRing overflow: do not accept extra key */
	return 0;
}


/** get byte from key ring (or '\\0' if key ring is empty) */
unsigned char GetKeyFromKeyRing(KeyRing *kr)
{
	unsigned char retval = '\0';

	kr->tail %= KEYRINGSIZE;

	if ((kr->head % KEYRINGSIZE) != kr->tail) {
		retval = kr->contents[kr->tail];
	        kr->tail = (kr->tail + 1) % KEYRINGSIZE;
	}

	/*  if (retval) fprintf(stderr, "We remove key: %d\n", retval); */
	return retval;
}



/** send message with arguments to the given handle */
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


/** send message with one byte argument to the given handle */
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


/** send message without data to the given handle */
void send_zerobyte_message(int fd, unsigned char msg)
{
	COMMAND_PACKET out;
	COMMAND_PACKET in;

	out.command = msg;
	out.data_length = 0;

	/* send message & calc CRC */
	send_packet(fd, &out, &in);
}


/** send out to the given handle; calc & send CRC when doing so */
static void
send_packet(int fd, COMMAND_PACKET *out, COMMAND_PACKET *in)
{
	write(fd, &out->command, 1);
	write(fd, &out->data_length, 1);
	if (out->data_length > 0)
		write(fd, out->data, out->data_length);

	/* calculate & send the CRC */
	out->crc.as_word = get_crc((unsigned char *) out, out->data_length + 2, 0xFFFF);
	write(fd, out->crc.as_bytes, 2);

	/**** TEST STUFF ****/
	// print_packet(out);

	/* Every time we send a message, we also check for an incoming one. */
	test_packet(fd, 0x40 | out->command, in); 
}


/** calculate CRC over given buffer with given length */
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


/* circular buffer: */
/*                  v peek                            */
/* -------------------------------------------------- */
/*           ^ tail              ^ head               */


/** initialize/empty receive buffer by resetting its pointers */
void EmptyReceiveBuffer(ReceiveBuffer *rb)
{
	rb->head = rb->tail = rb->peek = 0;
}


/** read given number of bytes from given file handle into receive buffer */
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
#endif /* defined(HAVE_SELECT) && defined(CFONTZ633_WRITE_DELAY) && (CFONTZ633_WRITE_DELAY > 0) */	

	if (number > MAX_DATA_LENGTH)
		number = MAX_DATA_LENGTH;
	BytesRead = read(fd, buffer, number);

	if (BytesRead == -1) {
		/* this shouldnot happen with the select() above */
		/* fprintf(stderr, "~~~Problem reading: %s .\n", strerror(errno)); */
	}
	else {
		int	i;

		/* fprintf(stderr, "Read %d Bytes:", BytesRead); */

		/* wrap write pointer to the receive buffer */
		rb->head %= RECEIVEBUFFERSIZE;

		/* store the bytes read */
		for (i = 0; i < BytesRead; i++) {
			/* fprintf(stderr, " %02x", buffer[i]); */
			rb->contents[rb->head] = buffer[i];

			/* increment write pointer (wrap if needed) */
			rb->head = (rb->head + 1) % RECEIVEBUFFERSIZE;
		}
		/* fprintf(stderr, "\n"); */
	}
}


/** return number of bytes available for reading in receive buffer */
int BytesAvail(ReceiveBuffer *rb)
{
	int avail_bytes = rb->head - rb->tail;

	if (avail_bytes < 0)
		avail_bytes += RECEIVEBUFFERSIZE;

	return(avail_bytes % RECEIVEBUFFERSIZE);
}


/** get next byte from receive buffer (return '\\0' if buffer is empty) */
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


/** return number of bytes available for peeking in receive buffer */
int PeekBytesAvail(ReceiveBuffer *rb)
{
	int avail_bytes = rb->head - rb->peek;

	if (avail_bytes < 0)
		avail_bytes += RECEIVEBUFFERSIZE;

	return(avail_bytes % RECEIVEBUFFERSIZE);
}


/** sync peek pointer with read pointer */
void SyncPeekPointer(ReceiveBuffer *rb)
{
	rb->peek = rb->tail;
}


/** accept ppeked data by syncing the read pointer to the peek pointer */
void AcceptPeekedData(ReceiveBuffer *rb)
{
	rb->tail = rb->peek;
}


/** peek next byte from receive buffer (return '\\0' if buffer is empty) */
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



/* I should use the value GIVE_UP and not reenter if there is no extra
 * byte read from the serial port
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
#endif /* defined(HAVE_SELECT) && defined(CFONTZ633_WRITE_DELAY) && (CFONTZ633_WRITE_DELAY > 0) */
}



/*============================================================================
 *                              check_for_packet()
 *
 * check_for_packet() will see if there is a valid packet in the input buffer.
 * If there is, it will copy it into incoming_command and return 1. If there
 * is not it will return 0. incoming_packet may get partially filled with
 * garbage if there is not a valid packet available.
 *----------------------------------------------------------------------------
 */


/* Let's return
 * O if we have no message but we should try again immediatly
 * 1 if we have a message correctly identified
 * 2 if we have no message and we should not retry until new input
 * So a loop should run as long as we have no 0
 * If we have a 2 we should avoid comming back there.
 */
static int
check_for_packet(int fd, COMMAND_PACKET *in, unsigned char expected_length)
{
	int i;
	int testcrc;

	SyncReceiveBuffer(&receivebuffer, fd, expected_length);

	//First off, there must be at least 4 bytes available in the input stream
	//for there to be a valid command in it (command, length, no data, CRC).
	if (BytesAvail(&receivebuffer) < 4) {
		/* fprintf(stderr, "Not enough bytes available for even the smallest message.\n"); */
		return(GIVE_UP); /* We don't need to retry before more byte are received */
	}

	/* Look into the buffer without removing the data. */
	SyncPeekPointer(&receivebuffer);

	/* look at potential command byte */
	in->command = PeekByte(&receivebuffer);

	/* Only commands 0 through MAX_COMMAND are valid */
	if (MAX_COMMAND < (0x3F & in->command)) {
		/* Throw out one byte of garbage. Next pass through should re-sync. */
		GetByte(&receivebuffer);
		/* fprintf(stderr, "###: Unknown command.\n"); */
		return(TRY_AGAIN);
	}

  	/* There is a valid command byte. Get the data_length. */
	in->data_length = PeekByte(&receivebuffer);

  	/* The data length must be within reason. */
  	if (MAX_DATA_LENGTH < in->data_length) {
		//Throw out one byte of garbage. Next pass through should re-sync.
		GetByte(&receivebuffer);
		/* fprintf(stderr, "###: Too long packet: %d.\n", in->data_length); */
		return(TRY_AGAIN);
	}

	// Now there must be at least in->data_length + sizeof(CRC) bytes
	// still available for us to continue.
	if ((int) PeekBytesAvail(&receivebuffer) < (in->data_length + 2)) {
		//It looked like a valid start of a packet, but it does not look
		//like the complete packet has been received yet.
		/* fprintf(stderr, "Not enough read to check the complete message.\n"); */
		return(GIVE_UP); /* Let's not return until more byte are available */
	}

	/* There is enough data to make a packet. Transfer over the data. */
	for (i = 0; i < in->data_length; i++)
		in->data[i] = PeekByte(&receivebuffer);

	//Now move over the CRC.
	in->crc.as_bytes[0] = PeekByte(&receivebuffer);
	in->crc.as_bytes[1] = PeekByte(&receivebuffer);
	//Now check the CRC.

	//Compute the expected CheckSum
	testcrc = get_crc((unsigned char *) in, in->data_length+2, 0xFFFF);

	if (in->crc.as_word == testcrc) {
		//This is a good packet. I'll be horn swaggled. Remove the packet
		//from the serial buffer.
		AcceptPeekedData(&receivebuffer);
		//Let our caller know that incoming_command has good stuff in it.
		/* print_packet(&outgoing_response); */

		return(GOOD_MSG);
	}

	/* The CRC did not match. Toss out one byte of garbage.
	* Next pass through should re-sync.
	*/
	GetByte(&receivebuffer);
	/* fprintf(stderr, "###: Wrong CheckSum. computed/real %04x:%04x \n",
  		   testcrc, incoming_command.crc.as_word); */
	return(TRY_AGAIN);
}


/*
 * This is a debugging function.
 * It should be removed or compiled in conditionally.
 * Currently it is still using printf for debugging.
 */
static void
print_packet(COMMAND_PACKET *packet)
{
	int i, cmd, top, len; 

	top = (0xC0 & (packet->command)) >> 6;
	cmd = (0x3F & (packet->command));
	len = packet->data_length;

	fprintf(stderr, "Message (%d,%d) %d [", top, cmd, len);

	//There is enough data to make a packet. Transfer over the data.
	for (i = 0; i < packet->data_length; i++)
		fprintf(stderr, " %02x", packet->data[i]);

	fprintf(stderr, " ] %02x %02x .\n", packet->crc.as_bytes[0], packet->crc.as_bytes[1]);
}

