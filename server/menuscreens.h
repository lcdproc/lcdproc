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

MenuEventFuncResult menuscreen_key_handler ();
/* This handler should be called by the real handler for the event */

#endif