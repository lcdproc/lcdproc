/*
  menus.c

  Defines the default menus the server provides.  Also includes the
  plug-in functions attached to menu items...

  So far, all menus are static, and not dynamically generated.  I'll
  have to find a way to fix this, since many menu items will be dynamic.
  (clients connected, screens available, etc...)
  
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "shared/debug.h"

#include "drivers/lcd.h"

#include "main.h"
#include "menus.h"
#include "render.h"
#include "serverscreens.h"

int Shutdown_func ();
int System_halt_func ();
int Reboot_func ();
int Close_func ();
int OK_func ();
int Time24_func (int input);
int Heartbeat_func (int input);
int Backlight_func (int input);
int Server_screen_func (int input);
int Contrast_func (int input);
int Backlight_Brightness_func (int input);
int Backlight_Off_Brightness_func (int input);
int Backlight_Off_func ();
int Backlight_On_func ();
int Backlight_Open_func ();

menu_item main_menu[] = {
	{"LCDproc", 0, 0},			  // Title
	{"Options", TYPE_MENU, (void *) options_menu},
	{"Screens", TYPE_MENU, (void *) screens_menu},
	{"Shutdown", TYPE_MENU, (void *) shutdown_menu},
	{0, 0, 0},

};

menu_item options_menu[] = {
	{"OPTIONS", TYPE_TITL, 0},	  // Title
//   "24-hour Time", TYPE_CHEK,    (void *)Time24_func,
	{"Contrast...", TYPE_SLID, (void *) Contrast_func},
	{"Backlight", TYPE_MENU, (void *) Backlight_menu},
	{"Heartbeat", TYPE_CHEK, (void *) Heartbeat_func},
//  { "Backlight...", TYPE_SLID,      (void *)Backlight_func},
//   "OK",         TYPE_FUNC,      (void *)OK_func,
//   "Close Menu",   TYPE_FUNC,    (void *)Close_func,
//   "Exit Program", TYPE_FUNC,    (void *)Shutdown_func,
//   "Another Title",TYPE_TITL,    0,
//   "Main menu?",   TYPE_MENU,      (void *)main_menu,
//   "Nothing!",     TYPE_FUNC,      0,
	{0, 0, 0},
};

menu_item screens_menu[] = {
	{"SCREENS", TYPE_TITL, 0},	  // Title
	{"Server Scr", TYPE_CHEK, (void *) Server_screen_func},
	{0, 0, 0},
};

menu_item shutdown_menu[] = {
	{"Shut Down...", TYPE_TITL, 0},	// Title
	{"Kill LCDproc", TYPE_FUNC, (void *) Shutdown_func},
	{"System halt", TYPE_FUNC, (void *) System_halt_func},
	{"Reboot", TYPE_FUNC, (void *) Reboot_func},
	{0, 0, 0},
};

menu_item Backlight_menu[] = {
	{"BACKLIGHT MENU", TYPE_TITL, 0},	// Title
	{"Brightness...", TYPE_SLID, (void *) Backlight_Brightness_func},
	{"\"Off\" Brightness", TYPE_SLID, (void *) Backlight_Off_Brightness_func},
	{"Backlight Mode:", TYPE_FUNC, 0},	// Label
	{" - Off", TYPE_FUNC, Backlight_Off_func},
	{" - On", TYPE_FUNC, Backlight_On_func},
	{" - Open", TYPE_FUNC, Backlight_Open_func},
	{0, 0, 0},
};

///////////////////////////////////////////////////////////////////////
// Plug-in functions for menu items
//

// Exits the program.
int
Shutdown_func ()
{
	exit_program (0);

	return MENU_KILL;
}

// Shuts down the system, if possible
int
System_halt_func ()
{
	int err;
	uid_t id;

	id = geteuid ();

	err = system ("init 0");

	if (err < 0)
		return MENU_KILL;
	if (err == 127)
		return MENU_KILL;

	// If we're root, exit
	if (id == 0)
		exit_program (0);

	// Otherwise, assume shutdown will fail; and show more stats.
	return MENU_KILL;
}

// Shuts down the system and restarts it
int
Reboot_func ()
{
	int err;
	uid_t id;

	id = geteuid ();

	err = system ("init 6");

	if (err < 0)
		return MENU_KILL;
	if (err == 127)
		return MENU_KILL;

	// If we're root, exit
	if (id == 0)
		exit_program (0);

	// Otherwise, assume shutdown will fail; and show more stats.
	return MENU_KILL;
}

int
Close_func ()
{
	return MENU_CLOSE;
}

int
OK_func ()
{
	return MENU_OK;
}

int
Time24_func (int input)
{
	static int status = 0;

	if (input == MENU_READ)
		return status;
	if (input == MENU_CHECK)
		status ^= 1;				  // does something.
	return (status | MENU_OK);
	// The status is "or"-ed with the MENU value to let do_menu()
	// know what to do after selecting the item.  (two return
	// values in one.  :)

	// Also, "MENU_OK" happens to be zero, so it does not matter
	// unless you want something else (like MENU_CLOSE)
}

int
Heartbeat_func (int input)
{
	if (input == MENU_READ)
		return (heartbeat != HEART_OFF);
	if (input == MENU_CHECK) {
		if (heartbeat)
			heartbeat = HEART_OFF;
		else
			heartbeat = HEART_OPEN;
	}
	return ((heartbeat != HEART_OFF) | MENU_OK);
}

int
Backlight_func (int input)
{
	int status = 128;

	switch (backlight) {
	case BACKLIGHT_OFF:
		if (input == MENU_READ)
			status = 0;
		if (input == MENU_PLUS) {
			backlight = BACKLIGHT_OPEN;
			status = 128;
		}
		if (input == MENU_MINUS) {
			status = 0;
		}
		break;
	case BACKLIGHT_ON:
		if (input == MENU_READ)
			status = 255;
		if (input == MENU_PLUS) {
			status = 255;
		}
		if (input == MENU_MINUS) {
			backlight = BACKLIGHT_OPEN;
			status = 128;
		}
		break;
	case BACKLIGHT_OPEN:
		if (input == MENU_READ)
			status = 128;
		if (input == MENU_PLUS) {
			backlight = BACKLIGHT_ON;
			backlight_state = BACKLIGHT_ON;
			status = 255;
		}
		if (input == MENU_MINUS) {
			backlight = BACKLIGHT_OFF;
			backlight_state = BACKLIGHT_OFF;
			status = 0;
		}
		break;
	}

	/*
	   if(input == MENU_READ) return (backlight != BACKLIGHT_OFF);
	   if(input == MENU_CHECK)
	   {
	   if(backlight) backlight = BACKLIGHT_OFF;
	   else backlight = BACKLIGHT_OPEN;
	   }
	 */
	lcd.backlight (backlight_state & BACKLIGHT_ON);

	return (status | MENU_OK);
}

