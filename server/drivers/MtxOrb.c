#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "lcd.h"
#include "MtxOrb.h"
#include "drv_base.h"

#include "../../shared/debug.h"
#include "../../shared/str.h"

static int custom = 0;

// TODO: Remove this custom_type if not in use anymore.
typedef enum {
   hbar = 1,
   vbar = 2,
   bign = 4,
   beat = 8
} custom_type;

// TODO: This is my ugglyest piece of code.
typedef enum {
   baru1 = 0,
   baru2 = 1,
   baru3 = 2,
   baru4 = 3,
   baru5 = 4,
   baru6 = 5,
   baru7 = 6,
   bard1 = 7,
   bard2 = 8,
   bard3 = 9,
   bard4 = 10,
   bard5 = 11,
   bard6 = 12,
   bard7 = 13,
   barr1 = 14,
   barr2 = 15,
   barr3 = 16,
   barr4 = 17,
   barl1 = 18,
   barl2 = 19,
   barl3 = 20,
   barl4 = 21,
   barw = 32,
   barb = 255
} bar_type;

static int fd;
static int clear = 1;

static int def[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int use[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };



static void MtxOrb_linewrap (int on);
static void MtxOrb_autoscroll (int on);
static void MtxOrb_cursorblink (int on);

// TODO:  Get rid of this variable?
lcd_logical_driver *MtxOrb;
// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
MtxOrb_init (lcd_logical_driver * driver, char *args)
{
   char *argv[64];
   int argc;
   struct termios portset;
   int i;
   int tmp;

   int contrast = 140;
   char device[256] = "/dev/lcd";
   int speed = B19200;


   MtxOrb = driver;


   //debug("MtxOrb_init: Args(all): %s\n", args);

   argc = get_args (argv, args, 64);

   /*
      for(i=0; i<argc; i++)
      {
      printf("Arg(%i): %s\n", i, argv[i]);
      }
    */

   for (i = 0; i < argc; i++) {
      //printf("Arg(%i): %s\n", i, argv[i]);
      if (0 == strcmp (argv[i], "-d") || 0 == strcmp (argv[i], "--device")) {
	 if (i + 1 > argc) {
	    fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
	    return -1;
	 }
	 strcpy (device, argv[++i]);
      } else if (0 == strcmp (argv[i], "-c") || 0 == strcmp (argv[i], "--contrast")) {
	 if (i + 1 > argc) {
	    fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
	    return -1;
	 }
	 tmp = atoi (argv[++i]);
	 if ((tmp < 0) || (tmp > 255)) {
	    fprintf (stderr, "MtxOrb_init: %s argument must between 0 and 255. Ussing default value.\n", argv[i]);
	 } else
	    contrast = tmp;
      } else if (0 == strcmp (argv[i], "-s") || 0 == strcmp (argv[i], "--speed")) {
	 if (i + 1 > argc) {
	    fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
	    return -1;
	 }
	 tmp = atoi (argv[++i]);
	 if (tmp == 1200)
	    speed = B1200;
	 else if (tmp == 2400)
	    speed = B2400;
	 else if (tmp == 9600)
	    speed = B9600;
	 else if (tmp == 19200)
	    speed = B19200;
	 else {
	    fprintf (stderr, "MtxOrb_init: %s argument must be 1200, 2400, 9600 or 19200. Ussing default value.\n", argv[i]);
	 }
      } else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
	 printf ("LCDproc Matrix-Orbital LCD driver\n" "\t-d\t--device\tSelect the output device to use [/dev/lcd]\n"
//              "\t-t\t--type\t\tSelect the LCD type (size) [20x4]\n"
		 "\t-c\t--contrast\tSet the initial contrast [140]\n" "\t-s\t--speed\t\tSet the communication speed [19200]\n" "\t-h\t--help\t\tShow this help information\n");
	 return -1;
      } else {
	 printf ("Invalid parameter: %s\n", argv[i]);
      }

   }

   // Set up io port correctly, and open it...
   fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1) {
      fprintf (stderr, "MtxOrb_init: failed (%s)\n", strerror (errno));
      return -1;
   }
   //else fprintf(stderr, "MtxOrb_init: opened device %s\n", device);
   tcgetattr (fd, &portset);
   // This is necessary in Linux, but does not exist in irix.
