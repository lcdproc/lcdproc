/** \file server/drivers/lcd_lib.c
 * LCD library of useful functions for drivers.
 *
 * Drawn from the "base driver" which really was the precursor
 * to this library.
 *
 * \todo Make use of the \c options parameter to specify SEAMLESS_HBARS,
 *       the height of hbars, etc.
 */

#include "lcd.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/**
 * This function places a hbar using the v0.5 API format and the given cellwidth.
 * It assumes that custom chars have been statically defined, so that number
 * 1 has 1 pixel, number 2 has 2 etc.
 *
 * LCDs that have the custom chars at other char numbers than 0 should put the
 * first custom char number in cc_offset.
 */
void
lib_hbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellwidth, int cc_offset)
{
	int total_pixels  = ((long) 2 * len * cellwidth + 1 ) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos ++ ) {

		int pixels = total_pixels - cellwidth * pos;

		if ( pixels >= cellwidth ) {
			/* write a "full" block to the screen... */
#if defined(SEAMLESS_HBARS)
			drvthis->chr (drvthis, x+pos, y, cellwidth + cc_offset);
#else
			drvthis->icon (drvthis, x+pos, y, ICON_BLOCK_FILLED);
#endif
		}
		else if ( pixels > 0 ) {
			/* write a partial block... */
			drvthis->chr (drvthis, x+pos, y, pixels + cc_offset);
			break;
		}
		else {
			; /* write nothing (not even a space) */
		}
	}
}

/**
 * This function places a vbar using the v0.5 API format and the given cellwidth.
 * It assumes that custom chars have been statically defined, so that number
 * 1 has 1 pixel, number 2 has 2 etc., just like in good old times.
 *
 * LCDs that have the custom chars at other char numbers than 0 should put the
 * first custom char number in cc_offset.
 */
void
lib_vbar_static (Driver *drvthis, int x, int y, int len, int promille, int options, int cellheight, int cc_offset)
{
	int total_pixels = ((long) 2 * len * cellheight + 1 ) * promille / 2000;
	int pos;

	for (pos = 0; pos < len; pos ++ ) {

		int pixels = total_pixels - cellheight * pos;

		if ( pixels >= cellheight ) {
			/* write a "full" block to the screen... */
			drvthis->icon (drvthis, x, y-pos, ICON_BLOCK_FILLED);
		}
		else if ( pixels > 0 ) {
			/* write a partial block... */
			drvthis->chr (drvthis, x, y-pos, pixels + cc_offset);
			break;
		}
		else {
			; /* write nothing (not even a space) */
		}
	}
}
