#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../../shared/LL.h"

#include "lcd.h"

#include "drv_base.h"

#ifdef MTXORB_DRV
#include "MtxOrb.h"
#endif

#ifdef CFONTZ_DRV
#include "CFontz.h"
#endif

#ifdef TEXT_DRV
#include "text.h"
#endif

#ifdef DEBUG_DRV
#include "debug.h"
#endif

#ifdef CURSES_DRV
#include "curses_drv.h"
#endif

#ifdef HD44780_DRV
#include "hd44780.h"
#endif

#ifdef SLI_DRV
#include "wirz-sli.h"
#endif

#ifdef JOY_DRV
#include "joy.h"
#endif

#ifdef IRMANIN_DRV
#include "irmanin.h"
#endif

#ifdef LIRCIN_DRV
#include "lircin.h"
#endif

// TODO: Make a Windows server, and clients...?


lcd_logical_driver lcd;

lcd_physical_driver drivers[] =
{
  { "base",   drv_base_init, },
#ifdef MTXORB_DRV
  { "MtxOrb",   MtxOrb_init, },
#endif
#ifdef CFONTZ_DRV
  { "CFontz",   CFontz_init, },
#endif
#ifdef HD44780_DRV
  { "HD44780",  HD44780_init, },
#endif
#ifdef SLI_DRV
  { "sli",      sli_init, },
#endif
#ifdef TEXT_DRV
  { "text",     text_init, },
#endif
#ifdef DEBUG_DRV
  { "debug",    debug_init, },
#endif
#ifdef CURSES_DRV
  { "curses",   curses_drv_init, },
#endif
#ifdef JOY_DRV
  { "joy",   joy_init, },
#endif
#ifdef IRMANIN_DRV
  { "irmanin",   irmanin_init, },
#endif
#ifdef LIRCIN_DRV
  { "lircin",	lircin_init, },
#endif
  { NULL,       NULL, },
  
};

LL *list;

////////////////////////////////////////////////////////////
// This sets up which driver to use and initializes stuff.
// 
int lcd_init(char *args)
{
//  int i;
   int err;
   
  list = LL_new();
  if(!list)
  {
     printf("Error allocating driver list.\n");
     return -1;
  }
  
  lcd_drv_init(NULL, NULL);

  err = lcd_add_driver("base", args);

  lcd.wid = 20;
  lcd.hgt = 4;

  return err;
  
  /*
  drv_base_init(args);

  for(i=0; drivers[i].name; i++)
  {
    if(!strcmp(driver, drivers[i].name))
    {
      return drivers[i].init(args);
    }
  }

  printf("Invalid driver: %s\n", driver);
  return -1;
  */
  
}


// TODO:  lcd_remove_driver()

int lcd_add_driver(char *driver, char *args)
{
   int i;
   
   lcd_logical_driver *add;
   
   for(i=0; drivers[i].name; i++)
   {

      //printf("Checking driver: %s\n", drivers[i].name);

      if(0 == strcmp(driver, drivers[i].name))
      {

	 //printf("Found driver: %s (%s)\n", drivers[i].name, driver);

	 add = malloc(sizeof(lcd_logical_driver));
	 if(!add)
	 {
	    printf("Couldn't allocate driver \"%s\".\n", driver);
	    return -1;
	 }
	 //printf("Allocated driver\n");
	 memset(add, 0, sizeof(lcd_logical_driver));

	 add->wid = lcd.wid;           add->hgt = lcd.hgt;
	 add->cellwid = lcd.cellwid;   add->cellhgt = lcd.cellhgt;

//	 printf("LCD driver info:\n\twid: %i\thgt: %i\n",
//	 	add->wid, add->hgt);
	 
	 add->framebuf = malloc(add->wid * add->hgt);

	 if(!add->framebuf)
	 {
	    printf("Couldn't allocate framebuffer for driver \"%s\".\n",
		   driver);
	    free(add);
	    return -1;
	 }
	 //printf("Allocated frame buffer\n");
	 
	 LL_Push(list, (void *)add);
	 
	 return drivers[i].init(add, args);
      }
   }

   return -1;
}

