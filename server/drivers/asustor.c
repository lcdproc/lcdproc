/** \file server/drivers/asustor.c
 * LCDd \c asustor driver for ASUSTOR NAS front-panel LCD modules.
 *
 * ASUSTOR systems expose the LCD microcontroller through a 115200 baud serial
 * port, usually /dev/ttyS1.  The protocol is not plain text: every operation is
 * wrapped in a small packet and acknowledged by the controller.
 *
 * Packet format:
 *   MESSAGE_TYPE DATA_LENGTH FUNCTION DATA... CHECKSUM
 *
 * MESSAGE_TYPE is 0xf0 for commands sent by the host and 0xf1 for replies from
 * the LCD controller.  DATA_LENGTH includes FUNCTION and DATA but not
 * MESSAGE_TYPE, DATA_LENGTH, or CHECKSUM.  CHECKSUM is the low byte of the sum
 * of all preceding bytes in the packet.
 *
 * This driver is based on the protocol behavior documented by the openlcmd
 * implementation from github.com/mafredri/lcm.
 */

/*  Copyright (C) 2026 Rob Freyder and contributors

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "lcd.h"
#include "asustor.h"
#include "shared/report.h"

#define ASUSTOR_COMMAND 0xf0
#define ASUSTOR_REPLY 0xf1

#define ASUSTOR_FON 0x11
#define ASUSTOR_FCLEAR 0x12
#define ASUSTOR_FSTATUS 0x22
#define ASUSTOR_FTEXT 0x27
#define ASUSTOR_FBUTTON 0x80
#define ASUSTOR_FFLUSH 0x00

#define ASUSTOR_REPLY_TIMEOUT_MS 100
#define ASUSTOR_RETRY_LIMIT 50
#define ASUSTOR_WRITE_DELAY_US 250
#define ASUSTOR_KEY_QUEUE_SIZE 8
#define ASUSTOR_REPLY_QUEUE_SIZE 8

typedef struct asustor_reply {
	unsigned char function;
	unsigned char status;
} Reply;

/** private data for the \c asustor driver */
typedef struct asustor_private_data {
	int fd;
	char device[200];
	char framebuf[ASUSTOR_WIDTH * ASUSTOR_HEIGHT];
	char lastbuf[ASUSTOR_WIDTH * ASUSTOR_HEIGHT];
	int changed;
	int display_on;
	/* Button events and command ACKs arrive on the same serial stream. */
	const char *key_queue[ASUSTOR_KEY_QUEUE_SIZE];
	int key_head;
	int key_tail;
	Reply reply_queue[ASUSTOR_REPLY_QUEUE_SIZE];
	int reply_head;
	int reply_tail;
	char info[255];
} PrivateData;

MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "asustor_";

static int asustor_open_serial(PrivateData *p);
static unsigned char asustor_checksum(const unsigned char *packet, size_t len);
static int asustor_send_packet(Driver *drvthis, const unsigned char *packet, size_t len);
static int asustor_read_packet(PrivateData *p, unsigned char *packet, size_t packet_size, int timeout_ms);
static int asustor_read_exact(PrivateData *p, unsigned char *buf, size_t len, int timeout_ms);
static int asustor_data_ready(PrivateData *p);
static int asustor_wait_reply(PrivateData *p, unsigned char function, int timeout_ms);
static void asustor_route_packet(PrivateData *p, const unsigned char *packet, int len);
static int asustor_dequeue_reply(PrivateData *p, unsigned char function);
static void asustor_enqueue_reply(PrivateData *p, unsigned char function, unsigned char status);
static const char *asustor_dequeue_key(PrivateData *p);
static void asustor_enqueue_key(PrivateData *p, const char *key);
static const char *asustor_key_name(unsigned char value);
static unsigned int asustor_elapsed_ms(const struct timeval *start);
static void asustor_force_flush(PrivateData *p);
static int asustor_send_simple(Driver *drvthis, unsigned char function, unsigned char value);
static int asustor_send_text(Driver *drvthis, int line, const char *text);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
asustor_init(Driver *drvthis)
{
	PrivateData *p;
	const char *device;

	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p) != 0)
		return -1;

	device = drvthis->config_get_string(drvthis->name, "Device", 0, ASUSTOR_DEFAULT_DEVICE);
	strncpy(p->device, device, sizeof(p->device));
	p->device[sizeof(p->device) - 1] = '\0';

	memset(p->framebuf, ' ', sizeof(p->framebuf));
	memset(p->lastbuf, 0, sizeof(p->lastbuf));
	snprintf(p->info, sizeof(p->info), "ASUSTOR NAS LCD driver on %s", p->device);

	if (asustor_open_serial(p) < 0)
		goto err_out;

	/* Wake and initialize the controller before LCDd starts drawing. */
	(void) asustor_send_simple(drvthis, ASUSTOR_FON, 1);
	(void) asustor_send_simple(drvthis, ASUSTOR_FSTATUS, 0);
	(void) asustor_send_simple(drvthis, ASUSTOR_FCLEAR, 1);
	p->display_on = 1;
	p->changed = 1;

	report(RPT_INFO, "%s: initialized ASUSTOR LCD on %s", drvthis->name, p->device);
	return 0;

