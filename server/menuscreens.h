/*
 * menuscreens.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 * Creates all menuscreens, menus and handles the keypresses for the
 * menuscreens.
 */

#ifndef MENUSCREENS_H
#define MENUSCREENS_H

#include "menu.h"
#include "screen.h"

extern Screen * menuscreen;

int init_menu();

bool is_menu_key (char * key);
/* This function indicates to the input part whether this key was the
 * reserved menu key.
 */

void menuscreen_key_handler (char *key);
/* This handler handles the keypresses for the menu.
 */

#endif