#ifndef IRIX
   cfmakeraw (&portset);
#endif
   cfsetospeed (&portset, speed);
   cfsetispeed (&portset, speed);
   tcsetattr (fd, TCSANOW, &portset);


   // Set display-specific stuff..
   MtxOrb_linewrap (1);
   MtxOrb_autoscroll (1);
   MtxOrb_cursorblink (0);


   if (!driver->framebuf) {
      fprintf (stderr, "MtxOrb_init: No frame buffer.\n");
      driver->close ();
      return -1;
   }

   // Set the functions the driver supports...

//   driver->clear =      (void *)-1;
   driver->clear = MtxOrb_clear;
   driver->string = (void *) -1;
//  driver->chr =        MtxOrb_chr;
   driver->chr = (void *) -1;
   driver->vbar = MtxOrb_vbar;
   driver->init_vbar = MtxOrb_init_vbar;
//   driver->init_vbar =  (void *)-1;
   driver->hbar = MtxOrb_hbar;
   driver->init_hbar = MtxOrb_init_hbar;
//   driver->init_hbar =  (void *)-1;
   driver->num = MtxOrb_num;
   driver->init_num = MtxOrb_init_num;

   driver->init = MtxOrb_init;
   driver->close = MtxOrb_close;
   driver->flush = MtxOrb_flush;
   driver->flush_box = MtxOrb_flush_box;
   driver->contrast = MtxOrb_contrast;
   driver->backlight = MtxOrb_backlight;
   driver->output = MtxOrb_output;
   driver->set_char = MtxOrb_set_char;
   driver->icon = MtxOrb_icon;
   driver->draw_frame = MtxOrb_draw_frame;

   driver->getkey = MtxOrb_getkey;

   MtxOrb_contrast (contrast);

   return fd;
}


// TODO: Check this quick hack to detect clear of the screen.
/////////////////////////////////////////////////////////////////
// Clear: catch up when the screen get clear to be able to
//  forget bar caracter not in use anymore and reuse the
//  slot for another bar caracter.
//
void
MtxOrb_clear ()
{
// REMOVE:  fprintf(stderr, "GLU: MtxOrb_clear.\n");
   drv_base_clear ();
   clear = 1;
}


/////////////////////////////////////////////////////////////////
// Clean-up
//
void
MtxOrb_close ()
{
   close (fd);

   if (MtxOrb->framebuf)
      free (MtxOrb->framebuf);

   MtxOrb->framebuf = NULL;
}


void
MtxOrb_flush ()
{
   MtxOrb_draw_frame (lcd.framebuf);
}


void
MtxOrb_flush_box (int lft, int top, int rgt, int bot)
{
   int y;
   char out[LCD_MAX_WIDTH];


//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

   for (y = top; y <= bot; y++) {
      sprintf (out, "%cG%c%c", 254, lft, y);
      write (fd, out, 4);
      write (fd, lcd.framebuf + (y * lcd.wid) + lft, rgt - lft + 1);

   }


}


/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
MtxOrb_chr (int x, int y, char c)
{
   y--;
   x--;

   lcd.framebuf[(y * lcd.wid) + x] = c;
}


