/**
 * \file shared/helpers.h
 * Shared small helpers for LCDd drivers, clients and server code.
 *
 * Non-static static inline functions, resolved at build time, with no
 * additional link dependencies: the same pattern the drivers already use
 * for shared/report.h.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include "shared/report.h"

/**
 * Write all bytes of buf to fd, looping on short writes.
 * Logs a warning (with errno) on failure and returns -1; on success 0.
 * A zero-byte write (no progress possible) is also treated as failure.
 * Callers that cannot handle a failure at the call site should treat -1
 * as "logged, nothing more to do".
 *
 * \param name  driver/module name used in the log message (e.g. "MtxOrb")
 * \param fd    file descriptor to write to
 * \param buf   bytes to write
 * \param len   number of bytes to write
 * \return 0 on success, -1 on failure
 */
static inline int
shared_write_all(const char *name, int fd, const unsigned char *buf, size_t len)
{
	const unsigned char *cur = buf;

	while (len > 0) {
		ssize_t wr = write(fd, cur, len);

		if (wr < 0) {
			report(RPT_WARNING, "%s: write failed (errno=%d)", name, errno);
			return -1;
		}
		if (wr == 0) {
			report(RPT_WARNING, "%s: zero-byte write", name);
			return -1;
		}
		cur += wr;
		len -= (size_t)wr;
	}
	return 0;
}

#endif /* HELPERS_H */
