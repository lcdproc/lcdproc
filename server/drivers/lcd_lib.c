#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include "lcd.h"

// ==================================================
// LCD library of useful functions for drivers
// ==================================================

// Drawn from the "base driver" which really was the precursor
// to this library....

// TODO: What should this really be?  Probably should be in the
// driver code or headers or something...
#define MAX_CUSTOM_CHARS 16

void
free_framebuf (struct lcd_logical_driver *driver) {
	if (!driver)
		return;

	if (driver->framebuf != NULL)
		free (driver->framebuf);

	driver->framebuf = NULL;
}

void
clear_framebuf (struct lcd_logical_driver *driver) {
	int framebuf_size;

	if (!driver)
		return;

	framebuf_size = driver->wid * driver->hgt;
	memset (driver->framebuf, ' ', framebuf_size);
}

int
new_framebuf (struct lcd_logical_driver *driver, char *oldbuf) {
	int i;

	if (driver->framebuf == NULL)
		return 1;
	if (oldbuf == NULL)
		return 1;

	for (i = 0; i < driver->wid * driver->hgt; i++) {
		if (driver->framebuf[i] < MAX_CUSTOM_CHARS)
			return 1;
		if (driver->framebuf[i] != oldbuf[i])
			return 1;
	}
	return 0;
}

void
insert_str_framebuf (struct lcd_logical_driver *driver, int x, int y, char *string) {
	int i;
	char buf[64];
	char *pos;

	if (!driver)
		return;

	x--; y--; // convert to zero-indexing

	if (x >= driver->wid) return;
	if (x < 0) x = 0;

	if (y >= driver->hgt) y = driver->hgt;
	if (y < 0) y = 0;

	if ((x + strlen(string)) > driver->wid)
		strncpy(buf, string, driver->wid - x - 1);
	else
		strncpy(buf, string, sizeof(string));

	pos = (driver->framebuf + (y * driver->wid) + x);
	strcpy(pos, buf);
}

void
insert_chr_framebuf (struct lcd_logical_driver *driver, int x, int y, char c) {
	if (!driver)
		return;

	x--; y--;

	if (x >= driver->wid) driver->wid;
	if (x < 0) x = 0;

	if (y >= driver->hgt) y = driver->hgt;
	if (y < 0) y = 0;

	driver->framebuf[(y * driver->wid) + x] = c;
}

