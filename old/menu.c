#include "menu.h"

/////////////////////////////////////////
//  THIS IS NOT READY YET!
/////////////////////////////////////////


/////////////////////////////////////////////////////////
// Read menu.h to find out how this works.
//




// Internal record of all important menu info...
typedef struct MenuInfo
{
  MenuItem *menu;	// User-specified menu information
  struct MenuInfo *parent;	// NULL if root-level
  int items;		// number of items in the menu
  int sel;		// selected item
} MenuInfo;



// Main menu function...
int do_menu(MenuItem *menu, MenuInfo *parent);


void FillMenuInfo(MenuInfo *m, MenuItem *menu, MenuInfo *parent);

void DrawMenu(MenuInfo *m);
void DrawMenuItem(MenuInfo *m, int item);


///////////////////////////////////////////////////////////////////
// User-callable function...
//
int menu(MenuItem *menu)
{
  int ret;

  CreateArrows();  // Set custom characters for menu use...
  
  ret = do_menu(menu, NULL);
  
  return ret;
}



int do_menu(MenuItem *menu, MenuInfo *parent)
{
  int current;
  int uparrow;
  int downarrow;

  int key;
  int err=0;
  

  while(! err)
  {
    // Draw the menu...

    // Wait for input

  }
  
}


int do_menu(MenuItem *menu, MenuInfo *parent)
{
  int i;
  int items;
  MenuInfo ThisMenu;
  int old_sel;
  int redraw=1;
  int ret=0;
  
  FillMenuInfo(&ThisMenu, menu, parent);

//FIXME:  Get input here...   Move the selection thingy

  DrawMenu(&ThisMenu);


  do{
//FIXME:  Get input here...   Move the selection thingy

    // Figure out which button should be highlighted...

    // And...  if the "execute" button was pressed...
    if() 
    {
      ThisMenu.sel = i;
    }
        
    DrawMenu(&ThisMenu);  
    redraw = 0;
    
    // Dismiss menu?
    if(  // cancel was pressed...
	 )
     ret = GUI_CONT;

    // pop up another menu?
    if(  // execute was pressed...
       &&ThisMenu.sel >= 0
       &&ThisMenu.menu[ThisMenu.sel].child)
    {
      i = do_menu(ThisMenu.menu[ThisMenu.sel].child, &ThisMenu);
      if(i == GUI_OK) ret = GUI_OK;
    }

    // Or call a function?
    else if( // execute was pressed...
            &&ThisMenu.sel >= 0
            &&ThisMenu.menu[ThisMenu.sel].func)
    {
      ret = ThisMenu.menu[ThisMenu.sel].func();
      //ret = GUI_OK;
    }
    
  } while(! ret);  

  return ret;
}



//////////////////////////////////////////////////////////////////////
// Initializes a menu...
//
void FillMenuInfo(MenuInfo *m, MenuItem *menu, MenuInfo * parent)
{
  m->menu = menu;
  m->parent = parent;
  m->sel = 0;
  
}                  


void DrawMenu(MenuInfo *m)
{
  int i;
  
  for(i=0; i<m->items; i++)
    DrawMenuItem(m, i);
}

void DrawMenuItem(MenuInfo *m, int item)
{
  // Figure out how much to scroll down...
  // Write each menu item...
  // Put arrows where needed (next to current item, and up/down arrow)
  // m->menu[item].text holds the text string...
}



