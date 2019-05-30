// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility lib for using hidraw devices in LCDd drivers.
 *
 * Copyright (C) 2017 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef HIDRAW_LIB_H
#define HIDRAW_LIB_H

#include <asm/types.h>
#include <linux/hidraw.h>
#include <linux/input.h>

#define LIB_HIDRAW_DESC_HDR_SZ		16

struct lib_hidraw_handle;

struct lib_hidraw_id {
	/* An entry entirely filled with zeros terminates the list of ids */
	struct hidraw_devinfo devinfo;
	/*
	 * Optional, may be used on devices with multiple USB interfaces to
	 * pick the right interface.
	 */
	unsigned char descriptor_header[LIB_HIDRAW_DESC_HDR_SZ];
};

struct lib_hidraw_handle *lib_hidraw_open(const struct lib_hidraw_id *ids);
void lib_hidraw_send_output_report(struct lib_hidraw_handle *handle,
				   unsigned char *data, int count);
void lib_hidraw_close(struct lib_hidraw_handle *handle);

#endif