/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
//
int
MtxOrb_contrast (int contrast)
{
   char out[4];
   static int status = 140;

   if (contrast > 0) {
      status = contrast;
      sprintf (out, "%cP%c", 254, status);
      write (fd, out, 3);
   }

   return status;
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
void
MtxOrb_backlight (int on)
{
   char out[4];
   if (on) {
      sprintf (out, "%cB%c", 254, 0);
      write (fd, out, 3);
   } else {
      sprintf (out, "%cF", 254);
      write (fd, out, 2);
   }
}


/////////////////////////////////////////////////////////////////
// Sets output port on or off
void
MtxOrb_output (int on)
{
   char out[4];
   if (on) {
      sprintf (out, "%cW", 254);
      write (fd, out, 2);
   } else {
      sprintf (out, "%cV", 254);
      write (fd, out, 2);
   }
}


/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
MtxOrb_linewrap (int on)
{
   char out[4];
   if (on)
      sprintf (out, "%cC", 254);
   else
      sprintf (out, "%cD", 254);
   write (fd, out, 2);
}


/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
MtxOrb_autoscroll (int on)
{
   char out[4];
   if (on)
      sprintf (out, "%cQ", 254);
   else
      sprintf (out, "%cR", 254);
   write (fd, out, 2);
}


// TODO: make sure this doesn't mess up non-VFD displays
/////////////////////////////////////////////////////////////////
// Toggle cursor blink on/off
//
static void
MtxOrb_cursorblink (int on)
{
   char out[4];
   if (on)
      sprintf (out, "%cS", 254);
   else
      sprintf (out, "%cT", 254);
   write (fd, out, 2);
}


//// TODO: Might not be needed anymore...
/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//
void
MtxOrb_init_vbar ()
{
   MtxOrb_init_all (vbar);
}

// TODO: Might not be needed anymore...
/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
MtxOrb_init_hbar ()
{
   MtxOrb_init_all (hbar);
}

// TODO: Finish the support for bar growing reverse way.
// TODO: Need a "y" as input also !!!
/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
// This is the new version ussing dynamic icon alocation
//
void
MtxOrb_vbar (int x, int len)
{
   unsigned char mapu[9] = { barw, baru1, baru2, baru3, baru4, baru5, baru6, baru7, barb };
   unsigned char mapd[9] = { barw, bard1, bard2, bard3, bard4, bard5, bard6, bard7, barb };

   int y;

// TODO: REMOVE THE NEXT LINE FOR TESTING ONLY...
//  len=-len;
// TODO: REMOVE THE PREVIOUS LINE FOR TESTING ONLY...

   if (len > 0) {
      for (y = lcd.hgt; y > 0 && len > 0; y--) {
	 if (len >= lcd.cellhgt)
	    MtxOrb_chr (x, y, 255);
	 else
	    MtxOrb_chr (x, y, MtxOrb_ask_bar (mapu[len]));

	 len -= lcd.cellhgt;
      }
   } else {
      len = -len;
      for (y = 2; y <= lcd.hgt && len > 0; y++) {
	 if (len >= lcd.cellhgt)
	    MtxOrb_chr (x, y, 255);
	 else
	    MtxOrb_chr (x, y, MtxOrb_ask_bar (mapd[len]));

	 len -= lcd.cellhgt;
      }
   }

}

// TODO: Finish the support for bar growing reverse way.
/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
// This is the new version ussing dynamic icon alocation
//
void
MtxOrb_hbar (int x, int y, int len)
{
   unsigned char mapr[6] = { barw, barr1, barr2, barr3, barr4, barb };
   unsigned char mapl[6] = { barw, barl1, barl2, barl3, barl4, barb };

   if (len > 0) {
      for (; x <= lcd.wid && len > 0; x++) {
	 if (len >= lcd.cellwid)
	    MtxOrb_chr (x, y, 255);
	 else
	    MtxOrb_chr (x, y, MtxOrb_ask_bar (mapr[len]));

	 len -= lcd.cellwid;

      }
   } else {
      len = -len;
      for (; x > 0 && len > 0; x--) {
	 if (len >= lcd.cellwid)
	    MtxOrb_chr (x, y, 255);
	 else
	    MtxOrb_chr (x, y, MtxOrb_ask_bar (mapl[len]));

	 len -= lcd.cellwid;

      }
   }

}

// TODO: Might not work, bignum is untested... an untested with dynamic bar.

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void
MtxOrb_init_num ()
{
   char out[3];
   if (custom != bign) {
      sprintf (out, "%cn", 254);
      write (fd, out, 2);
      custom = bign;
   }
}

// TODO: Might not work, bignum is untested... an untested with dynamic bar.

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
MtxOrb_num (int x, int num)
{
   char out[5];
   sprintf (out, "%c#%c%c", 254, x, num);
   write (fd, out, 4);
}


// TODO: This could be higly optimised if data where to be pre-computed.

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void
MtxOrb_set_char (int n, char *dat)
{
   char out[4];
   int row, col;
   int letter;

   if (n < 0 || n > 7)
      return;
   if (!dat)
      return;

   sprintf (out, "%cN%c", 254, n);
   write (fd, out, 3);

   for (row = 0; row < lcd.cellhgt; row++) {
      letter = 0;
      for (col = 0; col < lcd.cellwid; col++) {
	 letter <<= 1;
	 letter |= (dat[(row * lcd.cellwid) + col] > 0);
      }
      write (fd, &letter, 1);
   }
}


void
MtxOrb_icon (int which, char dest)
{
   char icons[3][5 * 8] = {
      {
       1, 1, 1, 1, 1,		// Empty Heart
       1, 0, 1, 0, 1,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       1, 0, 0, 0, 1,
       1, 1, 0, 1, 1,
       1, 1, 1, 1, 1,
       },

      {
       1, 1, 1, 1, 1,		// Filled Heart
       1, 0, 1, 0, 1,
       0, 1, 0, 1, 0,
       0, 1, 1, 1, 0,
       0, 1, 1, 1, 0,
       1, 0, 1, 0, 1,
       1, 1, 0, 1, 1,
       1, 1, 1, 1, 1,
       },

      {
       0, 0, 0, 0, 0,		// Ellipsis
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       1, 0, 1, 0, 1,
       },

   };

   if (custom == bign)
      custom = beat;
   MtxOrb_set_char (dest, &icons[which][0]);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized lcd.wid*lcd.hgt
//
void
MtxOrb_draw_frame (char *dat)
{
   char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
   int i;

   if (!dat)
      return;

   for (i = 0; i < lcd.hgt; i++) {
      sprintf (out, "%cG%c%c", 254, 1, i + 1);
      write (fd, out, 4);
      write (fd, dat + (lcd.wid * i), lcd.wid);
   }

}



/////////////////////////////////////////////////////////////
// returns one character from the keypad...
// (A-Z) on success, 0 on failure...
//
char
MtxOrb_getkey ()
{
   char in = 0;
   read (fd, &in, 1);
   return in;
}

void
MtxOrb_led (int which, int on)
{
   char out[4];

   if (which < 0 || which > 7)
      return;

   if (!which) {
      if (on)
	 sprintf (out, "%cV", 254);
      else
	 sprintf (out, "%cW", 254);
      write (fd, out, 2);
   } else {
      if (on)
	 sprintf (out, "%cV%c", 254, which);
      else
	 sprintf (out, "%cW%c", 254, which);
      write (fd, out, 3);
   }
}


/////////////////////////////////////////////////////////////////
// Ask for dynamic allocation of a custom caracter to be
//  a well none bar graphic. The function is suppose to
//  return a value between 0 and 7 but 0 is reserver for
//  heart beat.
//  This function manadge a cache of graphical caracter in use.
//  I really hope it is working and bug-less because it is not
//  completely tested, just a quick hack.
//
int
MtxOrb_ask_bar (int type)
{
   int i;
   int last_not_in_use;
   int pos;			// 0 is icon, 1 to 7 are free, 8 is not found.
   // TODO: Reuse graphic caracter 0 if heartbeat is not in use.

   // REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar(%d).\n", type);
   // Check if the screen was clear.
   if (clear) {			// If the screen was clear then graphic caracter are not in use.
      //REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| clear was set.\n");
      use[0] = 1;		// Heartbeat is always in use (not true but it help).
      for (pos = 1; pos < 8; pos++) {
	 use[pos] = 0;		// Other are not in use.
      }
      clear = 0;		// We made the special treatement.
   } else {
      // Nothing special if some caracter a curently in use (...) ?
   }

   // Search for a match with caracter already defined.
   pos = 8;			// Not found.
   last_not_in_use = 8;		// No empty slot to reuse.
   for (i = 1; i < 8; i++) {	// For all but first (heartbeat).
      if (!use[i])
	 last_not_in_use = i;	// New empty slot.
      if (def[i] == type)
	 pos = i;		// Founded (should break now).
   }

   if (pos == 8) {
      // REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| not found.\n");
      pos = last_not_in_use;
      // TODO: Best match/deep search is no more graphic caracter are available.
   }

   if (pos != 8) {		// A caracter is found (Best match could solve our problem).
      // REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| found at %d.\n", pos);
      if (def[pos] != type) {
	 MtxOrb_set_known_char (pos, type);	// Define a new graphic caracter.
	 def[pos] = type;	// Remember that now the caracter is available.
      }
      if (!use[pos]) {		// If the caracter is no yet in use (but defined).
	 use[pos] = 1;		// Remember it is in use (so protect it from re-use).
      }
   }

   if (pos == 8)		// TODO: Choose a character to approximate the graph
   {
      //pos=65; // ("A")?
      switch (type) {
      case baru1:
	 pos = '_';
	 break;
      case baru2:
	 pos = '.';
	 break;
      case baru3:
	 pos = ',';
	 break;
      case baru4:
	 pos = 'o';
	 break;
      case baru5:
	 pos = 'o';
	 break;
      case baru6:
	 pos = 'O';
	 break;
      case baru7:
	 pos = '8';
	 break;

      case bard1:
	 pos = '\'';
	 break;
      case bard2:
	 pos = '"';
	 break;
      case bard3:
	 pos = '^';
	 break;
      case bard4:
	 pos = '^';
	 break;
      case bard5:
	 pos = '*';
	 break;
      case bard6:
	 pos = 'O';
	 break;
      case bard7:
	 pos = '8';
	 break;

      case barr1:
	 pos = '-';
	 break;
      case barr2:
	 pos = '-';
	 break;
      case barr3:
	 pos = '=';
	 break;
      case barr4:
	 pos = '=';
	 break;

      case barl1:
	 pos = '-';
	 break;
      case barl2:
	 pos = '-';
	 break;
      case barl3:
	 pos = '=';
	 break;
      case barl4:
	 pos = '=';
	 break;

      case barw:
	 pos = ' ';
	 break;

      case barb:
	 pos = 255;
	 break;

      default:
	 pos = '?';
	 break;
      }
   }

   return (pos);
}


/////////////////////////////////////////////////////////////////
// Sets up a well known character for use.
//
void
MtxOrb_set_known_char (int car, int type)
{
   char all_bar[25][5 * 8] = {
      {
       0, 0, 0, 0, 0,		//  char u1[] = 
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       0, 0, 0, 0, 0,
       1, 1, 1, 1, 1,
       }, {
	   0, 0, 0, 0, 0,	//  char u2[] = 
	   0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0,
	   1, 1, 1, 1, 1,
	   1, 1, 1, 1, 1,
	   }, {
	       0, 0, 0, 0, 0,	//  char u3[] = 
	       0, 0, 0, 0, 0,
	       0, 0, 0, 0, 0,
	       0, 0, 0, 0, 0,
	       0, 0, 0, 0, 0,
	       1, 1, 1, 1, 1,
	       1, 1, 1, 1, 1,
	       1, 1, 1, 1, 1,
	       }, {
		   0, 0, 0, 0, 0,	//  char u4[] = 
		   0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0,
		   1, 1, 1, 1, 1,
		   1, 1, 1, 1, 1,
		   1, 1, 1, 1, 1,
		   1, 1, 1, 1, 1,
		   }, {
		       0, 0, 0, 0, 0,	//  char u5[] = 
		       0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0,
		       1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1,
		       }, {
			   0, 0, 0, 0, 0,	//  char u6[] = 
			   0, 0, 0, 0, 0,
			   1, 1, 1, 1, 1,
			   1, 1, 1, 1, 1,
			   1, 1, 1, 1, 1,
			   1, 1, 1, 1, 1,
			   1, 1, 1, 1, 1,
			   1, 1, 1, 1, 1,
			   }, {
			       0, 0, 0, 0, 0,	//  char u7[] = 
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       1, 1, 1, 1, 1,
			       }, {
				   1, 1, 1, 1, 1,	//  char d1[] = 
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   }, {
				       1, 1, 1, 1, 1,	//  char d2[] = 
				       1, 1, 1, 1, 1,
				       0, 0, 0, 0, 0,
				       0, 0, 0, 0, 0,
				       0, 0, 0, 0, 0,
				       0, 0, 0, 0, 0,
				       0, 0, 0, 0, 0,
				       0, 0, 0, 0, 0,
				       }, {
					   1, 1, 1, 1, 1,	//  char d3[] = 
					   1, 1, 1, 1, 1,
					   1, 1, 1, 1, 1,
					   0, 0, 0, 0, 0,
					   0, 0, 0, 0, 0,
					   0, 0, 0, 0, 0,
					   0, 0, 0, 0, 0,
					   0, 0, 0, 0, 0,
					   }, {
					       1, 1, 1, 1, 1,	//  char d4[] = 
					       1, 1, 1, 1, 1,
					       1, 1, 1, 1, 1,
					       1, 1, 1, 1, 1,
					       0, 0, 0, 0, 0,
					       0, 0, 0, 0, 0,
					       0, 0, 0, 0, 0,
					       0, 0, 0, 0, 0,
					       }, {
						   1, 1, 1, 1, 1,	//  char d5[] = 
						   1, 1, 1, 1, 1,
						   1, 1, 1, 1, 1,
						   1, 1, 1, 1, 1,
						   1, 1, 1, 1, 1,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   }, {
						       1, 1, 1, 1, 1,	//  char d6[] = 
						       1, 1, 1, 1, 1,
						       1, 1, 1, 1, 1,
						       1, 1, 1, 1, 1,
						       1, 1, 1, 1, 1,
						       1, 1, 1, 1, 1,
						       0, 0, 0, 0, 0,
						       0, 0, 0, 0, 0,
						       }, {
							   1, 1, 1, 1, 1,	//  char d7[] = 
							   1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1,
							   0, 0, 0, 0, 0,
							   }, {
							       1, 0, 0, 0, 0,	//  char r1[] = 
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       1, 0, 0, 0, 0,
							       }, {
								   1, 1, 0, 0, 0,	//  char r2[] = 
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   1, 1, 0, 0, 0,
								   }, {
								       1, 1, 1, 0, 0,	//  char r3[] = 
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       1, 1, 1, 0, 0,
								       }, {
									   1, 1, 1, 1, 0,	//  char r4[] = 
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   1, 1, 1, 1, 0,
									   }, {
									       0, 0, 0, 0, 1,	//  char l1[] = 
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       0, 0, 0, 0, 1,
									       }, {
										   0, 0, 0, 1, 1,	//  char l2[] = 
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   0, 0, 0, 1, 1,
										   }, {
										       0, 0, 1, 1, 1,	//  char l3[] = 
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       0, 0, 1, 1, 1,
										       }, {
											   0, 1, 1, 1, 1,	//  char l4[] = 
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   0, 1, 1, 1, 1,
											   }, {
											       1, 1, 1, 1, 1,	// Empty Heart
											       1, 0, 1, 0, 1,
											       0, 0, 0, 0, 0,
											       0, 0, 0, 0, 0,
											       0, 0, 0, 0, 0,
											       1, 0, 0, 0, 1,
											       1, 1, 0, 1, 1,
											       1, 1, 1, 1, 1,
											       }, {
												   1, 1, 1, 1, 1,	// Filled Heart
												   1, 0, 1, 0, 1,
												   0, 1, 0, 1, 0,
												   0, 1, 1, 1, 0,
												   0, 1, 1, 1, 0,
												   1, 0, 1, 0, 1,
												   1, 1, 0, 1, 1,
												   1, 1, 1, 1, 1,
												   }, {
												       0, 0, 0, 0, 0,	// Ellipsis
												       0, 0, 0, 0, 0,
												       0, 0, 0, 0, 0,
												       0, 0, 0, 0, 0,
												       0, 0, 0, 0, 0,
												       0, 0, 0, 0, 0,
												       0, 0, 0, 0, 0,
												       1, 0, 1, 0, 1,
												       }
   };

   MtxOrb_set_char (car, &all_bar[type][0]);
}



/////////////////STOP READING --- TRASH IS AT THE END////////////////



// TODO: Remove this code wich was use for developpement.
// PS: There might be reference to this code left, so keep it for some time.
void
MtxOrb_init_all (int type)
{
}
