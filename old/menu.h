#ifndef MENU_H
#define MENU_H

//////////////////////////////////////////////
// THIS IS NOT READY YET!
//////////////////////////////////////////////

/*

  Pull-down menus...
  
  To use a menu, first define it; then call menu().
  
  To define a menu, just declare an array of MenuItems.  The last element
  must have all NULL fields.
  
    MenuItem FileMenu[] =
    {
      "Open",           OpenFile_proc,          NULL,
      "Close",          CloseFile_proc,         NULL,
      "Save",           SaveFile_proc,          NULL,
      "Save As...",     NULL,                   SaveAsMenu,
      "Quit",           Quit_proc,              NULL,
      NULL,             NULL,                   NULL,
    };

  The first field is the text for the menu item.
  The second field is a function to call when the item is picked.
    The function should take no parameters, and return an int.
    The return values are:
        MENU_OK          Normal return value.  Menu will close.
        MENU_CONT        Menu will stay up after function returns.
  The third field is the child menu to display when the item is picked.
  
  Note that either the second or third field must be NULL.  If both are
  NULL, the item is considered to be just a label.  (for example, a title)
  It will be non-pickable if both are NULL.
  
*/


#define MENU_OK 1
#define MENU_CONT 2

typedef struct MenuItem
{
  char *text;
  int (*func)();
  struct MenuItem *child;
} MenuItem;



// This does a pull-down menu...
int menu(MenuItem *menu);


#endif
