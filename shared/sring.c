/** \file shared/sring.c
 * Circular buffer implementation for string processing.
 *
 * \todo Implement sring_peek() and sring_skip().
 */

/*-
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 2009, Markus Dolze
 */

#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
# include <stdio.h>
# include <ctype.h>
#endif

#include "sring.h"

/**
 * Allocate a new ring buffer data structure.
 * As this ring buffer is implemented using the 'Always Keep One Byte Open'
 * strategy, the internal data buffer is (iSize+1) large.
 *
 * \param iSize  Initial size of the ring buffer
 * \return       Pointer to the created ring buffer
 */
sring_buffer*
sring_create(int iSize)
{
	sring_buffer *buf;

	if ((buf = malloc(sizeof(*buf))) == NULL)
		return NULL;

	if ((buf->data = malloc(iSize + 1)) == NULL)
		return NULL;

	buf->size = iSize + 1;
	buf->w = 0;
	buf->r = 0;

	return buf;
}

/**
 * Free memory used by ring buffer.
 * \param buf  Ring buffer to work on
 */
void
sring_destroy(sring_buffer *buf)
{
	if (buf == NULL)
		return;

	free(buf->data);
	buf->data = NULL;
	free(buf);
}

/**
 * Clears the internal ring buffer.
 * Existing data is overwritten with NUL bytes.
 *
 * \param buf  Ring buffer to work on
 */
void
sring_clear(sring_buffer *buf)
{
	if (buf == NULL)
		return;

	buf->w = 0;
	buf->r = 0;
	memset(buf->data, '\0', buf->size);
}

/**
 * Get the number of bytes that can be written.
 * \param buf  Ring buffer to work on
 * \return     Byte count
 */
int
sring_getMaxWrite(sring_buffer *buf)
{
	int nBytes;

	if (buf == NULL)
		return 0;

	/* Use 'Always Keep One Byte Open' strategy */
	if (buf->w < buf->r)
		nBytes = buf->r - buf->w - 1;
	else
		nBytes = (buf->size - buf->w) + buf->r - 1;

	return nBytes;
}

/**
 * Get the number of bytes that can be read.
 * \param buf  Ring buffer to work on
 * \return     Byte count
 */
int
sring_getMaxRead(sring_buffer *buf)
{
	int nBytes;

	if (buf == NULL)
		return 0;

	if (buf->r <= buf->w)
		nBytes = buf->w - buf->r;
	else
		nBytes = (buf->size - buf->r) + buf->w;

	return nBytes;
}

/**
 * Write src_len bytes from src into ring buffer.
 * Fails if not all bytes can be written.
 *
 * \param buf  Ring buffer to work on
 * \param src  Pointer to source buffer
 * \param src_len  Number of bytes to write at most
 * \return     -1 if not all bytes can be written, 0 otherwise
 */
int
sring_write(sring_buffer *buf, char *src, int src_len)
{
	if (buf == NULL || src == NULL || src_len <= 0)
		return -1;

	/* XXX: Modify it to write as much as possible? */
	if (src_len > sring_getMaxWrite(buf))
		return -1;

	if (buf->w + src_len < buf->size) {
		memcpy(buf->data + buf->w, src, src_len);
		buf->w += src_len;
	}
	else {
		int firstBlockLen = buf->size - buf->w;
		int secondBlockLen = src_len - firstBlockLen;

		memcpy(buf->data + buf->w, src, firstBlockLen);
		memcpy(buf->data, src + firstBlockLen, secondBlockLen);

		buf->w = secondBlockLen;
	}

	return 0;
}

/**
 * Read dst_len bytes from ring buffer into destination.
 * Fails if buffer does not contains dst_len bytes to read from. The target
 * buffer must be allocated by the application before calling this function.
 *
 * \param buf  Ring buffer to work on
 * \param dst  Pointer to target buffer
 * \param dst_len  Number of bytes to read at most
 * \return     The number of bytes actually read
 */
int
sring_read(sring_buffer *buf, char *dst, int dst_len)
{
	if (buf == NULL || dst == NULL || dst_len <= 0)
		return -1;

	/* Do not read more than available */
	if (dst_len > sring_getMaxRead(buf))
		dst_len = sring_getMaxRead(buf);

	if (buf->r + dst_len < buf->size) {
		memcpy(dst, buf->data + buf->r, dst_len);
		buf->r += dst_len;
	}
	else {
		int firstBlockLen = buf->size - buf->r;
		int secondBlockLen = dst_len - firstBlockLen;

		memcpy(dst, buf->data + buf->r, firstBlockLen);

		if (secondBlockLen > 0)
			memcpy(dst + firstBlockLen, buf->data, secondBlockLen);

		buf->r = secondBlockLen;
	}

	return dst_len;
}

/**
 * Return the next string from the ring buffer.
 * The next string is a sequence of bytes terminated by \\r, \\n or \\0. The
 * memory for the string is allocated dynamically and must be free'd by the
 * application. The string is always NUL terminated, but does not include the
 * end character.
 *
 * \param buf  Ring buffer to work on
 * \return     Pointer to allocated string, NULL if no string is available
 */
char *
sring_read_string(sring_buffer *buf)
{
	int n;
	char *border;
	char *p;
	char *dst;
	int dst_len;

	if (buf == NULL)
		return NULL;

	n = sring_getMaxRead(buf);
	border = buf->data + buf->size;
	p = buf->data + buf->r;

	while (--n >= 0) {
		if (*p == '\r' || *p == '\n' || *p == '\0')
			break;
		p++;
		if (p == border)
			p = buf->data;
	};

	if (n == -1)
		return NULL;

	dst_len = sring_getMaxRead(buf) - n;
	if ((dst = malloc(dst_len)) == NULL)
		return NULL;

	sring_read(buf, dst, dst_len);
	dst[dst_len-1] = '\0';

	return dst;
}

/**
 * Print content of buffer to stdout.
 * Only enabled, if DEBUG is defined.
 *
 * \param buf  Ring buffer to work on
 */
void
sring_dump(sring_buffer *buf)
{
#ifdef DEBUG
	int a;

	if (buf == NULL)
		return;

	for (a = 0; a < buf->size; a++) {
		if (isprint(buf->data[a]))
			printf("'%c' ", buf->data[a]);
		else
			printf("0x%02X ", buf->data[a]);
	}
	printf("\n");
#endif
}
