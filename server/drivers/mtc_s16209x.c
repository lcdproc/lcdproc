/*
 * LCD Driver for MTC_S16209x LCD display, used with lcdproc (lcdproc.org)
 * Copyright (C) 2002 SecureCiRT, A SBU of Z-Vance Pte Ltd (Singapore)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*
 * Michael Boman - SecureCiRT Security Architect <michael.boman@securecirt.com>
 *
 * Heres a bit more info on the display.
 * It is the MTC-S16209x and is made by Microtips Technology Inc, Taiwan
 * The web page for it is http://www.microtips.com.tw
 *
 * The LCD is optional front panel for Gigabyte GS-SR104 system from
 * Gigabyte (http://www.gigabyte.com.tw)
 *
 * Any other implementations are not known. Please let me know if you
 * have encountered any other implementations.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/file.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "mtc_s16209x.h"
//#include "shared/str.h"
#include "report.h"

#define MTC_DEFAULT_DEVICE	"/dev/lcd"
#define MTC_DEFAULT_BRIGHTNESS	255

int my_error_handle;

char lcd_open[] = "\xFE\x28";	// From OpenCommPort()
char lcd_close[] = "\xFE\x37";	// From CloseCommPort()

char lcd_clearscreen[] = "\xFE\x01";	// From ClearScreen()

char lcd_hidecursor[] = "\xFE\x08";	// From LcdHide()
char lcd_showcursor[] = "\xFE\x0C";	// From LcdShow()

char lcd_changeline[] = "\xFE\xC0";	// From CHLine()

char lcd_setcursor[] = "\xFE\xC0";	// From Set_Cursor(). Add location to second byte [lcd_setcursor[1] + loc]
char lcd_setcursor_1[] = "\xFE\x80";	// First 16 bytes, add location to second byte. From Set_Cursor1() [lcd_setcursor_1[1] + loc]
char lcd_setcursor_2[] = "\xFE\xB0";	// Second 16 bytes (17-32), add location to second byte. From Set_Cursor1() [lcd_setcursor_2[1] + loc]

char lcd_gotoline1[] = "\xFE\x80";	// First character on the first row
char lcd_gotoline2[] = "\xFE\xC0";	// First character on the second row

char lcd_showunderline[] = "\xFE\x0E";	// From Show_UnderLine()
char lcd_hideunderline[] = "\xFE\x0B";	// From Hide_UnderLine()


static int custom = 0;

typedef enum
{
  hbar = 1,
  vbar = 2,
  bign = 4,
  beat = 8
}
custom_type;


static int fd;
char framebuf[2][16];
static int width = 16;		//was: LCD_DEFAULT_WIDTH; (is now hardcoded)
static int height = 2;		//was: LCD_DEFAULT_HEIGHT; (is now hardcoded)
static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;

//static void MTC_S16209X_hidecursor ();
static void MTC_S16209X_reboot ();

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "MTC_S16209X_";

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
MTC_S16209X_init (Driver * drvthis, char *args)
{
  struct termios portset;
  char device[256] = MTC_DEFAULT_DEVICE;

#ifdef CAN_REBOOT_LCD
  int reboot = 0;
#endif // CAN_REBOOT_LCD

#ifdef CAN_CONTROL_BACKLIGHT
  int tmp;
  int backlight_brightness = 255;
#endif // CAN_CONTROL_BACKLIGHT

  /* Read config file */

  /* What device should be used */
  strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
					     MTC_DEFAULT_DEVICE), sizeof(device));
  device[sizeof(device)-1] = '\0';

#ifdef CAN_CONTROL_BACKLIGHT
  /* Which backlight brightness */
  backlight_brightness = drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , MTC_DEFAULT_BRIGHTNESS);
  if ((backlight_brightness < 0) || (backlight_brightness > 255)) {
    report (RPT_WARNING, "MTC_S16209X_init: Brightness must be between 0 and 255. Using default value.\n");
    backlight_brightness = MTC_DEFAULT_BRIGHTNESS;
  }
#endif // CAN_CONTROL_BACKLIGHT
      
#ifdef CAN_REBOOT_LCD
  /* Reboot display? */
  reboot = drvthis->config_get_bool( drvthis->name , "Reboot", 0, 0);
  if (reboot)
    report (RPT_INFO, "LCDd: rebooting MTC_S16209x LCD...\n");