err_out:
	asustor_close(drvthis);
	return -1;
}

/**
 * Close the driver and release the serial port.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
asustor_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

MODULE_EXPORT int
asustor_width(Driver *drvthis)
{
	return ASUSTOR_WIDTH;
}

MODULE_EXPORT int
asustor_height(Driver *drvthis)
{
	return ASUSTOR_HEIGHT;
}

MODULE_EXPORT int
asustor_cellwidth(Driver *drvthis)
{
	return ASUSTOR_CELLWIDTH;
}

MODULE_EXPORT int
asustor_cellheight(Driver *drvthis)
{
	return ASUSTOR_CELLHEIGHT;
}

/**
 * Clear the LCDproc framebuffer.
 *
 * The physical display is updated from asustor_flush().  Deferring hardware
 * writes lets LCDd redraw a full screen without exposing transient states.
 */
MODULE_EXPORT void
asustor_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', sizeof(p->framebuf));
	p->changed = 1;
}

/**
 * Send changed framebuffer contents to the ASUSTOR controller.
 *
 * The controller accepts one full 16-character row per text command.  LCDproc
 * may call flush often, so skip the serial write if the framebuffer has not
 * changed since the last successful flush.
 */
MODULE_EXPORT void
asustor_flush(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char top[ASUSTOR_WIDTH + 1];
	char bottom[ASUSTOR_WIDTH + 1];

	if (p == NULL || !p->changed)
		return;

	if (memcmp(p->framebuf, p->lastbuf, sizeof(p->framebuf)) == 0) {
		p->changed = 0;
		return;
	}

	memcpy(top, p->framebuf, ASUSTOR_WIDTH);
	top[ASUSTOR_WIDTH] = '\0';
	memcpy(bottom, p->framebuf + ASUSTOR_WIDTH, ASUSTOR_WIDTH);
	bottom[ASUSTOR_WIDTH] = '\0';

	(void) asustor_send_text(drvthis, 0, top);
	(void) asustor_send_text(drvthis, 1, bottom);

	memcpy(p->lastbuf, p->framebuf, sizeof(p->framebuf));
	p->changed = 0;
}

/**
 * Write a string into the framebuffer using LCDproc's 1-based coordinates.
 */
MODULE_EXPORT void
asustor_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;

	if (p == NULL || y < 0 || y >= ASUSTOR_HEIGHT)
		return;

	for (i = 0; string[i] != '\0' && x < ASUSTOR_WIDTH; i++, x++) {
		if (x >= 0)
			p->framebuf[(y * ASUSTOR_WIDTH) + x] = string[i];
	}
	p->changed = 1;
}

/**
 * Write one character into the framebuffer using LCDproc's 1-based coordinates.
 */
MODULE_EXPORT void
asustor_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	x--;
	y--;

	if (p == NULL)
		return;
	if (x >= 0 && y >= 0 && x < ASUSTOR_WIDTH && y < ASUSTOR_HEIGHT) {
		p->framebuf[(y * ASUSTOR_WIDTH) + x] = c;
		p->changed = 1;
	}
}

/**
 * Return the next front-panel button event.
 *
 * The ASUSTOR controller sends button events asynchronously.  Because those
 * packets share the TTY with command acknowledgements, all input is routed
 * through queues instead of letting the read side consume packets intended for
 * the write side.
 */
