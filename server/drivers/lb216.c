#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_NCURSES_H
# include <ncurses.h>
#else
# include <curses.h>
#endif

#include "shared/str.h"
#include "shared/debug.h"
#include "lcd.h"
#include "lb216.h"
#include "drv_base.h"
#include "render.h"

static int custom=0;
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
        beat = 8 } custom_type;



static int fd;

static void LB216_hidecursor();
static void LB216_reboot();

// TODO:  Get rid of this variable?
lcd_logical_driver *LB216;
// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int LB216_init(lcd_logical_driver *driver, char *args)
{
   char *argv[64];
   int argc;
   struct termios portset;
   int i;
   int tmp;
   int reboot=0;
   
   char device[256] = "/dev/lcd";
   int speed=B9600;
   
   LB216 = driver;

   //debug("LB216_init: Args(all): %s\n", args);
   
   argc = get_args(argv, args, 64);

   /*
   for(i=0; i<argc; i++)
   {
      printf("Arg(%i): %s\n", i, argv[i]);
   }
   */
   
   for(i=0; i<argc; i++)
   {
      //printf("Arg(%i): %s\n", i, argv[i]);
      if(0 == strcmp(argv[i], "-d")  ||
	 0 == strcmp(argv[i], "--device"))
      {
	 if(i + 1 > argc) {
	    fprintf(stderr, "LB216_init: %s requires an argument\n",
		    argv[i]);
	    return -1;
	 }
	 strcpy(device, argv[++i]);
      }
      else if(0 == strcmp(argv[i], "-b")  ||
	 0 == strcmp(argv[i], "--brightness"))
      {
	 if(i + 1 > argc) {
	    fprintf(stderr, "LB216_init: %s requires an argument\n",
		    argv[i]);
	    return -1;
	 }
	 tmp = atoi(argv[++i]);
         if((tmp < 0) || (tmp > 255)){
	    fprintf(stderr, "LB216_init: %s argument must between 0 and 255. Using default value.\n",
		    argv[i]);
         } else backlight_brightness = tmp;
      }
      else if(0 == strcmp(argv[i], "-s")  ||
	 0 == strcmp(argv[i], "--speed"))
      {
	 if(i + 1 > argc) {
	    fprintf(stderr, "LB216_init: %s requires an argument\n",
		    argv[i]);
	    return -1;
	 }
	 tmp = atoi(argv[++i]);
         if(tmp==2400) speed=B2400;
         else if(tmp==9600) speed=B9600;
         else {
	    fprintf(stderr, "LB216_init: %s argument must be 2400, or 9600. Using default value.\n",
		    argv[i]);
         }
      }
      else if(0 == strcmp(argv[i], "-h")  ||
	 0 == strcmp(argv[i], "--help"))
      {
	 printf("LCDproc LB216 LCD driver\n"
		"\t-d\t--device\tSelect the output device to use [/dev/lcd]\n"
		"\t-t\t--type\t\tSelect the LCD type (size) [16x2]\n"
		"\t-b\t--brightness\tSet the initial brightness [255]\n"
		"\t-s\t--speed\t\tSet the communication speed [9600]\n"
		"\t-r\t--reboot\tReinitialize the LCD's BIOS\n"
		"\t-h\t--help\t\tShow this help information\n");
	 return -1;
      }
      else if(0 == strcmp(argv[i], "-r")  ||
	 0 == strcmp(argv[i], "--reboot"))
      {
	 printf("LCDd: rebooting LB216 LCD...\n");
	 reboot=1;
      }
      else
      {
	 printf("Invalid parameter: %s\n", argv[i]);
      }
      
   }
   
   // Set up io port correctly, and open it...
   fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY); 
   if (fd == -1) 
   {
      fprintf(stderr, "LB216_init: failed (%s)\n", strerror(errno));
      return -1;
   }
   //else fprintf(stderr, "LB216_init: opened device %s\n", device);
   tcgetattr(fd, &portset);

   // We use RAW mode
#ifdef HAVE_CFMAKERAW
   // The easy way
   cfmakeraw( &portset );
#else
   // The hard way
   portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                         | INLCR | IGNCR | ICRNL | IXON );
   portset.c_oflag &= ~OPOST;
   portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
   portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
   portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

   // Set port speed
   cfsetospeed (&portset, speed);
   cfsetispeed (&portset, B0);

   // Do it...
   tcsetattr(fd, TCSANOW, &portset);
   

   // Set display-specific stuff..
   if(reboot)
   {
      LB216_reboot();
      sleep(4);
      reboot=0;
   }
   sleep(1);
   LB216_hidecursor();
   LB216_backlight(backlight_brightness);

   
   if(!driver->framebuf)
   {
      fprintf(stderr, "LB216_init: No frame buffer.\n");
      driver->close();
      return -1;
   }


   // Set the functions the driver supports...

   driver->clear =      drv_base_clear;
   driver->string =     LB216_string;
   driver->chr =        LB216_chr;
   driver->vbar =       LB216_vbar;
   driver->init_vbar =  LB216_init_vbar;
   driver->hbar =       LB216_hbar;
   driver->init_hbar =  LB216_init_hbar;
   driver->num =        NULL;
   driver->init_num =   NULL;

   driver->init =       LB216_init;
   driver->close =      LB216_close;
   driver->flush =      LB216_flush;
   driver->flush_box =  NULL;
   driver->contrast =   NULL;
   driver->backlight =  LB216_backlight;
   driver->set_char =   LB216_set_char;
   driver->icon =       LB216_icon;
   driver->draw_frame = LB216_draw_frame;

   lcd.cellwid = 5;
   lcd.cellhgt = 8;

   debug("LB216: foo!\n");
   
   return fd;
}



