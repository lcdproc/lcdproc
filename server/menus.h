#ifndef MENUS_H
#define MENUS_H

#include "menu.h"

// These probably don't need to be defined here...
extern menu_item main_menu[];
extern menu_item options_menu[];
extern menu_item screens_menu[];
extern menu_item shutdown_menu[];
extern menu_item Backlight_menu[];

// Brings up the main menu...
void server_menu ();

#endif
