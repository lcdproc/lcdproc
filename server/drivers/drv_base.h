#ifndef LCD_DRV_BASE_H
#define LCD_DRV_BASE_H

/********************************************************************
  How to make a driver for LCDproc

  Note: Insert the name of your driver in place of the phrase "new_driver".
  
  1. Copy drv_base.c and drv_base.h to new_driver.c and new_driver.h.

  2. Decide which functions you want to override, which ones you want
     to leave alone, and which ones you don't want at all.  If you
     write your own driver functions, they will get called when
     appropriate.  Or, you can let the default driver "drv_base"
     handle a function for you.  But, if you really don't want a
     function, you can completely prevent your driver from providing
     it.  This is discussed in step 5.
  
  2.1. Remove all functions which you don't want to override, and the
     ones you don't want at all.

  3. Rename all functions from "drv_base_*" to "new_driver_*".

  4. Write the new driver functions.
     Be sure to do one of the following in/for your new_driver_close():
	- free the framebuffer lcd.framebuf
	- let the default driver handle close()
	- call drv_base_close()
     This will ensure that the frame buffer gets freed.

  5. Register your functions in your new_driver_init(), like the following:
       driver->clear = new_driver_clear;    // We want to handle this one
       driver->string = (void *)-1;         // Leave this to the default
       driver->getkey = NULL;               // This should never get called

     The convention here is to set NULL for all the functions which
     your driver doesn't handle (and which you don't want the default
     functions to be called for).  Or, set -1 to indicate that the
     driver should support the function but can use the default
     behavior in drv_base.c.  Or, to indicate that your driver should
     handle a function, just set it like clear() above.
       
  6. Add the driver to lcd.c, in the physical drivers section, protected
     by an #ifdef like the rest of the drivers are.

  7. Add your driver to the Makefile, and Makefile.config, in the same
     style as the existing ones.


  Notes:

  Don't call lcd.whatever() functions within your driver.  This causes
  a lot of potentially puzzling problems.

  However, feel free to access lcd.framebuf in your driver (but not in
  your init() function!).  It will always point to the frame buffer
  you should be writing to.  This will usually be your own frame
  buffer, but could potentially be another frame buffer, if another
  driver wants to access your functions.

  Your driver will be provided with a frame buffer which contains one
  byte per character on the LCD.  The default is a 20x4, so that's 80
  bytes.  You can free this and reallocate it to something else if you
  need to.  A graphical driver may want to do this, for example.

  If you don't set a function pointer, it will default to NULL, since
  they get nullified before the driver's init() is called.

  Assume that your driver may be added or removed dynamically, and
  that more than one instance may exist at a time.  In other words, it
  should init and close cleanly, and not depend on global variables.

  Special arguments will be passed through the "args" variable to your
  _init() function.  This is just a string, and you determine the
  syntax of it.  Please document the syntax so that it can easily be
  included in the lcdproc config file.  I suggest arguments such as
  "port=0x378" or "-device /dev/ttyS0".  These come directly from the
  lcdproc server config file, so they should be human-readable.  An
  example may look like this:

    # Set up two MtxOrb LCD's and a curses display on VT 2
    Driver MtxOrb -device /dev/ttyS0 -keypad on
    Driver curses -color off -size 20x4 -vt 2
    Driver MtxOrb -device /dev/ttyS3 -size 20x2 -keypad off
    # Before you ask, no, not all of these options are implemented yet.

  Also, the driver API (if it can be called that) may change soon.
  There seem to be several functions which just don't do anything
  important...  and there are other potential improvements too.

  And...  When in doubt, look at how the other drivers do it.  :)
    
******************************************************************/

extern lcd_logical_driver *drv_base;

int drv_base_init(struct lcd_logical_driver *driver, char *args);
void drv_base_close();
void drv_base_clear();
void drv_base_flush();
void drv_base_string(int x, int y, char string[]);
void drv_base_chr(int x, int y, char c);
int drv_base_contrast(int contrast);
void drv_base_backlight(int on);
void drv_base_output(int on);
void drv_base_vbar(int x, int len);
void drv_base_init_vbar();
void drv_base_hbar(int x, int y, int len);
void drv_base_init_hbar();
void drv_base_num(int x, int num);
void drv_base_init_num();
void drv_base_set_char(int n, char *dat);
void drv_base_icon(int which, char dest);
void drv_base_flush_box(int lft, int top, int rgt, int bot);
void drv_base_draw_frame(char *dat);
char drv_base_getkey();


#endif
