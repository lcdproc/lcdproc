/*
 * TextMode driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <syslog.h>

#include "lcd.h"
#include "text.h"
//#include "drv_base.h"

/* Ugly code extracted by David GLAUDE from lcdm001.c ;)*/
static char num_icon [10][4][3] = 	{{{' ','_',' '}, /*0*/
					  {'|',' ','|'},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ',' ',' '},/*1*/
					  {' ',' ','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*2*/
					  {' ','_','|'},
					  {'|','_',' '},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*3*/
					  {' ','_','|'},
					  {' ','_','|'},
					  {' ',' ',' '}},
					  {{' ',' ',' '},/*4*/
					  {'|','_','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*5*/
					  {'|','_',' '},
					  {' ','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*6*/
					  {'|','_',' '},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*7*/
					  {' ',' ','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*8*/
					  {'|','_','|'},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*9*/
					  {'|','_','|'},
					  {' ','_','|'},
					  {' ',' ',' '}}};
/* End of ugly code ;) by Rene Wagner */

// Variables
int width;
int height;
char * framebuf;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "text_";

//////////////////////////////////////////////////////////////////////////
////////////////////// For Text-Mode Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////


MODULE_EXPORT int
text_init (Driver *drvthis, char *args)
{
	// Set display sizes
	if( drvthis->request_display_width() > 0
	&& drvthis->request_display_height() > 0 ) {
		// Use size from primary driver
		width = drvthis->request_display_width();
		height = drvthis->request_display_height();
	}
	else {
		// Use default size
		width = LCD_DEFAULT_WIDTH;
		height = LCD_DEFAULT_HEIGHT;
	}

	// Allocate the framebuffer
	framebuf = (unsigned char *) malloc (width * height);
	memset (framebuf, ' ', width * height);

	return 0;
}

/////////////////////////////////////////////////////////////////
// Closes the device
//
MODULE_EXPORT void
text_close (Driver *drvthis)
{
	if (framebuf != NULL)
		free (framebuf);

	framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
text_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
text_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
text_clear (Driver *drvthis)
{
	memset (framebuf, ' ', width * height);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
text_flush (Driver *drvthis)
{
	int i, j;

	char out[LCD_MAX_WIDTH];

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	printf ("+%s+\n", out);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			out[j] = framebuf[j + (i * width)];
		}
		out[width] = 0;
		printf ("|%s|\n", out);

	}

	for (i = 0; i < width; i++) {
		out[i] = '-';
	}
	out[width] = 0;
	printf ("+%s+\n", out);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
text_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	x--; y--; // Convert 1-based coords to 0-based...

	for (i = 0; string[i]; i++) {
		framebuf[(y * width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
text_chr (Driver *drvthis, int x, int y, char c)
{
	y--; x--;

	framebuf[(y * width) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets the contrast
//
MODULE_EXPORT void
text_set_contrast (Driver *drvthis, int promille)
{
	/*
	printf("Contrast: %d\n", promille);
	*/
}

/////////////////////////////////////////////////////////////////
// Sets the backlight brightness
//
MODULE_EXPORT void
text_backlight (Driver *drvthis, int on)
{
	//PrivateData * p = (PrivateData*) drvthis->private_data;

/*
  if(on)
  {
    printf("Backlight ON\n");
  }
  else
  {
    printf("Backlight OFF\n");
  }
*/
}

/////////////////////////////////////////////////////////////////
// Writes a big number. (by Rene Wagner from lcdm001.c)
//
MODULE_EXPORT void
text_num (Driver *drvthis, int x, int num)
{
	//PrivateData * p = (PrivateData*) drvthis->private_data;

	int y, dx;

//  printf("BigNum(%i, %i)\n", x, num);
	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			text_chr (drvthis, x + dx, y, num_icon[num][y-1][dx]);
}

//void
//text_set_char (int n, char *dat)
//{
////  printf("Set Character %i\n", n);
//}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
MODULE_EXPORT void
text_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			text_chr (drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
text_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	int pos;

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			text_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}