/////////////////////////////////////////////////////////////////
// Clean-up
//
void LB216_close() 
{
  close (fd); 

  if(LB216->framebuf) free(LB216->framebuf);

  LB216->framebuf = NULL;
}


void LB216_flush()
{
   LB216_draw_frame(lcd.framebuf);
}



/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
void LB216_chr(int x, int y, char c) 
{
  //y--;
 // x--;
  
  //if(c < 32  &&  c >= 0) c += 128;
//  lcd.framebuf[(y*lcd.wid) + x] = c;
	char chr[1];
	sprintf(chr, "%c", c);
	LB216_string (x, y, chr);
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
void LB216_backlight(int on)
{
  char out[4];
  if(on)
  {
    sprintf(out, "%c%c", 254, 253);
  }
  else
  {
    sprintf(out, "%c%c", 254, 252);
  }
    write(fd, out, 2);
}


/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void LB216_hidecursor()
{
  char out[4];
  sprintf(out, "%c%c", 254,12);
  write(fd, out, 2);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void LB216_reboot()
{
  char out[4];
  sprintf(out, "%c%c", 254,1);
  write(fd, out, 2);
}


/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized lcd.wid*lcd.hgt
//
void LB216_draw_frame(char *dat)
{
  char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
  int i,j;
  
  if(!dat) return;

  sprintf(out, "%c%c", 254,80);
  write(fd, out, 2);

  for(j=0; j<lcd.hgt; j++) {
	if (j>=2) {
    	sprintf(out,"%c%c",254,148+(64*(j-2)));
	} else {
    	sprintf(out,"%c%c",254,128+(64*(j)));
	}
    write(fd, out, 2);
    for(i=0; i<lcd.wid; i++) {
      sprintf(out,"%c",dat[i+(j*lcd.wid)]);
      write(fd, out, 1);
    }
  }
}

void LB216_string (int x, int y, char string[])
{
   int i;
   char c;

//printf("%d,%d:%s\n",x,y,string);
   y--;x--;
   for(i=0; string[i]; i++)
   {
      c = string[i];
      switch(c)
      {
         case '\254': c = '#'; break;
      }
      lcd.framebuf[(y*lcd.wid) + x+i] = c;
   }

}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//
void LB216_init_vbar() 
{
  char a[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
  };
  char b[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char c[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char d[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char e[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char f[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char g[] = {
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };

  if(custom!=vbar) {
    LB216_set_char(1,a);
    LB216_set_char(2,b);
    LB216_set_char(3,c);
    LB216_set_char(4,d);
    LB216_set_char(5,e);
    LB216_set_char(6,f);
    LB216_set_char(7,g);
    custom=vbar;
  }
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void LB216_init_hbar() 
{

  char a[] = {
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
  };
  char b[] = {
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
  };
  char c[] = {
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
  };
  char d[] = {
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
  };
  char e[] = {
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };

  if(custom!=hbar) {
    LB216_set_char(1,a);
    LB216_set_char(2,b);
    LB216_set_char(3,c);
    LB216_set_char(4,d);
    LB216_set_char(5,e);
    custom=hbar;
  }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
void LB216_vbar(int x, int len) 
{
  char map[9] = {32, 1, 2, 3, 4, 5, 6, 7, 255 };
  

  int y;
  for(y=lcd.hgt; y > 0 && len>0; y--)
    {
      if(len >= lcd.cellhgt) LB216_chr(x, y, 255);
      else LB216_chr(x, y, map[len]);

      len -= lcd.cellhgt;
    }
  
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void LB216_hbar(int x, int y, int len)
{
  char map[7] = { 32, 1, 2, 3, 4, 5 };

  for(; x<=lcd.wid && len>0; x++)
    {
      if(len >= lcd.cellwid) LB216_chr(x,y,map[5]);
      else LB216_chr(x, y, map[len]);
      
	 //printf ("%d,",len);
      len -= lcd.cellwid;
      
    }
//	printf ("\n");

}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void LB216_set_char(int n, char *dat)
{
  char out[4];
  int row, col;
  int letter;

  if(n < 0 || n > 7) return;
  n=64+(8*n);
  if(!dat) return;

  sprintf(out, "%c%c", 254, n);
  write(fd, out, 2);

  for(row=0; row<lcd.cellhgt; row++)
  {
    letter = 1;
    for(col=0; col<lcd.cellwid; col++)
    {
      letter <<= 1;
      letter |= (dat[(row*lcd.cellwid) + col] > 0);
    }
	sprintf(out,"%c",letter);
    write(fd, out, 1);
  }
}

void LB216_icon(int which, char dest)
{
  char icons[3][8*8] = {
   {
     1,1,1,1,1,  // Empty Heart
     1,0,1,0,1,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,0,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },   

   {
     1,1,1,1,1,  // Filled Heart
     1,0,1,0,1,
     0,1,0,1,0,
     0,1,1,1,0,
     0,1,1,1,0,
     1,0,1,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },
   
   {
     0,0,0,0,0,  // Ellipsis
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,1,0,1,
   },
   
  };
  
  if(custom==bign) custom=beat;
  LB216_set_char(dest, &icons[which][0]);
}