// TODO: Put lcd_shutdown in the shutdown function...
int lcd_shutdown()
{
   lcd_logical_driver *driver;

   
   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 //printf("driver...\n");
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->close > 0)
	 {
	    driver->close();
	 }
	 else if((int)driver->close == -1)
	 {
	    drv_base->close();
	 }

	 LL_Shift(list);
	 // FIXME: This crashes!
	 //if(driver) free(driver);
	 //printf("...freed\n");
      }

   } while(LL_Length(list) > 0);
   
   return 0;
}



int lcd_drv_init(struct lcd_logical_driver *driver, char *args) 
{
//  printf("lcd_drv_init()\n");
  
  lcd.wid = LCD_MAX_WID;
  lcd.hgt = LCD_MAX_HGT;

  lcd.framebuf = NULL;
/*
  if(!lcd.framebuf) 
     lcd.framebuf = malloc(lcd.wid * lcd.hgt);

  if(!lcd.framebuf)
  {
     lcd_drv_close();
     return -1;
  }
  memset(lcd.framebuf, ' ', lcd.wid*lcd.hgt);
*/
// Debugging...
//  if(lcd.framebuf) printf("Frame buffer: %i\n", (int)lcd.framebuf);

  lcd.cellwid = 5;
  lcd.cellhgt = 8;
  
  // Set up these wrapper functions...
  lcd.clear = lcd_drv_clear;
  lcd.string = lcd_drv_string;
  lcd.chr = lcd_drv_chr;
  lcd.vbar = lcd_drv_vbar;
  lcd.hbar = lcd_drv_hbar;
  lcd.init_num = lcd_drv_init_num;
  lcd.num = lcd_drv_num;
  
  lcd.init = lcd_drv_init;
  lcd.close = lcd_drv_close;
  lcd.flush = lcd_drv_flush;
  lcd.flush_box = lcd_drv_flush_box;
  lcd.contrast = lcd_drv_contrast;
  lcd.backlight = lcd_drv_backlight;
  lcd.output = lcd_drv_output;
  lcd.set_char = lcd_drv_set_char;
  lcd.icon = lcd_drv_icon;
  lcd.init_vbar = lcd_drv_init_vbar;
  lcd.init_hbar = lcd_drv_init_hbar;
  lcd.draw_frame = lcd_drv_draw_frame;

  lcd.getkey = lcd_drv_getkey;
  
  
  return 1;  // 1 is arbitrary.  (must be 1 or more)
}



//////////////////////////////////////////////////////////////////////
// All functions below here call their respective driver functions...
//
// The way it works is this:
//   It loops through the list of drivers, calling each one to keep
//      them all synchronized.  During this loop...
//   - First, set the framebuffer to point to the drivers' framebuffer.
//       (this ensures that whatever driver gets called will operate on
//        the correct buffer)
//   - Then, it checks to see what sort of call to make.
//       driver->func() > 0 means it should call the driver function.
//       driver->func() == NULL means it should not make a call.
//       driver->func() == -1 means it should call the generic driver.
//////////////////////////////////////////////////////////////////////


void lcd_drv_close() 
{
   lcd_logical_driver *driver;

//   printf("lcd_drv_close()\n");

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->close > 0)
	 {
	    printf("Calling close()\n");
	    driver->close();
	 }
	 else if((int)driver->close == -1)
	 {
	    drv_base->close();
	 }
      }
   } while(LL_Next(list) == 0);
   
}

void lcd_drv_clear() 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->clear > 0)
	    driver->clear();
	 else if((int)driver->clear == -1)
	 {
	    drv_base->clear();
	 }
      }
   } while(LL_Next(list) == 0);
  
}


void lcd_drv_flush()
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->flush > 0)
	    driver->flush();
	 else if((int)driver->flush == -1)
	 {
	    drv_base->flush();
	 }
      }
   } while(LL_Next(list) == 0);
}


void lcd_drv_string(int x, int y, char string[]) 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->string > 0)
	    driver->string(x,y,string);
	 else if((int)driver->string == -1)
	 {
	    drv_base->string(x,y,string);
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_chr(int x, int y, char c) 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->chr > 0)
	    driver->chr(x,y,c);
	 else if((int)driver->chr == -1)
	 {
	    drv_base->chr(x,y,c);
	 }
      }
   } while(LL_Next(list) == 0);
}



