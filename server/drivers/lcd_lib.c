#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lcd.h"


// ==================================================
// LCD library of useful functions for drivers
// ==================================================

// Drawn from the "base driver" which really was the precursor
// to this library....

// TODO: What should this really be?  Probably should be in the
// driver code or headers or something...

/*
ANY DRIVER SHOULD SIMPLY FREE ITS FRAMEBUFFER AT CLOSE()
NO CHECKING FOR NULL NEEDED
void
free_framebuf (struct lcd_logical_driver *driver) {
	if (!driver)
		return;

	if (driver->framebuf != NULL)
		free (driver->framebuf);

	driver->framebuf = NULL;
}
*/

/*
NEED TO BE ADAPTED TO NEW SITUATION
void
clear_framebuf (struct lcd_logical_driver *driver) {
	int framebuf_size;

	if (!driver)
		return;

	framebuf_size = driver->wid * driver->hgt;
	memset (driver->framebuf, ' ', framebuf_size);
}
*/

/*
NEED TO BE ADAPTED TO NEW SITUATION
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
*/

/*
NEED TO BE ADAPTED TO NEW SITUATION
void
insert_str_framebuf (struct lcd_logical_driver *driver, int x, int y, char *string) {
	//int i;
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
*/

/*
NEED TO BE ADAPTED TO NEW SITUATION
void
insert_chr_framebuf (struct lcd_logical_driver *driver, int x, int y, char c) {
	if (!driver)
		return;

	x--; y--;

	if (x >= driver->wid) x = driver->wid;
	if (x < 0) x = 0;

	if (y >= driver->hgt) y = driver->hgt;
	if (y < 0) y = 0;

	driver->framebuf[(y * driver->wid) + x] = c;
}
*/

void
output_heartbeat (struct lcd_logical_driver *driver, int type) {
	static int timer = 0;
	int whichIcon;
	static int saved_type = HEARTBEAT_ON;

	if (type)
		saved_type = type;

	if (type == HEARTBEAT_ON) {
		// Set this to pulsate like a real heart beat...
		if ( (timer + 4) & 5 ) {
			whichIcon = ICON_HEART_OPEN;
		}
		else {
			whichIcon = ICON_HEART_FILLED;
		}

		driver->icon (driver, driver->width(driver), 1, whichIcon);

		/* OLD CODE
		// This defines a custom character EVERY time...
		// not efficient... is this necessary?
		driver->icon (driver, whichIcon, 0);

		// Put character on screen...
		driver->chr (driver, driver->wid, 1, 0);
		*/

		// change display...
		//driver->flush (driver);
	}

	timer++;
	timer &= 0x0f;
}


void
lib_hbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellwidth, int cc_offset)
/*
 * This function places a hbar using the v0.5 API format and the given cellwidth.
 * It assumes that custom chars have been statically defined, so that number
 * 1 has 1 pixel, number 2 has 2 etc...
 * LCDs that have the custom chars at other char numbers than 0 should put the
 * first custom char number in cc_offset.
 */
{
	int total_pixels  = ((long) 2 * len * cellwidth + 1 ) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos ++ ) {

		int pixels = total_pixels - cellwidth * pos;

		if( pixels >= cellwidth ) {
			/* write a "full" block to the screen... */
			drvthis->icon (drvthis, x+pos, y, ICON_BLOCK_FILLED);
		}
		else if( pixels > 0 ) {
			/* write a partial block... */
			drvthis->chr (drvthis, x+pos, y, pixels + cc_offset);
			break;
		}
		else {
			; /* write nothing (not even a space) */
		}
	}
}


void
lib_vbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellheight, int cc_offset)
/*
 * This function places a vbar using the v0.5 API format and the given cellwidth.
 * It assumes that custom chars have been statically defined, so that number
 * 1 has 1 pixel, number 2 has 2 etc., just like in good old times...
 * LCDs that have the custom chars at other char numbers than 0 should put the
 * first custom char number in cc_offset.
 */
{
	int total_pixels = ((long) 2 * len * cellheight + 1 ) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos ++ ) {

		int pixels = total_pixels - cellheight * pos;

		if( pixels >= cellheight ) {
			/* write a "full" block to the screen... */
			drvthis->icon (drvthis, x, y-pos, ICON_BLOCK_FILLED);
		}
		else if( pixels > 0 ) {
			/* write a partial block... */
			drvthis->chr (drvthis, x, y-pos, pixels + cc_offset);
			break;
		}
		else {
			; /* write nothing (not even a space) */
		}
	}
}