MODULE_EXPORT const char *
asustor_get_key(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char packet[24];
	const char *key;
	int len;

	if (p == NULL)
		return NULL;

	key = asustor_dequeue_key(p);
	if (key != NULL)
		return key;

	/*
	 * Button packets share the serial stream with command acknowledgements.
	 * Do a nonblocking poll and route every complete packet through the same
	 * queues used by the write side so get_key() never steals ACKs.
	 */
	while (asustor_data_ready(p) && (len = asustor_read_packet(p, packet, sizeof(packet), 5)) >= 5)
		asustor_route_packet(p, packet, len);

	return asustor_dequeue_key(p);
}

/**
 * Handle LCDproc backlight requests.
 *
 * On tested ASUSTOR units the front panel exposes a display on/off command,
 * not a separate backlight brightness control.  Ignoring off requests avoids
 * blanking the display during normal LCDproc screen transitions.
 */
MODULE_EXPORT void
asustor_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int target = on ? 1 : 0;

	if (p == NULL)
		return;

	/*
	 * LCDd toggles backlight/output state as part of heartbeat and screen
	 * activity.  On ASUSTOR this is the same command that blanks the entire
	 * front-panel LCD, so ignore off requests by default and keep the panel
	 * readable.
	 */
	if (!target || p->display_on == target)
		return;

	(void) asustor_send_simple(drvthis, ASUSTOR_FON, target);
	(void) asustor_send_simple(drvthis, ASUSTOR_FSTATUS, 0);
	p->display_on = target;
}

MODULE_EXPORT void
asustor_output(Driver *drvthis, int on)
{
	asustor_backlight(drvthis, on);
}

MODULE_EXPORT const char *
asustor_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return (p != NULL) ? p->info : "ASUSTOR NAS LCD driver";
}

static int
asustor_open_serial(PrivateData *p)
{
	struct termios portset;

	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (p->fd < 0) {
		report(RPT_ERR, "asustor: open(%s) failed: %s", p->device, strerror(errno));
		return -1;
	}

	if (tcgetattr(p->fd, &portset) < 0) {
		report(RPT_ERR, "asustor: tcgetattr(%s) failed: %s", p->device, strerror(errno));
		return -1;
	}

#ifdef HAVE_CFMAKERAW
	cfmakeraw(&portset);
#else
	portset.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	portset.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	portset.c_cc[VMIN] = 0;
	portset.c_cc[VTIME] = 1;
	cfsetospeed(&portset, B115200);
	cfsetispeed(&portset, B115200);

	if (tcsetattr(p->fd, TCSANOW, &portset) < 0) {
		report(RPT_ERR, "asustor: tcsetattr(%s) failed: %s", p->device, strerror(errno));
		return -1;
	}

	tcflush(p->fd, TCIOFLUSH);
	return 0;
}

/* ASUSTOR uses the low byte of the sum of all bytes before CHECKSUM. */
static unsigned char
asustor_checksum(const unsigned char *packet, size_t len)
{
	size_t i;
	unsigned int sum = 0;

	for (i = 0; i < len; i++)
		sum += packet[i];
	return (unsigned char) sum;
}

/*
 * Send a packet and wait for the matching command acknowledgement.
 *
 * Replies can arrive before the caller that needs them is waiting, because
 * get_key() also drains the serial port.  asustor_wait_reply() therefore
 * checks the reply queue before reading from the device.
 */
static int
asustor_send_packet(Driver *drvthis, const unsigned char *packet, size_t len)
{
	PrivateData *p = drvthis->private_data;
	unsigned char buf[32];
	int retry;

	if (p == NULL || len + 1 > sizeof(buf))
		return -1;

	memcpy(buf, packet, len);
	buf[len] = asustor_checksum(packet, len);

	for (retry = 0; retry < ASUSTOR_RETRY_LIMIT; retry++) {
		ssize_t written;

		written = write(p->fd, buf, len + 1);
		if (written == (ssize_t)(len + 1)
		    && asustor_wait_reply(p, packet[2], ASUSTOR_REPLY_TIMEOUT_MS) == 0) {
			usleep(ASUSTOR_WRITE_DELAY_US);
			return 0;
		}

		asustor_force_flush(p);
	}

	report(RPT_WARNING, "%s: no acknowledgement for function 0x%02x", drvthis->name, packet[2]);
	return -1;
}

