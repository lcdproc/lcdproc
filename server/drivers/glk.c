/*
 * MatrixOrbital GLK Graphic Display Driver
 *
 * http://www.matrixorbital.com
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "lcd.h"
#include "shared/str.h"
#include "glk.h"
#include "glkproto.h"

static GLKDisplay *  PortFD ;
/*  Initialize pseudo-CGRAM to empty */
static unsigned char  CGRAM[8] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

//////////////////////////////////////////////////////////////////////////
//////////////////// Matrix Orbital Graphical Driver /////////////////////
//////////////////////////////////////////////////////////////////////////

static unsigned char *  screen_contents = NULL ;
static int  fontselected = 0 ;
static int  gpo_count = 0 ;

static char *framebuf = NULL;
static int width = 0;
static int height = 0;
static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;
static int contrast;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "glk_";



////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
glk_init(Driver *drvthis, char *args)
{
   char *  argv[64];
   int  argc;
   char *  device = "/dev/lcd" ;
   speed_t  speed = B19200 ;
   int  i ;
   contrast = 560;

//   printf("glk_init()\n");

   argc = get_args( argv, args, 64 );
   for( i = 0 ; i < argc; ++i ) {
      if( 0 == strcmp( argv[i], "-d")
          || 0 == strcmp( argv[i], "--device" )
        ) {
         if( i+1 >= argc ) {
            fprintf( stderr, "glk: %s requires an argument\n", argv[i]);
            return( -1 );
         };
         device = argv[i+1] ;
         ++i ;
      } else if( 0 == strcmp( argv[i], "-c" )
                 || 0 == strcmp( argv[i], "--contrast" )
               ) {
         int  tmp ;
         if( i+1 >= argc ) {
            fprintf( stderr, "glk: %s requires an argument\n", argv[i]);
            return( -1 );
         };
         tmp = atoi( argv[i+1] );
         if( tmp < 0 || tmp > 255 ) {
            fprintf( stderr, "glk: %s argument must be between 0 and 255\n", argv[i]);
         } else {
            contrast = tmp ;
         };
         ++i ;
      } else if( 0 == strcmp( argv[i], "-s" )
                 || 0 == strcmp( argv[i], "--speed" )
               ) {
         int  tmp ;
         if( i+1 >= argc ) {
            fprintf( stderr, "glk: %s requires an argument\n", argv[i]);
            return( -1 );
         };
         tmp = atoi( argv[i+1] );
         if( tmp == 9600 ) {
            speed = B9600 ;
         } else if( tmp == 19200 ) {
            speed = B19200 ;
         } else if( tmp == 38400 ) {
            speed = B38400 ;
         } else {
            fprintf( stderr, "glk: %s argument (%s) not a valid speed.\n", argv[i], argv[i+1] );
         };
         ++i ;
      } else if( 0 == strcmp( argv[i], "-h" )
                 || 0 == strcmp( argv[i], "--help" )
               ) {
         printf(
"LCDproc Matrix-Orbital GLK Graphical LCD driver\n"
"\n"
"-d, --device   select the serial device to use [/dev/lcd]\n"
"-c, --contrast set the initial contrast value [560]\n"
"-s, --speed    set the serial port speed [19200]\n"
"-h, --help     display this help text\n"
         );
         return( -1 );
      } else {
         fprintf( stderr, "glk: Invalid parameter: %s\n", argv[i] );
      };
   };

  PortFD = glkopen( device, speed );
  if( PortFD == NULL ) {
    return -1 ;
  };

  // Query the module for a device type
  glkputl( PortFD, GLKCommand, 0x37, EOF );
  i = glkget( PortFD );
  if( i < 0 ) {
    fprintf( stderr, "glk: GLK did not respond to READ MODULE TYPE.\n" );
    return -1 ;
  } else {
    switch( i ) {
    case 0x10 :  // GLC12232
      width = 20 ; height = 4 ; break ;
    case 0x11 :  // GLC12864
      width = 20 ; height = 8 ; break ;
    case 0x12 :  // GLC128128
      width = 20 ; height = 16 ; break ;
    case 0x13 :  // GLC24064
      width = 40 ; height = 8 ; gpo_count = 1 ; break ;
    case 0x14 :  // GLK12864-25
      width = 20 ; height = 8 ; break ;
    case 0x15 :  // GLK24064-25
      width = 40 ; height = 8 ; gpo_count = 1 ; break ;
    case 0x21 :  // GLK128128-25
      width = 20 ; height = 16 ; break ;
    case 0x22 :  // GLK12232-25
      width = 20 ; height = 4 ; gpo_count = 2 ; break ;
    case 0x23 :  // GLK12232-25SM
      width = 20 ; height = 4 ; gpo_count = 2 ; break ;
    case 0x24 :  // GLK12232-25SM-Penguin
      width = 20 ; height = 4 ; gpo_count = 2 ; break ;
    default :
      fprintf( stderr, "glk: Unrecognized module type: 0x%02x\n", i );
      return( -1 );
    };
  };

  framebuf = malloc(width * height);
  screen_contents = malloc( width * height );

  if(framebuf == NULL || screen_contents == NULL ) {
    fprintf( stderr, "glk: Unable to allocate memory for screen buffers\n" );
    glk_close(drvthis);
    return -1;
  }

  memset(framebuf, ' ', width*height);

//  glk_clear();
//  glkputl( PortFD, GLKCommand, 0x58, EOF );

  // No font selected
  fontselected = -1 ;

  // Enable flow control
  glkflow( PortFD, 40, 2 );

  // Set read character timeout to 0
  glktimeout( PortFD, 0 );

  // Enable auto-transmit of up/down key events
  // This allows us to generate REPEAT keys distinct from
  //   normal keys using timeouts.  (see glk_getkey)
  glkputl( PortFD, GLKCommand, 0x7e, 1, GLKCommand, 0x41, EOF );

  // Set contrast
  glk_set_contrast( drvthis, contrast );

  return 0;
}