int
Server_screen_func (int input)
{
	if (input == MENU_READ)
		return (server_screen->priority < 256);
	if (input == MENU_CHECK) {
		if (server_screen->priority < 256)
			server_screen->priority = 256;
		else
			server_screen->priority = 128;
	}

	return (MENU_OK | (server_screen->priority < 256));
}

int
Contrast_func (int input)
{
	int status;

	status = lcd.contrast (-1);

	if (input == MENU_READ)
		return status;
	if (input == MENU_PLUS)
		status += 5;				  // does something.
	if (input == MENU_MINUS)
		status -= 5;				  // does something.

	if (status < 0)
		status = 0;
	if (status > 255)
		status = 255;
	lcd.contrast (status);

	return (status | MENU_OK);
}

void
server_menu ()
{
	debug ("server_menu()\n");

	do_menu (main_menu);

}

int
Backlight_Brightness_func (int input)
{
	int status = backlight_brightness;

	if (input == MENU_READ)
		return status;
	if (input == MENU_PLUS)
		status += 5;				  // does something.
	if (input == MENU_MINUS)
		status -= 5;				  // does something.

	if (status < 0)
		status = 0;
	if (status > 255)
		status = 255;
	lcd.backlight (status);

	backlight_brightness = status;

	return (status | MENU_OK);
}

int
Backlight_Off_Brightness_func (int input)
{
	int status = backlight_off_brightness;

	if (input == MENU_READ)
		return status;
	if (input == MENU_PLUS)
		status += 5;				  // does something.
	if (input == MENU_MINUS)
		status -= 5;				  // does something.

	if (status < 0)
		status = 0;
	if (status > 255)
		status = 255;
	lcd.backlight (status);

	backlight_off_brightness = status;

	return (status | MENU_OK);
}

int
Backlight_Off_func ()
{
	backlight_state = BACKLIGHT_OFF;
	backlight = BACKLIGHT_OFF;
	lcd.backlight (backlight_off_brightness);
	return MENU_OK;
}

int
Backlight_On_func ()
{
	backlight_state = BACKLIGHT_ON;
	backlight = BACKLIGHT_ON;
	lcd.backlight (backlight_brightness * (backlight_state & BACKLIGHT_ON));
	return MENU_OK;
}

int
Backlight_Open_func ()
{
	backlight = BACKLIGHT_OPEN;
	return MENU_OK;
}
