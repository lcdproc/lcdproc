#ifndef MENU_H
#define MENU_H

#if 0
/*************************************************************************
				Menus!

This is how the LCDproc menu stuff works...

Each item has three values:

        Title           -- Text
        Type            -- menu,  function, checkbox,  slider,   mover
        Data            -- Child, ExecFunc, CheckFunc, SlidFunc, ???

When an item is picked, do_menu() decides what to do based on type.
--"Menus" will recurse into the "data", assuming it is a child menu.
--"Function"-type items will have their function called.  
--CheckBox-type items will have their function called with a "read"
  parameter to get an on/off signal, and called with a "set" signal when
  picked.
--Sliders will have the same "read" thing, and the "set" function will
  take a plus or minus parameter.
--The Movers will act like a label until picked, and then the +/- keys
  will both rearrange the menu, and send the item a signal of some sort
  to indicate what happened.  It will act like a label again after the
  user presses Enter again.

The "Data" field will really be a "void *", which is the "generic"
data type in C...

Anyway, this sort of thing would be declared this way:

========================================================================

menu_item MainMenu[] = {
        "MENU",         0,              0,   // Title
        "Options",      TYPE_MENU,      (void *)OptionsMenu,
        "Kill LCDproc", TYPE_FUNC,      (void *)Shutdown_func,
        0,              0,              0,
};

menu_item OptionsMenu[] = {
        "OPTIONS",      TYPE_TITL,       0,   // Title
        "24-hour Time", TYPE_CHEK,      (void *)Time24_func,
        "Contrast...",  TYPE_SLID,      (void *)Contrast_func,
        0,              0,              0,
};

///////////////// Elsewhere, we declare these...

void Shutdown_func()
{
  // Do something here...
  return MENU_KILL;     // or MENU_CLOSE, or MENU_OK, or MENU_ERROR
}

int Time24_func(int input)
{
  if(input == MENU_READ) return status;
  if(input == MENU_CHECK) toggle_status();  // does something.
  return (status | MENU_OK);
        // The status is "or"-ed with the MENU value to let do_menu()
        // know what to do after selecting the item.  (two return
        // values in one.  :)

        // Also, "MENU_OK" happens to be zero, so it does not matter
        // unless you want something else (like MENU_CLOSE)
}

int Contrast_func(int input)
{
  if(input == MENU_READ) return status;
  if(input == MENU_PLUS) increment_status(); // does something.
  if(input == MENU_MINUS) decrement_status();// does something.
  return (status | MENU_OK); 
}

=====================================================================

Function return values:
    MENU_OK        Keeps menu open.
    MENU_CLOSE     Closes menu after item is picked.
                   Leaves the parent menus open.
    MENU_QUIT      Closes all menus after item is picked.
                   Like a normal pulldown menu.
    MENU_KILL      Same as MENU_QUIT, so far.
    MENU_ERROR     Um, for errors?  :)

Also, functions for sliders and checkboxes should return a status value
binary OR-ed with the MENU_ return value.  Checkboxes should return 0 or 1,
and sliders should return 0-255.

**************************************************************************/
#endif

// Return codes from selected menu items...
#define MENU_ERROR -0x7FFF0000
#define MENU_OK 0
#define MENU_CLOSE 0x10000
#define MENU_QUIT 0x20000
#define MENU_KILL 0x20000

// Menu item Types...
#define TYPE_TITL 0
#define TYPE_MENU 1
#define TYPE_FUNC 2
#define TYPE_CHEK 3
#define TYPE_SLID 4
#define TYPE_MOVE 5

#define CLIENT_MENU 0x100
#define CLIENT_FUNC 0x101
#define CLIENT_CHEK 0x102
#define CLIENT_SLID 0x103
#define CLIENT_MOVE 0x104

// User actions, sent to item-handling functions as input.
#define MENU_SELECT 1
#define MENU_CHECK 2
#define MENU_PLUS 3
#define MENU_MINUS 4
#define MENU_READ 5

typedef struct menu_item {
   char *text;
   int type;
   void *data;
} menu_item;

#define Menu menu_item *

int do_menu (Menu menu);

#endif