/////////////////////////////////////////////////////////////////
// Close the driver
//
MODULE_EXPORT void
glk_close(Driver *drvthis)
{
  glkclose( PortFD ) ;

  if(framebuf) free(framebuf);
  framebuf = NULL;
}


/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
glk_width (Driver *drvthis)
{
	return width;
}


/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
glk_height (Driver *drvthis)
{
	return height;
}


/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
#define CLEARCOUNT  (1000000)
static int  clearcount = 0 ;
void glk_clear_forced(Driver *drvthis)
{
//  puts( "REALLY CLEARING the display" );
  clearcount = CLEARCOUNT ;
  glkputl( PortFD, GLKCommand, 0x58, EOF );
  memset(screen_contents, ' ', width*height);
}
MODULE_EXPORT void
glk_clear(Driver *drvthis)
{
//  puts( "glk_clear( )" );
  memset(framebuf, ' ', width*height);
  if( --clearcount < 0 ) {
    glk_clear_forced(drvthis);
  };
}


//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
glk_flush(Driver *drvthis)
{
//   puts( "glk_flush( )" );
  char *  p ;
  char *  q ;
  int  x, y ;
  int  xs ;
  char *  ps = NULL ;

//  printf( "flush()\n" );

  p = framebuf ;
  q = screen_contents ;

  for( y = 0 ; y < height ; ++y ) {
    xs = -1 ;  /* XStart not set */
    for( x = 0 ; x < width ; ++x ) {
      if( *q == *p && xs >= 0 ) {
        /* Write accumulated string */
        glkputl( PortFD, GLKCommand, 0x79, xs*6+1, y*8, EOF );
        glkputa( PortFD, x - xs, ps );
//        printf( "draw_frame: Writing at (%d,%d) for %d\n", xs, y, x-xs );
        xs = -1 ;
      } else if( *q != *p && xs < 0 ) {
        /* Start new string of changes */
        ps = p ;
        xs = x ;
      };
      *q++ = *p++ ;  /* Update screen_contents from framebuf */
    };
    if( xs >= 0 ) {
      /* Write accumulated line */
      glkputl( PortFD, GLKCommand, 0x79, xs*6+1, y*8, EOF );
      glkputa( PortFD, width - xs, ps );
//      printf( "draw_frame: Writing at (%d,%d) for %d\n", xs, y, width-xs );
    };

  };  /* For y */

  return ;
}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
glk_string(Driver *drvthis, int x, int y, char string[])
{
  char *  p ;

//  printf( "glk_string( %d, %d, \"%s\" )\n", x, y, string );

  if( x > width || y > height ) {
     return ;
  };

  for( p = string ; *p && x <= width ; ++x, ++p ) {
    glk_chr( drvthis, x, y, *p );
  };

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
glk_chr(Driver *drvthis, int x, int y, char c)
{
  int  myc = (unsigned char) c ;
  x -= 1;  // Convert 1-based coords to 0-based...
  y -= 1;

  if( fontselected != 2 ) {
//    puts( "Switching to font 2" );
    /* Select font 2 */
    glkputl( PortFD, GLKCommand, 0x31, 2, EOF );
    fontselected = 2 ;
    /* Set font metrics */
    glkputl( PortFD, GLKCommand, 0x32, 1, 0, 1, 1, 32, EOF );
    /* Clear the screen */
    glk_clear_forced(drvthis);
  };

  if( myc >= 0 && myc <= 15 ) {
    /* CGRAM */
//    if( myc != 0 ) {
//      printf( "CGRAM changing %d => %d\n", myc, CGRAM[myc&7] );
//    };
    myc = CGRAM[myc&7] ;
  } else if( myc == 255 || myc == -1 ) {
    /* Solid block */
    myc = 133 ;
  } else if( (myc > 15 && myc < 32) || myc > 143 ) {
    fprintf( stderr, "Attempt to write %d to (%d,%d)\n", myc, x, y );
    myc = 133 ;
  };

  framebuf[(y*width) + x] = myc;

}


/////////////////////////////////////////////////////////////////
// Returns current contrast
// This is only the locally stored contrast, the contrast value
// cannot be retrieved from the LCD.
// Value 0 to 1000.
//
MODULE_EXPORT int
glk_get_contrast(Driver *drvthis)
{
	return contrast;
}


//////////////////////////////////////////////////////////////////////
// Sets the contrast of the display.  Value is 0-255, where 140 is
// what I consider "just right".
//
MODULE_EXPORT void
glk_set_contrast(Driver *drvthis, int promille)
{
  // Check it
  if( contrast < 0 || contrast > 1000 )
  	return;

  // Store it
  contrast = promille;

  // Do it
//    printf("Contrast: %i\n", contrast);
  glkputl( PortFD, GLKCommand, 0x50, (int) ((long)promille * 255 / 1000), EOF );
}

//////////////////////////////////////////////////////////////////////
// Turns the lcd backlight on or off...
//
MODULE_EXPORT void
glk_backlight(Driver *drvthis, int on)
{
  if(on) {
//    printf("Backlight ON\n");
    glkputl( PortFD, GLKCommand, 0x42, 0, EOF );
  } else {
//    printf("Backlight OFF\n");
    glkputl( PortFD, GLKCommand, 0x46, EOF );
  }
}

//////////////////////////////////////////////////////////////////////
// Sets general purpose outputs on or off
MODULE_EXPORT void
glk_output(Driver *drvthis, int on)
{
  if( gpo_count < 2 ) {
    if( on ) {  glkputl( PortFD, GLKCommand, 'W', EOF );
    } else {    glkputl( PortFD, GLKCommand, 'V', EOF );
    };
  } else {
    int  i;
    for( i = 1 ; i <= gpo_count ; ++i, on >>= 1 ) {
      if( on & 1 ) {
        glkputl( PortFD, GLKCommand, 'W', i, EOF );
      } else {
        glkputl( PortFD, GLKCommand, 'V', i, EOF );
      };
    };
  };
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for vertical bargraphs.
//
MODULE_EXPORT void
glk_init_vbar(Driver *drvthis)
{
//  printf("Vertical bars.\n");
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for horizontal bargraphs.
//
MODULE_EXPORT void
glk_init_hbar(Driver *drvthis)
{
//  printf("Horizontal bars.\n");
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for big numbers, if possible.
//
MODULE_EXPORT void
glk_init_num(Driver *drvthis)
{
//  printf("Big Numbers.\n");
  if( fontselected != 3 ) {
    /* Select Big Numbers font */
    glkputl( PortFD, GLKCommand, 0x31, 3, EOF );
    fontselected = 3 ;
    /* Set font metrics */
    glkputl( PortFD, GLKCommand, 0x32, 1, 0, 1, 1, 32, EOF );
    /* Clear the screen */
    glk_clear_forced(drvthis);
  };
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
MODULE_EXPORT void
glk_num(Driver *drvthis, int x, int num)
{
//  printf("BigNum(%i, %i)\n", x, num);
  framebuf[x-1] = num + '0' ;
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
MODULE_EXPORT void
glk_set_char(Driver *drvthis, int n, char *dat)
{
  printf("Set Character %i\n", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
glk_old_vbar(Driver *drvthis, int x, int len)
{
  int  y = height ;

//  printf( "glk_vbar( %d, %d )\n", x, len );
  while( len > cellheight ) {
    glk_chr( drvthis, x, y, 255 );
    --y ;
    len -= cellheight ;
  };

  if( y >= 0 ) {
    int  lastc ;
    switch( len ) {
    case 0 :  return ; break ;  /* Don't output a char */
    case 1 :  lastc = 138 ; break ;  /* One bar */
    case 2 :  lastc = 139 ; break ;
    case 3 :  lastc = 140 ; break ;
    case 4 :  lastc = 141 ; break ;
    case 5 :  lastc = 142 ; break ;
    case 6 :  lastc = 143 ; break ;
    default:  lastc = 133 ; break ;
    };
    glk_chr( drvthis, x, y, lastc );
  };
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
glk_old_hbar(Driver *drvthis, int x, int y, int len)
{
//  printf( "glk_hbar( %d, %d, %d )\n", x, y, len );
  while( len > cellwidth ) {
    glk_chr( drvthis, x, y, 255 );
    ++x ;
    len -= cellwidth ;
  };

  if( x <= width ) {
    int  lastc ;
    switch( len ) {
    case 0 :  lastc = ' ' ; break ;
    case 1 :  lastc = 134 ; break ;  /* One bar */
    case 2 :  lastc = 135 ; break ;
    case 3 :  lastc = 136 ; break ;
    case 4 :  lastc = 137 ; break ;
    default:  lastc = 133 ; break ;
    };
    glk_chr( drvthis, x, y, lastc );
  };
}


/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT void
glk_old_icon(Driver *drvthis, int which, int dest)
{
  /* TODO IMPLEMENTATION OF NEW API */
  /* any volonteers ? */

  unsigned char  old, new ;
  unsigned char *  p ;
  unsigned char *  q ;
  int  count ;

//  printf("Char %i set to icon %i\n", dest, which);

  if( dest < 0 || dest > 7 ) {
    /* Illegal custom character */
    return ;
  };

  /* which == 0  => empty heart   => 131
   * which == 1  => filled heart  => 132
   * which == 2  => ellipsis      => 128
   */
  switch( which ) {
  case 0:  new = 131 ; break ;
  case 1:  new = 132 ; break ;
  case 2:  new = 128 ; break ;
  default:  return ;  /* ERROR */
  };

  old = CGRAM[(int)dest] ;
  CGRAM[(int)dest] = new ;
  p = framebuf ;
  q = screen_contents ;

  /* Replace all old icons with new icon in new frame */
  for( count = width * height ; count ; --count ) {
    if( *q == old ) {
//      printf( "icon %d to %d at %d\n", old, new, q - screen_contents );
      *p = new ;
    };
    ++q ; ++p ;
  };

  return ;
}


//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
MODULE_EXPORT char
glk_old_getkey(Driver *drvthis)
{
  /* NOTE THAT THIS CODE IS NOT USED IN THE NEW API */
  /* It uses get_key instead, returning a string */

  int  c ;
  static int  key = -1 ;
  static struct timeval  lastkey ;
  struct timeval  now ;

//  puts( "glk_getkey( )" );

  c = glkgetc( PortFD );

  if( c >= 'A' && c <= 'Z' ) {
    /* Key down event */
    key = c ;
    gettimeofday( &lastkey, NULL );
//    printf( "KEY %c at %ld.%06ld\n", c, lastkey.tv_sec, lastkey.tv_usec );
  } else if( c >= 'a' && c <= 'z' ) {
    /* Key up event */
//    printf( "KEY %c UP\n", c );
    key = -1 ;
    c = 0 ;
  } else {
    /* Assume timeout */
    c = 0 ;
    if( key > 0 ) {
      int  msec_diff ;
      /* A key is down */
      gettimeofday( &now, NULL );
      msec_diff  = (now.tv_sec - lastkey.tv_sec) * 1000 ;
      msec_diff += (now.tv_usec - lastkey.tv_usec) / 1000 ;
//      printf( "KEY %c down for %d msec\n", key, msec_diff );
      if( msec_diff > 1000 ) {
        /* Generate repeat event */
        c = key | 0x20 ;  /* Upper case to lower case */
        ++lastkey.tv_sec ;  /* HACK HACK. repeat at 1 sec intervals */
//        printf( "KEY %c REPEAT\n", c );
      };
    };
  };

  /* Remap keys according to what LCDproc expects */
  switch( c ) {
  default :  break ;
  case 'V' : c = 'A' ; break ; /* Hold/Select */
  case 'P' : c = 'B' ; break ; /* Left  -- Minus */
  case 'Q' : c = 'C' ; break ; /* Right -- Plus */
  case 'L' : c = 'D' ; break ; /* Menu/Exit */
  case 'U' : c = 'E' ; break ; /* Up */
  case 'K' : c = 'F' ; break ; /* Down */

  case 'v' : c = 'N' ; break ;
  case 'p' : c = 'O' ; break ;
  case 'q' : c = 'P' ; break ;
  case 'l' : c = 'Q' ; break ;
  case 'u' : c = 'R' ; break ;
  case 'k' : c = 'S' ; break ;
  };

//  if( c ) {
//    printf( "KEY %c\n", c );
//  };

  return( c );
}
