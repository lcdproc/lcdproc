#include "lcd.h"
#include "menu.h"



extern menu_item MainMenu[];
extern menu_item OptionsMenu[];
extern int Shutdown_func();
extern int Close_func();
extern int OK_func();
extern int Time24_func();
extern int Contrast_func();


menu_item MainMenu[] = {
   "MAIN MENU",	   0,		   0,	// Title
   "Options",	   TYPE_MENU,	   (void *)OptionsMenu,
   "Kill LCDproc", TYPE_FUNC,	   (void *)Shutdown_func,
   "OK",	   TYPE_FUNC,	   (void *)OK_func,
   0,		   0,		   0,
   
};

menu_item OptionsMenu[] = {
   "OPTIONS MENU", TYPE_TITL,	   0,	// Title
   "24-hour Time", TYPE_CHEK,	   (void *)Time24_func,
   "Contrast...",  TYPE_SLID,	   (void *)Contrast_func,
   "OK",	   TYPE_FUNC,	   (void *)OK_func,
   "Close Menu",   TYPE_FUNC,	   (void *)Close_func,
   "Exit Program", TYPE_FUNC,	   (void *)Shutdown_func,
   "Another Title",TYPE_TITL,	   0,
   0,		   0,		   0,
};


///////////////// Elsewhere, we declare these...

int Shutdown_func()
{
   // Do something here...
   return MENU_KILL;
}

int Close_func()
{
   return MENU_CLOSE;
}

int OK_func()
{
   return MENU_OK;
}


int Time24_func(int input)
{
   static int status=0;
  
   if(input == MENU_READ) return status;
   if(input == MENU_CHECK) status ^= 1;	 // does something.
   return (status | MENU_OK);
   // The status is "or"-ed with the MENU value to let do_menu()
   // know what to do after selecting the item.	 (two return
   // values in one.  :)

   // Also, "MENU_OK" happens to be zero, so it does not matter
   // unless you want something else (like MENU_CLOSE)
}

int Contrast_func(int input)
{
   static int status=128;
  

   if(input == MENU_READ) return status;
   if(input == MENU_PLUS) status++; // does something.
   if(input == MENU_MINUS) status--; // does something.
   return (status | MENU_OK); 
}


void main()
{
   lcd_init("", "curses");
  
   do_menu(MainMenu);
  
}