#endif // CAN_REBOOT_LCD
      
  /* End of config file parsing */
  
  // Set up io port correctly, and open it...
  fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
    report(RPT_ERR, "MTC_S16209X_init: open(%s) failed (%s)\n", device, strerror(errno));
    return -1;
  }
  report(RPT_DEBUG, "MTC_S16209X_init: opened device %s\n", device);

  fcntl (fd, F_SETFL, 0);	// Set port for reading
  tcgetattr (fd, &portset);	// Get current port attributes
  cfsetispeed (&portset, B2400);	// Speed is hardcoded, seems like being the only speed setting it likes
  cfsetospeed (&portset, B2400);	// Speed is hardcoded, seems like being the only speed setting it likes
  portset.c_cflag |= CS8;
  portset.c_cflag |= CSTOPB;
  portset.c_cflag |= CREAD | HUPCL | CLOCAL;
  portset.c_iflag &=
    ~(IGNPAR | PARMRK | INLCR | IGNCR | ICRNL | ISTRIP | INPCK);
  portset.c_iflag |= BRKINT;
  portset.c_lflag &= (ICANON | ECHO);
  portset.c_oflag = 0;
  portset.c_lflag = 0;
  portset.c_cc[VMIN] = 1;
  portset.c_cc[VTIME] = 0;

  tcflush (fd, TCIFLUSH);	// Clear the port buffer
  tcsetattr (fd, TCSANOW, &portset);	// Apply the new settings

  my_error_handle = write (fd, lcd_open, sizeof (lcd_open));	// Send the init string to the LCD

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_init(): write(lcd_open) failed (%s)\n",
	   strerror(errno));

  my_error_handle = write (fd, lcd_clearscreen, sizeof (lcd_clearscreen));	// Clear the LCD, unbuffered

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_init(): write(lcd_clearscreen) failed (%s)\n",
	   strerror(errno));

  return 0;
}