int lcd_drv_contrast(int contrast) 
{
   int res=0;

   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->contrast > 0)
	 {
	    res=driver->contrast(contrast);
	    if(res >= 0) return res;
	 }
	 /*
	   else if((int)driver->contrast == -1)
	   {
	   res=drv_base->contrast(contrast);
	   }
	 */
      }
   } while(LL_Next(list) == 0);

   return res;
}

void lcd_drv_backlight(int on)
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->backlight > 0)
	    driver->backlight(on);
	 else if((int)driver->backlight == -1)
	 {
	    drv_base->backlight(on);
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_output(int on)
{
   lcd_logical_driver *driver;
   LL_Rewind(list);
   do
   {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
	 if((int)driver->output > 0)
	    driver->output(on);
	 else if((int)driver->output == -1)
	 {
	    drv_base->output(on);
	 }
      }
   } while(LL_Next(list) == 0);
}


void lcd_drv_init_vbar() 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->init_vbar > 0)
	    driver->init_vbar();
	 else if((int)driver->init_vbar == -1)
	 {
	    drv_base->init_vbar();
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_init_hbar() 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->init_hbar > 0)
	    driver->init_hbar();
	 else if((int)driver->init_hbar == -1)
	 {
	    drv_base->init_hbar();
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_init_num() 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->init_num > 0)
	    driver->init_num();
	 else if((int)driver->init_num == -1)
	 {
	    drv_base->init_num();
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_num(int x, int num) 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->num > 0)
	    driver->num(x,num);
	 else if((int)driver->num == -1)
	 {
	    drv_base->num(x,num);
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_set_char(int n, char *dat)
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->set_char > 0)
	    driver->set_char(n,dat);
	 else if((int)driver->set_char == -1)
	 {
	    drv_base->set_char(n,dat);
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_vbar(int x, int len) 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->vbar > 0)
	    driver->vbar(x,len);
	 else if((int)driver->vbar == -1)
	 {
	    drv_base->vbar(x,len);
	 }
      }
   } while(LL_Next(list) == 0);
}

void lcd_drv_hbar(int x, int y, int len) 
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->hbar > 0)
	    driver->hbar(x,y,len);
	 else if((int)driver->hbar == -1)
	 {
	    drv_base->hbar(x,y,len);
	 }
      }
   } while(LL_Next(list) == 0);
}


void lcd_drv_icon(int which, char dest)
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->icon > 0)
	    driver->icon(which,dest);
	 else if((int)driver->icon == -1)
	 {
	    drv_base->icon(which,dest);
	 }
      }
   } while(LL_Next(list) == 0);
}


void lcd_drv_flush_box(int lft, int top, int rgt, int bot)
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->flush_box > 0)
	    driver->flush_box(lft,top,rgt,bot);
	 else if((int)driver->flush_box == -1)
	 {
	    drv_base->flush_box(lft,top,rgt,bot);
	 }
      }
   } while(LL_Next(list) == 0);
  
}


// TODO:  Check whether lcd.draw_frame() should really take a framebuffer
// TODO:   as an argument, or if it should always use lcd.framebuf
void lcd_drv_draw_frame(char *dat)
{
   lcd_logical_driver *driver;

   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->draw_frame > 0)
	    driver->draw_frame(dat);
	 else if((int)driver->draw_frame == -1)
	 {
	    drv_base->draw_frame(dat);
	 }
      }
   } while(LL_Next(list) == 0);

}


char lcd_drv_getkey()
{
   lcd_logical_driver *driver;

   char key;
   
   LL_Rewind(list);
   do {
      driver = (lcd_logical_driver *)LL_Get(list);
      if(driver)
      {
	 lcd.framebuf = driver->framebuf;
   
	 if((int)driver->getkey > 0)
	 {
	    key = driver->getkey();
	    if(key) return key;
	 }
	 else if((int)driver->getkey == -1)
	 {
	    key = drv_base->getkey();
	    if(key) return key;
	 }
      }
   } while(LL_Next(list) == 0);

   return 0;
}