/*
 * Read one complete ASUSTOR packet.
 *
 * The serial stream can contain stale bytes after LCDd restart or failed
 * writes.  Resynchronise by scanning for the command/reply prefix, then trust
 * only packets with plausible length and valid checksum.
 */
static int
asustor_read_packet(PrivateData *p, unsigned char *packet, size_t packet_size, int timeout_ms)
{
	unsigned char checksum;
	unsigned char b;
	size_t total;
	struct timeval start;

	gettimeofday(&start, NULL);

	for (;;) {
		if (asustor_read_exact(p, &b, 1, timeout_ms) < 0)
			return -1;
		if (b == ASUSTOR_COMMAND || b == ASUSTOR_REPLY)
			break;
		if (timeout_ms > 0 && asustor_elapsed_ms(&start) >= (unsigned int) timeout_ms)
			return -1;
	}

	packet[0] = b;
	if (asustor_read_exact(p, packet + 1, 2, timeout_ms) < 0)
		return -1;

	if ((packet[0] == ASUSTOR_REPLY && packet[1] > 1)
	    || (packet[0] == ASUSTOR_COMMAND && packet[1] > 16))
		return -1;

	total = 3 + packet[1] + 1;
	if (total > packet_size || total < 5)
		return -1;

	if (asustor_read_exact(p, packet + 3, packet[1] + 1, timeout_ms) < 0)
		return -1;

	checksum = asustor_checksum(packet, total - 1);
	if (checksum != packet[total - 1])
		return -1;

	return (int) total;
}

/* Read exactly len bytes from the nonblocking serial port before timeout. */
static int
asustor_read_exact(PrivateData *p, unsigned char *buf, size_t len, int timeout_ms)
{
	size_t pos = 0;
	struct timeval start;

	gettimeofday(&start, NULL);
	while (pos < len) {
		ssize_t n;

		n = read(p->fd, buf + pos, len - pos);
		if (n > 0) {
			pos += n;
			continue;
		}
		if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			return -1;

		if (timeout_ms == 0)
			return -1;

		if (asustor_elapsed_ms(&start) >= (unsigned int) timeout_ms)
			return -1;

		usleep(1000);
	}

	return 0;
}