/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
MTC_S16209X_close (Driver * drvthis)
{

  flock (fd, LOCK_EX);
  my_error_handle = write (fd, lcd_close, sizeof (lcd_close));	// Send the close code to LCD
  flock (fd, LOCK_UN);

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_close(): Write() failed! (%s)\n",
           strerror(errno));

  usleep (10);

  if (fd)
    close (fd);

}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
MTC_S16209X_width (Driver * drvthis)
{
  return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
MTC_S16209X_height (Driver * drvthis)
{
  return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
MTC_S16209X_clear (Driver * drvthis)
{
  memset (framebuf, ' ', sizeof (framebuf));	// Buffered clearscreen
}


/////////////////////////////////////////////////////////////////
// Flushes the framebuffer to the LCD
//
MODULE_EXPORT void
MTC_S16209X_flush (Driver * drvthis)
{
/* TODO: Do we really have a flush for this thing? Do we need to? How do we do it? */
/* TODO Update: yes, we need to buffer and flush - else the LCD looks slow, and flicker a lot */

  // 1st step: flush 1st line:
  flock (fd, LOCK_EX);
  my_error_handle = write (fd, lcd_gotoline1, sizeof (lcd_gotoline1));	// Go to the first row
  my_error_handle = write (fd, framebuf[0], sizeof (framebuf[0]));	// Send the first row data to LCD
  flock (fd, LOCK_UN);

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_flush(): Couldn't write 1st line (%s)\n",
	   strerror(errno));

  // 2nd step: flush 2nd line:
  flock (fd, LOCK_EX);
  my_error_handle = write (fd, lcd_gotoline2, sizeof (lcd_gotoline2));	// Go to the second row
  my_error_handle = write (fd, framebuf[1], sizeof (framebuf[1]));	// Send the second row data to LCD
  flock (fd, LOCK_UN);

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_flush(): Couldn't write 2nd line (%s)\n",
	   strerror(errno));

  // Wait until serial port cache has been emptied (else clients gets
  // the message to bugger off after a while)
  tcdrain (fd);

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
MTC_S16209X_chr (Driver * drvthis, int x, int y, char c)
{

  x--;				// Computers like to count from 0, not 1
  y--;				// Computers like to count from 0, not 1

  framebuf[y][x] = c;

}

#ifdef CAN_CONTROL_BACKLIGHT
/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
MODULE_EXPORT void
MTC_S16209X_backlight (Driver * drvthis, int on)
{
/* TODO: Can the backlights be controlled? Can't find anything in the docs */

}
#endif //CAN_CONTROL_BACKLIGHT

#ifdef THIS_PART_SHOULD_BE_REMOVED
/////////////////////////////////////////////////////////////////
// Get rid of the blinking cursor
//
static void
MTC_S16209X_hidecursor ()
{

  flock (fd, LOCK_EX);
  my_error_handle = write (fd, lcd_hidecursor, sizeof (lcd_hidecursor));
  flock (fd, LOCK_UN);

  if (my_error_handle < 0)
    report(RPT_WARNING, "MTC_S16209X_hidecursor(): Write failed: %s\n",
	   strerror(errno));

}
#endif // THIS_PART_SHOULD_BE_REMOVED

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
MTC_S16209X_reboot ()
{

  flock (fd, LOCK_EX);
  write (fd, lcd_open, sizeof (lcd_open));	// TODO: Will this acctually reboot the LCD? Don't know
  flock (fd, LOCK_UN);

}

MODULE_EXPORT void
MTC_S16209X_string (Driver * drvthis, int x, int y, char string[])
{
  int i;

  x--;				// Computers like to count from 0, not 1
  y--;				// Computers like to count from 0, not 1

  for (i = 0; i < strlen (string); i++)
    {
      framebuf[y][x + i] = string[i];
    }
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before MTC_S16209X->vbar()
//
MODULE_EXPORT void
MTC_S16209X_init_vbar (Driver * drvthis)
{
  char a[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
  };

  char b[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  char c[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  char d[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  char e[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  char f[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  char g[] = {
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  if (custom != vbar)
    {
      MTC_S16209X_set_char (drvthis, 1, a);
      MTC_S16209X_set_char (drvthis, 2, b);
      MTC_S16209X_set_char (drvthis, 3, c);
      MTC_S16209X_set_char (drvthis, 4, d);
      MTC_S16209X_set_char (drvthis, 5, e);
      MTC_S16209X_set_char (drvthis, 6, f);
      MTC_S16209X_set_char (drvthis, 7, g);
      custom = vbar;
    }
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
MODULE_EXPORT void
MTC_S16209X_init_hbar (Driver * drvthis)
{

  char a[] = {
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
  };
  char b[] = {
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
  };
  char c[] = {
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
  };
  char d[] = {
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
  };
  char e[] = {
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  if (custom != hbar)
    {
      MTC_S16209X_set_char (drvthis, 1, a);
      MTC_S16209X_set_char (drvthis, 2, b);
      MTC_S16209X_set_char (drvthis, 3, c);
      MTC_S16209X_set_char (drvthis, 4, d);
      MTC_S16209X_set_char (drvthis, 5, e);
      custom = hbar;
    }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
MTC_S16209X_vbar (Driver * drvthis, int x, int len)
{
  char map[9] = { 32, 1, 2, 3, 4, 5, 6, 7, 255 };


  int y;
  for (y = height; y > 0 && len > 0; y--)
    {
      if (len >= cellheight)
	MTC_S16209X_chr (drvthis, x, y, 0xFF);
      else
	MTC_S16209X_chr (drvthis, x, y, map[len]);

      len -= cellheight;
    }

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
MTC_S16209X_hbar (Driver * drvthis, int x, int y, int len)
{
  char map[7] = { 32, 1, 2, 3, 4, 5 };

  for (; x <= width && len > 0; x++)
    {
      if (len >= cellwidth)
	MTC_S16209X_chr (drvthis, x, y, map[5]);
      else
	MTC_S16209X_chr (drvthis, x, y, map[len]);

      //printf ("%d,",len);
      len -= cellwidth;

    }
//      printf ("\n");

}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
MTC_S16209X_set_char (Driver * drvthis, int n, char *dat)
{
  char out[4];
  int row, col;
  int letter;

  //return (0);

  if (n < 0 || n > 7)
    return;
  n = 64 + (8 * n);
  if (!dat)
    return;

  snprintf (out, sizeof (out), "%c%c", 0xFE, n);
  flock (fd, LOCK_EX);
  write (fd, out, 2);
  flock (fd, LOCK_UN);

  for (row = 0; row < cellheight; row++)
    {
      letter = 1;

      for (col = 0; col < cellwidth; col++)
	{
	  letter <<= 1;
	  letter |= (dat[(row * cellwidth) + col] > 0);
	}

      snprintf (out, sizeof (out), "%c", letter);

      flock (fd, LOCK_EX);
      write (fd, out, 1);
      flock (fd, LOCK_UN);

    }
}

MODULE_EXPORT int
MTC_S16209X_icon (Driver * drvthis, int x, int y, int icon)
{
  static char heart_open[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 0, 0, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1
  };

  static char heart_filled[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 1, 1, 0,
    1, 0, 1, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1
  };

  switch (icon)
    {

    case ICON_BLOCK_FILLED:
      MTC_S16209X_chr (drvthis, x, y, 0xFF);
      break;

    case ICON_HEART_FILLED:
      MTC_S16209X_set_char (drvthis, 0, heart_filled);
      MTC_S16209X_chr (drvthis, x, y, 0);
      break;

    case ICON_HEART_OPEN:
      MTC_S16209X_set_char (drvthis, 0, heart_open);
      MTC_S16209X_chr (drvthis, x, y, 0);
      break;

    default:
      return -1;

    }
  return 0;
}

