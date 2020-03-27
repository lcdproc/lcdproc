// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility lib for using hidraw devices in LCDd drivers.
 *
 * Copyright (C) 2017 Hans de Goede <hdegoede@redhat.com>
 */
 
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hidraw_lib.h"
#include "shared/report.h"

struct lib_hidraw_handle {
	/* For re-opening the device if we loose connection */
	const struct lib_hidraw_id *ids;
	int fd;
};

static int lib_hidraw_open_device(const char *device,
				  const struct lib_hidraw_id *ids)
{
	struct hidraw_report_descriptor descriptor;
	struct hidraw_devinfo devinfo;
	int i, err, fd;

	fd = open(device, O_RDWR);
	if (fd == -1)
		return -1;

	err = ioctl(fd, HIDIOCGRAWINFO, &devinfo);
	if (err == -1) {
		close(fd);
		return -1;
	}

	descriptor.size = LIB_HIDRAW_DESC_HDR_SZ;
	err = ioctl(fd, HIDIOCGRDESC, &descriptor);
	if (err == -1) {
		close(fd);
		return -1;
	}

	for (i = 0; ids[i].devinfo.bustype; i++) {
		if (memcmp(&devinfo, &ids[i].devinfo, sizeof(devinfo)))
			continue;

		if (ids[i].descriptor_header[0] == 0 ||
		    (descriptor.size >= LIB_HIDRAW_DESC_HDR_SZ &&
		     memcmp(descriptor.value, ids[i].descriptor_header,
			    LIB_HIDRAW_DESC_HDR_SZ) == 0))
			break; /* Found it */
	}
	if (!ids[i].devinfo.bustype) {
		close(fd);
		return -1;
	}

	return fd;
}

static int lib_hidraw_find_device(const struct lib_hidraw_id *ids)
{
	char devname[PATH_MAX];
	struct dirent *dirent;
	int fd = -1;
	DIR *dir;

	dir = opendir("/dev");
	if (dir == NULL)
		return -1;

	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_type != DT_CHR ||
		    strncmp(dirent->d_name, "hidraw", 6))
			continue;

		strcpy(devname, "/dev/");
		strcat(devname, dirent->d_name);

		fd = lib_hidraw_open_device(devname, ids);
		if (fd != -1)
			break;
	}

	closedir(dir);

	return fd;
}

struct lib_hidraw_handle *lib_hidraw_open(const struct lib_hidraw_id *ids)
{
	struct lib_hidraw_handle *handle;
	int fd;

	fd = lib_hidraw_find_device(ids);
	if (fd == -1)
		return NULL;

	handle = calloc(1, sizeof(*handle));
	if (!handle) {
		close(fd);
		return NULL;
	}

	handle->fd = fd;
	handle->ids = ids;
	return handle;
}

void lib_hidraw_send_output_report(struct lib_hidraw_handle *handle,
				   unsigned char *data, int count)
{
	int result;

	if (handle->fd != -1) {
		result = write(handle->fd, data, count);
		/* Device unplugged / lost connection ? */
		if (result == -1 && errno == ENODEV) {
			report(RPT_WARNING, "Lost hidraw device connection");
			close(handle->fd);
			handle->fd = -1;
		}
	}

	/*
	 * If we've lost the device this may be due to a temporary connection
	 * loss (Bluetooth), or the device may have dropped of the bus to
	 * re-appear with a different (compatible) product-id, such as e.g.
	 * the G510 keyboard does when (un)plugging the headphones.
	 * To deal with these kinda temporary device losses we try to
	 * re-acquire the device here.
	 */
	if (handle->fd == -1) {
		handle->fd = lib_hidraw_find_device(handle->ids);
		if (handle->fd != -1) {
			report(RPT_WARNING, "Successfully re-opened hidraw device");
			write(handle->fd, data, count);
		}
	}
}

void lib_hidraw_close(struct lib_hidraw_handle *handle)
{
	if (handle->fd != -1)
		close(handle->fd);
	free(handle);
}