/* Nonblocking poll used by get_key() so LCDd is never stalled for input. */
static int
asustor_data_ready(PrivateData *p)
{
	fd_set rfds;
	struct timeval timeout;
	int ret;

	FD_ZERO(&rfds);
	FD_SET(p->fd, &rfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	ret = select(p->fd + 1, &rfds, NULL, NULL, &timeout);
	return ret > 0 && FD_ISSET(p->fd, &rfds);
}

/*
 * Wait for an acknowledgement to the given ASUSTOR function.
 *
 * Non-matching packets are not discarded: button events and replies for other
 * commands are routed into queues so later callers can still consume them.
 */
static int
asustor_wait_reply(PrivateData *p, unsigned char function, int timeout_ms)
{
	unsigned char packet[24];
	struct timeval start;
	int len;

	if (asustor_dequeue_reply(p, function) == 0)
		return 0;

	gettimeofday(&start, NULL);

	while (asustor_elapsed_ms(&start) < (unsigned int) timeout_ms) {
		len = asustor_read_packet(p, packet, sizeof(packet), timeout_ms);
		if (len < 5)
			continue;
		if (packet[0] == ASUSTOR_REPLY && packet[2] == function && packet[3] == 0)
			return 0;
		asustor_route_packet(p, packet, len);
		if (asustor_dequeue_reply(p, function) == 0)
			return 0;
	}

	return -1;
}

/* Route validated packets into either the reply queue or button queue. */
static void
asustor_route_packet(PrivateData *p, const unsigned char *packet, int len)
{
	const char *key;

	if (len < 5)
		return;

	if (packet[0] == ASUSTOR_REPLY) {
		asustor_enqueue_reply(p, packet[2], packet[3]);
		return;
	}

	if (packet[0] == ASUSTOR_COMMAND && packet[2] == ASUSTOR_FBUTTON) {
		key = asustor_key_name(packet[3]);
		if (key != NULL) {
			report(RPT_DEBUG, "asustor: button %s", key);
			asustor_enqueue_key(p, key);
		}
	}
}

/*
 * Find a successful ACK for function.
 *
 * The small ring buffer is intentionally lossy: stale ACKs are less important
 * than keeping the serial protocol moving if the controller sends unexpected
 * traffic.
 */
static int
asustor_dequeue_reply(PrivateData *p, unsigned char function)
{
	int i;

	for (i = p->reply_head; i != p->reply_tail; i = (i + 1) % ASUSTOR_REPLY_QUEUE_SIZE) {
		if (p->reply_queue[i].function == function && p->reply_queue[i].status == 0) {
			p->reply_head = (i + 1) % ASUSTOR_REPLY_QUEUE_SIZE;
			return 0;
		}
	}

	return -1;
}

/* Queue an ACK/status reply, dropping the oldest entry on overflow. */
static void
asustor_enqueue_reply(PrivateData *p, unsigned char function, unsigned char status)
{
	int next = (p->reply_tail + 1) % ASUSTOR_REPLY_QUEUE_SIZE;

	if (next == p->reply_head)
		p->reply_head = (p->reply_head + 1) % ASUSTOR_REPLY_QUEUE_SIZE;

	p->reply_queue[p->reply_tail].function = function;
	p->reply_queue[p->reply_tail].status = status;
	p->reply_tail = next;
}

/* Return the next queued LCDproc key name. */
static const char *
asustor_dequeue_key(PrivateData *p)
{
	const char *key;

	if (p->key_head == p->key_tail)
		return NULL;

	key = p->key_queue[p->key_head];
	p->key_head = (p->key_head + 1) % ASUSTOR_KEY_QUEUE_SIZE;
	return key;
}

/* Queue a front-panel key, dropping the oldest entry on overflow. */
static void
asustor_enqueue_key(PrivateData *p, const char *key)
{
	int next = (p->key_tail + 1) % ASUSTOR_KEY_QUEUE_SIZE;

	if (next == p->key_head)
		p->key_head = (p->key_head + 1) % ASUSTOR_KEY_QUEUE_SIZE;

	p->key_queue[p->key_tail] = key;
	p->key_tail = next;
}

/* Convert ASUSTOR button codes to LCDproc key names. */
static const char *
asustor_key_name(unsigned char value)
{
	switch (value) {
	case 1:
		return "Up";
	case 2:
		return "Down";
	case 3:
		return "Escape";
	case 4:
		return "Enter";
	default:
		return NULL;
	}
}

static unsigned int
asustor_elapsed_ms(const struct timeval *start)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	return (unsigned int) ((now.tv_sec - start->tv_sec) * 1000
			       + (now.tv_usec - start->tv_usec) / 1000);
}

/*
 * Send the controller's flush function twice to recover from a failed command.
 * This mirrors observed openlcmd behavior and helps after a partial write or
 * stale bytes left in the packet stream.
 */
static void
asustor_force_flush(PrivateData *p)
{
	unsigned char flush[] = { ASUSTOR_COMMAND, 0x01, ASUSTOR_FFLUSH, 0x00, 0x00 };

	flush[4] = asustor_checksum(flush, 4);
	(void) write(p->fd, flush, sizeof(flush));
	(void) write(p->fd, flush, sizeof(flush));
	usleep(ASUSTOR_WRITE_DELAY_US);
}

/* Send a one-byte ASUSTOR command such as display on or clear. */
static int
asustor_send_simple(Driver *drvthis, unsigned char function, unsigned char value)
{
	unsigned char packet[] = { ASUSTOR_COMMAND, 0x01, function, value };

	return asustor_send_packet(drvthis, packet, sizeof(packet));
}

/* Send exactly one row of text; callers pad/truncate to ASUSTOR_WIDTH. */
static int
asustor_send_text(Driver *drvthis, int line, const char *text)
{
	unsigned char packet[21];

	packet[0] = ASUSTOR_COMMAND;
	packet[1] = 0x12;
	packet[2] = ASUSTOR_FTEXT;
	packet[3] = (line == 0) ? 0 : 1;
	packet[4] = 0;
	memcpy(packet + 5, text, ASUSTOR_WIDTH);

	return asustor_send_packet(drvthis, packet, sizeof(packet));
}
